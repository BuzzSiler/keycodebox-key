#include "kcbutils.h"
#include "kcbsystem.h"
#include "kcbcommon.h"
#include <QString>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace kcb
{

    static QString DESTRUCTOR_MSG = QStringLiteral("Running the %1 destructor.");


    void ClassAllocation::DestructorMsg(const QString& value)
    {
        KCB_DEBUG_TRACE(DESTRUCTOR_MSG.arg(value));
    }

    void ClassAllocation::DestructorMsg(const QObject * const object)
    {
        DestructorMsg(object->metaObject()->className());
    }

    template<typename T, typename... Args>
    static std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    unsigned int countSetBits(int n) 
    { 
        unsigned int count = 0; 
        while (n) 
        { 
            n &= (n-1) ; 
            count++; 
        } 
        return count; 
    }

    QPixmap CreateQrCode(const QByteArray& data)
    {
        // KCB_DEBUG_ENTRY;
        QString stdOut;
        QString stdErr;
        int status;

        ExecuteCommand(QString("python"), QStringList() << QString("/home/pi/kcb-config/scripts/genqrcode.py") << QString(data), stdOut, stdErr, status);

        if (!status)
        {
            if (QFile::exists("/home/pi/kcb-config/scripts/qrcode.png"))
            {
                if (QFile::exists("/home/pi/kcb-config/scripts/qrcode.png"))
                {
                    QPixmap pm = QPixmap("/home/pi/kcb-config/scripts/qrcode.png", "PNG");
                    QFile::remove("/home/pi/kcb-config/scripts/qrcode.png");
                    return pm;
                }
            }
        }

        KCB_DEBUG_TRACE("empty pixmap");
        return QPixmap();
    }

    QChar GetRandomChar()
    {
        QVector<QChar> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                   '.', '_', '-', ';', ':', '!', '+', '?', '/', '\'', '"', '*', '#', '^', '$', '%', '@', '&', '\\', '`', '~', '<', '>', '{', '}', '[', ']', '(', ')', ' '
                                   };

        int index = qrand() % alphabet.count();

        return alphabet[index];
    }

    QStringList CreateRandomValues(int num_values, int code_length)
    {
        QStringList codes;
        for (int ii = 0; ii < num_values; ++ii)
        {
            int value = qrand();
            int length = QString::number(value).length();
            QString code;
            if (length > code_length)
            {
                code = QString::number(value).right(code_length);
            }
            else
            {
                code = QString("%1").arg(value, length, 10, QChar('0'));
            }

            codes.append(code);
        };

        return codes;
    }

    QString JsonToString(const QJsonObject& json)
    {
        return QJsonDocument(json).toJson(QJsonDocument::Compact);
    }

    QJsonObject StringToJson(const QString& str)
    {
        return QJsonDocument::fromJson(str.toUtf8()).object();
    }

    QDateTime StringToDateTime(const QString& datetime)
    {
        return QDateTime::fromString(datetime, DATETIME_FORMAT);
    }
}
