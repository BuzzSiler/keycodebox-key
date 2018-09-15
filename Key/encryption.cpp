#include "encryption.h"
#include <QDebug>
#include <QVariant>
#include <QCryptographicHash>
#include <QString>
#include <QDateTime>
#include <QByteArray>
//#include <sqlite3ext.h>
#include "simplecrypt.h"

CEncryption::CEncryption()
{
}

QByteArray CEncryption::hashed(QString strToHash) 
{
    return QCryptographicHash::hash(strToHash.toUtf8(), QCryptographicHash::Sha1);
}


QString CEncryption::encryptString(QString strIn)
{
    SimpleCrypt crypto(0x6812dd9955aeb1dc);
    return crypto.encryptToString(strIn);
}

QString CEncryption::decryptString(QString strIn)
{
    SimpleCrypt crypto(0x6812dd9955aeb1dc);
    return crypto.decryptToString(strIn);
}

// Simple XOR with an offset into key
std::string CEncryption::encryptDecryptOld(int nVal, std::string toEncrypt)
{
    std::string skey = "P%!~A}%c4fpv]2$rYF;&SvF43@Ukba~a$!Fz9A;eQQ>zUH?7O'7N,zmK-Ryu";

    std::string output = toEncrypt;

    for (uint32_t i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ skey[(i + nVal) % (skey.size())];

    return output;
}

std::string CEncryption::encryptDecryptOld(int nVal, std::string toEncrypt, std::string key)
{
    std::string skey = key; // "P%!~A}%c4fpv]2$rYF;&SvF43@Ukba~a$!Fz9A;eQQ>zUH?7O'7N,zmK-Ryu";

    std::string output = toEncrypt;

    for (uint32_t i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ skey[(i + nVal) % (skey.size())];

    return output;
}

/**
 * @brief roundDateTime
 * @param res in minutes
 * @param datetime
 * @return datetime rounded up to nearest minutes res
 */
QDateTime &CEncryption::roundDateTime(int res, QDateTime &datetime)
{
    qDebug() << "datetime in:" << datetime.toString("yyyy-MM-dd HH:mm:ss");

    // res is in minutes
    int resSecs = res * 60;
    uint unixtime  = QDateTime::currentDateTime().toTime_t(); // current unix time
    uint roundUnit = unixtime + (resSecs - unixtime % resSecs); // round it up to the res
    QDateTime start;
    QDateTime hourLater;
    start.setTime_t(roundUnit); // set start datetime to rounded time
    hourLater.setTime_t(roundUnit + resSecs); // set finish time to
    datetime = start;

    qDebug() << "datetime rounded:" << datetime.toString("yyyy-MM-dd HH:mm:ss");

    return datetime;
}
