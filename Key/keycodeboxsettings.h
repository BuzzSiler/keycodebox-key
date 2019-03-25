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

typedef struct
{
    QString address;
    QString mask;
    QString broadcast;
    QString gateway;
    QString dns;
} NETWORK_SETTINGS;

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

        static void setNetworkingSettings(NETWORK_SETTINGS const & settings);
        static NETWORK_SETTINGS getNetworkingSettings();

        static void EnableStaticAddressing();
        static void DisableStaticAddressing();
        static bool StaticAddressingEnabled();   
        static QString GetVncPort();
        static void SetVncCredentials(QString port, QString password);


    private:
        static QJsonObject m_json_obj;
        static QString m_filename;
        static CABINET_VECTOR m_cabinet_info;

        static void JsonFromFile();
        static void JsonToFile();
        static void createDefault();
        static void SetEnableStaticAddressing();
        static void ClearEnableStaticAddressing();
};

#endif // KEYCODEBOXSETTINGS_H
