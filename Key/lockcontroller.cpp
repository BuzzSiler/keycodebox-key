#include "lockcontroller.h"

#include <bitset>
#include <unistd.h>
#include <string>

#include <QDebug>
#include <QString>
#include <QTimer>

#include "serialport.h"
#include "usbcontroller.h"
#include "usbprovider.h"
#include "kcbcommon.h"
#include "keycodeboxsettings.h"


#define MAIN_APP_GENS_SEQ 0x08
#define REQUEST_ADDL_IMMED_MSG_ACK 0x10
#define MAIN_APP_SENDS_MSG 0x20
#define SEND_MSG_ON_REVERSE_LOOP 0x40
#define SEND_MSG_ON_FORWARD_LOOP 0x80



union inttohex_t {
    struct {
        unsigned char byte_low;
        unsigned char byte_high;
    } bytes;
    uint16_t number;
} inttohex;

static unsigned char seqNum = 0;

static uint16_t GLOBAL_ADDRESS = 0xA000;

CLockController::CLockController(QObject *parent) : QObject(parent)
{
}

void CLockController::initController()
{
    // Get the lock controller serial device
    _pport = UsbProvider::GetLockControllerDevice();

    if(_pport)
    {
        this->setStateConnected();
    }
    else
    {
        KCB_DEBUG_TRACE("USB->serial adapter not found");
    }
}

uint16_t CLockController::ReadSoftwareVersion(uint16_t addr)
{
    #define SOFTWARE_VERSION_OFFSET (32)
    return ReadEeprom(addr, SOFTWARE_VERSION_OFFSET);
}

QString CLockController::SoftwareVersionToString(uint16_t version)
{
    uint8_t major = (version & 0xFF00) >> 8;
    uint8_t minor = (version & 0x00FF);
    return QString("%1.%2").
                    arg(major, 2, 10, QChar('0')).
                    arg(minor, 2, 10, QChar('0'));
}

void CLockController::ReadBoardEeprom(uint16_t addr)
{
    // KCB_DEBUG_ENTRY;
    // Read Board Data
    // PIC18F46K22 allocated EEPROM locations in Code V1.17.
    //      EEPROM_Door_Address_Start   ;D'0000' ;First Door Address located here; 2 bytes
    //      EEPROM_Door_Address_End     ;D'0002' ;Last Door Address located here; 2 bytes
    //      EEPROM_SoftwareVer          ;D'0032'
    //      EEPROM_MyAddr               ;D'0036' ;Master Address located here;  2 bytes
    //      EEPROM_BroadAddr            ;D'0038' ;Global Address located here;  2 bytes
    //      EEPROM_FixedData_3456       ;D'0044'

    #define SWAP_BYTES(value) ( (value & 0x00ff) << 8 | (value & 0xff00) >> 8 )

    uint16_t value = ReadEeprom(addr, 0);
    uint16_t door_start_addr = SWAP_BYTES(value);
    value = ReadEeprom(addr, 2);
    uint16_t door_end_addr = SWAP_BYTES(value);
    uint16_t software_version = ReadSoftwareVersion(addr);
    UpdateDetectProgress();
    value = ReadEeprom(addr, 36);
    uint16_t board_addr = SWAP_BYTES(value);

    // value = ReadEeprom(addr, 38);
    // uint16_t broadcast_addr = SWAP_BYTES(value);
    // uint16_t fixed_data = ReadEeprom(addr, 44);
    // KCB_DEBUG_TRACE("Door Start Address" << door_start_addr);
    // KCB_DEBUG_TRACE("Door End Address" << door_end_addr);
    // KCB_DEBUG_TRACE("Software Version" << hex << software_version);
    // KCB_DEBUG_TRACE("Board Addr" << hex << board_addr);
    // KCB_DEBUG_TRACE("Broadcast Addr" << hex << broadcast_addr);
    // KCB_DEBUG_TRACE("Fixed Data" << hex << fixed_data);

    int num_locks = (door_end_addr - door_start_addr) + 1;

    KeyCodeBoxSettings::AddCabinet({"KCB32",
                                    num_locks,
                                    door_start_addr,
                                    door_end_addr,
                                    SoftwareVersionToString(software_version),
                                    QString("%1").arg(board_addr, 4, 16, QChar('0'))});

    // KCB_DEBUG_EXIT;
}

