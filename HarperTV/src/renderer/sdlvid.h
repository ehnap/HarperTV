#pragma once

#include <QObject>

class AVFrame;
class SDL_Window;
class SDL_Renderer;
class SDL_Texture;
class MediaInfo;
class VideoRenderThread;

class SdlVidRenderer : public QObject
{
	Q_OBJECT
public:
	explicit SdlVidRenderer(MediaInfo* info);
	~SdlVidRenderer();

	void Init(SDL_Window* w);
	void RenderFrame(AVFrame* avf);

private slots:
	void Render();

private:
	SDL_Renderer* renderer_;
	SDL_Texture* texture_;
	MediaInfo* media_info_;
	VideoRenderThread* thread_ = nullptr;
};
