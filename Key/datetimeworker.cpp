#include "datetimeworker.h"

#include <QDebug>
#include <QThread>

DateTimeWorker::DateTimeWorker(QObject *parent) : QObject(parent)
{

}

void DateTimeWorker::onTimeout()
{
    qDebug()<<"DateTimeWorker::onTimeout get called from?: "<<QThread::currentThreadId();
}


