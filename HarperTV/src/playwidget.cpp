#include "pub.h"
#include "playwidget.h"
#include "mediainfo.h"
#include "decoder/ffmpeg.h"
#include "renderer/sdlvid.h"
#include "renderer/sdlaud.h"
#include <iostream>
#include <QTimer>

PlayWidget::PlayWidget(QWidget* parent) 
    : QWidget(parent)
    , window_(nullptr)
    , vid_renderer_(nullptr)
    , aud_renderer_(nullptr)
    , decoder_(nullptr)
    , media_info_(nullptr)
{
    
    setAttribute(Qt::WA_PaintOnScreen, true);

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // SDL初始化失败
        qWarning("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    // 创建SDL窗口
    window_ = SDL_CreateWindowFrom((void*)winId());
    if (!window_) {
        // 创建窗口失败
        qWarning("SDL_CreateWindowFrom failed: %s", SDL_GetError());
        return;
    }
}

PlayWidget::~PlayWidget()
{
    // 清理SDL资源
    if (vid_renderer_) {
        delete vid_renderer_;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
        
    SDL_Quit();
}

void PlayWidget::play()
{
    SetMediaInfo(new MediaInfo("http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8"));
    decoder_ = new FFmpegDecoder(media_info_);
    decoder_->Init();

    vid_renderer_ = new SdlVidRenderer(media_info_);
    vid_renderer_->Init(window_);

    auto ps = decoder_->GetAudioStreamParameters();
    aud_renderer_ = new SdlAudRenderer(media_info_);
    aud_renderer_->Init(ps);

    decoder_->start();
}

void PlayWidget::SetMediaInfo(MediaInfo* info)
{
    media_info_ = info;
}

void PlayWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);

    if (window_)
        SDL_SetWindowSize(window_, width(), height());
}

QPaintEngine* PlayWidget::paintEngine() const
{
    return nullptr;
}