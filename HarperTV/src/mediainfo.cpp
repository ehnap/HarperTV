#include "mediainfo.h"
#include <QMutex>

MediaInfo::MediaInfo(const QString& url)
	: url_(url)
	, aud_queue_mutex_(new QMutex(QMutex::NonRecursive))
	, vid_queue_mutex_(new QMutex(QMutex::NonRecursive))
{
}

MediaInfo::~MediaInfo()
{
}

void MediaInfo::EnqueueAudioFrame(AVFrame* f)
{
	if (!f) {
		return;
	}
	aud_queue_mutex_->lock();
	decoded_audio_frames_.enqueue(f);
	aud_queue_mutex_->unlock();
}

void MediaInfo::EnqueueVideoFrame(AVFrame* f)
{
	if (!f) {
		return;
	}
	vid_queue_mutex_->lock();
	decoded_video_frames_.enqueue(f);
	vid_queue_mutex_->unlock();
}

AVFrame* MediaInfo::DequeueAudioFrame()
{
	AVFrame* f = nullptr;
	aud_queue_mutex_->lock();
	if (!decoded_audio_frames_.isEmpty()) {
		f = decoded_audio_frames_.dequeue();
	}
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
	vid_queue_mutex_->unlock();
	return f;
}
