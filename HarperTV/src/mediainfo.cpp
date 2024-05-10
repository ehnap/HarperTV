#include "pub.h"
#include "mediainfo.h"
#include <QMutex>
#include <QWaitCondition>


MediaInfo::MediaInfo(const QString& url)
	: url_(url)
	, aud_queue_mutex_(new QMutex())
	, vid_queue_mutex_(new QMutex())
	, aud_wait_condition_(new QWaitCondition())
	, vid_wait_condition_(new QWaitCondition())
	, decoder_(nullptr)
{
}

MediaInfo::~MediaInfo()
{
}

QString MediaInfo::GetMediaUrl() const
{
	return url_;
}

void MediaInfo::SetDecoder(FFmpegDecoder* d)
{
	decoder_ = d;
}

FFmpegDecoder* MediaInfo::GetDecoder() const
{
	return decoder_;
}

void MediaInfo::EnqueueAudioFrame(AVFrame* f)
{
	if (!f) {
		return;
	}
	aud_queue_mutex_->lock();
	decoded_audio_frames_.enqueue(f);
	aud_wait_condition_->wait(aud_queue_mutex_);
	aud_queue_mutex_->unlock();
}

void MediaInfo::EnqueueVideoFrame(AVFrame* f)
{
	if (!f) {
		return;
	}
	vid_queue_mutex_->lock();
	decoded_video_frames_.enqueue(f);
	vid_wait_condition_->wait(vid_queue_mutex_);
	vid_queue_mutex_->unlock();
}

AVFrame* MediaInfo::DequeueAudioFrame()
{
	AVFrame* f = nullptr;
	aud_queue_mutex_->lock();
	if (!decoded_audio_frames_.isEmpty()) {
		f = decoded_audio_frames_.dequeue();
	}
	aud_wait_condition_->wakeAll();
	aud_queue_mutex_->unlock();
	return f;
}

AVFrame* MediaInfo::DequeueVideoFrame()
{
	AVFrame* f = nullptr;
	vid_queue_mutex_->lock();
	if (!decoded_video_frames_.isEmpty()) {
		f = decoded_video_frames_.dequeue();
	}
	vid_wait_condition_->wakeAll();
	vid_queue_mutex_->unlock();
	return f;
}
