
#include <iostream>
#include <stdlib.h>
#include <boost/thread.hpp>

#include "crystalfontzcontroller.h"
#include "usbcontroller.h"

using namespace boost;

void CCrystalFontzController::initController()
{
    std::string sDevNum = _pusbController->getDevicePortString(_sDeviceType, _sFilterString, _sFindDevicePrefix);

    std::cout << "initController() sDevNum:" << sDevNum.c_str() << "\t";

    if(!sDevNum.empty())
    {
        std::string sDevice = "/dev/ttyUSB";
        sDevice += sDevNum;

        std::cout << "\tsDevice:" << sDevice.c_str();

        _pport = _pusbController->getPortForDevice("CrystalLCD", sDevice);
        if(_pport) {
            this->setStateConnected();
        }
  }
    else {
        std::cout << "\tError: device number not found for:" << _sDeviceType.toStdString() << "\n";
    }
}

bool CCrystalFontzController::_testWriteToOpenDevice()
{
    std::cout << "_testWriteToOpenDevice()" << "\n";
    if(!_pport->is_open())
    {
        std::cout << "\tport not open" << "\n";
        return false;
    }

    std::cout << "\tWriting:'Hello World!" << "\n";
//    unsigned char commands[] = { 0x1E };
    unsigned char helloWorld[] = { "Hello World!" };
    _pport->write_some(asio::buffer( helloWorld, sizeof(helloWorld) ));

    return true;
}

void CCrystalFontzController::rebootDisplay() {
    if(_pport->is_open())
    {
        unsigned char commands[1] = { 0x1A };
        _pport->write_some(asio::buffer( commands, 1 ));
    }
}


void CCrystalFontzController::hideCursor() {
    if(_pport->is_open())
    {
        // 0x04 = hide blinking cursor
        unsigned char commands[1] = { 0x04 };
        _pport->write_some(asio::buffer(commands, 1));
    }
}

void CCrystalFontzController::showUnderlineCursor() {
    if(_pport->is_open())
    {
        // 0x05 = show Underline Cursor
        unsigned char commands[1] = { 0x05 };
        _pport->write_some(asio::buffer(commands, 1));
    }
}

void CCrystalFontzController::showBlockCursor() {
    if(_pport->is_open())
    {
        // 0x06 = show Block Blinking Cursor
        unsigned char commands[1] = { 0x06 };
        _pport->write_some(asio::buffer(commands, 1));
    }
}


void CCrystalFontzController::moveCursorTopLeft()
{
    if(_pport->is_open())
    {
        // 0x01 = move cursor to top-left (home position)
        unsigned char commands[1] = { 0x01 };
        _pport->write_some(asio::buffer(commands, 1));
    }
}

void CCrystalFontzController::clearDisplay() {
    if(_pport->is_open())
    {
        // 0x0C = form feed (clears the display)
        // 0x01 = move cursor to top-left (home position)
        // 0x04 = hide blinking cursor
        unsigned char commands[3] = { 0x0C, 0x01, 0x04 };
        _pport->write_some(asio::buffer(commands, 3));
    }
}

/**
 * @brief backlightLevel
 * @param nlevel (0=0ff to 100=full on)
 */
void CCrystalFontzController::backlightLevel(uint8_t nlevel) {
    if(_pport->is_open())
    {
        if(nlevel > 100) { nlevel = 100; }

        // 0x0E = backlight command
        // nLevel = level ((byte) 0=off up to 100=full on)
        unsigned char commands[2] = { 0x0E, (unsigned char)nlevel };
        _pport->write_some(asio::buffer(commands, 2)); // backlight
    }
}

void CCrystalFontzController::contrastLevel(uint8_t nlevel) {
    if(_pport->is_open())
    {
        if(nlevel > 100) { nlevel = 100; }

        // 0x0E = backlight command
        // nLevel = level ((byte) 0=off up to 100=full on)
        unsigned char commands[2] = { 0x0F, (unsigned char)nlevel };
        _pport->write_some(asio::buffer(commands, 2)); // backlight
    }
}

void CCrystalFontzController::setCursorPosition(uint8_t column, uint8_t row) {
    if(_pport->is_open())
    {
        // 0x11 = Set Cursor Position
        // <col> = column value 0-15
        // <row> = row value 0-3
        if(column>15) column=15;
        if(row>3) row = 3;
        unsigned char commands[3] = { 0x11, (unsigned char)column, (unsigned char)row };
        _pport->write_some(asio::buffer(commands, 3));
    }
}

void CCrystalFontzController::writeToDisplay(std::string strToDisplay)
{
    if(_pport->is_open())
    {
        _pport->write_some(asio::buffer( strToDisplay.c_str(), strToDisplay.length()));
    }
}


void CCrystalFontzController::backlightOff() {
    backlightLevel(0);
}

void CCrystalFontzController::backlightOn() {
    backlightLevel(100);
}

CCrystalFontzController::~CCrystalFontzController()
{
    if(_pport->is_open())
    {
        _pport->close();
    }
}

void CCrystalFontzController::setup() {

    rebootDisplay();

    backlightOn();

    clearDisplay();

    _testWriteToOpenDevice();

    writeToDisplay("Yo!Yo!Ma!");
}


CCrystalFontzController::CCrystalFontzController(QObject *parent) : QObject(parent),
    _pport(0), _pusbController(0)
{

}


