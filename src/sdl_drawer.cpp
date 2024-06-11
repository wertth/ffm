//
// Created by 缪浩楚 on 2024/6/9.
//
#include "SDL2/SDL.h"
#include "drawer.h"

SDL_DRAWER::SDL_DRAWER() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("无法初始化SDL: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }
}


void SDL_DRAWER::draw() {

}

[[maybe_unused]] SDL_Window* SDL_DRAWER::addWindow(const char *title, uint32_t width, uint32_t height, int x, int y, uint32_t flags) {
    SDL_Window* window = SDL_CreateWindow(title,x,y,width,height,flags);
    if(!m_windows.count(window)) {
        m_windows.emplace(window, new windowMatchRenderContext());
        m_active = window;
        return window;
    } else {
        // already has the window
        return nullptr;
    }
}

void SDL_DRAWER::hide(void *window) {

}

void SDL_DRAWER::show(void *window) {

}

void SDL_DRAWER::setDrawDevice() {

}

void SDL_DRAWER::mainLoop(const std::function<bool()>& func) {
    running = SDL_TRUE;
    while(func()) {
        SDL_Renderer* renderer = m_windows.at(m_active)->renderer;
        SDL_Texture* texture = m_windows.at(m_active)->texture;
        SDL_PollEvent(&m_event);
        if(m_event.type == SDL_QUIT) {
            running = SDL_FALSE;
            break;
        }
        SDL_RenderCopy(renderer,                        // sdl renderer
                       texture,                         // sdl texture
                       NULL,                                // src rect, if NULL copy texture
                       NULL                           // dst rect
        );
        // B8. 执行渲染，更新屏幕显示
        SDL_RenderPresent(renderer);
        SDL_Delay(40);
    }
}

void SDL_DRAWER::setRender(SDL_Window *window, SDL_Renderer *render) {
    if(m_windows.count(window)) {
        m_windows[window]->renderer = render;
    }
}

void SDL_DRAWER::setTexture(SDL_Window *window, SDL_Texture *texture) {
    if(m_windows.count(window)) {
        m_windows[window]->texture = texture;
    }
}
