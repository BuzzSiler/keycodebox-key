#ifndef CFINGERPRINTREADER_H
#define CFINGERPRINTREADER_H

#include <QObject>
#include <usb.h>

#include <sys/ioctl.h>
#include <string.h>
#include <string>
#include <iostream>
#include <libusb.h>

#include "usbcontroller.h"

//static void dev_open_cb(struct fp_dev *dev, int status, void *user_data);

class CFingerprintReader : public QObject
{
    Q_OBJECT
    CUSBController  *_usbController;

private:
    bool openDeviceHandle();
    static void enrollStageCb(struct fp_dev *dev, int result,
				struct fp_print_data *print, struct fp_img *img, void *user_data);
    static void enrollCancelCb(struct fp_dev *dev, void *user_data);
    static void identifyCb(struct fp_dev *dev, int result, size_t match_offset, struct fp_img *img, void *user_data);
    static void verifyCancelCb(struct fp_dev *dev, void *user_data);
public:
    explicit CFingerprintReader(QObject *parent = 0);
    ~CFingerprintReader();

    bool initFingerprintReader();
    bool initEnrollment(QString sCode);
    bool cancelEnrollment();
    void resetEnrollmentStage();
    
    bool initVerify();
    bool loadEnrollData(QString sCode);
    const char *buildSavePath(QString sCode);
    bool cancelVerify();
    
signals:
    void __onFingerprintStageComplete(int, int, QString);
    void __onVerifyFingerprintComplete(bool, QString);
    void __onIdentifiedFingerprint(QString sCode, QString sCode2);
    
    public slots:
    void start();
    void handleEvents();
};

#endif // CFINGERPRINTREADER_H
