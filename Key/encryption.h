#ifndef CENCRYPTION_H
#define CENCRYPTION_H

#include <string>
#include <QtGlobal>

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
    static QByteArray encryptWithKey(const QByteArray& data, const quint64 key);
    static QByteArray decryptWithKey(const QByteArray& data, const quint64 key);
    static QDateTime &roundDateTime(int res, QDateTime &datetime);

};

#endif // CENCRYPTION_H
