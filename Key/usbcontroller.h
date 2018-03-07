#ifndef CSERIALCONTROLLER_H
#define CSERIALCONTROLLER_H

#include <map>
#include <QtCore>
#include <hidapi/hidapi.h>

/**
 * @brief The CUSBController class
 *   Determines USB attached hardware.
 *   Determines the type of device attached to each port.
 *     - signals admin-setup request for input if serial ports undetermined.
 */
class CUSBController : public QObject
{
        Q_OBJECT

    public:
        CUSBController();
        ~CUSBController();

        int CountDevicePorts(QString deviceType, QString filterString);
        std::string getDevicePortString(QString deviceType, QString filterString, QString findDeviceString);
        std::string getDevicePortString(QString deviceType, QString filterString, QString findDeviceString, int nFindPos);

    signals:
        void __SerialPortChanged();
        void __USBDiskChanged();

    private:
        void initController();
        void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
    
};


#endif // CSERIALCONTROLLER_H
