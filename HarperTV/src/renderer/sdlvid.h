#pragma once

class AVFrame;
class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class SdlVidRenderer
{
public:
	explicit SdlVidRenderer();
	~SdlVidRenderer();

	void Init(SDL_Window* w);
	void RenderFrame(AVFrame* avf);

private:
	SDL_Renderer* renderer_;
	SDL_Texture* texture_;
};
