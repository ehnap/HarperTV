#pragma once

#include <QString>
#include <QQueue>

class AVFrame;
class QMutex;

class MediaInfo
{
public:
	explicit MediaInfo(const QString& url);
	~MediaInfo();

	void EnqueueAudioFrame(AVFrame* f);
	void EnqueueVideoFrame(AVFrame* f);

	AVFrame* DequeueAudioFrame();
	AVFrame* DequeueVideoFrame();

private:
	QString url_;
	QMutex* aud_queue_mutex_;
	QMutex* vid_queue_mutex_;
	QQueue<AVFrame*> decoded_audio_frames_;
	QQueue<AVFrame*> decoded_video_frames_;
};