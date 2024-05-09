#pragma once
#include <QWidget>

class SdlVidRenderer;
class SdlAudRenderer;
class FFmpegDecoder;
class MediaInfo;

class PlayWidget : public QWidget {
public:
    PlayWidget(QWidget* parent = nullptr);
    ~PlayWidget();

    void play();
    void SetMediaInfo(MediaInfo* info);

protected:
    void resizeEvent(QResizeEvent* e) override;
    QPaintEngine* paintEngine() const override;

private:
    void vidRenderFrame();
    void audRenderFrame();

private:
    SdlVidRenderer* vid_renderer_;
    SdlAudRenderer* aud_renderer_;
    SDL_Window* window_;
    QMutex m_mutex;
    QWaitCondition m_frameAvailable;
    
    QQueue<AVFrame*> vid_frame_queue_;
    QQueue<AVFrame*> aud_frame_queue_;
    FFmpegDecoder* decoder_;
    MediaInfo* media_info_;
};