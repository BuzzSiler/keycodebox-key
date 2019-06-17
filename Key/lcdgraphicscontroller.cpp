#include "lcdgraphicscontroller.h"

#include <QDebug>
#include <cstdlib>
#include <unistd.h>


// Note: The following sys directory doesn't exist in the current image.  Consequently, brightness
// control doesn't work.  It is being investigated, but is disabled in the interim.


CLCDGraphicsController::CLCDGraphicsController(QObject *parent) : 
    QObject(parent),
    _timerDim(this)
{

}

void CLCDGraphicsController::startScreenDimTimer()
{
    connect(&_timerDim, SIGNAL(timeout()), this, SLOT(dimScreen()));
    _timerDim.start(30000);
}

void CLCDGraphicsController::setBrightness(unsigned int unValue)
{
    std::string sCmd = "";
    if( unValue > 255 ) 
    { 
        unValue = 255; 
    }

    sCmd = std::string("sudo bash -c 'echo ") + std::to_string(unValue) + " > /sys/class/backlight/rpi_backlight/brightness'";
    system(sCmd.c_str());
}

void CLCDGraphicsController::turnBacklightOn()
{
    system("sudo bash -c 'echo 1 > /sys/class/backlight/rpi_backlight/bl_power'");
}

void CLCDGraphicsController::turnBacklightOff()
{
    system("sudo bash -c 'echo 0 > /sys/class/backlight/rpi_backlight/bl_power'");
}

bool CLCDGraphicsController::isLCDAttached()
{
    if( access( "/sys/class/backlight/rpi_backlight/brightness", F_OK ) != -1 ) 
    {
        // file exists
        return true;
    } else 
    {
        // file doesn't exist
        return false;
    }
}

void CLCDGraphicsController::dimScreenMid()
{
    setBrightness(50);
    QTimer::singleShot(10000, this, SLOT(dimScreenLow()));
}

void CLCDGraphicsController::dimScreenLow()
{
    setBrightness(25);
}

void CLCDGraphicsController::dimScreenOff()
{
    setBrightness(255);
}

