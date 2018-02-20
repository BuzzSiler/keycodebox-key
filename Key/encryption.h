#ifndef CENCRYPTION_H
#define CENCRYPTION_H

#include <stdint-gcc.h>
#include <string>
#include <QDebug>
#include <QVariant>
#include <QString>
#include <QCryptographicHash>
#include <QDateTime>
#include <sqlite3.h>
#include <sqlite3ext.h>
#include "simplecrypt.h"


class CEncryption
{
private:

public:
    CEncryption();

    static QByteArray hashed(QString strToHash) 
    {
        return QCryptographicHash::hash(strToHash.toUtf8(), QCryptographicHash::Sha1);
    }


    static QString encryptString(QString strIn)
    {
        SimpleCrypt crypto(0x6812dd9955aeb1dc);
        return crypto.encryptToString(strIn);
    }

    static QString decryptString(QString strIn)
    {
        SimpleCrypt crypto(0x6812dd9955aeb1dc);
        return crypto.decryptToString(strIn);
    }

    // Simple XOR with an offset into key
    static std::string encryptDecryptOld(int nVal, std::string toEncrypt)
    {
        std::string skey = "P%!~A}%c4fpv]2$rYF;&SvF43@Ukba~a$!Fz9A;eQQ>zUH?7O'7N,zmK-Ryu";

        std::string output = toEncrypt;

        for (uint32_t i = 0; i < toEncrypt.size(); i++)
            output[i] = toEncrypt[i] ^ skey[(i + nVal) % (skey.size())];

        return output;
    }

    static std::string encryptDecryptOld(int nVal, std::string toEncrypt, std::string key)
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
    static QDateTime &roundDateTime(int res, QDateTime &datetime)
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


    static void calculatePredictiveCodeOld(uint32_t nLockNum, std::string skey, QDateTime datetime, std::string *poutEncrypt, uint32_t maxLength, QString *psTmp)
    {
        uint32_t un32Sum = 0;
        uint32_t unLockTmp = 0;
        uint32_t unEncSumDate = 0;
        uint32_t i;
        uint32_t unStartOff = 0;
        uint32_t unCharPos = 0;

        *psTmp += " Date:" + datetime.toString("yyyy-MM-dd HH:mm");

        // Encrypt and sum the date for an ok multiplier
        std::string sdt = datetime.toString("yyyy-MM-dd HH:mm").toStdString();
        // Encrypt the rounded date as "yyyy-MM-dd HH:mm"
        std::string encDate = encryptDecryptOld(nLockNum * 3, sdt, skey);

        *psTmp += QString(" EncDate:") + encDate.c_str();

        // Add sum encDate
        for(i=0;i<encDate.size();i++)
        {
            unEncSumDate += (uint32_t)encDate[i];
        }
        unLockTmp = nLockNum * unEncSumDate;
        unStartOff = unLockTmp % skey.size();    // Get an offset into key

        unCharPos = unStartOff;  // s.b. somewhere within the key byte position range.
        // Sum key - sums only unStartOff (offset) count bytes in key starting at unStartOff (offset)
        // Wraps to beginning of key to keep summing if hits the end of the key
        for(uint32_t i=0;i<unStartOff;i++, unCharPos++)
        {
            if(unCharPos < skey.size() )
            {
                un32Sum += skey[unCharPos] * nLockNum;
            } else {
                unCharPos = 0;  // Wrap to beginning of key to keep summing
            }
        }
        // un32Sum should be good consistent, wacky value to begin

        // Sum key chars * unLockTmp
        for(uint32_t i=0;i<skey.size();i++)
        {
            un32Sum += skey[i] * unLockTmp;
        }
        *psTmp += " un32Sum (key[...]*unLockTmp):" + QVariant(un32Sum).toString();

        // Encode the sumkey
        std::string encSumKey = encryptDecryptOld(un32Sum, QVariant(un32Sum).toString().toStdString(), skey);
        // Add sum encSumKey
        for(uint32_t i=0;i<encSumKey.size();i++)
        {
            un32Sum += (uint32_t)encSumKey[i] * unLockTmp;
        }
        *psTmp += QString(" encSum:") + encSumKey.c_str();


        // Add sum encDate
        for(uint32_t i=0;i<encDate.size();i++)
        {
            un32Sum += (uint32_t)encDate[i];
        }
        // Add locknum
        un32Sum += (uint32_t)nLockNum;

        *poutEncrypt = QVariant(un32Sum).toString().toStdString();
        *psTmp += QString("  outEncrypt:") + poutEncrypt->c_str();
        if(poutEncrypt->size() > maxLength) {
            *poutEncrypt = poutEncrypt->substr(poutEncrypt->size()-maxLength, maxLength);
        }
        *psTmp += QString(" trunc outEncrypt:") + poutEncrypt->c_str();
    }

