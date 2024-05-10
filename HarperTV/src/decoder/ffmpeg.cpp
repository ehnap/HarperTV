﻿#include "../pub.h"
#include "ffmpeg.h"
#include "../mediainfo.h"
#include <iostream>

FFmpegDecoder::FFmpegDecoder(MediaInfo* f)
    : QThread(nullptr)
    , media_info_(f)
{
    avformat_network_init();
}

bool FFmpegDecoder::Init()
{
    std::string url = media_info_->GetMediaUrl().toStdString();
    if (avformat_open_input(&avc_, url.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Could not open input stream" << std::endl;
        return false;
    }

    // 读取流信息
    if (avformat_find_stream_info(avc_, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        avformat_close_input(&avc_);
        return false;
    }

    // 找到流
    for (unsigned int i = 0; i < avc_->nb_streams; i++) {
        if (avc_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vid_stream_index_ = i;
        }
        if (avc_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            aud_stream_index_ = i;
        }
    }
    if (vid_stream_index_ == -1) {
        std::cerr << "Could not find video stream" << std::endl;
        avformat_close_input(&avc_);
        return false;
    }

    if (aud_stream_index_ == -1) {
        std::cerr << "Could not find audio stream" << std::endl;
        avformat_close_input(&avc_);
        return false;
    }

    if (!InitVideoCodecContext()) {
        return false;
    }

    if (!InitAudioCodecContext()) {
        return false;
    }
    
    media_info_->SetDecoder(this);
    return true;
}

int FFmpegDecoder::GetAudioBufferSize(AVFrame* avf)
{
    if (!avf) {
        return 0;
    }
    if (!aud_codec_context_) {
        return 0;
    }
    int audio_buffer_size = av_samples_get_buffer_size(NULL, aud_codec_context_->ch_layout.nb_channels, avf->nb_samples, aud_codec_context_->sample_fmt, 1);
    return audio_buffer_size;
}

AudStreamParameters FFmpegDecoder::GetAudioStreamParameters()
{
    AVCodecParameters* codec_parameters = avc_->streams[aud_stream_index_]->codecpar;
    int sample_rate = codec_parameters->sample_rate;
    enum AVSampleFormat sample_format = (AVSampleFormat)codec_parameters->format;
    int frame_size = codec_parameters->frame_size;
    int channels = codec_parameters->ch_layout.nb_channels;

    AudStreamParameters ps;
    ps.channels = channels;
    ps.frame_size = frame_size;
    ps.sample_format = (AVSampleFormat)codec_parameters->format;
    ps.sample_rate = sample_rate;
    return ps;
}

int FFmpegDecoder::GetDelayTime(AVFrame* avf)
{
    // 计算等待时间以保持视频同步
    int delay_time = avf->pts * av_q2d(avc_->streams[vid_stream_index_]->time_base) * 1000 - SDL_GetTicks();
    return delay_time;
}

void FFmpegDecoder::run()
{
    // 解码并将帧添加到队列
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate frame" << std::endl;
        avcodec_free_context(&vid_codec_context_);
        avcodec_free_context(&aud_codec_context_);
        avformat_close_input(&avc_);
        return;
    }

    while (av_read_frame(avc_, &packet) >= 0) {
        if (packet.stream_index == vid_stream_index_) {
            //video
            avcodec_send_packet(vid_codec_context_, &packet);
            av_frame_unref(frame);
            avcodec_receive_frame(vid_codec_context_, frame);
            AVFrame* newFrame = av_frame_clone(frame);
            if (newFrame) {
                media_info_->EnqueueVideoFrame(newFrame);
            }
        } else if (packet.stream_index == aud_stream_index_) {
            avcodec_send_packet(aud_codec_context_, &packet);
            av_frame_unref(frame);
            int ret = avcodec_receive_frame(aud_codec_context_, frame);
            if (ret == 0) {
                /*
                // 创建一个新的AVFrame来存储转换后的数据
                AVFrame* out_frame = av_frame_alloc();
                out_frame->ch_layout = frame->ch_layout;
                out_frame->format = AV_SAMPLE_FMT_S16;
                out_frame->sample_rate = 22050;
                out_frame->nb_samples = frame->nb_samples;

                // 假设输出样本数和输入样本数相同

               // 分配数据缓冲区
                av_frame_get_buffer(out_frame, 0);

                // 进行重采样
                if (swr_convert(audio_convert_ctx_, out_frame->data, out_frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples) < 0) {
                }
                */
                AVFrame* newFrame = av_frame_clone(frame);
                media_info_->EnqueueAudioFrame(newFrame);
            }
            else if (ret == AVERROR_EOF) {
                // 所有的输入数据都已经被解码并返回
                break;
            }
            else if (ret == AVERROR(EAGAIN)) {
                // 需要更多的输入数据才能返回下一帧
                continue;
            }
            else {
                // 处理其他错误情况
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    avcodec_free_context(&vid_codec_context_);
    avcodec_free_context(&aud_codec_context_);
    avformat_close_input(&avc_);
}

bool FFmpegDecoder::InitVideoCodecContext()
{
    AVCodecParameters* codecParameters = avc_->streams[vid_stream_index_]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        std::cerr << "Unsupported codec" << std::endl;
        avformat_close_input(&avc_);
        return false;
    }
    vid_codec_context_ = avcodec_alloc_context3(codec);
    if (!vid_codec_context_) {
        std::cerr << "Could not allocate codec context" << std::endl;
        avformat_close_input(&avc_);
        return false;
    }
    if (avcodec_parameters_to_context(vid_codec_context_, codecParameters) < 0) {
        std::cerr << "Could not initialize codec context" << std::endl;
        avcodec_free_context(&vid_codec_context_);
        avformat_close_input(&avc_);
        return false;
    }
    if (avcodec_open2(vid_codec_context_, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        avcodec_free_context(&vid_codec_context_);
        avformat_close_input(&avc_);
        return false;
    }
    
    return true;
}

bool FFmpegDecoder::InitAudioCodecContext()
{
    // 获取音频解码器上下文
    const AVCodec* codec = avcodec_find_decoder(avc_->streams[aud_stream_index_]->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "找不到音频解码器\n");
        return false;
    }

    aud_codec_context_ = avcodec_alloc_context3(codec);
    if (!aud_codec_context_) {
        fprintf(stderr, "无法分配解码器上下文\n");
        return false;
    }

    if (avcodec_parameters_to_context(aud_codec_context_, avc_->streams[aud_stream_index_]->codecpar) < 0) {
        fprintf(stderr, "无法填充解码器上下文\n");
        return false;
    }

    // 打开解码器
    if (avcodec_open2(aud_codec_context_, codec, NULL) < 0) {
        fprintf(stderr, "无法打开解码器\n");
        return false;
    }

    AVChannelLayout outChannelLayout;
    AVChannelLayout inChannelLayout = avc_->streams[aud_stream_index_]->codecpar->ch_layout;
    outChannelLayout = AV_CHANNEL_LAYOUT_MONO;

    if (swr_alloc_set_opts2(&audio_convert_ctx_, 
        &outChannelLayout,
        AV_SAMPLE_FMT_S16, 
        22050,
        &avc_->streams[aud_stream_index_]->codecpar->ch_layout,
        (AVSampleFormat)avc_->streams[aud_stream_index_]->codecpar->format,
        avc_->streams[aud_stream_index_]->codecpar->sample_rate,
        0,
        NULL) < 0) {
        fprintf(stderr, "无法打开转换器\n");
        return false;
    }

    swr_init(audio_convert_ctx_);

    return true;
}
