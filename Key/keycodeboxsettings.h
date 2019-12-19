#ifndef KEYCODEBOXSETTINGS_H
#define KEYCODEBOXSETTINGS_H

#include <QObject>
#include <QTime>
#include <QString>
#include <QJsonObject>
#include <QVector>
#include <QPair>

#include "autocodegenerator.h"

class QJsonParseError;


typedef std::pair<QJsonObject, QJsonParseError> JsonObjErrPair;

typedef struct cab_info
{
    QString model;
    int   num_locks;
    int   start;
    int   stop;
    QString sw_version;
    QString addr;
    bool operator<(const cab_info& a) const
    {
        return start < a.start;
    }
} CABINET_INFO;

typedef QVector<CABINET_INFO> CABINET_VECTOR;

enum class SelectionType { DISABLED, SINGLE, MULTI };

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
        static int getTotalLocks();

        static void ClearCabinetConfig();
        static void AddCabinet(CABINET_INFO const &cab);

        static bool isDisplaySet();
        static void setDisplay();

        static void setNetworkingSettings(NETWORK_SETTINGS const & settings);
        static NETWORK_SETTINGS getNetworkingSettings();

        static void EnableStaticAddressing();
        static void DisableStaticAddressing();
        static bool StaticAddressingEnabled();   
        static QString GetVncPort();
        static void SetVncCredentials(QString port, QString password);
        static void SetLockSelectionSingle();
        static void SetLockSelectionMulti();
        static void SetLockSelectionDisabled();
        static bool IsLockSelectionSingle();
        static bool IsLockSelectionMulti();
        static bool IsLockSelectionEnabled();
        static void EnableAutoCode();
        static void DisableAutoCode();
        static bool IsAutoCodeEnabled();
        static bool IsAutoCodeCommitted();

        static AutoCodeSettings GetAutoCodeSettings();
        static void setAutoCodeSettings(AutoCodeSettings settings);
        static void SetAutoCodeNextGenDateTime(const QDateTime& datetime);
        static QByteArray GetAutoCodeKey();
        static QString GetAutoCodePassword();
        static QDateTime GetAutoCodeNextGenDateTime();
        static void EnableAutoCodeEmail();
        static void DisableAutoCodeEmail();
        static bool IsInternetTimeEnabled();
        static void EnableInternetTime();
        static void DisableInternetTime();
        static void EnableApplyAccessTypeToAllCodes();
        static void DisableApplyAccessTypeToAllCodes();
        static bool IsApplyAccessTypeToAllCodes();

    private:
        static QJsonObject m_json_obj;
        static QString m_filename;
        static CABINET_VECTOR m_cabinet_info;

        static void JsonFromFile();
        static void JsonToFile();
        static void createDefault();
        static void SetEnableStaticAddressing();
        static void ClearEnableStaticAddressing();
        static void SetLockSelectionType(SelectionType value);
        static void SetAutoCodeEnableState(bool state);
        static void SetAutoCodeDefaults();
        static SelectionType GetLockSelectionType();
        static void SetInternetTimeSetting(bool value);
        static bool GetInternetTimeSetting();
        static void SetApplyAccessTypeToAllCodesSettings(bool value);
        static bool GetApplyAccessTypeToAllCodesSettings();
        static uint16_t validateNumLocks(int value);
        static QPair<uint16_t, uint16_t> validateStartStop(int start, int stop, uint16_t num_locks, uint16_t total_locks);

};

#endif // KEYCODEBOXSETTINGS_H
