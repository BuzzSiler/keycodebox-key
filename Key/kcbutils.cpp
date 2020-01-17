#include "kcbutils.h"

#include <QString>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "kcbsystem.h"
#include "kcbcommon.h"

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

    namespace Utils
    {
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

	        auto status = ExecuteCommand(QString("python"), QStringList() << QString(KCB_SCRIPTS_PATH + "/genqrcode.py") << QString(data));

	        if (!status.exitCode)
	        {
	            if (QFile::exists(KCB_SCRIPTS_PATH + "/qrcode.png"))
	            {
	                if (QFile::exists(KCB_SCRIPTS_PATH + "/qrcode.png"))
	                {
	                    QPixmap pm = QPixmap(KCB_SCRIPTS_PATH + "/qrcode.png", "PNG");
	                    QFile::remove(KCB_SCRIPTS_PATH + "/qrcode.png");
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
        
        QJsonObject JsonFromFile(QString const &filename)
        {
            QString val;
            QFile file;
            QJsonDocument doc;

            // KCB_DEBUG_ENTRY;
        
            file.setFileName(filename);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            val = file.readAll();
            //KCB_DEBUG_TRACE(val);
            file.close();
            doc = QJsonDocument::fromJson(val.toUtf8());
            QJsonObject json_obj = doc.object();

            // KCB_DEBUG_TRACE(JsonToString(json_obj));
            // KCB_DEBUG_EXIT;
            return json_obj;
        }
        
        void JsonToFile(QString const &filename, QJsonObject const &json)
        {
            QString val;
            QFile file;
            // KCB_DEBUG_ENTRY;
            QJsonDocument doc(json);

            //KCB_DEBUG_TRACE(JsonToString(json));

            file.setFileName(filename);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            file.write(doc.toJson());
            file.close();
            // KCB_DEBUG_EXIT;
        }

        template <class T>
        void setValue(QJsonObject& json, QString const & key, T value)
        {
            if (json.contains(key))
            {
                json[key] = QJsonValue(value);
            }
            else
            {
                json.insert(key, QJsonValue(value));
            }
        }

        template <class T>
        T getValue(QJsonObject& json, QString const & key, T default_value)
        {
            // KCB_DEBUG_ENTRY;
            T result = default_value;
            if (json.contains(key))
            {
                result = json[key].toInt();
            }
            else
            {
                setValue<T>(json, key, result);
            }

            // KCB_DEBUG_EXIT;
            return result;
        }

        void setIntValue(QJsonObject& json, QString const & key, int value)
        {
            // KCB_DEBUG_ENTRY;
            if (json.contains(key))
            {
                json[key] = QJsonValue(value);
            }
            else
            {
                json.insert(key, QJsonValue(value));
            }
            // KCB_DEBUG_EXIT;
        }

        int getIntValue(QJsonObject& json, QString const & key, int const & default_value)
        {
            // KCB_DEBUG_ENTRY;
            int result = default_value;
            if (json.contains(key))
            {
                result = json[key].toInt();
            }
            else
            {
                setIntValue(json, key, result);
            }

            // KCB_DEBUG_EXIT;
            return result;
        }

        void setBoolValue(QJsonObject& json, QString const & key, bool value)
        {
            // KCB_DEBUG_ENTRY;
            if (json.contains(key))
            {
                json[key] = QJsonValue(value);
            }
            else
            {
                json.insert(key, QJsonValue(value));
            }
            // KCB_DEBUG_EXIT;
        }

        bool getBoolValue(QJsonObject& json, QString const & key, bool const & default_value)
        {
            // KCB_DEBUG_ENTRY;
            bool result = default_value;
            if (json.contains(key))
            {
                result = json[key].toBool();
            }
            else
            {
                setBoolValue(json, key, result);
            }

            // KCB_DEBUG_EXIT;
            return result;
        }

        void setStringValue(QJsonObject& json, QString const & key, QString value)
        {
            // KCB_DEBUG_ENTRY;
            if (json.contains(key))
            {
                json[key] = QJsonValue(value);
            }
            else
            {
                json.insert(key, QJsonValue(value));
            }
            // KCB_DEBUG_EXIT;
        }

        QString getStringValue(QJsonObject& json, QString const & key, QString const & default_value)
        {
            // KCB_DEBUG_ENTRY;
            QString result = default_value;
            if (json.contains(key))
            {
                result = json[key].toString();
            }
            else
            {
                setStringValue(json, key, result);
            }

            // KCB_DEBUG_EXIT;
            return result;
        }

        bool fileExists(QString const & filename)
        {
            return QFile::exists(filename);
        }
    }


}
