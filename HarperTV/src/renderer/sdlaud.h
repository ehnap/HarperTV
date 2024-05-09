#pragma once

#include "../decoder/streamparameters.h"
#include <QObject>

class FFmpegDecoder;
class AudioRenderThread;
class AVFrame;
class MediaInfo;

class SdlAudRenderer : public QObject
{
	Q_OBJECT
public:
	explicit SdlAudRenderer(MediaInfo* info);
	~SdlAudRenderer();
	
	void Init(AudStreamParameters params);
	void RenderFrame(AVFrame* avf, FFmpegDecoder* d);

private slots:
	void Render();

private:
	MediaInfo* media_info_;
	AudioRenderThread* thread_;
};