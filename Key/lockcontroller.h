#ifndef CLOCKCONTROLLER_H
#define CLOCKCONTROLLER_H

#include <QObject>
#include <stdio.h>
#include "serialport.h"
class SerialPort;


#define MAX_PULSE_COUNT                     250
#define MAX_PULSE_ON_TIME                   500     // milliseconds
#define VOLTAGE_DOUBLER_ACTIVATION_DELAY    1000        // milliseconds
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

class CUSBController;
class CLocksStatus;

/**
 * @brief The CLockController class
 * root@safepak-1:/home/pi# ls -al /dev/serial/by-id | grep -Ei '(USB-Serial.*ttyUSB)'
lrwxrwxrwx 1 root root  13 Aug  2 19:18 usb-Prolific_Technology_Inc._USB-Serial_Controller_D-if00-port0 -> ../../ttyUSB0
lrwxrwxrwx 1 root root  13 Aug  3 07:32 usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0 -> ../../ttyUSB2
 */
class CLockController : public QObject
{
    Q_OBJECT


    public:
        explicit CLockController(QObject *parent = 0);

        void initController();
        bool isConnected() 
        {
            return __connectedState == ECONNECTED;
        }

        void openLock(uint16_t nLockNum);
        void openLocks(QString lockNums);
        void openLockWithPulse(uint16_t nLockNum, uint8_t nPulseCount, uint16_t nPulseOnTimeMS, uint16_t nPulseOffTimeMS);
        uint64_t inquireLockStatus(uint8_t unBanks);  // Single bit per Lock up to 64.

        CLocksStatus* getLockStatus() { return _plockStatus ? _plockStatus : 0;}

    signals:
        void __OnLocksStatus(QObject &locksStatus);

    public slots:
        void OnLocksStatusRequest();

    private:
        SerialPort* _pport;

        QString     _sFilterString0 = "USB-Serial.*ttyUSB";
        QString     _sFilterString1 = "USB2.0-Ser.*ttyUSB";
        QString     _sFilterString2 = "usb-FTDI.*ttyUSB";
        QString     _sDeviceType = "serial";
        QString     _sFindDevicePrefix = "ttyUSB";

        bool        _bLockStateRead = false;    // False until read at least once
        uint64_t    _un64LockLocks = 0; // 1 bit for each up to 64

    private: // Object Connections
        CLocksStatus    *_plockStatus = 0;

        uint8_t     ENOT_CONNECTED = 0;
        uint8_t     ECONNECTED = 1;
        uint8_t __connectedState = ENOT_CONNECTED;

    private: // methods

        void setStateConnected() {
            __connectedState = ECONNECTED;
        }

        void setStateDisconnected() {
            __connectedState = ENOT_CONNECTED;
        }

        std::string int_to_bin(uint16_t number);
        std::string to_hex(uint16_t to_convert);

    protected:
        void readLockStateCmd(uint8_t nLockNum);
        std::string readCommandResponse();
};


class CLocksStatus : public QObject
{
    Q_OBJECT

private:
    uint64_t            _un64LockLocks = 0;
    CLockController     *_pLockController = 0;

public:
    explicit CLocksStatus() {}

    void setLockController(CLockController *pLockController) { _pLockController = pLockController; }

    void setLockState(uint64_t locks) { _un64LockLocks = locks; }

    void openLocks(QString Locks) 
    {
        _pLockController->openLocks(Locks);
    }
};


#endif // CLOCKCONTROLLER_H
