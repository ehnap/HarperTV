#include "../pub.h"
#include "sdlvid.h"

#include <iostream>

SdlVidRenderer::SdlVidRenderer()
    : texture_(nullptr)
    , renderer_(nullptr)
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

    int width = 778;
    int height = 578;
    SDL_GetWindowSize(w, &width, &height);

    // 创建SDL纹理
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 192, 144);
    if (!texture_) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return;
    }
}

void SdlVidRenderer::RenderFrame(AVFrame* avf)
{
    // 渲染图像帧
    SDL_UpdateYUVTexture(texture_, NULL, 
        avf->data[0], avf->linesize[0], 
        avf->data[1], avf->linesize[1], 
        avf->data[2], avf->linesize[2]);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}
