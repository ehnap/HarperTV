#include "audiorenderthread.h"

AudioRenderThread::AudioRenderThread(MediaInfo* info)
	: QThread(nullptr)
	, media_info_(info)
{

}

AudioRenderThread::~AudioRenderThread()
{
}

void AudioRenderThread::run()
{
	while (1) {
		emit render();
	}
}
