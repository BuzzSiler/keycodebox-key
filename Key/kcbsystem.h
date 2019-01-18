#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

#include <QString>
#include <QStringList>
#include <QByteArray>


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
    void TakeAndStorePicture(QString filename="");
    bool HasCamera();
    QByteArray GetImageAsByteArray(QString filename="", bool delete_file=true);
    void BackupDatabase();
}
#endif // KCBSYSTEM_H
