#ifndef CLOCKCONTROLLER_H
#define CLOCKCONTROLLER_H

#include <stdio.h>

#include <QObject>
#include <QString>

#include "serialport.h"

#define MAX_PULSE_COUNT                     250
#define MAX_PULSE_ON_TIME                   500         // milliseconds
#define VOLTAGE_DOUBLER_ACTIVATION_DELAY    1000        // milliseconds
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

class CUSBController;

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

        void detectHardware();
        void setLockRanges();

    signals:
        void __OnLocksStatus(QObject &locksStatus);
        void DiscoverHardwareProgressUpdate(int);

    private:
        SerialPort* _pport;

        QString     _sFilterString0 = "USB-Serial.*ttyUSB";
        QString     _sFilterString1 = "USB2.0-Ser.*ttyUSB";
        QString     _sFilterString2 = "usb-FTDI.*ttyUSB";
        QString     _sDeviceType = "serial";
        QString     _sFindDevicePrefix = "ttyUSB";

        bool        _bLockStateRead = false;    // False until read at least once
        uint64_t    _un64LockLocks = 0; // 1 bit for each up to 64


        uint8_t     ENOT_CONNECTED = 0;
        uint8_t     ECONNECTED = 1;
        uint8_t __connectedState = ENOT_CONNECTED;
        int update_status = 0;

        void setStateConnected() {
            __connectedState = ECONNECTED;
        }

        void setStateDisconnected() {
            __connectedState = ENOT_CONNECTED;
        }

        std::string int_to_bin(uint16_t number);
        std::string to_hex(uint16_t to_convert);

        uint16_t ReadSoftwareVersion(uint16_t addr);
        QString SoftwareVersionToString(uint16_t version);
        void inquireLockStatus();

    protected:
        enum class RESPONSE { OPEN_LOCK, READ_EEPROM };

        void readLockStateCmd(uint8_t nLockNum);
        std::string readCommandResponse();

        QByteArray SendCommand(QByteArray &cmd);
        uint8_t CalcChecksum(QByteArray const &cmd);
        void SetChecksum(QByteArray &cmd);
        uint16_t ReadEeprom(uint16_t addr, uint16_t offset);
        void SetSequenceNumber(QByteArray &msg);
        uint8_t GetSequenceNumber(QByteArray const &msg);
        QByteArray CreateReadEepromCommand(uint16_t addr, uint16_t offset);
        QByteArray CreateOpenLockCommand(uint16_t addr);
        bool ValidateResponse(RESPONSE type, QByteArray const &command, QByteArray const &response);
        uint16_t GetEepromResponseValue(RESPONSE type, QByteArray const &response);
        bool IsResponse(uint8_t control);
        void ReadBoardEeprom(uint16_t addr);
        QByteArray CreateSearchNetworkCommand(uint16_t addr);
        void LocateMaster();
        uint16_t SearchNetwork(uint16_t addr);
        bool IsErrorResponse(QByteArray const &response);
        void ProcessErrorResponse(QByteArray const &response);
        void SetBoardLockStartStop(uint16_t addr, uint8_t start, uint8_t stop);
        void UpdateDetectProgress();
};

#endif // CLOCKCONTROLLER_H