QByteArray CLockController::CreateSearchNetworkCommand(uint16_t addr)
{
    // KCB_DEBUG_ENTRY;
    QByteArray command(9, 0x00);

    command[0] = 0x5D;
    command[1] = 0x9A;
    inttohex.number = addr;
    command[2] = inttohex.bytes.byte_low;
    command[3] = inttohex.bytes.byte_high;
    uint8_t control_byte = 0;
    control_byte |= MAIN_APP_GENS_SEQ;
    control_byte |= SEND_MSG_ON_FORWARD_LOOP;
    command[4] = control_byte;

    // KCB_DEBUG_EXIT;
    return command;
}

uint16_t CLockController::SearchNetwork(uint16_t addr)
{
    QByteArray command = CreateSearchNetworkCommand(addr);
    QByteArray response = SendCommand(command);
    // The board master address will be in the response
    uint16_t board_master_address = response[2] | response[3] << 8;
    return board_master_address;
}

void CLockController::SetBoardLockStartStop(uint16_t addr, uint8_t start, uint8_t stop)
{
    // Read EEPROM to set the upper address byte
    (void) ReadEeprom(addr, 0);

    QByteArray command(9, 0x00);

    command[0] = 0x5D;
    command[1] = 0x84;
    inttohex.number = addr;
    command[2] = inttohex.bytes.byte_low;
    command[3] = inttohex.bytes.byte_high;
    uint8_t control_byte = 0;
    control_byte |= MAIN_APP_GENS_SEQ;
    control_byte |= SEND_MSG_ON_FORWARD_LOOP;
    command[4] = control_byte;
    command[5] = 0x00;
    command[6] = start;
    command[7] = 0x00;

    QByteArray response = SendCommand(command);

    command[5] = 0x02;
    command[6] = stop;
    command[7] = 0x00;

    response = SendCommand(command);
}

void CLockController::LocateMaster()
{
    // The maximum number of boxes that can exist is 4, each containing 64 locks, for a total of 256

    // 1. Send network search message to A000 -> Get board address
    // 2. Send network search message to A001 -> Get new board address (if same board address then only one board in the system)
    // 3. Send network search message to A002 -> Get new board address (if same board address as first, then there are two boards in the system)
    // 4. Send network search message to A003 -> Get new board address (if same board address as first, then there are three boards in the system)
    // 5. Send network search message to A004 -> Get new board address (if same board address as first, then there are four boards in the system)
    QVector<uint16_t> addresses;
    uint16_t addr;
    uint16_t global_addr = GLOBAL_ADDRESS;
    addr = SearchNetwork(global_addr);
    // KCB_DEBUG_TRACE("address" << hex << addr);
    UpdateDetectProgress();

    if (addr != 0)
    {
        addresses.append(addr);

        for (int ii = 1; ii < 5; ++ii)
        {
            addr = SearchNetwork(global_addr + ii);
            // KCB_DEBUG_TRACE("address" << addr);
            UpdateDetectProgress();
            if (addr == 0 || addresses[0] == addr)
            {
                break;
            }
            addresses.append(addr);
        }
    }

    UpdateDetectProgress();
    KeyCodeBoxSettings::ClearCabinetConfig();

    // KCB_DEBUG_TRACE("board address" << addresses);
    int start = 1;
    int stop = 32;
    foreach (auto ba, addresses)
    {
        UpdateDetectProgress();
        ReadBoardEeprom(ba);
        UpdateDetectProgress();
        start += 32;
        stop += 32;
    }
    UpdateDetectProgress();
}

QByteArray CLockController::SendCommand(QByteArray &cmd)
{
    // KCB_DEBUG_ENTRY;
    QByteArray response(9, 0x00);
    if (isConnected())
    {
        SetSequenceNumber(cmd);
        SetChecksum(cmd);
        // KCB_DEBUG_TRACE("command:" << qPrintable(cmd.toHex()));
        (void) _pport->WriteData(cmd);
        (void) _pport->ReadData(response);
        // KCB_DEBUG_TRACE("response:" << qPrintable(response.toHex()));
    }
    // KCB_DEBUG_EXIT;
    return response;
}

void CLockController::SetSequenceNumber(QByteArray &msg)
{
    // KCB_DEBUG_TRACE("sequence number" << seqNum);
    uint8_t ctrl_seq_num = static_cast<uint8_t>(msg[4]);
    ctrl_seq_num |= seqNum;
    msg[4] = static_cast<char>(ctrl_seq_num);
    seqNum = (seqNum + 1) & 0x07;
}

