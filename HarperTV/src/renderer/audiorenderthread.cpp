#include "audiorenderthread.h"
#include "../mediainfo.h"

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
		auto f = media_info_->DequeueAudioFrame();
		if (!f) {
			msleep(10);
			continue;
		}
		emit render(f);
		msleep(47);
	}
}
