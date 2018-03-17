#ifndef USBPROVIDER_H
#define USBPROVIDER_H

#include <string>

/* The purpose of this module is to wrap the usbcontroller module.  Presently, the usbcontroller module is instance-based
   and is passed around to all of the modules that need access to USB devices.  The usbprovider module will be implemented
   as a class with static methods and temporarily the usbcontroller will become a static member variable of the usbprovider.
*/

class CUSBController;
class SerialPort;

class UsbProvider final
{
    public:
        static void Initialize();
        static SerialPort* GetLockControllerDevice();
        static std::string GetMagTekDevicePortString();
        
    private:
        UsbProvider() {}
        static CUSBController* m_ctrl;
        static SerialPort* m_serialport;

        static std::string GetLockControllerDeviceString();

        explicit UsbProvider(const UsbProvider& rhs) = delete;
        UsbProvider& operator= (const UsbProvider& rhs) = delete;
        
};


#endif