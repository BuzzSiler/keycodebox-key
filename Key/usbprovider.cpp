#include "usbprovider.h"

#include <QDebug>

#include "usbcontroller.h"
#include "serialport.h"
#include "logger.h"

#define SP_BAUD_RATE (19200)
#define SP_WRITE_TIMEOUT (500)
#define SP_READ_TIMEOUT (750)

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
    int nCountDevs = m_ctrl->CountDevicePorts(_sDeviceType, _sFilterString0);
    if ( nCountDevs > 1 ) 
    {
        KCB_DEBUG_TRACE("More than one USB->serial adapter found. Will need to reconcile.");
        return nullptr;
    }

    std::string sDevNum = m_ctrl->getDevicePortString(_sDeviceType, _sFilterString0, _sFindDevicePrefix);

    // check for the alternative usb-serial converter type
    if(sDevNum.empty())
    {
        nCountDevs = m_ctrl->CountDevicePorts(_sDeviceType, _sFilterString1);
        if ( nCountDevs > 1 ) 
        {
            KCB_DEBUG_TRACE("More than one USB->serial adapter found. Will need to reconcile.");
            return nullptr;
        }
        sDevNum = m_ctrl->getDevicePortString(_sDeviceType, _sFilterString1, _sFindDevicePrefix);
    }

    if(sDevNum.empty())
    {
        nCountDevs = m_ctrl->CountDevicePorts(_sDeviceType, _sFilterString2);
        if ( nCountDevs > 1 ) 
        {
            KCB_DEBUG_TRACE("More than one USB->serial adapter found. Will need to reconcile.");
            return nullptr;
        }
        sDevNum = m_ctrl->getDevicePortString(_sDeviceType, _sFilterString2, _sFindDevicePrefix);
    }

    if(!sDevNum.empty())
    {
        std::string sDevice = "/dev/ttyUSB";
        sDevice += sDevNum;
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
            m_serialport = new SerialPort(QString::fromStdString(sDevice), SP_BAUD_RATE, SP_WRITE_TIMEOUT, SP_READ_TIMEOUT);
        }
    }

    return m_serialport;
}


std::string UsbProvider::GetMagTekDevicePortString()
{
    int nCountDevs = m_ctrl->CountDevicePorts(deviceType, filterString);
    if ( nCountDevs > 1 ) 
    {
        KCB_DEBUG_TRACE("More than one hid magnetic card reader usb-c216 found. Will need to reconcile.");
        return "";
    }
  
    return m_ctrl->getDevicePortString(deviceType, filterString, devicePrefix);
}
