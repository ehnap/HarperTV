#include "stdafx.h"
#include "playwidget.h"
#include "decodethread.h"
#include <iostream>
#include <QTimer>

PlayWidget::PlayWidget(QWidget* parent) 
    : QWidget(parent), sdlWindow(nullptr), sdlRenderer(nullptr) {
    
    setAttribute(Qt::WA_PaintOnScreen, true);

    // ��ʼ��SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // SDL��ʼ��ʧ��
        qWarning("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    // ����SDL����
    sdlWindow = SDL_CreateWindowFrom((void*)winId());
    if (!sdlWindow) {
        // ��������ʧ��
        qWarning("SDL_CreateWindowFrom failed: %s", SDL_GetError());
        return;
    }

    // ����SDL��Ⱦ��
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        // ������Ⱦ��ʧ��
        qWarning("SDL_CreateRenderer failed: %s", SDL_GetError());
        return;
    }

    // ����SDL����
    m_texture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 800, 600);
    if (!m_texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return;
    }

    // ������ʱ��
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayWidget::renderFrame);
    timer->start(33); // 30֡ÿ��
}

PlayWidget::~PlayWidget()
{
    // ����SDL��Դ
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

    // ��Ⱦͼ��֡
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
    // ��ȾSDL����
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(sdlRenderer);

    // ����һ����
    int centerX = 400; // �������X����
    int centerY = 300; // �������Y����
    int radius = 50;   // ��İ뾶
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // ���û�����ɫΪ��ɫ
    for (int x = centerX - radius; x <= centerX + radius; x++) {
        for (int y = centerY - radius; y <= centerY + radius; y++) {
            if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radius * radius) {
                SDL_RenderDrawPoint(sdlRenderer, x, y); // ����һ��Բ���еĵ�
            }
        }
    }

    SDL_RenderPresent(sdlRenderer);
    */
}
