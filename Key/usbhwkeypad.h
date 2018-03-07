#ifndef CUSBHWKEYPAD_H
#define CUSBHWKEYPAD_H

#include <QObject>
#include <usb.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <iostream>
#include <hidapi/hidapi.h>

#include "usbcontroller.h"

#define MAX_STR 255
#define VID     0x04d8
#define PID     0xffd5

class CUSBHWKeypad : public QObject
{
    Q_OBJECT

    hid_device *_handle;

private:
    void openDeviceHandle();
    int parseKeyboardCodes(std::string sKeyboardData, std::string *pkey);

public:
    explicit CUSBHWKeypad(QObject *parent = 0);
    ~CUSBHWKeypad();

    void initHWKeyboard();
    std::string readHWKeyboard();
    void readHWKeyboardLoop();

signals:
    void __signalHWKeypadKeypress(char cKeyValue);

public slots:

};


#endif // CUSBHWKEYPAD_H
