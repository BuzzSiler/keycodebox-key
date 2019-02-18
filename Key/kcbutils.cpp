#include "kcbutils.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "kcbcommon.h"

namespace kcb
{

    static QString CONSTRUCTOR_MSG = QStringLiteral("Running the %1 constructor.");
    static QString DESTRUCTOR_MSG = QStringLiteral("Running the %1 constructor.");

    void Utils::ConstructorMsg(const QString& value)
    {
        qDebug() << CONSTRUCTOR_MSG.arg(value);
    }

    void Utils::ConstructorMsg(const QObject * const object)
    {
        ConstructorMsg(object->metaObject()->className());
    }

    void Utils::DestructorMsg(const QString& value)
    {
        qDebug() << DESTRUCTOR_MSG.arg(value);
    }

    void Utils::DestructorMsg(const QObject * const object)
    {
        DestructorMsg(object->metaObject()->className());
    }

    void Utils::JsonFromFile(QString const& path, QJsonObject& json_obj)
    {
        QString val;
        QFile file;
        QJsonDocument doc;

        KCB_DEBUG_ENTRY;
        file.setFileName(path);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        val = file.readAll();
        KCB_DEBUG_TRACE(val);
        file.close();
        doc = QJsonDocument::fromJson(val.toUtf8());
        json_obj = doc.object();
        KCB_DEBUG_EXIT;
    }

    void Utils::JsonToFile(QString const& path, QJsonObject& json_obj)
    {
        QString val;
        QFile file;
        KCB_DEBUG_ENTRY;
        QJsonDocument doc(json_obj);
        file.setFileName(path);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(doc.toJson());
        file.close();
        KCB_DEBUG_EXIT;
    }

}
