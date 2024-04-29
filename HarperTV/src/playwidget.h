#pragma once
#include <QWidget>

class SDL_Window;
class SDL_Renderer;

class PlayWidget : public QWidget {
public:
    PlayWidget(QWidget* parent = nullptr);
    ~PlayWidget();

private:
    void renderFrame();

private:
    SDL_Window* sdlWindow;  
    SDL_Renderer* sdlRenderer; 
};