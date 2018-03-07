#ifndef CHIDREADER_H
#define CHIDREADER_H

#include <QObject>
//#include <usb.h>

#include <sys/ioctl.h>
#include <strings.h>
#include <iostream>
#include <libusb.h>
#include <hidapi/hidapi.h>

#include "usbcontroller.h"

#define MAX_STR 255

#define VID2    0x076B
#define PID2    0x5427

class CHWKeyboardReader : public QObject
{
    Q_OBJECT
    hid_device *_handle;

    uint16_t    _unVID, _unPID;

private:
    bool openDeviceHandle();
    void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
    bool floatXInputDevice();
    
    void setVID_PID(uint16_t unVID, uint16_t unPID) { _unVID = unVID; _unPID = unPID; }

    unsigned char convertHIDKeyboardChar(unsigned char byIn);

    QString convertHexStringToNumString(QString inHex);
    bool closeDeviceHandle();
public:
    explicit CHWKeyboardReader(QObject *parent = 0);
    ~CHWKeyboardReader();

    bool initHIDReader(uint16_t unVID, uint16_t unPID);

    QString readHIDReader();
    void readHIDReaderLoop();
    int parseHIDReaderCodes(QString sCardData, QString *pcode1, QString *pcode2);

    void testOne();

    void TestTwo();

signals:
    void __onHIDSwipeCodes(QString sCode1, QString sCode2);
    void _newHIDData(QString newData);

public slots:
    void start();
};

#endif // CHIDReader_H
