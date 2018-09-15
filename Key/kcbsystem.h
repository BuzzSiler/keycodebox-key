#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

#include <QString>
#include <QStringList>


namespace kcb
{
    void ExecuteCommand(QString program, QStringList arguments, QString& stdOut, QString& stdErr, int& status);
    void GetRpiSerialNumber(QString& serial_number);
    void SetVNCCredentials(QString vnc_port, QString vnc_password);
    QString GetGatewayAddress();
    QString GetHostAddress();
    QString GetBcastAddress();
    QString GetMacAddress();
    QString GetNetworkMask();              
    bool FPingAddress(QString address);
    bool UpdateAppFile(QString filename_fq);
    void UnmountUsb(QString path);
    void Reboot();
}
#endif // KCBSYSTEM_H
