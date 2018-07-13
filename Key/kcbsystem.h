#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

#include <QString>
#include <QStringList>


namespace kcb
{
    void ExecuteCommand(QString program, QStringList arguments, QString& stdOut, QString& stdErr, int& status);
    void GetRpiSerialNumber(QString& serial_number);
    void SetVNCCredentials(QString vnc_port, QString vnc_password);
}
#endif // KCBSYSTEM_H
