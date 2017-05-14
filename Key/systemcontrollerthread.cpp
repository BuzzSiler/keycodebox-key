#include <QDebug>
#include <QCoreApplication>
#include "systemcontrollerthread.h"

CSystemControllerThread::CSystemControllerThread(QObject *parent) : QThread(parent)
{

}

//void CSystemControllerThread::run()
//{
////    if(!_psystemControllerObj)
////    {
////        qDebug() << "System Controller Object has not been set.";
////        return;
////    }

//    qDebug() << "System Controller Thread started.";

//    while(1) {
//        QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
//        exec();
//    }
//}

