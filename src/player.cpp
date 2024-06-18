//
// Created by 缪浩楚 on 2024/6/14.
//
#include "player.h"
#include "SDL.h"
#include <cstring>
//#include <iostream>
#include "logger.hpp"
extern "C" {
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}



void Player::main_event_loop() {
    // main thread

    while(true) {
        SDL_WaitEvent(&m_event);
//        printf("1\n");
        switch (m_event.type) {
            case FF_QUIT_EVENT:
            case SDL_QUIT:
                doExit();
                return;
        }
    }
}

void decodeAudioThread(VideoState* s ) {
    for (;;) {
        AVPacket * packet = s->audioQueue->pop();
        if(!packet) {
            SDL_Delay(10);
            continue;
        }
        Logger::log("audio packet size: {} index:{}", packet->size, packet->stream_index);

        if(s->abort) break;
        av_packet_free(&packet);
    }
}

void sdlDraw(VideoState * s,AVFrame* f, DrawerCtx& drawCtx) {
    SDL_UpdateYUVTexture(drawCtx.texture, nullptr, f->data[0],            // y plane
                         f->linesize[0],        // y pitch
                         f->data[1],            // u plane
                         f->linesize[1],        // u pitch
                         f->data[2],            // v plane
                         f->linesize[2]);
    SDL_RenderCopy(drawCtx.renderer, drawCtx.texture, nullptr, nullptr);
    SDL_RenderPresent(drawCtx.renderer);
}
void decodeVideoThread(VideoState* s) {
    AVCodecParameters* parameters = nullptr;
    AVCodecContext* codecContext = nullptr;
    const AVCodec* codec = avcodec_find_decoder(s->video->codecpar->codec_id);

    parameters = s->video->codecpar;

    // alloc
    codecContext = avcodec_alloc_context3(codec);
    int ret =0;
    ret = avcodec_open2(codecContext,codec, nullptr);
    if(ret < 0) throw std::runtime_error("");
    ret = avcodec_parameters_to_context(codecContext, parameters);
    if(ret < 0) throw std::runtime_error("");

    int bufferSize = av_image_get_buffer_size(codecContext->pix_fmt, parameters->width, parameters->height,1);
    if(bufferSize < 0) throw std::runtime_error("parse fmt err");
    auto* buffer = (uint8_t*)av_mallocz(bufferSize);

    AVFrame* original = av_frame_alloc(), *yuv = av_frame_alloc();
    av_image_fill_arrays(yuv->data, yuv->linesize, buffer, codecContext->pix_fmt, codecContext->width, codecContext->height, 1);


    struct SwsContext* swsContext;
    swsContext = sws_getContext(codecContext->width, codecContext->height,codecContext->pix_fmt,
                                codecContext->width,codecContext->height,AV_PIX_FMT_YUV420P,SWS_BICUBIC,
                                nullptr, nullptr, nullptr);
    DrawerCtx drawCtx = s->drawCtx[s->video->index];

    for (;;) {
        AVPacket * packet = s->videoQueue->pop();
        if(!packet) {
            SDL_Delay(10);
            continue;
        }
        Logger::log("video packet size: {} index:{}", packet->size, packet->stream_index);
        ret = avcodec_send_packet(codecContext, packet);
        if(ret < 0) throw std::runtime_error("");
        avcodec_receive_frame(codecContext, original);
        if(ret < 0) throw std::runtime_error("");

        sws_scale(swsContext, (uint8_t* const *)original->data, original->linesize, 0,
                  codecContext->height,
                  yuv->data,
                  yuv->linesize);

        sdlDraw(s, yuv, drawCtx);

        SDL_Delay(1000 / 25);

        if(s->abort) break;
        av_packet_free(&packet);
    }

    avcodec_free_context(&codecContext);
    av_free(buffer);
    av_frame_free(& yuv);
    av_frame_free(& original);
}




void decodeThread(VideoState* s, SDL_Event& event) {
    AVFormatContext* context = s->formatContext;
    AVPacket* packet = av_packet_alloc();
    try {
        int ret = 0;
        if(s->video) s->decodeVideo = std::async(std::launch::async, decodeVideoThread, s);
        if(s->audio) s->decodeAudio =  std::async(std::launch::async, decodeAudioThread, s);
        for(;;) {
            if(s->abort) break;
            if(s->videoQueue->size >= MAX_VIDEOQ_SIZE || s->audioQueue->size >= MAX_AUDIOQ_SIZE) {
                SDL_Delay(10);
                continue;
            }
            ret = av_read_frame( context,  packet);
            if(ret < 0 || packet->size == 0) {
//            event.type = FF_QUIT_EVENT;
//            SDL_PushEvent(&event);
                break;
            }
            Logger::log("receive {} {}", packet->size, packet->stream_index);
            if(packet->stream_index == s->videoStreamIndex) {
                s->videoQueue->push(packet);
            } else if(packet->stream_index == s->audioStreamIndex) {
                s->audioQueue->push(packet);
            }

        }
    } catch (std::exception& err) {
        Logger::log("err: {}", err.what());
    }

    av_packet_free(&packet);
}


