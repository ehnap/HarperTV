#define SDL_MAIN_HANDLED
#include <SDL.h>
#undef main
#include <QtWidgets>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

typedef AVFrame* PAVFrame;
