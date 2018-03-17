#include "usbhwkeypad.h"

CUSBHWKeypad::CUSBHWKeypad(QObject *parent) : 
    QObject(parent),
    _handle(0)
{

}

CUSBHWKeypad::~CUSBHWKeypad()
{
    if(_handle) {
        hid_close(_handle);
    }
}

void CUSBHWKeypad::initHWKeyboard()
{
    openDeviceHandle();
}

std::string CUSBHWKeypad::readHWKeyboard()
{
    int res;
    std::string strRead = "";
    unsigned char buf[65];
    int i;

    res = hid_read_timeout(_handle, buf, 65, -1);  // blocking wait
    for (i = 0; i < res; i++)
        strRead += buf[i];
    // Read requested state. hid_read() has been set to be
    // non-blocking by the call to hid_set_nonblocking() above.
    // This loop demonstrates the non-blocking nature of hid_read().
    res = 0;
    while (res == 0) {
        res = hid_read(_handle, buf, sizeof(buf));
        if (res < 0)
        {
            //std::cout << "Unable to read()\n";
        }
        usleep(500*1000);  // 500 ms
    }

    // Print out the returned buffer.
    for (i = 0; i < res; i++)
        strRead += buf[i];

    return strRead;

}

void CUSBHWKeypad::readHWKeyboardLoop()
{
    std::string sKeyboardData;
    std::string key;
    int         nRC;

    while(1)    // Or on flag
    {
        try {
            sKeyboardData = readHWKeyboard();

            std::cout << sKeyboardData << "\n";

            nRC = parseKeyboardCodes(sKeyboardData, &key);
            if(nRC > 0) {
                // Succeeded in getting at least one code
            }
        } catch (const std::runtime_error &e)
        {
            std::cout << "inquireDoorStatus() runtime error:\t" << e.what();
        }
    }
}



void CUSBHWKeypad::openDeviceHandle()
{
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    _handle = hid_open(VID, PID, NULL);

    if(_handle) {
        std::cout << "hid_opened!\n";
    } else {
        std::cout << "failed to open hid\n";
    }
}

int CUSBHWKeypad::parseKeyboardCodes(std::string sKeyboardData, std::string *pkey)
{
    Q_UNUSED(sKeyboardData);
    Q_UNUSED(pkey);
    return 0;
}
