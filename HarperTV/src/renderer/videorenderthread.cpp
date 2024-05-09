#include "videorenderthread.h"

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

	}
}
