#include "stdafx.h"
#include "harpertv.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HarperTV w;
    w.show();
    return a.exec();
}
