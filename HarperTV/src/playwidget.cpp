#include "stdafx.h"
#include "playwidget.h"
#include <QTimer>

PlayWidget::PlayWidget(QWidget* parent) 
    : QWidget(parent), sdlWindow(nullptr), sdlRenderer(nullptr) {
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

    // ������ʱ��
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayWidget::renderFrame);
    timer->start(16); // ���ö�ʱ���������λ����
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

void PlayWidget::renderFrame()
{
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
}
