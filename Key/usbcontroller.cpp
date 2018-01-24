
#include <cstdlib>
#include <iostream>

#include "usbcontroller.h"
#include <QList>
#include <QDebug>
#include <QString>
#include <QtCore/qiterator.h>
#include <iterator>
#include <algorithm>

CUSBController::CUSBController()
{
    initController();
}

CUSBController::~CUSBController()
{
    // Finalize the hidapi library
    (void)hid_exit();
}



void CUSBController::initController()
{
    // Initialize the hidapi library
    (void)hid_init();

}


void CUSBController::ExtractCommandOutput(FILE *pF, std::string &rtnStr)
{
    char cChar = '\0';

    while(!feof(pF))
    {
        cChar = fgetc(pF);
        rtnStr += cChar;
    }
}

/**
 * @brief CSerialController::ScanForPorts
 * root@safepak-1:/home/pi# ls -al /dev/serial/by-id | grep -Ei '(usb.*tty)'
 * lrwxrwxrwx 1 root root  13 Aug  3 07:27 usb-Crystalfontz_Crystalfontz_CFA634-USB_LCD_CF019704-if00-port0 -> ../../ttyUSB1
 * lrwxrwxrwx 1 root root  13 Aug  2 19:18 usb-Prolific_Technology_Inc._USB-Serial_Controller_D-if00-port0 -> ../../ttyUSB0
 * lrwxrwxrwx 1 root root  13 Aug  3 07:32 usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0 -> ../../ttyUSB2
 *
 * USB to Serial - used to connect to LockController
 * find USB-Serial for USB to serial converter
 *  - if more than one then request user input (in admin-setup mode)
 * root@safepak-1:/home/pi# ls -al /dev/serial/by-id | grep -Ei '(USB-Serial.*ttyUSB)'
 * lrwxrwxrwx 1 root root  13 Aug  2 19:18 usb-Prolific_Technology_Inc._USB-Serial_Controller_D-if00-port0 -> ../../ttyUSB0
 * lrwxrwxrwx 1 root root  13 Aug  3 07:32 usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0 -> ../../ttyUSB2
 *
 * find Crystalfontz for display
 ** root@safepak-1:/home/pi# ls -al /dev/serial/by-id | grep -Ei '(Crystalfontz.*ttyUSB)'
 * lrwxrwxrwx 1 root root  13 Aug  3 07:27 usb-Crystalfontz_Crystalfontz_CFA634-USB_LCD_CF019704-if00-port0 -> ../../ttyUSB1
 *
 * Note: also find USB stick by
 * ls -al /dev/disk/by-id
 *
 * example:
 * ls -al /dev/disk/by-id | grep -Ei '(USB_DISK.*sda)'
 * lrwxrwxrwx 1 root root   9 Aug  3 07:42 usb-_USB_DISK_Pro_070B2C5442A9F137-0:0 -> ../../sda
 * lrwxrwxrwx 1 root root  10 Aug  3 07:42 usb-_USB_DISK_Pro_070B2C5442A9F137-0:0-part1 -> ../../sda1
 */



std::string CUSBController::getDevicePortString(QString deviceType, QString filterString, QString findDeviceString)
{
    return CUSBController::getDevicePortString(deviceType, filterString, findDeviceString, 1);
}


