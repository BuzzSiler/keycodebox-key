#ifndef CCRYSTALFONTZCONTROLLER_H
#define CCRYSTALFONTZCONTROLLER_H

#include <QObject>
#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/basic_serial_port.hpp>

#include "usbcontroller.h"

using namespace boost;

/**
 * @brief The CCrystalFontzController class
 * ls -al /dev/serial/by-id | grep -Ei '(Crystalfontz.*ttyUSB)'
 * lrwxrwxrwx 1 root root  13 Aug  3 07:27 usb-Crystalfontz_Crystalfontz_CFA634-USB_LCD_CF019704-if00-port0 -> ../../ttyUSB1
 */
class CCrystalFontzController : public QObject
{
    Q_OBJECT

    asio::serial_port *_pport;

    QString     _sFilterString = "Crystalfontz.*ttyUSB";
    QString     _sDeviceType = "serial";
    QString     _sFindDevicePrefix = "ttyUSB";

private: // Object Connections
    CUSBController  *_pusbController;

    uint8_t ENOT_CONNECTED = 0;
    uint8_t ECONNECTED = 1;

    uint8_t __connectedState = ENOT_CONNECTED;

private: // methods
    void initController();

    bool _testWriteToOpenDevice();

    void rebootDisplay();

    void clearDisplay();

    void backlightLevel(uint8_t nlevel);

    void backlightOff();

    void backlightOn();

    void setStateConnected() {
        __connectedState = ECONNECTED;
    }

    void setStateDisconnected() {
        __connectedState = ENOT_CONNECTED;
    }

public:
    explicit CCrystalFontzController(QObject *parent = 0);

    CUSBController& getUSBController() {
        return *_pusbController;
    }

    void setUSBController(CUSBController &usbController) {
        _pusbController = &usbController;
        initController();
    }
    ~CCrystalFontzController();

    void setup();
    void moveCursorTopLeft();
    void hideCursor();

    void showUnderlineCursor();
    void showBlockCursor();
    void contrastLevel(uint8_t nlevel);
    void setCursorPosition(uint8_t column, uint8_t row);
    void writeToDisplay(std::string strToDisplay);

    bool isConnected() {
        return __connectedState == ECONNECTED;
    }

signals:

public slots:
};

#endif // CCRYSTALFONTZCONTROLLER_H
