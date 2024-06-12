//
// Created by 缪浩楚 on 2024/6/9.
//

#ifndef FFM_DRAWER_H
#define FFM_DRAWER_H
#include <SDL2/SDL.h>
//#include "vector"
#include <map>
#include <thread>
#include <future>
// interface
class DRAWER {
public:
    static uint32_t m_frameRate;
    virtual ~DRAWER() = default;
    virtual void draw() = 0;
    virtual void hide(void* window) = 0;
    virtual void show(void* window) = 0;
    virtual void setDrawDevice() = 0;
};


class SDL_DRAWER: public DRAWER{
private:
    typedef struct {
        std::shared_ptr<SDL_Renderer> renderer;
        std::shared_ptr<SDL_Texture> texture;
        std::function<void()> refreshFlow;
        uint32_t frameRate;
    } windowMatchRenderContext;

    std::map<SDL_Window*, std::unique_ptr<windowMatchRenderContext>> m_windows{};

    std::future<void> m_refresher;

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
            SDL_DestroyWindow(key);
        }
        SDL_Quit();
    }

    [[maybe_unused]]
    SDL_Window* addWindow(const char* title, uint32_t width, uint32_t height,int x, int y, uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    void setRender(SDL_Window* window, SDL_Renderer * render);
    void setTexture(SDL_Window* window, SDL_Texture* texture);

    void draw() override ;
    void hide(void *window) override;
    void show(void *window) override;
    void setDrawDevice() override;
    void mainLoop(SDL_Window* window , const std::function<bool()>& func, uint32_t frequency);

};


#endif //FFM_DRAWER_H
