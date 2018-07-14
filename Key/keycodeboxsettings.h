#ifndef KEYCODEBOXSETTINGS_H
#define KEYCODEBOXSETTINGS_H

#include <QObject>
#include <QTime>
#include <QString>
#include <QJsonObject>

class QJsonParseError;


typedef std::pair<QJsonObject, QJsonParseError> JsonObjErrPair;

class KeyCodeBoxSettings : public QObject
{
        Q_OBJECT
    public:
        explicit KeyCodeBoxSettings(QObject *parent, const QString& filename="");

        bool isFleetwaveEnabled();

    private:
        QJsonObject m_json_obj;
        QString m_filename;

        void JsonFromFile();
        void JsonToFile();
};

#endif // KEYCODEBOXSETTINGS_H
