#ifndef CSYSTEMCONTROLLERTHREAD_H
#define CSYSTEMCONTROLLERTHREAD_H

#include <QThread>
#include "systemcontroller.h"


class CSystemControllerThread : public QThread
{
    Q_OBJECT
public:
    CSystemControllerThread(QObject *parent = 0);

private:
    CSystemController   *_psystemControllerObj;
    QTimer              *_ptimer;

    // QThread interface
protected:
//    void run();

public:
    void setSystemController(CSystemController *obj) {
        _psystemControllerObj = obj;
    }
};


#endif // CSYSTEMCONTROLLERTHREAD_H
