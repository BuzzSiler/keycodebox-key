#include "keycodeboxmain.h"
#include <QtGlobal>
#include <QApplication>
#include <QtCore>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("motif");

    MainWindow w;
    w.show();
    return a.exec();
}
