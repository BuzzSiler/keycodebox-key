#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

class QString;


namespace kcb
{

    void GetRpiSerialNumber(QString& serial_number);
    void SetVNCCredentials(QString vnc_port, QString vnc_password);

}
#endif // KCBSYSTEM_H
