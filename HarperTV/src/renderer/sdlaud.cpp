#include "../pub.h"
#include "sdlaud.h"
#include "../decoder/ffmpeg.h"

SdlAudRenderer::SdlAudRenderer()
{
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
    if (SDL_OpenAudio(&wanted_spec, &obtained_spec) < 0) {
        fprintf(stderr, "无法打开音频设备: %s\n", SDL_GetError());
        return;
    }
    SDL_PauseAudio(0); // 开始播放音频
}

void SdlAudRenderer::RenderFrame(AVFrame* avf, FFmpegDecoder* d)
{
    // 成功接收音频帧，渲染音频
    uint8_t* audio_buffer = avf->data[0];
    int audio_buffer_size = d->GetAudioBufferSize(avf);
    SDL_QueueAudio(1, audio_buffer, audio_buffer_size);
}
