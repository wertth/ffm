//
// Created by 缪浩楚 on 2024/6/9.
//
#include "SDL2/SDL.h"
#include "drawer.h"
#include <future>
#include <iostream>

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

uint32_t DRAWER::m_frameRate = 60;

SDL_DRAWER::SDL_DRAWER() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("无法初始化SDL: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }
    running = SDL_TRUE;
}


void SDL_DRAWER::draw() {
// define refresh thread
//    m_refresher = std::async(std::launch::async,[this]() {
//        while (running) {
//            for (auto& [window, context] : m_windows) {
//                context->refreshFlow();
//            }
//        }
//    });
//    m_refresher.get();
}

[[maybe_unused]] SDL_Window* SDL_DRAWER::addWindow(const char *title, uint32_t width, uint32_t height, int x, int y, uint32_t flags) {
    SDL_Window* window = SDL_CreateWindow(title,x,y,width,height,flags);
    if(!window) throw std::runtime_error("add window error");
    if(!m_windows.count(window)) {
        m_windows.emplace(window, std::make_unique<windowMatchRenderContext>());
        m_active = window;
        m_windows.at(m_active)->frameRate = 25;

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

void SDL_DRAWER::mainLoop(SDL_Window* window , const std::function<bool()>& func, uint32_t frequency) {
    m_windows[window]->refreshFlow = [func, this, &frequency]() {
        while(running) {
            SDL_Event event;
            if(!func()) {
                event.type = SFM_BREAK_EVENT;
                SDL_PushEvent(&event);
                break;
            }
            SDL_Renderer* renderer =m_windows.at(m_active)->renderer.get();
            SDL_Texture* texture = m_windows.at(m_active)->texture.get();
            if (!renderer || !texture) {
                break;
            }
            SDL_RenderCopy(renderer,                        // sdl renderer
                           texture,                         // sdl texture
                           nullptr,                                // src rect, if NULL copy texture
                           nullptr                           // dst rect
            );
//            std::cout << "sub" ;
            // B8. 执行渲染，更新屏幕显示
            SDL_RenderPresent(renderer);
            if(frequency) {
                m_windows[m_active]->frameRate = frequency;
            }

            event.type = SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
            SDL_Delay(1000 / m_windows[m_active]->frameRate);
        }
    };

    m_refresher = std::async(std::launch::async, m_windows[window]->refreshFlow);

//    int i = 0;
    while(running) {
//        std::cout<< i++ << std::endl;
        SDL_WaitEvent(&m_event);
        if(m_event.type == SDL_QUIT) {
            running = SDL_FALSE;
            break;
        } else if( m_event.type == SFM_BREAK_EVENT ) {
            running = SDL_FALSE;
            break;
        }
    }

    if (m_refresher.valid()) {
        m_refresher.get();
    }
}

void SDL_DRAWER::setRender(SDL_Window *window, SDL_Renderer *render) {
    if(m_windows.count(window)) {
        m_windows[window]->renderer.reset(render, SDL_DestroyRenderer);
    }
}

void SDL_DRAWER::setTexture(SDL_Window *window, SDL_Texture *texture) {
    if(m_windows.count(window)) {
        m_windows[window]->texture.reset(texture, SDL_DestroyTexture);
    }
}
