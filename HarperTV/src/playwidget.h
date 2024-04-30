#pragma once
#include <QWidget>

class SDL_Window;
class SDL_Renderer;

class PlayWidget : public QWidget {
public:
    PlayWidget(QWidget* parent = nullptr);
    ~PlayWidget();

    void play();

protected:
    void resizeEvent(QResizeEvent* e) override;
    QPaintEngine* paintEngine() const override;

private:
    void renderFrame();

private:
    SDL_Window* sdlWindow;  
    SDL_Renderer* sdlRenderer; 
    SDL_Texture* m_texture;
    QMutex m_mutex;
    QWaitCondition m_frameAvailable;
    QQueue<AVFrame*> m_frameQueue;
};