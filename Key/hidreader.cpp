#include "hidreader.h"
#include <unistd.h>
#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <iostream>


CHWKeyboardReader::CHWKeyboardReader(QObject *parent) : 
    QObject(parent),
    _handle(0)
{    
}

CHWKeyboardReader::~CHWKeyboardReader()
{
    if(_handle) 
    {
        hid_close(_handle);
    }
}


void CHWKeyboardReader::ExtractCommandOutput(FILE *pF, std::string &rtnStr)
{
    char cChar = '\0';

    while(!feof(pF))
    {
        cChar = fgetc(pF);
        rtnStr += cChar;
    }
}

bool CHWKeyboardReader::floatXInputDevice()
{
    FILE *pF;

    std::string filterString = "'HID OMNIKEY 5427 CK'";
    std::string systemCmd = "xinput list | grep " + filterString;
    std::string sOutput = "";
    std::string parseToken = "id=";
    std::string xInputId = "";
    std::string floatCmd = "xinput float ";

    pF = popen(systemCmd.c_str(), "r");
    if(!pF)
    {
        qDebug() << "CHWKeyboardReader::floatXInputDevice(), failed poppen()";
        return false;
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    qDebug() << "CHWKeyboardReader::floatXInputDevice(), sOuput = " << QString::fromStdString(sOutput);

    size_t idPos = sOutput.find(parseToken);

    if( idPos != std::string::npos )
    {
        xInputId = sOutput.substr(idPos + parseToken.length(), 1);

        qDebug() << "CHWKeyboardReader::floatXInputDevice(), float xinput id: " << QString::fromStdString(xInputId);

        floatCmd += xInputId;
        std::system(floatCmd.c_str());
        return true;
    }

    return false;
}

bool CHWKeyboardReader::openDeviceHandle()
{

    //legacy code!
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    _handle = hid_open(_unVID, _unPID, NULL);

    if(_handle) 
    {
        qDebug() << "hid_opened!\n";
        hid_set_nonblocking(_handle, 1);
	floatXInputDevice();
        return true;
    } 
    else 
    {
        qDebug() << "failed to open hid\n";
        return false;
    }
    return false;
}

bool CHWKeyboardReader::closeDeviceHandle()
{
    if(_handle) 
    {
        hid_close(_handle);
        _handle = 0;
    }

    return true;
}

unsigned char CHWKeyboardReader::convertHIDKeyboardChar(unsigned char byIn)
{
    // Lookup table
    unsigned char aConvertTable[] =
    { 0x00, 0x00, 0x00, 0x00,
      'a','b','c','d','e','f','g','h','i','j','k',
    'l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
      '1','2','3','4','5','6','7','8','9','0',
    0x0d };

    if(sizeof(aConvertTable) >= (int)byIn) 
    {
        return aConvertTable[(int)byIn];
    } 
    else 
    {
        return 0x00;
    }
}

void CHWKeyboardReader::start()
{
    qDebug() << "CHIDReader::start()";
    while(1) {
        readHIDReaderLoop();
        QCoreApplication::processEvents();
    }
}

bool CHWKeyboardReader::initHIDReader(uint16_t vid, uint16_t pid)
{
    setVID_PID(vid, pid);
    return openDeviceHandle();
}


QString CHWKeyboardReader::readHIDReader()
{
    int res;
    std::string strRead = "";
    std::string strHID = "";
    unsigned char buf[65];
    int i;
    bool bEndFound = false;
    std::string strResult = "";
    int digitCount = 0;
    
    // Read requested state. hid_read() has been set to be
    // non-blocking by the call to hid_set_nonblocking() above.
    // This loop demonstrates the non-blocking nature of hid_read().
    while(bEndFound == false)
    {
        res = 0;
        while (res == 0) 
        {
            res = hid_read(_handle, buf, sizeof(buf));
            if (res < 0)
            {
                //qDebug() << "Unable to read()\n";
            }
            usleep(50*1000);  // 50 ms
        }
        //QByteArray array((char*)buf, res);
        if(buf[2] == 0x28) 
        {
            // Enter key found
            bEndFound = true;
        }
        else if(buf[2] != 0x00) 
        {
            qDebug() << "Byte:" << QVariant(buf[2]).toString();
            strHID += convertHIDKeyboardChar(buf[2]);
            qDebug() << "HID:" << strHID.c_str();
        }
    }

    for(i=strHID.length() - 1; i >= 0; i--)
    {
	    strResult += strHID[i];
	    digitCount++;
    }
    
    std::string xdotool = "xdotool key ";
    for(i = digitCount - 1; i > -1; i--)
    {
        std::string tempCmd = xdotool + strResult[i];
        qDebug() << "tempCmd: " << QString::fromStdString(tempCmd);
        std::system(tempCmd.c_str());
        usleep(50);
    }
    
    // Note: It is necessary to reverse the string when creating the xdotool command,
    // but we must return the code in the order in was received.
    return strHID.c_str();
}


QString CHWKeyboardReader::convertHexStringToNumString(QString inHex)
{
    bool bOk;
    // Trim off the last 8 chars to convert to a number
    if(inHex.size() > 8) {
        inHex = inHex.mid(inHex.size()-8);
    }

    unsigned int unValue = inHex.toUInt(&bOk, 16);
    if(bOk) 
    {
        return QVariant( (unsigned int)unValue).toString();
    } 
    else 
    {
        return "";
    }
}

/**
 * @brief CHIDReader::readCardReaderLoop
 */
void CHWKeyboardReader::readHIDReaderLoop()
{
    QString sCardData;

    while(1)    // Or on flag
    {
        try {
            sCardData = readHIDReader();

            // sCardData is in hex
            qDebug() << "HID Read:" << sCardData;

            // Succeeded in getting at least one code
            qDebug() << "Read some codes";
            if(sCardData.size() > 0 )
            {
                qDebug() << "Code before conversion:" << sCardData;
// It doesn't make any sense to convert these values.  The HID code
// is not a numbers, it's characters and so it should be treated as
// such.  Converting it to a number causes it to not match what's in
// the database, i.e., there is no conversion done when assigning the
// code because xdotool inserts the text directly into the LineEdit;
// nor, should there be.  Use the code as presented.
#ifdef CODE_WITH_NO_PURPOSE
                sCardData = convertHexStringToNumString(sCardData);
                qDebug() << "Code after conversion:" << sCardData;
#endif                
                emit __onHIDSwipeCodes(sCardData, "");
            }
        } 
        catch (const std::runtime_error &e)
        {
            qDebug() << "readHIDReaderLoop() runtime error:\t" << e.what();
        }
    }
}

int CHWKeyboardReader::parseHIDReaderCodes(QString sCardData, QString *pcode1, QString *pcode2)
{
    // Parse the card data for codes
    QRegExp re("\\d+");
    QStringList list;
    int pos = 0;
    while ((pos = re.indexIn(sCardData, pos)) != -1)
    {
        list << re.cap(1);
        pos += re.matchedLength();
    }

    qDebug() << "Code Parsing size:" << list.size() << " count:" << list.count();
    *pcode1 = "";
    *pcode2 = "";
    if( list.size() > 0 )
    {
        *pcode1 = list[0];
        qDebug() << "Code 1:" << pcode1;
    }
    if( list.size() > 1 )
    {
        *pcode2 = list[1];
        qDebug() << "Code 2:" << pcode2;
    }

    return list.size();
}

void CHWKeyboardReader::testOne()
{
    int res;

    unsigned char buf[65];
    wchar_t wstr[MAX_STR];
    int i;

    qDebug() << "CHIDReader::testOne()" << wstr;

//    // Initialize the hidapi library
//    res = hid_init();

    // Read the Manufacturer String
    res = hid_get_manufacturer_string(_handle, wstr, MAX_STR);
    qDebug() << "Manufacturer String:" << wstr;

    // Read the Product String
    res = hid_get_product_string(_handle, wstr, MAX_STR);
    qDebug() << "Product String:" << wstr;

    // Read the Serial Number String
    res = hid_get_serial_number_string(_handle, wstr, MAX_STR);
    qDebug() << "Serial Number String: (" << wstr[0] << ") " << wstr;

    // Read Indexed String 1
    res = hid_get_indexed_string(_handle, 1, wstr, MAX_STR);
    qDebug() << "Indexed String 1: " << wstr;

    // Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(_handle, 1);

    // Request state (cmd 0x81). The first byte is the report number (0x0).
    buf[0] = 0x0;
    buf[1] = 0x81;
    res = hid_write(_handle, buf, 65);

    // Read requested state
    res = hid_read_timeout(_handle, buf, 65, -1);  // blocking wait
    for (i = 0; i < res; i++)
        qDebug() << buf[i];
    // Read requested state. hid_read() has been set to be
    // non-blocking by the call to hid_set_nonblocking() above.
    // This loop demonstrates the non-blocking nature of hid_read().
//    res = 0;
//    while (res == 0) {
//        res = hid_read(_handle, buf, sizeof(buf));
////            if (res == 0)
////                printf("waiting...\n");
//        if (res < 0)
//            qDebug() << "Unable to read()\n";
//        usleep(5*1000);
//    }

    qDebug() << "Data read:\n   ";
    // Print out the returned buffer.
    for (i = 0; i < res; i++)
        qDebug() << buf[i];
    qDebug();

}

void CHWKeyboardReader::TestTwo()
{
    int res;
    unsigned char buf[65];
    #define MAX_STR 255
    wchar_t wstr[MAX_STR];
    hid_device *handle;
    int i;

    // Enumerate and print the HID devices on the system
    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    while (cur_dev) {
        printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
            cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
        printf("\n");
        printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
        printf("  Product:      %ls\n", cur_dev->product_string);
        printf("\n");
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);


    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(0x76b, 0x5428, NULL);

    // Read the Manufacturer String
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    res = hid_get_product_string(handle, wstr, MAX_STR);
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    printf("Serial Number String: %ls", wstr);
    printf("\n");

    // Send a Feature Report to the device
    buf[0] = 0x2; // First byte is report number
    buf[1] = 0xa0;
    buf[2] = 0x0a;
    res = hid_send_feature_report(handle, buf, 17);

    // Read a Feature Report from the device
    buf[0] = 0x2;
    res = hid_get_feature_report(handle, buf, sizeof(buf));

    // Print out the returned buffer.
    qDebug() << "Feature Report\n   ";
    for (i = 0; i < res; i++)
        qDebug() << QString("%02hhx ").arg(buf[i]);
    qDebug() << "\n";

    // Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(handle, 1);

    // Send an Output report to toggle the LED (cmd 0x80)
    buf[0] = 1; // First byte is report number
    buf[1] = 0x80;
    res = hid_write(handle, buf, 65);

    // Send an Output report to request the state (cmd 0x81)
    buf[1] = 0x81;
    hid_write(handle, buf, 65);

    // Read requested state
    res = hid_read(handle, buf, 65);
    if (res < 0)
    {
        //printf("Unable to read()\n");
    }

    // Print out the returned buffer.
    for (i = 0; i < res; i++)
    {
        qDebug() << QString("buf[%d]: %d\n").arg(i, buf[i]);
    }
}


