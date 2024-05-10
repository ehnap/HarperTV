#include "pub.h"
#include "harpertv.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<PAVFrame>("PAVFrame");//зЂВс
    HarperTV w;
    w.resize(800, 600);
    w.show();
    w.play();

    return a.exec();
}
