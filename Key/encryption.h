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
    static void calculatePredictiveCodeOld(uint32_t nLockNum, std::string skey, QDateTime datetime, std::string *poutEncrypt, uint32_t maxLength, QString *psTmp);
    static void calculatePredictiveCode(uint32_t nLockNum, std::string skey, QDateTime datetime, std::string *poutEncrypt, uint32_t maxLength, QString *psTmp);

};

#endif // CENCRYPTION_H
