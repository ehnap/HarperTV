#include "../pub.h"
#include "sdlaud.h"
#include "../decoder/ffmpeg.h"
#include "audiorenderthread.h"
#include "../mediainfo.h"

SdlAudRenderer::SdlAudRenderer(MediaInfo* info)
    : QObject(nullptr)
    , media_info_(info)
{
    thread_ = new AudioRenderThread(info);
    connect(thread_, &AudioRenderThread::render, this, &SdlAudRenderer::Render);
}

SdlAudRenderer::~SdlAudRenderer()
{
}

void SdlAudRenderer::Init(AudStreamParameters params)
{
    SDL_AudioSpec wanted_spec, obtained_spec;

    wanted_spec.freq = params.sample_rate;  // 采样率
    wanted_spec.format = AUDIO_S16SYS; // 音频格式（signed 16-bit samples）
    wanted_spec.channels = params.channels;   // 声道数
    wanted_spec.samples = params.frame_size; // 每个缓冲区的样本数
    wanted_spec.callback = NULL; // 指定回调函数，如果使用回调方式
    wanted_spec.userdata = NULL; // 回调函数参数

    // 打开音频设备
    if (SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &obtained_spec, true) < 0) {
        fprintf(stderr, "无法打开音频设备: %s\n", SDL_GetError());
        return;
    }
    SDL_PauseAudio(0); // 开始播放音频
    thread_->start();
}

void SdlAudRenderer::Render(PAVFrame f)
{
    RenderFrame(f, media_info_->GetDecoder());
    av_frame_free(&f);
}

void SdlAudRenderer::RenderFrame(AVFrame* avf, FFmpegDecoder* d)
{
    // 成功接收音频帧，渲染音频
    uint8_t* audio_buffer = avf->data[0];
    int audio_buffer_size = d->GetAudioBufferSize(avf);
    if (audio_buffer_size > 0) {
        SDL_QueueAudio(1, audio_buffer, audio_buffer_size);
    } else {
        SDL_QueueAudio(1, audio_buffer, avf->linesize[0]);
    }
    
}
