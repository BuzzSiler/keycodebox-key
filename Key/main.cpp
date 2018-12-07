#include "keycodeboxmain.h"
#include <QtGlobal>
#include <QApplication>
#include <QScopedPointer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QLoggingCategory>
#include <QtCore>

QScopedPointer<QFile>   main_log_file;
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("motif");

    main_log_file.reset(new QFile("/home/pi/kcb-config/logs/messages.log"));
    main_log_file.data()->open(QFile::Append | QFile::Text);
    qInstallMessageHandler(messageHandler);

    MainWindow w;
    w.show();
    return a.exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Open stream file writes
    QTextStream out(main_log_file.data());
    // Write the date of recording
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    // By type determine to what level belongs message
    switch (type)
    {
    //case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }
    // Write to the output category of the message and the message itself
    out << context.category << ": " << msg << endl;
    out.flush();    // Clear the buffered data
}
