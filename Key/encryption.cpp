#include "encryption.h"
#include <QDebug>
#include <QVariant>
#include <QCryptographicHash>
#include <QString>
#include <QDateTime>
#include <QByteArray>
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

QByteArray CEncryption::encryptWithKey(const QByteArray& data, const quint64 key)
{
    SimpleCrypt crypto(key);
    return crypto.encryptToByteArray(data);
}

QByteArray CEncryption::decryptWithKey(const QByteArray& data, const quint64 key)
{
    SimpleCrypt crypto(key);
    return crypto.decryptToByteArray(data);
}

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