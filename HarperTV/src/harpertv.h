#pragma once

#include <QtWidgets/QWidget>

class PlayWidget;

class HarperTV : public QWidget
{
    Q_OBJECT

public:
    HarperTV(QWidget *parent = nullptr);
    ~HarperTV();

    void play();

private:
    PlayWidget* pw;
};
