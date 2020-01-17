#ifndef KEYCODEBOXSETTINGS_H
#define KEYCODEBOXSETTINGS_H

#include <QObject>
#include <QTime>
#include <QString>
#include <QJsonObject>
#include <QVector>
#include <QPair>
#include <QStringList>

#include "cabinet_type.h"
#include "autocodegenerator.h"
#include "kcbsystem.h"


class QJsonParseError;


typedef std::pair<QJsonObject, QJsonParseError> JsonObjErrPair;


enum class SelectionType { DISABLED, SINGLE, MULTI };

struct NetworkSettings
{
    NetworkSettings(QString const & address, QString const & mask, QString const & broadcast, QString const & gateway, QString const & dns) :
        address(address), mask(mask), broadcast(broadcast), gateway(gateway), dns(dns) {}
    QString address;
    QString mask;
    QString broadcast;
    QString gateway;
    QString dns;
};

struct VncSettings
{
    VncSettings(QString const & port = "", QString const & password = "", bool enable = false) : port(port), password(password), enable(enable) {}
    QString port;
    QString password;
    bool enable;
};

struct SmtpSettings
{
    SmtpSettings(QString const & server, int port, int type, QString const & username, QString const & password) :
        server(server), port(port), type(type), username(username), password(password) {}
    QString server;
    int port;
    int type;
    QString username;
    QString password;
};

class KeyCodeBoxSettings : public QObject
{
    Q_OBJECT
    public:
        enum class PowerDownTimeout { NONE = 0,
                                      SEC_10 = 10,
                                      SEC_30 = 30,
                                      MIN_1 = 60,
                                      MIN_5 = 60*5,
                                      MIN_10 = 60*10,
                                      MIN_30 = 60*30 };

        static bool Validate();
        static QString GetBrandingImagePath();
        static QString GetUserImagePath();
        static QString GetSystemImagePath();

        static int getNumCabinets();
        static int getLocksPerCabinet(int cab_index);
        static kcb::CABINET_COLLECTION getCabinetsInfo();
        static int getTotalLocks();
        static int getMaxLocks(int cab_index);

        static void ClearCabinetConfig();
        static void AddCabinet(kcb::CABINET_INFO const &cab);

        static bool isDisplaySet();
        static void setDisplay();

        static void setNetworkingSettings(NetworkSettings const & settings);
        static NetworkSettings getNetworkingSettings();

        static void EnableStaticAddressing();
        static void DisableStaticAddressing();
        static bool StaticAddressingEnabled();

        static QString GetVncPort();
        static void setVncSettings(VncSettings const & settings);
        static VncSettings getVncSettings();
        static void setSmtpSettings(SmtpSettings const & settings);
        static SmtpSettings getSmtpSettings();

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
        static void setDisplayShowHideButton(bool value);
        static void setDisplayPowerDownTimeout(PowerDownTimeout timeout);
        static void setDisplayTakeReturnButtons(bool value);
        static bool getDisplayShowHideButton();
        static PowerDownTimeout getDisplayPowerDownTimeout();
        static bool getDisplayTakeReturnButtons();
        static QStringList getQuestions();
        static void setAdminName(QString const & value);
        static QString getAdminName();
        static void setAdminEmail(QString const & value);
        static QString getAdminEmail();
        static void setAdminPhone(QString const & value);
        static QString getAdminPhone();
        static void setAdminPassword(QString const & value);
        static QString getAdminPassword();
        static void setAssistPassword(QString const & value);
        static QString getAssistPassword();
        static void setEnableHwDiscoveryOnStartup(bool value);
        static bool getEnableHwDiscoveryOnStartup();
        static void SetShowCardId(bool state);
        static bool GetShowCardId();
        static QString GetBrandingImageFilename();

        static void OverrideBrandingImage(QString const& filename);
        static void RestoreDefaultBrandingImage();

        static void SetHostAddress(QString const &value);
        static QString GetHostAddress();
        static void SetNetworkMask(QString const &value);
        static QString GetNetworkMask();
        static void SetBcastAddress(QString const &value);
        static QString GetBcastAddress();
        static void SetGatewayAddress(QString const &value);
        static QString GetGatewayAddress();
        static void SetDnsAddress(QString const & value);
        static QString GetDnsAddress();
        static QString GetMacAddress();

    private:
        static QJsonObject m_json_obj;
        static QString m_filename;
        static kcb::CABINET_COLLECTION m_cabinet_info;

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
        static void setStringValue(QString const & key, QString const &  value);
        static QString getStringValue(QString const & key, QString const & default_value = QString(""));
        static void setIntValue(QString const & key, int value);
        static int getIntValue(QString const& key, int default_value = 0);
        static void setBoolValue(QString const & key, bool value);
        static bool getBoolValue(QString const & key, bool default_value);
        static void writeVncCredentials(VncSettings const & settings);
        static uint16_t validateNumLocks(int value);
        static uint16_t validateMaxLocks(uint16_t max_locks);
        static uint16_t validateTotalLocks(uint16_t total_num_locks, uint16_t max_locks);
        static QPair<uint16_t, uint16_t> validateStartStop(int start, int stop, uint16_t num_locks, uint16_t total_locks);

        static void CreateDefaultConfigJson();
        static void CreateDefaults(QString const& config_path);
        static bool ValidateStorage(QString const& path, QString const& config);
        static bool ValidateConfiguration(QString const& path, QString const& file);

        static QString GetStaticNetworkInfo(kcb::NETWORK_INFO_TYPE type);
        static QString GetDynamicNetworkInfo(kcb::NETWORK_INFO_TYPE type);
        static QString GetNetworkInfo(kcb::NETWORK_INFO_TYPE type);

};

#endif // KEYCODEBOXSETTINGS_H
