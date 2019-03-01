#include "timefunctions.h"

#include <QTime>
#include <QCoreApplication>



void delaySec(int seconds)
{
    QTime dieTime= QTime::currentTime().addSecs(seconds);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void delayMili(int mili)
{
    mili = mili;
    if (mili < 1)
        mili = 1;
    QTime dieTime= QTime::currentTime().addMSecs(mili);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
