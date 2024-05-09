#pragma once
#include <QThread>

class MediaInfo;

class AudioRenderThread : public QThread
{
public:
	explicit AudioRenderThread(MediaInfo* info);
	~AudioRenderThread();

signals:
	void render();

protected:
	void run() override;

private:
	MediaInfo* media_info_;
};