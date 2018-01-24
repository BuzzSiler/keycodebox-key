#include <bitset>
#include <unistd.h>
#include <string>
#include <boost/format.hpp>
#include "lockcontroller.h"
#include "usbcontroller.h"
#include "blockingreader.h"


CLockController::CLockController(QObject *parent) : QObject(parent)
{

}

void CLockController::initController()
{
    _sFilterString0 = "USB-Serial.*ttyUSB";
    _sFilterString1 = "USB2.0-Ser.*ttyUSB";
    _sFilterString2 = "usb-FTDI.*ttyUSB";
    _sDeviceType = "serial";
    _sFindDevicePrefix = "ttyUSB";

    if( !_plockStatus )
    {
        _plockStatus = new CLocksStatus();
    }

    _plockStatus->setLockController(this);

    //TODO: Look up the saved setting for USB cables

    qDebug() << "searching for USB->serial adapter version 1...";
    // Check for more than one Serial Cable -- we can't tell them apart at the USB-Serial level.
    int nCountDevs = _pusbController->CountDevicePorts(_sDeviceType, _sFilterString0);
    if( nCountDevs > 1 ) {
        // Can't continue
        std::cout << "More than one USB->serial adapter found. Will need to reconcile.\n";
        return;
    }

    std::string sDevNum = _pusbController->getDevicePortString(_sDeviceType, _sFilterString0, _sFindDevicePrefix);

    std::cout << "initController() sDevNum:" << sDevNum.c_str() << "\t";

    // check for the alternative usb-serial converter type
    if(sDevNum.empty())
      {
	qDebug() << "USB->serial adapter version 1 not found, searching for version 2...";
	nCountDevs = _pusbController->CountDevicePorts(_sDeviceType, _sFilterString1);
	if( nCountDevs > 1 ) {
	  // Can't continue
	  std::cout << "More than one USB->serial adapter found. Will need to reconcile.\n";
	  return;
	}
	sDevNum = _pusbController->getDevicePortString(_sDeviceType, _sFilterString1, _sFindDevicePrefix);
	std::cout << "initController() sDevNum:" << sDevNum.c_str() << "\t";
      }

    // check for the alternative FTDI usb-serial converter type
    if(sDevNum.empty())
      {
	qDebug() << "USB->serial adapter version 2 not found, searching for version 3...";
	nCountDevs = _pusbController->CountDevicePorts(_sDeviceType, _sFilterString2);
	if( nCountDevs > 1 ) {
	  // Can't continue
	  std::cout << "More than one USB->serial adapter found. Will need to reconcile.\n";
	  return;
	}
	sDevNum = _pusbController->getDevicePortString(_sDeviceType, _sFilterString2, _sFindDevicePrefix);
	std::cout << "initController() sDevNum:" << sDevNum.c_str() << "\t";
      }
 
    if(!sDevNum.empty())
    {
        std::string sDevice = "/dev/ttyUSB";
        sDevice += sDevNum;

        std::cout << "\tsDevice:" << sDevice.c_str();

        _pport = _pusbController->getPortForDevice("LockControl", sDevice);
        if(_pport) {
            this->setStateConnected();
/*            this->openLock(1);
            try {
                this->readCommandResponse();    // 500ms wait...
            } catch (const std::runtime_error &e)
            {
                std::cout << "runtime error:" << e.what();
            }
            this->openLock(3);
            try {
                this->readCommandResponse();    // 500ms wait...
            } catch (const std::runtime_error &e)
            {
                std::cout << "runtime error:" << e.what();
            }
            this->openLock(5);
            try {
                this->readCommandResponse();    // 500ms wait...
            } catch (const std::runtime_error &e)
            {
                std::cout << "runtime error:" << e.what();
            }
            this->openLock(7);
            try {
                this->readCommandResponse();    // 500ms wait...
            } catch (const std::runtime_error &e)
            {
                std::cout << "runtime error:" << e.what();
            }

            std::cout << "Lock State(1):\n";
            readLockStateCmd(17);

            try {
                this->readCommandResponse();
            } catch (const std::runtime_error &e)
            {
                std::cout << "runtime error:\n";
                std::cout << "\t\t" << e.what();
            }

            try {
                _un64LockLocks = inquireLockStatus(4);
                _bLockStateRead = true;
            }
            catch (const std::runtime_error &e)
            {
                std::cout << "runtime error inquireLockStatus(): " << e.what() << "\n";
            }

            // Test
            uint64_t    nMask = 0x0000000000000001;
            std::cout << "\nLocks:";
            for(uint64_t n=0;n<64;n++) {
                std::cout << ((_un64LockLocks & (nMask << n)) != 0 ? '1' : '0');
            }
            std::cout << "\n";

            int nLock = 1;
            bool bLock = isLockLock(nLock);
            std::cout << "Lock #" << nLock << ":" << bLock << "\n";
            nLock = 3;
            bLock = isLockLock(nLock);
            std::cout << "Lock #" << nLock << ":" << bLock << "\n";
            nLock = 5;
            bLock = isLockLock(nLock);
            std::cout << "Lock #" << nLock << ":" << bLock << "\n";
*/
        }
  }
    else {
      std::cout << "USB->serial adapter version 2 not found, aborting...";
        std::cout << "\tError: device number not found for:" << _sDeviceType.toStdString() << "\n";
    }

}

