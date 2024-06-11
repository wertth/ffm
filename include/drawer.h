//
// Created by 缪浩楚 on 2024/6/9.
//

#ifndef FFM_DRAWER_H
#define FFM_DRAWER_H
#include <SDL2/SDL.h>
//#include "vector"
#include <map>
// interface
class DRAWER {
public:
//    DRAWER () = delete;
    virtual ~DRAWER() = default;
    virtual void draw() = 0;
    virtual void hide(void* window) = 0;
    virtual void show(void* window) = 0;
    virtual void setDrawDevice() = 0;
    virtual void mainLoop(const std::function<bool()>& func) = 0;
};

//enum RUNNING_STATUS {
//    running,
//    pause,
//    stop
//};

class SDL_DRAWER: public DRAWER{
private:
    typedef struct {
        SDL_Renderer* renderer;
        SDL_Texture* texture;
    } windowMatchRenderContext;
    std::map<SDL_Window*, windowMatchRenderContext* > m_windows{};
    SDL_Window* m_active = nullptr;
    SDL_Event m_event {};
    SDL_bool running = SDL_FALSE;
public:
    SDL_DRAWER();
    ~SDL_DRAWER() override {
        quit();
    };
    inline
    void quit() {
        for(auto& [key, val]: m_windows) {
            if(val->renderer) {
                SDL_RenderClear(val->renderer);
            }
            if(val->texture) {
                SDL_DestroyTexture(val->texture);
            }
            delete val;
            SDL_DestroyWindow(key);
            SDL_Quit();
        }
    }

    [[maybe_unused]]
    SDL_Window* addWindow(const char* title, uint32_t width, uint32_t height,int x, int y, uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    void setRender(SDL_Window* window, SDL_Renderer * render);
    void setTexture(SDL_Window* window, SDL_Texture* texture);

    void draw() override ;
    void hide(void *window) override;
    void show(void *window) override;
    void setDrawDevice() override;
    void mainLoop(const std::function<bool()>& func) override;
};


#endif //FFM_DRAWER_H
