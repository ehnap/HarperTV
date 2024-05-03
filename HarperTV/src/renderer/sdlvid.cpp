#include "../pub.h"
#include "sdlvid.h"

#include <iostream>

SdlVidRenderer::SdlVidRenderer()
{
}

SdlVidRenderer::~SdlVidRenderer()
{
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
        
}

void SdlVidRenderer::Init(SDL_Window* w)
{
    // 创建SDL渲染器
    renderer_ = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        // 创建渲染器失败
        qWarning("SDL_CreateRenderer failed: %s", SDL_GetError());
        return;
    }

    // 创建SDL纹理
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 800, 600);
    if (!texture_) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return;
    }
}

void SdlVidRenderer::RenderFrame(AVFrame* avf)
{
    // 渲染图像帧
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = 192;
    r.h = 144;
    SDL_UpdateTexture(texture_, &r, avf->data[0], avf->linesize[0]);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);

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
