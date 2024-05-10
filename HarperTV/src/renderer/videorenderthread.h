#pragma once
#include "../pub.h"
#include <QThread>

class MediaInfo;
class VideoRenderThread : public QThread
{
	Q_OBJECT
public:
	explicit VideoRenderThread(MediaInfo* info);
	~VideoRenderThread();

signals:
	void render(PAVFrame f);

protected:
	void run() override;

private:
	MediaInfo* media_info_;
};