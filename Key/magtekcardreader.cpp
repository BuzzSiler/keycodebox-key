#include "magtekcardreader.h"

#include <QDebug>
#include <iostream>
#include <string>
#include "usbprovider.h"

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
      qDebug() << "CMagTekCardReader::floatXInputDevice(), failed poppen()";
      return false;
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    qDebug() << "CMagTekCardReader::floatXInputDevice(), sOutput = " << QString::fromStdString(sOutput);

    size_t idPos = sOutput.find(parseToken);

    if( idPos != std::string::npos )
    {
        xInputId = sOutput.substr(idPos + parseToken.length(), 1);

        qDebug() << "CMagTekCardREader::floatXInputDevice(), float xinput id: " << QString::fromStdString(xInputId);

        floatCmd += xInputId;
        std::system(floatCmd.c_str());
        return true;
    }
    return false;
}

bool CMagTekCardReader::openDeviceHandle()
{
    qDebug() << "CMagTekCardReader::openDeviceHandle(), searching for hid magnetic card reader usb-c216";
    QString filterString = "usb-c216.*event";
    QString deviceType = "input";
    QString devicePrefix = "event";

    std::string sDevNum = UsbProvider::GetMagTekDevicePortString();
    if(sDevNum.empty())
    {
        qDebug() << "CMagTekCardReader::openDeviceHandle(), parsing of found device string failed";  
    }
    else
    {
        // need to programatically generate this (scan for devices)
        std::string eventPath = "/dev/input/event" + sDevNum;
    
        if ((fileDescriptor = open(eventPath.c_str(), O_RDONLY)) < 0) 
        {
            perror("evtest");
            return false;
        }
      
        if (ioctl(fileDescriptor, EVIOCGVERSION, &version)) 
        {
            perror("evtest: can't get version");
            return false;
        }
  
        fprintf(stderr,"evdev input driver version is %d.%d.%d\n",
          version >> 16, (version >> 8) & 0xff, version & 0xff);
  
        ioctl(fileDescriptor, EVIOCGID, id);
        fprintf(stderr, "Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
          id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);
        
        ioctl(fileDescriptor, EVIOCGNAME(sizeof(name)), name);
        fprintf(stderr, "Input device name: \"%s\"\n", name);
        
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

/**
 * @brief CMagTekCardReader::loop
 *  Blocking card read
 * @return
 */
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
            printf("yyy\n");
            perror("\nevtest: error reading");
            return;
        }
        else
        {
            for (i = 0; i < (int) (readDescriptor / sizeof(struct input_event)); i++)
            {        
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
                                qDebug() << "\n\tATTENTION: Observed new magstripe code: " << QString::number(finalCode[0]) << QString::number(finalCode[1]) << QString::number(finalCode[2]) << QString::number(finalCode[3]) << QString::number(finalCode[4]);

                                std::string xdotool = "xdotool key ";
                                std::string tempCmd = xdotool + std::to_string(finalCode[0]);
                                std::system(tempCmd.c_str());
                                usleep(50);
                                tempCmd = xdotool + std::to_string(finalCode[1]);
                                std::system(tempCmd.c_str());
                                usleep(50);
                                tempCmd = xdotool + std::to_string(finalCode[2]);
                                std::system(tempCmd.c_str());
                                usleep(50);
                                tempCmd = xdotool + std::to_string(finalCode[3]);
                                std::system(tempCmd.c_str());
                                usleep(50);
                                tempCmd = xdotool + std::to_string(finalCode[4]);
                                std::system(tempCmd.c_str());
                                usleep(50);
                                
                                currentCodeLength = 0;
                                consecutiveZeros = 0;

                                break;
                            }
                        }
                    }
                    else
                    {
                        /*
                        printf("Event: time %ld.%06ld, type %d, code %d, value %d\n",
                        ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                        ev[i].code,
                        ev[i].value);
                        */
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
    qDebug() << "CMagTekCardReader::start()";
    loop();
}

