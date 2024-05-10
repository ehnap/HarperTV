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
    SdlVidRenderer* vid_renderer_;
    SdlAudRenderer* aud_renderer_;
    SDL_Window* window_;
    QWaitCondition m_frameAvailable;
    FFmpegDecoder* decoder_;
    MediaInfo* media_info_;
};