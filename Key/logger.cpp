#include "logger.h"

#include <QFile>
#include <QDir>
#include <QScopedPointer>
#include <QTextStream>
#include <QDateTime>

namespace kcb
{
    static const QString LOG_PATH = "/home/pi/kcb-config/logs";
    static const QString LOG_FILENAME = LOG_PATH + "/messages.log";

    static const int NUM_LOGS = 10;

    static Logger::LOG_LEVEL log_level = Logger::LEVEL_INFO;

    static QString newFilename(QString const path = LOG_PATH)
    {
        static int counter = 1;

        QString filename = QString("messages%1.log").arg(counter, 2, 10, QLatin1Char( '0' ));
        counter = (counter % NUM_LOGS) + 1;
        return QString("%1/%2").arg(path).arg(filename);
    }

    static bool canBeLogged(QtMsgType type)
    {
        bool can_be_logged = false;

        switch (type)
        {
            // case QtInfoMsg:
            //     can_be_logged = log_level <= Logger::LEVEL_INFO;
            //     break;

            case QtDebugMsg:
                can_be_logged = log_level <= Logger::LEVEL_DEBUG;
                break;

            case QtWarningMsg:
                can_be_logged = log_level <= Logger::LEVEL_WARNING;
                break;

            case QtCriticalMsg:
                can_be_logged = log_level <= Logger::LEVEL_CRITICAL;
                break;

            case QtFatalMsg:
                can_be_logged = log_level <= Logger::LEVEL_FATAL;
                break;

            default:
                break;
        }

        return can_be_logged;
    }

    static const QString typeToString(const QtMsgType type)
    {
        QString level_str = "Unknown";
        switch (type)
        {
            // case QtInfoMsg:
            //     level_str = "INFO";
            //     break;

            case QtDebugMsg:
                level_str = "DEBUG";
                break;

            case QtWarningMsg:
                level_str = "WARN";
                break;

            case QtCriticalMsg:
                level_str = "CRIT";
                break;

            case QtFatalMsg:
                level_str = "FATAL";
                break;

            default:
                break;
        }

        return level_str;
    }

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        if (QFile::exists(LOG_FILENAME))
        {
            QFile outFileCheck(LOG_FILENAME);
            if (outFileCheck.size() > (10 * 1024 * 1024))
            {
                QString filename = newFilename();
                if (QFile::exists(filename))
                {
                    QFile::remove(filename);
                }
                outFileCheck.copy(filename);
                outFileCheck.resize(0);
            }
        }

        if (canBeLogged(type))
        {
            // Open stream file writes
            QFile outFile(LOG_FILENAME);
            outFile.open(QIODevice::WriteOnly | QFile::Append | QFile::Text);
            QTextStream out(&outFile);

            out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

            out << QString("%1").arg(context.category, 5) << " " << QString("%1").arg(typeToString(type), 5);

            if (type == QtDebugMsg)
            {
                out << " [Method: " << context.function << ", Line: " << context.line << "]";
            }

            out << " " << msg << endl;
            out.flush();    // Clear the buffered data
        }
    }

    //------------------Static Methods---------------------------

    void Logger::installHandler()
    {
        qInstallMessageHandler(messageHandler);
    }

    void Logger::setLevel(LOG_LEVEL value)
    {
        log_level = value;
    }

    Logger::LOG_LEVEL Logger::level()
    {
        return log_level;
    }

}