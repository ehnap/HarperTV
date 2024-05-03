#include "pub.h"
#include "playwidget.h"
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
    decoder_ = new FFmpegDecoder(&m_mutex, &m_frameAvailable, &vid_frame_queue_, &aud_frame_queue_);
    decoder_->Init("http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8");

    vid_renderer_ = new SdlVidRenderer();
    vid_renderer_->Init(window_);

    auto ps = decoder_->GetAudioStreamParameters();
    aud_renderer_ = new SdlAudRenderer();
    aud_renderer_->Init(ps);

    QTimer::singleShot(33, Qt::PreciseTimer, this, &PlayWidget::vidRenderFrame);
    QTimer::singleShot(47, Qt::PreciseTimer, this, &PlayWidget::audRenderFrame);

    decoder_->start();
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

void PlayWidget::vidRenderFrame()
{
    int t = 33;
    do {
        if (!vid_renderer_)
            break;
        //m_mutex.lock();
        AVFrame* frame;
        if (!vid_frame_queue_.isEmpty()) {
            frame = vid_frame_queue_.dequeue();
        } else {
            break;
        }
        vid_renderer_->RenderFrame(frame);
        auto dt = decoder_->GetDelayTime(frame);
        t = dt > 0 ? dt : t;
        av_frame_free(&frame);
    } while (false);
    
    QTimer::singleShot(t, Qt::PreciseTimer, this, &PlayWidget::vidRenderFrame);
}

void PlayWidget::audRenderFrame()
{
    int t = 47;
    do {
        if (!aud_renderer_)
            break;

        AVFrame* frame;
        if (!aud_frame_queue_.isEmpty()) {
            frame = aud_frame_queue_.dequeue();
        } else {
            break;
        }
        aud_renderer_->RenderFrame(frame, decoder_);
        auto dt = decoder_->GetDelayTime(frame);
        t = dt > 0 ? dt : t;
        av_frame_free(&frame);
    } while (false);

    QTimer::singleShot(t, Qt::PreciseTimer, this, &PlayWidget::audRenderFrame);
}
