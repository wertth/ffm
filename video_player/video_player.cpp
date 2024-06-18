//
// Created by 缪浩楚 on 2024/6/9.
//
#include "drawer.h"
#include <cstdio>
#include <stdexcept>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
int main() {
    const char* filePath = "./assets/test.avi";
    FILE * fp = fopen(filePath, "r");


    AVFormatContext* avFormatContext = nullptr;
    AVCodecParameters* codecParameters =nullptr;
    AVCodecContext * codecContext = nullptr;

    int ret = 0;


    avFormatContext = avformat_alloc_context();
    ret = avformat_open_input(&avFormatContext, filePath, NULL, NULL);
    if(ret < 0) throw std::runtime_error("open input error");
    ret = avformat_find_stream_info(avFormatContext, NULL);
    if(ret < 0) throw std::runtime_error("find stream info error");

    AVStream* video = nullptr;
    for(int i = 0; i < avFormatContext->nb_streams; ++i) {
        if(avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video = avFormatContext->streams[i];
            codecParameters = video->codecpar;
        }
    }
    if(!video) throw std::runtime_error("find video stream info error");
    if(!codecParameters) throw std::runtime_error("find video codecParams info error");

    const AVCodec* codec =  avcodec_find_decoder(codecParameters->codec_id);
    codecContext = avcodec_alloc_context3(codec);
    if(!codecContext) throw std::runtime_error("alloc a codec Context err");
    ret = avcodec_open2(codecContext, codec, nullptr);
    if(ret < 0) throw std::runtime_error("open a codec Context err");



    ret = avcodec_parameters_to_context(codecContext, codecParameters);
    if(ret < 0) throw std::runtime_error("open a codec Context err");


    AVFrame* raw = av_frame_alloc(), * yuv = av_frame_alloc();
    int buffSize = av_image_get_buffer_size(codecContext->pix_fmt, codecParameters->width, codecParameters->height,1);

    auto* buffer = (uint8_t* )av_malloc(buffSize);


    av_image_fill_arrays(yuv->data, yuv->linesize, buffer, codecContext->pix_fmt, codecContext->width, codecContext->height, 1);

    struct SwsContext* swsContext;

    swsContext = sws_getContext(codecContext->width, codecContext->height,codecContext->pix_fmt,
                                codecContext->width,codecContext->height,AV_PIX_FMT_YUV420P,SWS_BICUBIC,
                                nullptr, nullptr, nullptr);

    SDL_DRAWER drawer;
    SDL_Window * window =  drawer.addWindow("simple yuv player", codecContext->width, codecContext->height,SDL_WINDOWPOS_CENTERED,
                     SDL_WINDOWPOS_CENTERED);
     SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
     SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, -1, codecContext->width, codecContext->height);

     drawer.setRender(window, renderer);
     drawer.setTexture(window, texture);
     AVPacket* packet = av_packet_alloc();

     // fps
     AVRational  rational = codecContext->framerate;

     drawer.mainLoop(window ,[&]() -> bool {
         if(av_read_frame(avFormatContext, packet) ==  0) {
             avcodec_send_packet(codecContext, packet);
             avcodec_receive_frame(codecContext, raw);

             sws_scale(swsContext, (uint8_t* const *)raw->data, raw->linesize, 0,
                       codecContext->height,
                       yuv->data,
                       yuv->linesize);
             SDL_UpdateYUVTexture(texture, nullptr, yuv->data[0],            // y plane
                                  yuv->linesize[0],        // y pitch
                                  yuv->data[1],            // u plane
                                  yuv->linesize[1],        // u pitch
                                  yuv->data[2],            // v plane
                                  yuv->linesize[2]);
             return true;
         } else {
             av_packet_unref(packet);
             return false;
         }
     }, rational.num / rational.den);

    sws_freeContext(swsContext);
    av_free(buffer);
    av_frame_free(&yuv);
    av_frame_free(&raw);
    av_packet_free(&packet);
//    avcodec_parameters_free(&codecParameters); 不必释放， 因为这里 para 由 formatCtx提供
    avcodec_close(codecContext);
    avformat_close_input(&avFormatContext);

}