#pragma once
#include <QThread>

class MediaInfo;
class VideoRenderThread : public QThread
{
public:
	explicit VideoRenderThread(MediaInfo* info);
	~VideoRenderThread();

signals:
	void render();

protected:
	void run() override;

private:
	MediaInfo* media_info_;
};