uint8_t CLockController::GetSequenceNumber(QByteArray const &msg)
{
    return static_cast<uint8_t>(msg[4]) & 0x07;
}

void CLockController::SetChecksum(QByteArray &cmd)
{
    cmd[8] = CalcChecksum(cmd);
}

uint8_t CLockController::CalcChecksum(QByteArray const &cmd)
{
    uint8_t nchksum = 0;
    for(int i=1; i<8; i++)
    {
        nchksum += cmd[i];
    }
    return nchksum;
}

bool CLockController::IsResponse(uint8_t control)
{
    return control & MAIN_APP_SENDS_MSG;
}

void CLockController::ProcessErrorResponse(QByteArray const &response)
{
    /*
        Byte 0 Command Start 0x5D 
        Byte 1 OP code 0x89  From KD8/12 Error Message 
        Byte 2 Unit Address Low KD8/12 Master Address L 
        Byte 3 Unit Address High KD8/12 Master Address H 
        Byte 4 Sequence number 
            Bits0-2 = KD8/12 new seq#  
            Bit-3=1 seq# generated by KD8/12 
            Bit-4=0 no msg ACK 
            Bit-5=1 msg from KD8/KD12 
            Bit-6=1  send FWD Loop 
            Bit-7=1  send RVS Loop 
        Byte 5 Data 1 
            Bit-0 Tx Queue Error 
            Bit-1 Stack Overflow 
            Bit-2 Stack Underflow 
            Bit-3 Rx Queue Reset 
            Bit-4 EEPROM Writes Not Enabled 
            Bit-5 Inc_EEPROM_Word_Overflow (Coin Count > 65K) 
            Bit-6  
            Bit-7 Queue Over-write problem, queue reset (Que Addr Below) 
        Byte 6 Data 2 
            Bit-0 x 
            Bit-1 x 
            Bit-2 x  
            Bit-3 x 
            Bit-4  
            Bit-5  
            Bit-6  
            Bit-7 
        Byte 7 Data 3 
            Bit-0 Chksum Error 
            Bit-1 Invalid OpCode 
            Bit-2 OpCode not implemented 
            Bit-3 
            Bit-4  
            Bit-5  
            Bit-6 Error_Do_EEPROM_Action_WRERR 
            Bit-7 Error_Do_EEPROM_Action_BadOpCode/QueCnt 
        Byte 8 Check Sum New CheckSum
    */

    // We need to gather up these bits and store as protocol statistics
    KCB_DEBUG_TRACE("Error Message Received" << response.toHex());

}

bool CLockController::IsErrorResponse(QByteArray const &response)
{
    return (response[0] == 0x5D) && (response[1] == 0x89);
}

