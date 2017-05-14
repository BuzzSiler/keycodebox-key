#ifndef CLCDGRAPHICSCONTROLLER_H
#define CLCDGRAPHICSCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QTimerEvent>

class CLCDGraphicsController : public QObject
{
    Q_OBJECT

private:
    QTimer      _timerDim;

    void startScreenDimTimer();     // Default 30 seconds

public:
    explicit CLCDGraphicsController(QObject *parent = 0);

    void setBrightness(unsigned int unValue); // 0 -> 255
    void turnBacklightOn();
    void turnBacklightOff();

    bool isLCDAttached();

signals:

public slots:

    void dimScreenMid();
    void dimScreenLow();

    void dimScreenOff();    // Max
};

#endif // CLCDGRAPHICSCONTROLLER_H
