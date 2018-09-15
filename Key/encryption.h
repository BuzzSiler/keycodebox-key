#ifndef CENCRYPTION_H
#define CENCRYPTION_H

#include <string>

class QString;
class QDateTime;
class QByteArray;


class CEncryption
{
private:

public:
    CEncryption();

    static QByteArray hashed(QString strToHash);
    static QString encryptString(QString strIn);
    static QString decryptString(QString strIn);
    static std::string encryptDecryptOld(int nVal, std::string toEncrypt);
    static std::string encryptDecryptOld(int nVal, std::string toEncrypt, std::string key);
    static QDateTime &roundDateTime(int res, QDateTime &datetime);

};

#endif // CENCRYPTION_H
