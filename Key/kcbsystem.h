#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

#include <QString>
#include <QStringList>
#include <QByteArray>


class QRect;
class QWidget;


namespace kcb
{
    void ExecuteCommand(QString program, QStringList arguments, QString& stdOut, QString& stdErr, int& status);
    QString GetRpiSerialNumber();
    void SetHostAddress(QString const &value);
    QString GetHostAddress();
    void SetNetworkMask(QString const &value);
    QString GetNetworkMask();      
    void SetBcastAddress(QString const &value);
    QString GetBcastAddress();
    void SetGatewayAddress(QString const &value);
    QString GetGatewayAddress();
    void SetDnsAddress(QString const &value);
    QString GetDnsAddress();
    QString GetMacAddress();
    void EnableStaticAddressing();
    void DisableStaticAddressing();
    bool StaticAddressingEnabled();
    bool FPingAddress(QString address);
    bool UpdateAppFile(QString filename_fq);
    void UnmountUsb(QString path);
    void Reboot();
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
    void SetupDisplay();
    void RestartNetworkInterface();
    bool isVncConnectionActive();
    void AddStaticAddressing(QString values);
    void RemoveStaticAddressing();
    QString IpAddrSubnetMaskToCidr(QString ip_addr, QString subnet_mask);
    void GetIpAddressAndStatus(QString &ip_address, bool &can_ping, bool &can_multicast);
    QString GetSystemId();
    void EnableInternetTime();
    void DisableInternetTime();
    void SetDateTime(const QDateTime& datetime);
    void SetTimeZone(const QString& timezone);

}
#endif // KCBSYSTEM_H
