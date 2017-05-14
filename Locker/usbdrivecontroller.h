#ifndef CUSBDRIVECONTROLLER_H
#define CUSBDRIVECONTROLLER_H

#include <QObject>

/**
 * @brief The CUSBDriveController class
 *
 *  * Note: also find USB stick by
 * ls -al /dev/disk/by-id
 *
 * example:
 * ls -al /dev/disk/by-id | grep -Ei '(USB_DISK.*sda)'
 * lrwxrwxrwx 1 root root   9 Aug  3 07:42 usb-_USB_DISK_Pro_070B2C5442A9F137-0:0 -> ../../sda
 * lrwxrwxrwx 1 root root  10 Aug  3 07:42 usb-_USB_DISK_Pro_070B2C5442A9F137-0:0-part1 -> ../../sda1
 */
class CUSBDriveController : public QObject
{
    Q_OBJECT

    QString     _sFilterString = "USB_DISK.*sda";
    QString     _sDeviceType = "disk";
    QString     _sFindDeviceString = "sda";

    void initController();

public:
    explicit CUSBDriveController(QObject *parent = 0);

signals:

public slots:

};


#endif // CUSBDRIVECONTROLLER_H
