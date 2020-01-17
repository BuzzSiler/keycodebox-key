#include "keycodeboxmain.h"
#include <QtGlobal>
#include <QApplication>
#include <QtCore>
#include <QThread>

#include "logger.h"
#include "kcbstartup.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("motif");

    kcb::Logger::installHandler();
    kcb::Logger::setLevel(kcb::Logger::LEVEL_INFO);

    if (!kcb::Startup())
    {
        KCB_DEBUG_TRACE("Failed startup");
        return -1;
    }
    
    MainWindow w;
    return a.exec();
}
