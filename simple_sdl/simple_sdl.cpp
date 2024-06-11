#include "SDL2/SDL.h"

int main(int argc, char* argv[]) {
    SDL_Window* window;      // 声明一个窗口
    SDL_Renderer* renderer;  // 声明一个渲染器
    SDL_Event event;         // 声明一个事件
    SDL_bool running = SDL_TRUE; // 控制游戏循环的标志

    // 初始化SDL环境
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("无法初始化SDL: %s", SDL_GetError());
        return 1;
    }

    // 创建一个窗口
    window = SDL_CreateWindow(
            "SDL 示例",                  // 窗口标题
            SDL_WINDOWPOS_CENTERED,      // 初始x位置
            SDL_WINDOWPOS_CENTERED,      // 初始y位置
            512,                         // 宽度，以像素为单位
            512,                         // 高度，以像素为单位
            0                            // 窗口标志
    );

    if (!window) {
        SDL_Log("窗口创建失败: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // 为窗口创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("渲染器创建失败: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 设置渲染器的绘制颜色（RGBA）
    SDL_SetRenderDrawColor(renderer, 64, 255, 128, 255);

    while (running) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
        }

        // 清除当前渲染目标
        SDL_RenderClear(renderer);

        // 执行渲染操作，更新窗口
        SDL_RenderPresent(renderer);

        // 等待一段时间
        SDL_Delay(16); // 约60帧每秒
    }

    // 清理资源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
