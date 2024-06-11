#include <iostream>
#include "fmt/core.h"
#include <stdexcept>
extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}
#include "SDL.h"
#include "SDL_rect.h"
#include "SDL_video.h"
#include "SDL_render.h"

int findVideoStreamIndex(AVFormatContext *formatCtx);

bool isVideo(AVStream *pStream);

int main(int argc, char** argv) {
//    fmt::print("the version of ffmpeg is {}", av_version_info());

    AVFormatContext* avFormatContext = nullptr;
    AVCodecParameters* avCodecParameters = nullptr;
    AVCodecContext* avCodecContext = nullptr;
    AVPacket* avPacket = nullptr;
    struct SwsContext* swsContext;
    int video_stream_index = -1;


    SDL_Window* screen = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* sdlTexture = nullptr;
    SDL_Rect            sdlRect;
    SDL_Event event;         // 声明一个事件
    SDL_bool running = SDL_TRUE; // 循环的标志

    int ret = 0;
    if(argc < 2 ) {
        fmt::println("error args ");
    }
    const char* filePath = argv[1];
    ret = avformat_open_input(&avFormatContext, filePath, NULL, NULL );

    if(ret != 0) {
        throw std::runtime_error("open media file error");
    }


    ret = avformat_find_stream_info(avFormatContext, NULL);

    if(ret != 0) throw std::runtime_error("read media format error");

    av_dump_format(avFormatContext, 0, filePath, 0);

    video_stream_index = findVideoStreamIndex(avFormatContext);
    if(-1 == video_stream_index) throw std::runtime_error("no video stream found!!");

    //
//    AVStream* videoStream = avFormatContext->streams[video_stream_index];
    avCodecParameters = avFormatContext->streams[video_stream_index]->codecpar;
    const AVCodec* avCodec  =  avcodec_find_decoder(avCodecParameters->codec_id);

    if(nullptr == avCodec) throw std::runtime_error("find codec err");

    avCodecContext = avcodec_alloc_context3(avCodec);
    ret = avcodec_parameters_to_context(avCodecContext, avCodecParameters);

    if(ret != 0) throw std::runtime_error("Fill the codec context err");

    ret = avcodec_open2(avCodecContext, avCodec, NULL);

    if(ret != 0) throw std::runtime_error("init AVCodecContext err");

    AVFrame* raw = av_frame_alloc();
    AVFrame* yuv = av_frame_alloc();

    // calc need buffer size in memory
    int buffSize = av_image_get_buffer_size(avCodecContext->pix_fmt,
                                            avCodecContext->width,
                                            avCodecContext->height,
                                            1);

    auto* buffer = (uint8_t*)av_malloc(buffSize);

    av_image_fill_arrays(yuv->data, yuv->linesize, buffer, avCodecContext->pix_fmt, avCodecContext->width, avCodecContext->height, 1);

    swsContext = sws_getContext(avCodecContext->width,
                                avCodecContext->height,
                                avCodecContext->pix_fmt,
                                avCodecContext->width,
                                avCodecContext->height,
                                AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC,
                                NULL,
                                NULL,
                                NULL);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("SDL_Init() failed: %s\n", SDL_GetError());
        return -1;
    }
    screen = SDL_CreateWindow("Simplest ffmpeg player's Window",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              avCodecContext->width,
                              avCodecContext->height,
                              SDL_WINDOW_SHOWN
    );

    if (screen == nullptr)
    {
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return -1;
    }

    // B3. 创建SDL_Renderer
    //     SDL_Renderer：渲染器
    //
    sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                    SDL_PIXELFORMAT_IYUV,
                                    SDL_TEXTUREACCESS_STREAMING,
                                   avCodecContext->width,
                                   avCodecContext->height);
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = avCodecContext->width;
    sdlRect.h = avCodecContext->height;

    avPacket = static_cast<AVPacket *>(av_malloc(sizeof(AVPacket)));
    while(av_read_frame(avFormatContext, avPacket) == 0 && running) {
        if(avPacket->stream_index == video_stream_index) {
            ret = avcodec_send_packet(avCodecContext, avPacket);
            if(ret != 0) throw std::runtime_error("send packet to codec err");

            ret = avcodec_receive_frame(avCodecContext, raw);
            if(ret != 0) throw std::runtime_error("receive frame err");

            sws_scale(
                    swsContext,
                    static_cast<const uint8_t *const*>(raw->data),
                    raw->linesize,
                    0,
                    avCodecContext->height,
                    yuv->data,
                    yuv->linesize
                    );
            printf("Decoded frame: width=%d, height=%d\n", raw->width, raw->height);

            SDL_UpdateYUVTexture(sdlTexture,                   // sdl texture
                                 &sdlRect,                     // sdl rect
                                 yuv->data[0],            // y plane
                                 yuv->linesize[0],        // y pitch
                                 yuv->data[1],            // u plane
                                 yuv->linesize[1],        // u pitch
                                 yuv->data[2],            // v plane
                                 yuv->linesize[2]         // v pitch
            );

            // B6. 使用特定颜色清空当前渲染目标
//            SDL_RenderClear(sdlRenderer);
            // B7. 使用部分图像数据(texture)更新当前渲染目标
            SDL_RenderCopy(sdlRenderer,                        // sdl renderer
                           sdlTexture,                         // sdl texture
                           NULL,                                // src rect, if NULL copy texture
                           &sdlRect                            // dst rect
            );
            // B8. 执行渲染，更新屏幕显示
            SDL_RenderPresent(sdlRenderer);
            SDL_Delay(40);
            SDL_PollEvent(&event);
            if(event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
        }

        av_packet_unref(avPacket);
    }

    SDL_Quit();
    sws_freeContext(swsContext);
    av_free(buffer);
    av_frame_free(&yuv);
    av_frame_free(&raw);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);


    return 0;
}

int findVideoStreamIndex(AVFormatContext *formatCtx) {
    for(int i = 0; i < formatCtx->nb_streams; ++i) {
        if(isVideo(formatCtx->streams[i])) return i;
    }
    return -1;
}

bool isVideo(AVStream *pStream) {
    return pStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO;
}