std::string CLockController::int_to_bin(uint16_t number)
{
    const std::string result = std::bitset<16>(number).to_string();
    return result.substr(result.find("1", 0));
}

std::string CLockController::to_hex(uint16_t to_convert){
    std::string result;
    std::stringstream ss;
    ss << std::hex << to_convert;
    ss >> result;
    return result;
}

union inttohex_t {
    struct {
        unsigned char byte_low;
        unsigned char byte_high;
    } bytes;
    uint16_t number;
} inttohex;
/**
 * @brief CLockController::openLock
 * @param nLockNum
 *
 * Notes:
 *  0x5D = Command Start
 *  0x8E = Tx, Relay/Lock Control
 *  <low addr> = Lock Address Low Byte
 *  <high addr> = Lock Address High Byte
 *  <seq #> = Sequence# & Ctrl (b0-2=rolling seq#, b3=0, b4= no-request msg ACK, b5= msg from MainApp,
 *        b6 = 1 Send msg on Reverse Loop, b7 = 1 Send msg on Forward Loop
 *  <Data 1> = Pulse Count (1 thru 250)
 *  <Data 2> = On Time * 10mS]
 *  <Data 3> = Off Time * 10mS
 *  <Check Sum> = Sum of Bytes 1 thru 7
 */
void CLockController::openLock(uint16_t nLockNum)
{
    static unsigned char    seqNum = 0;
    qDebug() << "CLockController::openLock #" << QVariant(nLockNum).toString();
    if(this->isConnected() && _pport->is_open())
    {
        unsigned char commands[9] = { 0x5D, 0x8E, 0x01, 0x00, 0xC8, 0x01, 0x20, 0x00, 0x8A };

        inttohex.number = nLockNum;

        commands[3] = inttohex.bytes.byte_high;
        commands[2] = inttohex.bytes.byte_low;

        commands[4] |= seqNum;
        commands[4] |= 0x50;    // Send msg on Reverse Loop?
        commands[4] |= 0x80;    // Send msg on Fwd loop?

        uint8_t nchksum = 0;
        for(int i=1; i<8; i++)
        {
            nchksum += commands[i];
        }
//        std::cout << "  checksum:" << std::to_string(nchksum) << "\n";
        commands[8] = (unsigned char)nchksum;
        _pport->write_some(asio::buffer(commands, 9));

        seqNum += 0x01;
        if(seqNum > 0x07)
        {
            seqNum = 0x00;
        }
    }
}

/**
 * @brief CLockController::openLockWithPulse
 * @param nLockNum
 * @param nPulseCount
 * @param nPulseOnTimeMS
 * @param nPulseOffTimeMS
 */
