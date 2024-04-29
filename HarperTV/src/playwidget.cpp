#include "stdafx.h"
#include "playwidget.h"
#include <QTimer>

PlayWidget::PlayWidget(QWidget* parent) 
    : QWidget(parent), sdlWindow(nullptr), sdlRenderer(nullptr) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // SDL初始化失败
        qWarning("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    // 创建SDL窗口
    sdlWindow = SDL_CreateWindowFrom((void*)winId());
    if (!sdlWindow) {
        // 创建窗口失败
        qWarning("SDL_CreateWindowFrom failed: %s", SDL_GetError());
        return;
    }

    // 创建SDL渲染器
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        // 创建渲染器失败
        qWarning("SDL_CreateRenderer failed: %s", SDL_GetError());
        return;
    }

    // 创建定时器
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayWidget::renderFrame);
    timer->start(16); // 设置定时器间隔，单位毫秒
}

PlayWidget::~PlayWidget()
{
    // 清理SDL资源
    if (sdlRenderer)
        SDL_DestroyRenderer(sdlRenderer);
    if (sdlWindow)
        SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
}

void PlayWidget::renderFrame()
{
    // 渲染SDL画面
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(sdlRenderer);

    // 绘制一个球
    int centerX = 400; // 球的中心X坐标
    int centerY = 300; // 球的中心Y坐标
    int radius = 50;   // 球的半径
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // 设置绘制颜色为白色
    for (int x = centerX - radius; x <= centerX + radius; x++) {
        for (int y = centerY - radius; y <= centerY + radius; y++) {
            if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radius * radius) {
                SDL_RenderDrawPoint(sdlRenderer, x, y); // 绘制一个圆形中的点
            }
        }
    }

    SDL_RenderPresent(sdlRenderer);
}
