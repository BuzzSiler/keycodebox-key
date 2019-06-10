#include "keycodeboxmain.h"
#include <QtGlobal>
#include <QApplication>
#include <QtCore>
#include <QThread>

#include "logger.h"
#include "kcbsystem.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("motif");

    kcb::Logger::installHandler();
    kcb::Logger::setLevel(kcb::Logger::LEVEL_INFO);
    kcb::BackupDatabase();

    // Note: This function may invoke a reboot
    kcb::SetupDisplay();
    
    qDebug() << QThread::currentThreadId();

    MainWindow w;
    return a.exec();
}
