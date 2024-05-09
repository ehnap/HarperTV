#pragma once

#include <QString>
#include <QQueue>

class AVFrame;
class QMutex;
class FFmpegDecoder;

class MediaInfo
{
public:
	explicit MediaInfo(const QString& url);
	~MediaInfo();

	QString GetMediaUrl() const;

	void SetDecoder(FFmpegDecoder* d);
	FFmpegDecoder* GetDecoder() const;

	void EnqueueAudioFrame(AVFrame* f);
	AVFrame* DequeueAudioFrame();

	void EnqueueVideoFrame(AVFrame* f);
	AVFrame* DequeueVideoFrame();

private:
	QString url_;
	QMutex* aud_queue_mutex_;
	QMutex* vid_queue_mutex_;
	QQueue<AVFrame*> decoded_audio_frames_;
	QQueue<AVFrame*> decoded_video_frames_;
	FFmpegDecoder* decoder_;
};