#include "magtekcardreader.h"

#include <QDebug>
#include <iostream>
#include <string>
#include "usbprovider.h"
#include "logger.h"

CMagTekCardReader::CMagTekCardReader(QObject *parent) : 
    QObject(parent),
    _handle(0)
{ 
}

CMagTekCardReader::~CMagTekCardReader()
{
    if(_handle) 
    {
        hid_close(_handle);
        _handle = 0;
    }
}

void CMagTekCardReader::ExtractCommandOutput(FILE *pF, std::string &rtnStr)
{
    char cChar = '\0';

    while(!feof(pF))
    {
        cChar = fgetc(pF);
        rtnStr += cChar;
    }
}

bool CMagTekCardReader::floatXInputDevice()
{
    FILE *pF;

    std::string filterString = "'HID c216'";
    std::string systemCmd = "xinput list | grep " + filterString;
    std::string sOutput = "";
    std::string parseToken = "id=";
    std::string xInputId = "";
    std::string floatCmd = "xinput float ";

    pF = popen(systemCmd.c_str(), "r");
    if(!pF)
    {
      KCB_WARNING_TRACE("failed poppen()");
      return false;
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    // KCB_DEBUG_TRACE("sOutput = " << QString::fromStdString(sOutput));

    size_t idPos = sOutput.find(parseToken);

    if( idPos != std::string::npos )
    {
        xInputId = sOutput.substr(idPos + parseToken.length(), 1);

        // KCB_DEBUG_TRACE("float xinput id: " << QString::fromStdString(xInputId));

        floatCmd += xInputId;
        std::system(floatCmd.c_str());
        return true;
    }
    return false;
}

bool CMagTekCardReader::openDeviceHandle()
{
    // KCB_DEBUG_TRACE("searching for hid magnetic card reader usb-c216");
    QString filterString = "usb-c216.*event";
    QString deviceType = "input";
    QString devicePrefix = "event";

    std::string sDevNum = UsbProvider::GetMagTekDevicePortString();
    if(sDevNum.empty())
    {
        KCB_DEBUG_TRACE("parsing of found device string failed");
    }
    else
    {
        // need to programatically generate this (scan for devices)
        std::string eventPath = "/dev/input/event" + sDevNum;
    
        if ((fileDescriptor = open(eventPath.c_str(), O_RDONLY)) < 0) 
        {
            KCB_WARNING_TRACE("failed to open /dev/input/event");
            return false;
        }
      
        if (ioctl(fileDescriptor, EVIOCGVERSION, &version)) 
        {
            KCB_WARNING_TRACE("failed to get version");
            return false;
        }
  
        // KCB_DEBUG_TRACE("evdev input driver version is " << (version >> 16) << "." << ((version >> 8) & 0xff) << "." << (version & 0xff));
  
        ioctl(fileDescriptor, EVIOCGID, id);
        // KCB_DEBUG_TRACE("Input device ID: bus" << "0x" << id[ID_BUS] << "vendor 0x" << id[ID_VENDOR] << "product 0x" << id[ID_PRODUCT] << "version 0x" << id[ID_VERSION]);
        
        ioctl(fileDescriptor, EVIOCGNAME(sizeof(name)), name);
        // KCB_DEBUG_TRACE("Input device name: " << name);
        
        return floatXInputDevice();
    }
    return false;
}

bool CMagTekCardReader::initMagTekCardReader()
{
    return openDeviceHandle();
}

int CMagTekCardReader::codeToInteger(int x)
{
    // code to numeric value
    // 11 -> 0, 2 -> 1, 3 -> 2, 4 -> 3,
    // 5 -> 4, 6 -> 5, 7 -> 6, 8 -> 7,
    // 9 -> 8, 10 -> 9
    int characterMap [] = { -1, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };

    // KCB_DEBUG_EXIT;
    if( (x >= 0) && (x < 12) )
    {
        return characterMap[x];
    }
    return -1;
}

bool CMagTekCardReader::isStartCode(int x)
{
    return ( x == 39 ) ? true : false;
}

void CMagTekCardReader::loop()
{

    int i, idx;
    bool capturingIntegers = false;
    int currentCode[40];
    char codeElement;
    char finalCode[5];
    int consecutiveZeros = 0;
    int currentCodeLength = 0;
    bool foundStartCharacter = false;

    while( 1 )
    {
        readDescriptor = read(fileDescriptor, ev, sizeof(struct input_event) * 1500);
        
        if (readDescriptor < (int) sizeof(struct input_event))
        {

            KCB_CRITICAL_TRACE("Error reading events (incorrect data size)");
            return;
        }
        else
        {
            int num_events = (int) (readDescriptor / sizeof(struct input_event));

            for (i = 0; i < num_events; i++)
            {
                //KCB_DEBUG_TRACE("type" << ev[i].type << "code" << ev[i].code << "value" << ev[i].value);

                if (ev[i].type == EV_SYN || ev[i].value == 0 )
                {
                    // ignore these
                }
                else if (ev[i].type == EV_MSC && (ev[i].code == MSC_RAW || ev[i].code == MSC_SCAN))
                {
                    // ignore these
                }
                else
                {
                    // if we're not capturingIntegers and the next event is an
                    //   integer, begin capturingIntegers 
                    if( isStartCode(ev[i].code) )
                    {
                        foundStartCharacter = true;
                        continue;
                    }
                    else
                    {
                        capturingIntegers = false;
                        if( foundStartCharacter )
                        {
                            if( codeToInteger(ev[i].code) != -1 )
                            {
                                capturingIntegers = true;
                            }
                        }
                    }
                    
                    if ( foundStartCharacter && !capturingIntegers)
                    {
                        foundStartCharacter = false;
                        if ( currentCodeLength > 4 )
                        {
                            for (idx=0; idx<currentCodeLength; idx++)
                            {
                                if( currentCode[idx] == 0 )
                                {
                                    consecutiveZeros++;
                                }
                                else
                                {
                                    consecutiveZeros = 0;
                                }
                            }

                            if ( consecutiveZeros < 4)
                            {
                                for(idx=0; idx<5; idx++)
                                {
                                    codeElement = currentCode[currentCodeLength - 1 - idx];
                                    finalCode[4 - idx] = codeElement;
                                }

                                QString code = QString("%1%2%3%4%5").arg(QString::number(finalCode[0])).arg(QString::number(finalCode[1])).arg(QString::number(finalCode[2])).arg(QString::number(finalCode[3])).arg(QString::number(finalCode[4]));
                                emit __onCardSwipe(code, "");

                                currentCodeLength = 0;
                                consecutiveZeros = 0;

                                break;
                            }
                        }
                    }
                    else
                    {
                        currentCode[currentCodeLength] = codeToInteger(ev[i].code);
                        currentCodeLength++;
                    }
                }
            }
        }
        QCoreApplication::processEvents();
    }
}

void CMagTekCardReader::start()
{
    // KCB_DEBUG_ENTRY;
    loop();
    // KCB_DEBUG_EXIT;
}

