#ifndef KCBSYSTEM_H
#define KCBSYSTEM_H

class QString;


namespace kcb
{

    void GetRpiSerialNumber(QString& serial_number);
    void SetVNCCredentials(QString vnc_port, QString vnc_password);
    bool SendChevinTakeRequest(QString code, QString& lockNum);
    bool SendChevinTakeComplete(QString code, QString lockNum);
    bool SendChevinReturnRequest(QString code, QString& lockNum, QString& question1, QString& question2, QString& question3);
    bool SendChevinReturnComplete(QString lockNum, QString answer1, QString answer2, QString answer3);
}
#endif // KCBSYSTEM_H