void CLockController::openLockWithPulse(uint16_t nLockNum, uint8_t nPulseCount, uint16_t nPulseOnTimeMS, uint16_t nPulseOffTimeMS)
{
    static unsigned char    seqNum = 0;

    if(this->isConnected() && _pport->is_open())
    {
        unsigned char commands[9] = { 0x5D, 0x8E, 0x01, 0x00, 0xC8, 0x01, 0x32, 0x00, 0x8A };

        std::string sLockNum = to_hex(nLockNum);

        if(nPulseCount > MAX_PULSE_COUNT ) {
            nPulseCount = MAX_PULSE_COUNT;
        }
        if(nPulseOnTimeMS > MAX_PULSE_ON_TIME) {
            nPulseOnTimeMS = MAX_PULSE_ON_TIME;
        }
        nPulseOnTimeMS = nPulseOnTimeMS / 10;

        if( nPulseOffTimeMS < VOLTAGE_DOUBLER_ACTIVATION_DELAY )
        {
            nPulseOffTimeMS = VOLTAGE_DOUBLER_ACTIVATION_DELAY;
        }
        nPulseOffTimeMS = nPulseOffTimeMS / 10;
        std::string sPulseCount = to_hex(nPulseCount);
        std::string sPulseOn = to_hex(nPulseOnTimeMS);
        std::string sPulseOff = to_hex(nPulseOffTimeMS);
        std::cout << "openLockWithPulse:" << sLockNum << " Pulse ON (/10):" << sPulseOn << " Pulse OFF(/10):" << sPulseOff << "\n";

        if(sLockNum.length()>1) {
            commands[3] = sLockNum[1];
            commands[3] -= 0x30;
        }
        commands[2] = sLockNum[0];
        commands[2] -= 0x30;

        commands[4] |= seqNum;
//        commands[4] |= 0x50;    // Send msg on Reverse Loop?
//        commands[4] |= 0x80;    // Send msg on Fwd loop?

        commands[5] = sPulseCount[0];
        commands[6] = sPulseOn[0];
        commands[7] = sPulseOff[0];

        uint8_t nchksum = 0;
        for(int i=1; i<8; i++)
        {
            nchksum += commands[i];
        }

//        std::cout << "  checksum:" << std::to_string(nchksum) << "\n";
        commands[8] = (unsigned char)nchksum;

        std::cout << "openLockWithPulse():";
        for(uint i=0;i<sizeof(commands);i++)
        {
            std::cout << (boost::format("0x%02x ") % static_cast<int>(commands[i]));
        }
        std::cout << "\n";


        _pport->write_some(asio::buffer(commands, 9));

        seqNum += 0x01;
        if(seqNum > 0x07)
        {
            seqNum = 0x00;
        }
    }
}


/**
 * @brief CLockController::readLockState
 * @param nLockNum
 * @return string read
 * Notes:
 *  0x5D = Command Start
 *  0x8A = Tx, Relay/Lock Status Inquiry
 *  <low addr> = Lock Address Low Byte
 *  <high addr> = Lock Address High Byte
 *  <seq #> = Sequence# & Ctrl (b0-2=rolling seq#, b3=0, b4= no-request msg ACK, b5= msg from MainApp,
 *        b6 = 1 Send msg on Reverse Loop, b7 = 1 Send msg on Forward Loop
 *  <Data 1> = Pulse Count (1 thru 250)
 *  <Data 2> = On Time * 10mS]
 *  <Data 3> = Off Time * 10mS
 *  <Check Sum> = Sum of Bytes 1 thru 7
 */
void CLockController::readLockStateCmd(uint8_t nLockNum)
{
   if(this->isConnected() && _pport->is_open())
   {
       unsigned char commands[9] = { 0x5D, 0x8A, 0x01, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x53 };
       commands[2] = (unsigned char)nLockNum;

       uint8_t nchksum = 0;
       for(int i=1; i<8; i++)
       {
           nchksum += commands[i];
       }

//       std::cout << "  checksum:" << std::to_string(nchksum) << "\n";

       commands[8] = (unsigned char)nchksum;

       std::cout << "Command: ";
       for(uint i=0;i<sizeof(commands);i++)
       {
           std::cout << (boost::format("0x%02x ") % static_cast<int>(commands[i]));
       }
       std::cout << "\n";

       _pport->write_some(asio::buffer(commands, 9));
   }
}


