#include <bitset>
#include <unistd.h>
#include <string>
#include "lockcontroller.h"
#include "serialport.h"
#include "usbcontroller.h"
#include "usbprovider.h"
#include <QDebug>
#include <QString>


CLockController::CLockController(QObject *parent) : QObject(parent)
{
}

void CLockController::initController()
{
    if( !_plockStatus )
    {
        _plockStatus = new CLocksStatus();
    }

    _plockStatus->setLockController(this);


    // Get the lock controller serial device
    _pport = UsbProvider::GetLockControllerDevice();

    if(_pport) 
    {
        this->setStateConnected();
    }
    else
    {
        qDebug() << "USB->serial adapter not found";
    }    
}

std::string CLockController::int_to_bin(uint16_t number)
{
    const std::string result = std::bitset<16>(number).to_string();
    return result.substr(result.find("1", 0));
}

std::string CLockController::to_hex(uint16_t to_convert)
{
    std::string result;
    result = QString(QByteArray::number(to_convert, 16)).toStdString();
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
    if(this->isConnected())
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
        qDebug() << "  checksum:" << QString::number(nchksum);
        commands[8] = (unsigned char)nchksum;
        _pport->WriteData(QByteArray((char *) commands, 9));

        seqNum += 0x01;
        if(seqNum > 0x07)
        {
            seqNum = 0x00;
        }

        QByteArray response;
        int num_bytes = _pport->ReadData(response);
    }
}

void CLockController::openLocks(QString lockNums)
{
    // lockNums is a string representation of a list of locks to be opened
    //    e.g. 1,2,5,16,22,23,30
    //    - If a single lock is specified, there will be no commas
    //         e.g. 5
    //    - If more than one lock is specified, then
    //         e.g. 1,4,6
    // Note: No spaces will present

    if (lockNums.contains(','))
    {
        QStringList list_str = lockNums.split(',');
        foreach (QString lock_str, list_str)
        {
            openLock(lock_str.toInt());
        }
    }
    else
    {
        openLock(lockNums.toInt());
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

    if(this->isConnected())
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
        qDebug() << "openLockWithPulse:" << QString::fromStdString(sLockNum) << " Pulse ON (/10):" << QString::fromStdString(sPulseOn) << " Pulse OFF(/10):" << QString::fromStdString(sPulseOff);

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

        qDebug() << "  checksum:" << QString::number(nchksum) << "\n";
        commands[8] = (unsigned char)nchksum;

        qDebug() << "openLockWithPulse():";
        for(uint i=0;i<sizeof(commands);i++)
        {
            qDebug() << QString("%1").arg(commands[i], 2, 16, QChar('0'));
        }
        qDebug() << "\n";

        _pport->WriteData(QByteArray((char *) commands, 9));

        seqNum += 0x01;
        if(seqNum > 0x07)
        {
            seqNum = 0x00;
        }

        QByteArray response;
        int num_bytes = _pport->ReadData(response);        
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
    qDebug() << "CLockController::readLockStateCmd";
    if(this->isConnected())
    {
        unsigned char commands[9] = { 0x5D, 0x8A, 0x01, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x53 };
        commands[2] = (unsigned char)nLockNum;

        uint8_t nchksum = 0;
        for(int i=1; i<8; i++)
        {
            nchksum += commands[i];
        }

        qDebug() << "  checksum:" << QString::number(nchksum) << "\n";

        commands[8] = (unsigned char)nchksum;

        qDebug() << "Command: ";
        for(uint i=0;i<sizeof(commands);i++)
        {
            qDebug() << QString("%1").arg(commands[i], 2, 16, QChar('0'));
           
        }
        qDebug() << "\n";

        _pport->WriteData(QByteArray((char *) commands, 9));
    }
}


std::string CLockController::readCommandResponse()
{
    std::string rsp = "";
    int  nCount = 0;

    qDebug() << "CLockController::readCommandResponse";

    if( this->isConnected() && _pport ) 
    {
        QByteArray data;
        nCount = _pport->ReadData(data);
        rsp = QString(data).toStdString();        
    }

    if (nCount < 9) {
        // it must have timed out.
        throw std::runtime_error("Read timed out!");
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

    qDebug() << "CLockController::inquireLockStatus";
    qDebug() << "Lock State(1):";

    if( unBanks > 4 ) 
    {
        unBanks = 4;
    }

    for(int i=0;i<unBanks;i++)
    {
        readLockStateCmd(i*16+1);   // Read locks 1-16, 17-32, 33-48,  or 49-64
        try 
        {
            sResponse = this->readCommandResponse();
        } 
        catch (const std::runtime_error &e)
        {
            qDebug() << "inquireLockStatus() runtime error:\t" << e.what();
            _un64LockLocks = 0xFFFFFFFFFFFFFFFF;
            return _un64LockLocks;
        }
        qDebug() << "Command Response Count:" << sResponse.length();
        qDebug() << "Hex Response:";
        for(uint j=0;j<sResponse.length();j++)
        {
            qDebug() << QString("%1").arg(sResponse[j], 2, 16, QChar('0'));
        }

        qDebug() << "\n";

        // Populate to 64 bit unsigned return value
        ucLocks[0] = (unsigned char)sResponse[6];
        ucLocks[1] = (unsigned char)sResponse[7];

        // Print out for test/check now
        qDebug() << "Locks[" << i*16+1 << "+]:";
        for(uint8_t n=0;n<8;n++) 
        {
            qDebug() << ((ucLocks[0] & (0x01 << n)) != 0x00 ? '1' : '0');
        }
        qDebug() << ":";
        for(uint8_t n=0;n<8;n++) 
        {
            qDebug() << ((ucLocks[1] & (0x01 << n)) != 0x00 ? '1' : '0');
        }
        qDebug() << "\n";

        un64ShiftValue = ucLocks[0];
        un64Locks = un64Locks | (un64ShiftValue << (i*2)*8);
        un64ShiftValue = ucLocks[1];
        un64Locks = un64Locks | (un64ShiftValue << (i*2+1)*8);
    }
    _un64LockLocks = un64Locks;
    _bLockStateRead = true;
    _plockStatus->setLockState(_un64LockLocks);
    //qDebug << "saved banks..." << QString::number(un64Locks) << "\n";

    return un64Locks;
}

void CLockController::OnLocksStatusRequest()
{
    emit __OnLocksStatus(*_plockStatus);
}