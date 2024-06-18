//
// Created by 缪浩楚 on 2024/6/13.
//

#ifndef FFM_PLAYER_H
#define FFM_PLAYER_H
extern "C" {
#include "libavformat/avformat.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};
#include "SDL.h"
#include "SDL_thread.h"

#include "drawer.h"
#include <future>
#define FF_QUIT_EVENT   (SDL_USEREVENT + 2)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
//const uint32_t PACKET_LIST_MAX_LEN = 30;

typedef struct PackList {
    AVPacket * packet = nullptr;
    struct PackList* next = nullptr;
} PackList;
// 解码所用的packetList， 适用fifo
class PacketQueue { // FIFO list
public:
    PackList *first {}, *end {};
    int size = 0;
    bool abort = false;
    SDL_mutex* mutex = SDL_CreateMutex();
    SDL_cond* cond= SDL_CreateCond();
    ~PacketQueue();
    [[maybe_unused]]AVPacket* pop();
    void push(AVPacket* p);
};

typedef struct DrawerCtx {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int width;
    int height;
} DrawerCtx;
typedef struct VideoState {
    bool abort = false;

    char filePath[48];

    std::future<void> decodeVideo;
    std::future<void> decodeAudio;

    PacketQueue* videoQueue;
    PacketQueue* audioQueue;

    //
    AVFormatContext* formatContext = nullptr;

    std::vector<DrawerCtx> drawCtx;

    AVStream* audio {};
    AVStream* video {};

    int videoStreamIndex = 0;
    int audioStreamIndex = 0;

    std::mutex videoMutex;
    std::mutex audioMutex;
} VideoState;



class Player {

private:

    int initFlags  = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
    VideoState * m_videoState{};
    SDL_Event m_event{};

    std::future<void> decode;
    std::future<void> decodeVideo;
    std::future<void> decodeAudio;
public:
    explicit Player(const char* filePath);
    ~Player();
    void main_event_loop();

    void doExit();


    void open_input(const char* filePath);

//    void open_close(VideoState* v);






};


#endif //FFM_PLAYER_H
