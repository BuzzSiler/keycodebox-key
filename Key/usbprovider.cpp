#include "usbprovider.h"
#include "usbcontroller.h"
#include <QDebug>
#include "serialport.h"

// Lock Controller Strings
static QString     _sFilterString0 = "USB-Serial.*ttyUSB";
static QString     _sFilterString1 = "USB2.0-Ser.*ttyUSB";
static QString     _sFilterString2 = "usb-FTDI.*ttyUSB";
static QString     _sDeviceType = "serial";
static QString     _sFindDevicePrefix = "ttyUSB";

// MagTek Controller Strings
static QString filterString = "usb-c216.*event";
static QString deviceType = "input";
static QString devicePrefix = "event";


CUSBController* UsbProvider::m_ctrl = nullptr;
SerialPort* UsbProvider::m_serialport = nullptr;

void UsbProvider::Initialize()
{
    if (m_ctrl == nullptr)
    {
        m_ctrl = new CUSBController;
    }
}

std::string UsbProvider::GetLockControllerDeviceString()
{
    qDebug() << "searching for USB->serial adapter version 1...";
    // Check for more than one Serial Cable -- we can't tell them apart at the USB-Serial level.
    int nCountDevs = m_ctrl->CountDevicePorts(_sDeviceType, _sFilterString0);
    if ( nCountDevs > 1 ) 
    {
        // Can't continue
        qDebug() << "More than one USB->serial adapter found. Will need to reconcile.\n";
        return nullptr;
    }

    std::string sDevNum = m_ctrl->getDevicePortString(_sDeviceType, _sFilterString0, _sFindDevicePrefix);

    qDebug() << "initController() sDevNum:" << sDevNum.c_str() << "\t";

    // check for the alternative usb-serial converter type
    if(sDevNum.empty())
    {
        qDebug() << "USB->serial adapter version 1 not found, searching for version 2...";
        nCountDevs = m_ctrl->CountDevicePorts(_sDeviceType, _sFilterString1);
        if ( nCountDevs > 1 ) 
        {
            // Can't continue
            qDebug() << "More than one USB->serial adapter found. Will need to reconcile.\n";
            return nullptr;
        }
        sDevNum = m_ctrl->getDevicePortString(_sDeviceType, _sFilterString1, _sFindDevicePrefix);
        qDebug() << "initController() sDevNum:" << sDevNum.c_str() << "\t";
    }

    // check for the alternative FTDI usb-serial converter type
    if(sDevNum.empty())
    {
        qDebug() << "USB->serial adapter version 2 not found, searching for version 3...";
        nCountDevs = m_ctrl->CountDevicePorts(_sDeviceType, _sFilterString2);
        if ( nCountDevs > 1 ) 
        {
            // Can't continue
            qDebug() << "More than one USB->serial adapter found. Will need to reconcile.\n";
            return nullptr;
        }
        sDevNum = m_ctrl->getDevicePortString(_sDeviceType, _sFilterString2, _sFindDevicePrefix);
        qDebug() << "initController() sDevNum:" << sDevNum.c_str() << "\t";
    }

    if(!sDevNum.empty())
    {
        std::string sDevice = "/dev/ttyUSB";
        sDevice += sDevNum;

        qDebug() << "\tsDevice:" << sDevice.c_str();

        return sDevice;
    }

    return std::string();
}

SerialPort* UsbProvider::GetLockControllerDevice()
{    
    if (m_serialport == nullptr)
    {
        std::string sDevice = GetLockControllerDeviceString();

        if(!sDevice.empty())
        {
            m_serialport = new SerialPort(QString::fromStdString(sDevice), 19200, 10, 10);
        }
    }

    return m_serialport;
}


std::string UsbProvider::GetMagTekDevicePortString()
{
    int nCountDevs = m_ctrl->CountDevicePorts(deviceType, filterString);
    if ( nCountDevs > 1 ) 
    {
        // Can't continue
        qDebug() << "CMagTekCardReader::openDeviceHandle(), More than one hid magnetic card reader usb-c216 found. Will need to reconcile.\n";
        return "";
    }
  
    return m_ctrl->getDevicePortString(deviceType, filterString, devicePrefix);
}
