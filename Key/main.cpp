#include "keycodeboxmain.h"
#include <QtGlobal>
#include <QApplication>
#include <QtCore>

#include "logger.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("motif");

    kcb::Logger::installHandler();
    kcb::Logger::setLevel(kcb::Logger::LEVEL_INFO);

    MainWindow w;
    w.show();
    return a.exec();
}
