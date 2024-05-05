#pragma once

#include "streamparameters.h"

#include <Queue>
#include <QThread>

class QMutex;
class AVFrame;
class QWaitCondition;
class AVFormatContext;
class SwsContext;
class SwrContext;

class FFmpegDecoder : public QThread
{
public:
    FFmpegDecoder(
        QMutex* mutex,
        QWaitCondition* frame_available,
        QQueue<AVFrame*>* vid_frame_queue,
        QQueue<AVFrame*>* aud_frame_queue);
    
    bool Init(const std::string& url);
    int GetAudioBufferSize(AVFrame* avf);
    AudStreamParameters GetAudioStreamParameters();
    int GetDelayTime(AVFrame* avf);

    void run() override;

private:
    bool InitVideoCodecContext();
    bool InitAudioCodecContext();

private:
    std::string url_;
    QMutex* mutex_;
    QWaitCondition* frame_available_;
    QQueue<AVFrame*>* vid_frame_queue_;
    QQueue<AVFrame*>* aud_frame_queue_;
    int aud_stream_index_ = -1;
    int vid_stream_index_ = -1;
    AVFormatContext* avc_ = nullptr;
    AVCodecContext* aud_codec_context_ = nullptr;
    AVCodecContext* vid_codec_context_ = nullptr;
    int w_ = 0;
    int h_ = 0;
    SwrContext* audio_convert_ctx_ = nullptr;
};