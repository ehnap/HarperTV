#include "../pub.h"
#include "ffmpeg.h"
#include <iostream>

FFmpegDecoder::FFmpegDecoder(
	QMutex* mutex, 
	QWaitCondition* frame_available,
	QQueue<AVFrame*>* vid_frame_queue,
    QQueue<AVFrame*>* aud_frame_queue)
	: mutex_(mutex)
	, frame_available_(frame_available)
	, vid_frame_queue_(vid_frame_queue)
    , aud_frame_queue_(aud_frame_queue)
{
    avformat_network_init();
}

bool FFmpegDecoder::Init(const std::string& url)
{
    url_ = url;
    // 打开输入流
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
                mutex_->lock();
                vid_frame_queue_->enqueue(newFrame);
                mutex_->unlock();
            }
        } else if (packet.stream_index == aud_stream_index_) {
            avcodec_send_packet(aud_codec_context_, &packet);
            av_frame_unref(frame);
            int ret = avcodec_receive_frame(aud_codec_context_, frame);
            if (ret == 0) {
                // 计算重采样转换后的样本数量,从而分配缓冲区大小
                int64_t nCvtBufSamples = av_rescale_rnd(frame->nb_samples, 22050, frame->sample_rate, AV_ROUND_UP);

                // 创建输出音频帧
                AVFrame* pOutFrame = av_frame_alloc();
                pOutFrame->format = AV_SAMPLE_FMT_S16;
                pOutFrame->sample_rate = 22050;
                pOutFrame->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
                swr_convert_frame(audio_convert_ctx_, pOutFrame, frame);
                pOutFrame->pts = frame->pts;      // pts等时间戳沿用
                pOutFrame->pkt_dts = frame->pkt_dts;

                //AVFrame* newFrame = av_frame_clone(frame);
                aud_frame_queue_->enqueue(pOutFrame);
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
    AVChannelLayout inChannelLayout = aud_codec_context_->ch_layout;
    outChannelLayout.nb_channels = 2;
    outChannelLayout = AV_CHANNEL_LAYOUT_STEREO;

    if (swr_alloc_set_opts2(&audio_convert_ctx_, 
        &outChannelLayout,
        AV_SAMPLE_FMT_S16, 
        22050,
        &inChannelLayout,
        AV_SAMPLE_FMT_FLTP,
        22050, 
        0,
        NULL) < 0) {
        fprintf(stderr, "无法打开转换器\n");
        return false;
    }

    swr_init(audio_convert_ctx_);

    return true;
}
