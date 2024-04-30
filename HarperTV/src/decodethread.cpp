#include "stdafx.h"
#include <iostream>
#include "decodethread.h"
#include <QApplication>

DecodeThread::DecodeThread(const std::string& url,
    QMutex* mutex,
    QWaitCondition* frameAvailable,
    QQueue<AVFrame*>* frameQueue)
    : m_url(url)
    , m_mutex(mutex)
    , m_frameAvailable(frameAvailable)
    , m_frameQueue(frameQueue)
{
}

void DecodeThread::run() {

    avformat_network_init();

    // 打开输入流
    AVFormatContext* formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, m_url.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Could not open input stream" << std::endl;
        return;
    }

    // 读取流信息
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // 找到视频流
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        std::cerr << "Could not find video stream" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // 创建解码器上下文
    AVCodecParameters* codecParameters = formatCtx->streams[videoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        std::cerr << "Unsupported codec" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        std::cerr << "Could not allocate codec context" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }
    if (avcodec_parameters_to_context(codecCtx, codecParameters) < 0) {
        std::cerr << "Could not initialize codec context" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // 创建转换器上下文
    SwsContext* swsCtx = sws_getContext(codecParameters->width, codecParameters->height, codecCtx->pix_fmt,
        codecParameters->width, codecParameters->height, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx) {
        std::cerr << "Could not initialize conversion context" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // 解码并将帧添加到队列
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate video frame" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        sws_freeContext(swsCtx);
        return;
    }

    while (av_read_frame(formatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            avcodec_send_packet(codecCtx, &packet);
            avcodec_receive_frame(codecCtx, frame);

            AVFrame* newFrame = av_frame_clone(frame);
            if (newFrame) {
                m_mutex->lock();
                m_frameQueue->enqueue(newFrame);
                m_frameAvailable->wakeAll();
                m_mutex->unlock();
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    sws_freeContext(swsCtx);
}
