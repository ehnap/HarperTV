#include "stdafx.h"
#include "harpertv.h"
#include "decodethread.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HarperTV w;
    w.resize(800, 600);
    w.show();

    w.play();

    return a.exec();
}