    static void calculatePredictiveCode(uint32_t nLockNum, std::string skey, QDateTime datetime, std::string *poutEncrypt, uint32_t maxLength, QString *psTmp)
    {
        uint32_t un32Sum = 0;
        uint32_t unLockTmp = 0;
        uint32_t unEncSumDate = 0;
        uint32_t i;
        uint32_t unStartOff = 0;
        uint32_t unCharPos = 0;

        *psTmp += " Date:" + datetime.toString("yyyy-MM-dd HH:mm");

        // Encrypt and sum the date for an ok multiplier
        std::string sdt = datetime.toString("yyyy-MM-dd HH:mm").toStdString();
        // Encrypt the rounded date as "yyyy-MM-dd HH:mm"
        std::string encDate = encryptString(sdt.c_str()).toStdString();

        *psTmp += QString(" EncDate:") + encDate.c_str();

        // Add sum encDate
        for(i=0;i<encDate.size();i++)
        {
            unEncSumDate += (uint32_t)encDate[i];
        }
        unLockTmp = nLockNum * unEncSumDate;
        unStartOff = unLockTmp % skey.size();    // Get an offset into key

        unCharPos = unStartOff;  // s.b. somewhere within the key byte position range.
        // Sum key - sums only unStartOff (offset) count bytes in key starting at unStartOff (offset)
        // Wraps to beginning of key to keep summing if hits the end of the key
        for(uint32_t i=0;i<unStartOff;i++, unCharPos++)
        {
            if(unCharPos < skey.size() )
            {
                un32Sum += skey[unCharPos] * nLockNum;
            } else {
                unCharPos = 0;  // Wrap to beginning of key to keep summing
            }
        }
        // un32Sum should be good consistent, wacky value to begin

        // Sum key chars * unLockTmp
        for(uint32_t i=0;i<skey.size();i++)
        {
            un32Sum += skey[i] * unLockTmp;
        }
            *psTmp += " un32Sum (key[...]*unLockTmp):" + QVariant(un32Sum).toString();

        // Encode the sumkey
        //        std::string encSumKey = encryptDecrypt(un32Sum, QVariant(un32Sum).toString().toStdString(), skey);
        std::string encSumKey = encryptString(QVariant(un32Sum).toString()).toStdString();
        // Add sum encSumKey
        for(uint32_t i=0;i<encSumKey.size();i++)
        {
            un32Sum += (uint32_t)encSumKey[i] * unLockTmp;
        }
        *psTmp += QString(" encSum:") + encSumKey.c_str();


        // Add sum encDate
        for(uint32_t i=0;i<encDate.size();i++)
        {
            un32Sum += (uint32_t)encDate[i];
        }
        // Add locknum
        un32Sum += (uint32_t)nLockNum;

        *poutEncrypt = QVariant(un32Sum).toString().toStdString();
        *psTmp += QString("  outEncrypt:") + poutEncrypt->c_str();
        if(poutEncrypt->size() > maxLength) {
            *poutEncrypt = poutEncrypt->substr(poutEncrypt->size()-maxLength, maxLength);
        }
        *psTmp += QString(" trunc outEncrypt:") + poutEncrypt->c_str();
    }


};

#endif // CENCRYPTION_H
