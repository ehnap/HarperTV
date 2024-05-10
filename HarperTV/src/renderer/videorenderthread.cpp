#include "videorenderthread.h"
#include "../mediainfo.h"

VideoRenderThread::VideoRenderThread(MediaInfo* info)
	: QThread(nullptr)
	, media_info_(info)
{
}

VideoRenderThread::~VideoRenderThread()
{
}

void VideoRenderThread::run()
{
	while (1) {
		auto f = media_info_->DequeueVideoFrame();
		if (!f) {
			msleep(10);
			continue;
		}
		emit render(f);
		msleep(33);
	}
}
