#pragma once

#include "../decoder/streamparameters.h"

class FFmpegDecoder;

class SdlAudRenderer
{
public:
	explicit SdlAudRenderer();
	~SdlAudRenderer();
	
	void Init(AudStreamParameters params);
	void RenderFrame(AVFrame* avf, FFmpegDecoder* d);

};