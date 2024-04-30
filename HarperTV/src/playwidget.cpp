#include "stdafx.h"
#include "playwidget.h"
#include "decodethread.h"
#include <iostream>
#include <QTimer>

PlayWidget::PlayWidget(QWidget* parent) 
    : QWidget(parent), sdlWindow(nullptr), sdlRenderer(nullptr) {
    
    setAttribute(Qt::WA_PaintOnScreen, true);

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

    // 创建SDL纹理
    m_texture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 800, 600);
    if (!m_texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return;
    }

    // 创建定时器
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayWidget::renderFrame);
    timer->start(33); // 30帧每秒
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

void PlayWidget::play()
{
    DecodeThread* thread = new DecodeThread("http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8", &m_mutex, &m_frameAvailable, &m_frameQueue);
    thread->start();
}

void PlayWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);

    if (sdlWindow)
        SDL_SetWindowSize(sdlWindow, width(), height());
}

QPaintEngine* PlayWidget::paintEngine() const
{
    return nullptr;
}

void PlayWidget::renderFrame()
{
    //m_mutex.lock();
    AVFrame* frame;
    if (!m_frameQueue.isEmpty()) {
        frame = m_frameQueue.dequeue();
    }
    else {
        return;
    }
    
    //m_mutex.unlock();

    // 渲染图像帧
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = 192;
    r.h = 144;
    SDL_UpdateTexture(m_texture, &r, frame->data[0], frame->linesize[0]);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, m_texture, nullptr, nullptr);
    SDL_RenderPresent(sdlRenderer);

    av_frame_free(&frame);
    /*
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
    */
}
