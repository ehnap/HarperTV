#pragma once
#include "streamparameters.h"
#include <QThread>

class AVFrame;
class AVFormatContext;
class SwsContext;
class SwrContext;
class MediaInfo;

class FFmpegDecoder : public QThread
{
public:
    FFmpegDecoder(MediaInfo* f);
    
    bool Init();
    int GetAudioBufferSize(AVFrame* avf);
    AudStreamParameters GetAudioStreamParameters();
    int GetDelayTime(AVFrame* avf);

protected:
    void run() override;

private:
    bool InitVideoCodecContext();
    bool InitAudioCodecContext();

private:
    int aud_stream_index_ = -1;
    int vid_stream_index_ = -1;
    AVFormatContext* avc_ = nullptr;
    AVCodecContext* aud_codec_context_ = nullptr;
    AVCodecContext* vid_codec_context_ = nullptr;
    int w_ = 0;
    int h_ = 0;
    SwrContext* audio_convert_ctx_ = nullptr;
    MediaInfo* media_info_ = nullptr;
};