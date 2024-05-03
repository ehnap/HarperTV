#include "pub.h"
#include <QHBoxLayout>
#include "harpertv.h"
#include "playwidget.h"


HarperTV::HarperTV(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    pw = new PlayWidget(this);
    mainLayout->addWidget(pw);
}

HarperTV::~HarperTV()
{}

void HarperTV::play()
{
    pw->play();
}
