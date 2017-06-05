#ifndef CMAGTEKCARDREADER_H
#define CMAGTEKCARDREADER_H

#include <QObject>
#include <usb.h>

#include <sys/ioctl.h>
#include <strings.h>
#include <iostream>
#include <libusb.h>
#include <hidapi/hidapi.h>

#include <stdint.h>

#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#ifndef EV_SYN
#define EV_SYN 0
#endif

#include "usbcontroller.h"

#define MAX_STR 255

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

class CMagTekCardReader : public QObject
{
    Q_OBJECT
    CUSBController  *_usbController;
    hid_device *_handle;

    int fileDescriptor, readDescriptor;
    struct input_event ev[64];
    int version;
    unsigned short id[4];
    char name[256];
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
    int abs[5];

private:
    bool openDeviceHandle();
    void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
    bool floatXInputDevice();
    int codeToInteger(int x);
    bool isStartCode(int x);
    
public:
    explicit CMagTekCardReader(QObject *parent = 0);
    ~CMagTekCardReader();
    bool initMagTekCardReader();
    void loop();

signals:
    void __onCardSwipe(QString sCode1, QString sCode2);
    void _newCardData(QString newData);

public slots:
    void start();
};

#endif // CMAGTEKCARDREADER_H