std::string CLockController::readCommandResponse()
{
    std::string rsp = "";

    if( this->isConnected() && _pport ) {
        CBlockingReader reader(*_pport, 2000);    // Read serial with timeout.

        int  nCount = 0;
        char c;

        // read from the serial port until we get a
        // \n or until a read times-out (500ms)
        while (reader.readChar(c) && nCount++ < 9) {
            rsp += c;
        }

        if (nCount < 9) {
            // it must have timed out.
            throw std::runtime_error("Read timed out!");
        }
    }
    return rsp;
}


/**
 * @brief CLockController::inquireLockStatus
 * @param unBanks
 * @return
 *
 * Read the board to determine which lock solenoids are attached to the board.
 */
uint64_t CLockController::inquireLockStatus(uint8_t unBanks)
{
    uint64_t        un64Locks = 0;
    uint64_t        un64ShiftValue = 0;
    unsigned char   ucLocks[2];
    std::string sResponse;
//    std::string sLocks="";
//    sLocks = std::to_string(un64Locks);

    std::cout << "Lock State(1):\n";

    if( unBanks > 4 ) {
        unBanks = 4;
    }
    for(int i=0;i<unBanks;i++)
    {
        readLockStateCmd(i*16+1);   // Read locks 1-16, 17-32, 33-48,  or 49-64
        try {
            sResponse = this->readCommandResponse();
        } catch (const std::runtime_error &e)
        {
            std::cout << "inquireLockStatus() runtime error:\t" << e.what();
            _un64LockLocks = 0xFFFFFFFFFFFFFFFF;
            return _un64LockLocks;
        }
        std::cout << "Command Response Count:" << sResponse.length() << "\n";
        std::cout << "Hex Response:";
        for(uint i=0;i<sResponse.length();i++)
        {
            std::cout << (boost::format("0x%02x ") % static_cast<int>(sResponse[i]));
        }

        std::cout << "\n";

        // Populate to 64 bit unsigned return value
        ucLocks[0] = (unsigned char)sResponse[6];
        ucLocks[1] = (unsigned char)sResponse[7];

        // Print out for test/check now
        std::cout << "Locks[" << i*16+1 << "+]:";
        for(uint8_t n=0;n<8;n++) {
            std::cout << ((ucLocks[0] & (0x01 << n)) != 0x00 ? '1' : '0');
        }
        std::cout << ":";
        for(uint8_t n=0;n<8;n++) {
            std::cout << ((ucLocks[1] & (0x01 << n)) != 0x00 ? '1' : '0');
        }
        std::cout << "\n";

        un64ShiftValue = ucLocks[0];
        un64Locks = un64Locks | (un64ShiftValue << (i*2)*8);
        un64ShiftValue = ucLocks[1];
        un64Locks = un64Locks | (un64ShiftValue << (i*2+1)*8);
    }
    _un64LockLocks = un64Locks;
    _bLockStateRead = true;
    _plockStatus->setLockState(_un64LockLocks);
//    std::cout << "saved banks..." << std::to_string(un64Locks) << "\n";

    return un64Locks;
}

void CLockController::OnLocksStatusRequest()
{
    emit __OnLocksStatus(*_plockStatus);
}

/**
 * @brief CLockController::isLockLock
 * @param nLockNum value from 1 to 64
 * @return 1 = Lock state has been read and the Lock lock solenoid is connect, 0 = Lock state read and not solenoid not connected,
 *   -1 = Lock state is not read
 */
int CLockController::isLock(uint16_t nLockNum)
{
    nLockNum = nLockNum;
    return 1;
//    if( _bLockStateRead )
//    {
//        // Test
//        uint64_t    un64Mask = 0x0000000000000001;
//        uint64_t    un64LockNum = (unsigned long long)nLockNum - 1LL;  // Set Lock num to zero based number
//        if ((_un64LockLocks & (un64Mask << un64LockNum)) != 0 )
//        {
//            return 1;
//        } else {
//            return 0;
//        }
//    } else {
//        return -1;
//    }

}


