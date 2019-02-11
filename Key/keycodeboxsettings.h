#ifndef KEYCODEBOXSETTINGS_H
#define KEYCODEBOXSETTINGS_H

#include <QObject>
#include <QTime>
#include <QString>
#include <QJsonObject>
#include <QVector>
#include <QPair>

class QJsonParseError;


typedef std::pair<QJsonObject, QJsonParseError> JsonObjErrPair;

typedef struct 
{
    QString model;
    int   num_locks;
    int   start;
    int   stop;
} CABINET_INFO;

typedef QVector<CABINET_INFO> CABINET_VECTOR;

class KeyCodeBoxSettings : public QObject
{
    Q_OBJECT
    public:
        static bool isFleetwaveEnabled();
        static int getNumCabinets();
        static int getLocksPerCabinet(int cab_index);
        static CABINET_VECTOR getCabinetsInfo();
        static bool isDisplaySet();
        static void setDisplay();

    private:
        static QJsonObject m_json_obj;
        static QString m_filename;
        static CABINET_VECTOR m_cabinet_info;

        static void JsonFromFile();
        static void JsonToFile();
        static void createDefault();

};

#endif // KEYCODEBOXSETTINGS_H