// s from the src streams, attach the essential params to v
void prepareDecodeStreamCtx(VideoState* v, int index) {
    AVFormatContext* context = v->formatContext;
    AVStream* s = context->streams[index];

    if(!s) throw std::runtime_error("no specific stream!");
    AVCodecParameters* parameters = s->codecpar;
    AVMediaType type = s->codecpar->codec_type;
    switch (type) {
        case AVMediaType::AVMEDIA_TYPE_AUDIO:
            v->audio = s;
            v->audioStreamIndex = index;
            break;
        case AVMediaType::AVMEDIA_TYPE_VIDEO: {
            v->video = s;
            v->videoStreamIndex = index;
            SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, parameters->width,
                                                  parameters->height, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
            SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, -1, parameters->width,
                                                     parameters->height);
            v->drawCtx.emplace_back( DrawerCtx {window,renderer,texture, parameters->width, parameters->height});
            break;
        }
        default:
            break;
    }
}



void Player::open_input(const char *filePath) {
    int ret = 0;
    AVFormatContext* context = nullptr;
    context = avformat_alloc_context();

    if(nullptr == context) throw std::runtime_error("avformat context alloc err");

    ret = avformat_open_input(&context, filePath, NULL, NULL);
    if(ret < 0) throw std::runtime_error("avformat open file err");
    ret = avformat_find_stream_info(context, NULL);
    m_videoState = (VideoState*) av_mallocz(sizeof(VideoState));
    if(!m_videoState) throw std::runtime_error("av malloc err");

    m_videoState->formatContext = context;
    strcpy(m_videoState->filePath, filePath);

    m_videoState->videoQueue = (PacketQueue *)av_mallocz(sizeof(PacketQueue));
    m_videoState->audioQueue = (PacketQueue *) av_mallocz(sizeof(PacketQueue));



    // prepare streams

    for(int i = 0; i < context->nb_streams; i++) {
        AVStream* stream = context->streams[i];
        switch (stream->codecpar->codec_type) {
            case AVMEDIA_TYPE_AUDIO:
            case AVMEDIA_TYPE_VIDEO: {
                prepareDecodeStreamCtx(m_videoState, i);
                break;
            }
            default:
                break;
        }
    }

    decode = std::async(std::launch::async, decodeThread, m_videoState, std::ref(m_event));

}

void Player::doExit() {
    Logger::log("packet size: {} index:{}", m_videoState->audioQueue->size, m_videoState->videoQueue->size);

}

Player::Player(const char *filePath) {
    try {
        open_input(filePath);
    } catch (std::exception& err) {
//        std::cerr << err.what() << '\n';
    }
}


AVPacket* PacketQueue::pop() {
    if(abort) return nullptr;
    SDL_LockMutex(mutex);

    PackList* p = this->first;

    if(p) {
        first = first->next;
        size-=p->packet->size;
        if(nullptr == first)end = nullptr;
        SDL_UnlockMutex(mutex);
        return p->packet;
    }
    SDL_UnlockMutex(mutex);
    return nullptr;
}


void PacketQueue::push(AVPacket *packet) {
//    if(size >=  PACKET_LIST_MAX_LEN) return;
    SDL_LockMutex(mutex);
    auto* p = (PackList *) av_malloc(sizeof(PackList ));
    AVPacket * mp = av_packet_alloc();
    memcpy(mp, packet, sizeof(AVPacket));

    p->packet = mp;
    p->next = nullptr;
    if(!first) {
        first = p;
        end = first;
    } else {
        end->next = p;
        end = end->next;
    }
    size+= packet->size;

    SDL_UnlockMutex(mutex);
}

PacketQueue::~PacketQueue() {
    PackList * p = first;
    while(p) {
        if(p->packet) av_packet_free(&p->packet);
        p = p -> next;
    }
    SDL_DestroyMutex(mutex);
    SDL_DestroyCond(cond);
}
