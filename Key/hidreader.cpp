#include "hidreader.h"

#include <unistd.h>
#include <iostream>

#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QByteArray>

#include "logger.h"


CHWKeyboardReader::CHWKeyboardReader(QObject *parent) : 
    QObject(parent),
    _handle(0)
{
}

CHWKeyboardReader::~CHWKeyboardReader()
{
    if (_handle)
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
        KCB_DEBUG_TRACE("failed poppen");
        return false;
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    KCB_DEBUG_TRACE("sOuput = " << QString::fromStdString(sOutput));

    size_t idPos = sOutput.find(parseToken);

    if( idPos != std::string::npos )
    {
        xInputId = sOutput.substr(idPos + parseToken.length(), 1);

        KCB_DEBUG_TRACE("float xinput id: " << QString::fromStdString(xInputId));

        floatCmd += xInputId;
        std::system(floatCmd.c_str());
        return true;
    }

    return false;
}

bool CHWKeyboardReader::openDeviceHandle()
{

    _handle = hid_open(_unVID, _unPID, NULL);

    if(_handle) 
    {
        hid_set_nonblocking(_handle, 1);
        floatXInputDevice();
        return true;
    } 
    else 
    {
        KCB_DEBUG_TRACE("Keyboard Reader HID device not found");
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
            // KCB_DEBUG_TRACE("Byte:" << QVariant(buf[2]).toString());
            strHID += convertHIDKeyboardChar(buf[2]);
            // KCB_DEBUG_TRACE("HID:" << strHID.c_str());
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
        KCB_DEBUG_TRACE("tempCmd: " << QString::fromStdString(tempCmd));
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
            // KCB_DEBUG_TRACE("HID Read:" << sCardData);

            // Succeeded in getting at least one code
            // KCB_DEBUG_TRACE("Read some codes");
            if(sCardData.size() > 0 )
            {
                // KCB_DEBUG_TRACE("Code before conversion:" << sCardData);
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
            KCB_DEBUG_TRACE("runtime error" << QString(e.what()));
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

    // KCB_DEBUG_TRACE("Code Parsing size:" << list.size() << " count:" << list.count());
    *pcode1 = "";
    *pcode2 = "";
    if( list.size() > 0 )
    {
        *pcode1 = list[0];
    }
    if( list.size() > 1 )
    {
        *pcode2 = list[1];
    }

    return list.size();
}