bool CLockController::ValidateResponse(RESPONSE type, QByteArray const &command, QByteArray const &response)
{
    if (IsErrorResponse(response))
    {
        ProcessErrorResponse(response);
        return false;
    }

    uint8_t control = static_cast<uint8_t>(response[4]);
    uint8_t length = static_cast<uint8_t>(response.length());

    bool is_response = IsResponse(control);

    bool valid_control = static_cast<uint8_t>(command[4]) & control;

    bool has_additional_msg_ack = control & REQUEST_ADDL_IMMED_MSG_ACK;

    bool correct_num_bytes = false;
    bool valid_cs = false;
    bool valid_seq_num = false;
    if (has_additional_msg_ack)
    {
        if (length == 18)
        {
            correct_num_bytes = true;

            QByteArray first_msg = response.left(9);
            QByteArray additional_msg = response.right(9);

            uint8_t cs = static_cast<uint8_t>(first_msg[8]);
            uint8_t calc_cs = CalcChecksum(first_msg);
            bool first_valid_cs = cs == calc_cs;

            cs = static_cast<uint8_t>(additional_msg[8]);
            calc_cs = CalcChecksum(additional_msg);
            bool second_valid_cs = cs == calc_cs;
            valid_cs = first_valid_cs && second_valid_cs;

            uint8_t cmd_seq_num = GetSequenceNumber(command);
            uint8_t rsp_seq_num = GetSequenceNumber(first_msg);
            uint8_t addl_seq_num = GetSequenceNumber(additional_msg);
            valid_seq_num = (cmd_seq_num == rsp_seq_num) && (cmd_seq_num == addl_seq_num);

        }
    }
    else if (length == 9)
    {
        correct_num_bytes = true;

        uint8_t cs = static_cast<uint8_t>(response[8]);
        uint8_t calc_cs = CalcChecksum(response);
        valid_cs = cs == calc_cs;

        uint8_t cmd_seq_num = GetSequenceNumber(command);
        uint8_t rsp_seq_num = GetSequenceNumber(response);
        valid_seq_num = cmd_seq_num == rsp_seq_num;
    }

    bool valid_addr = false;
    if (type == RESPONSE::OPEN_LOCK)
    {
        valid_addr = (command[2] == response[2]) && (command[3] == response[3]);
        if (has_additional_msg_ack)
        {
            valid_addr = valid_addr && (command[2+9] == response[2+9]);
        }
    }
    else if (type == RESPONSE::READ_EEPROM)
    {
        valid_addr = (command[2] == response[2]) && (command[3] == response[3]) && (command[5] == response[5]);
    }
    else
    {
        valid_addr = true;
    }

    if (!is_response)
    {
        KCB_DEBUG_TRACE("response not indicated");
    }

    if (!correct_num_bytes)
    {
        KCB_DEBUG_TRACE("Incorrect response length");
    }

    if (!valid_control)
    {
        KCB_DEBUG_TRACE("Incorrect control");
    }

    if (!valid_cs)
    {
        KCB_DEBUG_TRACE("Incorrect checksum");
    }

    if (!valid_seq_num)
    {
        KCB_DEBUG_TRACE("Incorrect sequence");
    }

    return (is_response && correct_num_bytes && valid_control && valid_cs && valid_seq_num && valid_addr);
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

QByteArray CLockController::CreateReadEepromCommand(uint16_t addr, uint16_t offset)
{
    // KCB_DEBUG_ENTRY;
    QByteArray command(9, 0x00);

    command[0] = 0x5D;
    command[1] = 0x83;
    inttohex.number = addr;
    command[2] = inttohex.bytes.byte_low;
    command[3] = inttohex.bytes.byte_high;
    uint8_t control_byte = 0;
    control_byte &= ~MAIN_APP_GENS_SEQ;
    control_byte &= ~REQUEST_ADDL_IMMED_MSG_ACK;
    control_byte &= ~MAIN_APP_SENDS_MSG;
    control_byte |= SEND_MSG_ON_REVERSE_LOOP;
    control_byte |= SEND_MSG_ON_FORWARD_LOOP;
    command[4] = control_byte;
    inttohex.number = offset;
    command[5] = inttohex.bytes.byte_low;
    command[6] = inttohex.bytes.byte_high;

    // KCB_DEBUG_EXIT;
    return command;
}

QByteArray CLockController::CreateOpenLockCommand(uint16_t addr)
{
    // KCB_DEBUG_ENTRY;
    QByteArray command(9, 0x00);

    command[0] = 0x5D;
    command[1] = 0x8E;
    inttohex.number = addr;
    command[2] = inttohex.bytes.byte_low;
    command[3] = inttohex.bytes.byte_high;
    uint8_t control_byte = 0;
    control_byte &= ~MAIN_APP_GENS_SEQ;
    control_byte = REQUEST_ADDL_IMMED_MSG_ACK;
    control_byte &= ~MAIN_APP_SENDS_MSG;
    control_byte |= SEND_MSG_ON_REVERSE_LOOP;
    control_byte |= SEND_MSG_ON_FORWARD_LOOP;
    command[4] = control_byte;
    command[5] = 0x01;
    command[6] = 0x20;

    // KCB_DEBUG_EXIT;
    return command;
}

uint16_t CLockController::GetEepromResponseValue(RESPONSE type, QByteArray const &response)
{
    if (type == RESPONSE::READ_EEPROM)
    {
        return response[6] << 8 | response[7];
    }

    return 0;
}

uint16_t CLockController::ReadEeprom(uint16_t addr, uint16_t offset)
{
    // KCB_DEBUG_ENTRY;
    QByteArray command = CreateReadEepromCommand(addr, offset);
    QByteArray response = SendCommand(command);
    bool is_valid = ValidateResponse(RESPONSE::READ_EEPROM, command, response);

    if (is_valid)
    {
        uint16_t value = GetEepromResponseValue(RESPONSE::READ_EEPROM, response);
        return value;
    }

    KCB_DEBUG_TRACE("Invalid response" << command.toHex() << response.toHex());
    return 0;

    // KCB_DEBUG_EXIT;
}

void CLockController::openLock(uint16_t nLockNum)
{
    // KCB_DEBUG_ENTRY;

    QByteArray command = CreateOpenLockCommand(nLockNum);
    QByteArray response = SendCommand(command);

    ValidateResponse(RESPONSE::OPEN_LOCK, command, response);

    // KCB_DEBUG_EXIT;
}

void CLockController::openLocks(QString lockNums)
{
    // KCB_DEBUG_ENTRY;
    // lockNums is a string representation of a list of locks to be opened
    //    e.g. 1,2,5,16,22,23,30
    //    - If a single lock is specified, there will be no commas
    //         e.g. 5
    //    - If more than one lock is specified, then
    //         e.g. 1,4,6
    // Note: No spaces will present

    if (lockNums.contains(','))
    {
        auto list_str = lockNums.split(',');
        foreach (auto lock_str, list_str)
        {
            openLock(lock_str.toInt());
        }
    }
    else
    {
        openLock(lockNums.toInt());
    }
    // KCB_DEBUG_EXIT;
}

void CLockController::readLockStateCmd(uint8_t nLockNum)
{
    if(this->isConnected())
    {
        unsigned char commands[9] = { 0x5D, 0x8A, 0x01, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x53 };
        commands[2] = (unsigned char)nLockNum;

        uint8_t nchksum = 0;
        for(int i=1; i<8; i++)
        {
            nchksum += commands[i];
        }

        // KCB_DEBUG_TRACE("checksum:" << QString::number(nchksum));

        commands[8] = (unsigned char)nchksum;

        // qDebug() << "Command: ";
        // for(uint i=0;i<sizeof(commands);i++)
        // {
        //     qDebug() << QString("%1").arg(commands[i], 2, 16, QChar('0'));
           
        // }
        // qDebug() << "\n";

        _pport->WriteData(QByteArray((char *) commands, 9));
    }
}


std::string CLockController::readCommandResponse()
{
    // KCB_DEBUG_ENTRY;
    std::string rsp = "";
    int  nCount = 0;

    if( this->isConnected() && _pport ) 
    {
        QByteArray data;
        nCount = _pport->ReadData(data);
        if (nCount >= 9)
        {
            rsp = QString(data).toStdString();
        }
        else
        {
            KCB_WARNING_TRACE("Invalid response length" << nCount);
        }
    }

    // KCB_DEBUG_EXIT;
    return rsp;
}


void CLockController::inquireLockStatus()
{
    // KCB_DEBUG_ENTRY;

    CABINET_VECTOR cabs = KeyCodeBoxSettings::getCabinetsInfo();

    int success_count = 0;
    for (int ii = 0; ii < cabs.count(); ++ii)
    {
        QString sw_version("");
        bool ok;
        int addr = cabs[ii].addr.toInt(&ok, 16);
        if (ok)
        {
            uint16_t sw_version_bytes = ReadSoftwareVersion(addr);
            sw_version = SoftwareVersionToString(sw_version_bytes);
        }

        if (sw_version == cabs[ii].sw_version)
        {
            success_count++;
        }
    }

    if (success_count == cabs.count())
    {
        KCB_DEBUG_TRACE("configured cabinets successfully queried");
    }
    else
    {
        KCB_CRITICAL_TRACE("failure communicating with configured cabinets");
    }

    // KCB_DEBUG_EXIT;
}

void CLockController::UpdateDetectProgress()
{
    // KCB_DEBUG_ENTRY;
    emit DiscoverHardwareProgressUpdate(update_status);
    update_status += 7;
    // KCB_DEBUG_EXIT;
}

void CLockController::detectHardware()
{
    KCB_DEBUG_ENTRY;
    update_status = 0;
    UpdateDetectProgress();
    LocateMaster();
    emit DiscoverHardwareProgressUpdate(100);
    KCB_DEBUG_EXIT;
}

void CLockController::setLockRanges()
{
    // KCB_DEBUG_ENTRY;
    CABINET_VECTOR cabinets = KeyCodeBoxSettings::getCabinetsInfo();

    foreach (auto cab, cabinets)
    {
        // KCB_DEBUG_TRACE("cab start" << cab.start << "cab stop" << cab.stop << "total" << cab.num_locks);

        bool ok;
        int addr = cab.addr.toInt(&ok, 16);
        if (ok)
        {
            SetBoardLockStartStop(addr, cab.start, cab.stop);
        }
        else
        {
            KCB_DEBUG_TRACE("Failed to convert address" << cab.addr << "to int");
        }
    }

    // KCB_DEBUG_EXIT;
}
