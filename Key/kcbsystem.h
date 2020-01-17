#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QNetworkInterface>


class QRect;
class QWidget;


namespace kcb
{
    struct ExecuteCommandStatus
    {
        QString stdOut;
        QString stdErr;
        int exitCode;
    };

    typedef enum { HOST_ADDRESS, BCAST_ADDRESS, NETWORK_MASK, GATEWAY_ADDRESS, DNS_ADDRESS, MAC_ADDRESS } NETWORK_INFO_TYPE;

    ExecuteCommandStatus ExecuteCommand(QString program, QStringList arguments);
    QString GetRpiSerialNumber();
    // void SetHostAddress(QString const &value);
    // QString GetHostAddress();
    // void SetNetworkMask(QString const &value);
    // QString GetNetworkMask();      
    // void SetBcastAddress(QString const &value);
    // QString GetBcastAddress();
    // void SetGatewayAddress(QString const &value);
    // QString GetGatewayAddress();
    // void SetDnsAddress(QString const &value);
    // QString GetDnsAddress();
    // QString GetMacAddress();
    // void EnableStaticAddressing();
    // void DisableStaticAddressing();
    // bool StaticAddressingEnabled();
    bool FPingAddress(QString address);
    bool UpdateAppFile(QString filename_fq);
    void UnmountUsb(QString path);
    void Reboot();
    void Shutdown();
    void TakeAndStorePicture(QString filename="");
    bool HasCamera();
    QByteArray GetImageAsByteArray(QString filename="", bool delete_file=true);
    void BackupDatabase();
    void TurnOffDisplay();
    void TurnOnDisplay();
    bool isDisplayPowerOn();
    void GetScreenDimensions(int&  width, int& height);
    void GetAvailableGeometry(QRect& rect);
    void SetWindowParams(QWidget* window);
    void updateDisplayConfig();
    void RestartNetworkInterface();
    bool isVncConnectionActive(QString const& vncPort);
    void AddStaticAddressing(QString values);
    void RemoveStaticAddressing();
    QString IpAddrSubnetMaskToCidr(QString ip_addr, QString subnet_mask);
    void GetIpAddressAndStatus(QString &ip_address, bool &can_ping, bool &can_multicast);
    QString GetSystemId();
    void EnableInternetTime();
    void DisableInternetTime();
    void SetDateTime(const QDateTime& datetime);
    void SetTimeZone(const QString& timezone);
    bool IsStorageMounted(QString const &mountPoint);
    bool FileExists(QString const &fullpath);
    bool DirExists(QString const &path);
    void setVncEnabled();
    void setVncDisabled();
    void setVncCredentials(QString const & port, QString const & password);

    QString ParseNetworkInfo(NETWORK_INFO_TYPE type);
    QString ParseGatewayAddress();
    QString ParseDnsAddress();

    QList<QNetworkInterface> GetQualifiedInterfaces();



}
#endif // KCBSYSTEM_H