std::string CUSBController::getDevicePortString(QString deviceType, QString filterString, QString findDeviceString, int nFindPos)
{
    std::string     aCmd;
    FILE *pF;

    Q_UNUSED(nFindPos);   
 
    aCmd = "ls -al /dev/";
    aCmd += deviceType.toStdString();
    aCmd += "/by-id/ | grep -iE '(";
    aCmd += filterString.toStdString();
    aCmd += ")'";


    std::cout << "getDevicePortString(...): aCmd:" << aCmd << "\n";
    
    pF = popen(aCmd.c_str(), "r");
    if(!pF)
    {
        std::cout << "\tfailed to popen(aCmd, 'r');\n";

        return "";
    }
    std::string sOutput = "";

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);
    
    std::cout << "\textracted output:" << sOutput.c_str() << "\n";

    int devicePos = sOutput.find("/" + findDeviceString.toStdString());
    if( devicePos > -1 )
      devicePos += 1;
    
    bool foundPort = false;
    std::string sPort;

    int deviceLength = findDeviceString.toStdString().length();

    std::cout << "findDeviceString: " << findDeviceString.toStdString();
    std::cout << "\tsOutput: " << sOutput;
    std::cout << "\tdevicePos: " << devicePos;
    std::cout << "\tdeviceLength: " << deviceLength;
    std::cout << "\tx: " << devicePos+deviceLength-1;
    std::cout << "\ty: " << devicePos+deviceLength;
    
    
    //sPort = sOutput.substr(devicePos+deviceLength, devicePos+deviceLength);
    //std::cout << "\tport:" << sPort;
    if(devicePos > -1)
      {
	sPort = sOutput[devicePos+deviceLength];
	std::cout << "\tport:" << sPort;
	foundPort = true;
      }
    
    if(!foundPort)
      {
	std::cout << "\tnot found! returning/n";
	return "";
      }
    
    sPort += "\0x00";

    return sPort;

}

asio::serial_port *CUSBController::findPortByPortName(std::string portName)
{
    tyitorMap   it;

    it = mapPorts.find(portName);
    if(it != mapPorts.end()) {
        return it->second->_pport;
    } else {
        return 0;
    }
}


asio::serial_port *CUSBController::createPort( std::string portName, std::string sDevice,
        asio::serial_port_base::baud_rate baud, asio::serial_port_base::flow_control flow,
        asio::serial_port_base::parity parity, asio::serial_port_base::stop_bits stops,
        asio::serial_port_base::character_size charSize)
{
    pstPort pport = new stPort();
    pport->_pport = new asio::serial_port( pport->_io );
    boost::system::error_code ec;
    pport->_pport->open(sDevice, ec);

    pport->_pport->set_option( baud );
    pport->_pport->set_option( flow );
    pport->_pport->set_option( parity );
    pport->_pport->set_option( stops );
    pport->_pport->set_option( charSize );

    // Raw Error Handling - remove
    if ( ec )
    {
        std::cout << "error : port_->open() failed...com_port_name=" << sDevice.c_str() << ", e=" << ec.message().c_str() << std::endl;
    }
    else {
        std::cout << "\t\tPRODUCTION: open\n";
    }

    if( pport ) {
        mapPorts.insert(std::make_pair(portName, pport));
//        mapPorts.emplace(portName, pport);
        return pport->_pport;
    }
    else {
        return 0;
    }
}

asio::serial_port *CUSBController::getPortForDevice(
        std::string portName,
        std::string sDevice,
        asio::serial_port_base::baud_rate baud,
        asio::serial_port_base::flow_control flow,
        asio::serial_port_base::parity parity,
        asio::serial_port_base::stop_bits stops,
        asio::serial_port_base::character_size charSize )
{
    asio::serial_port *port = findPortByPortName(portName);

    if(port == 0 ) {
        // Create Port
        port = createPort(portName, sDevice, baud, flow, parity, stops, charSize);
    }

    return port;
}


int CUSBController::CountDevicePorts(QString deviceType, QString filterString)
{
    //
    FILE        *pF;
    int         nCount = 0;
    std::string aCmd;

    aCmd = "ls -al /dev/";
    aCmd += deviceType.toStdString();
    aCmd += "/by-id/ | grep -iE '(";
    aCmd += filterString.toStdString();
    aCmd += ")'";

    pF = popen(aCmd.c_str(), "r");
    if(!pF)
    {
        return -1;
    }

    char cChar = '\0';
    while(!feof(pF))
    {
        cChar = fgetc(pF);
        if(cChar == '\n')
        {
            nCount++;
        }
    }

    fclose(pF);
    
    return nCount;
}
