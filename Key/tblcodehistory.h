#ifndef CTBLCODEHISTORY_H
#define CTBLCODEHISTORY_H

#include <string>
#include <iostream>
#include <cstdlib>
#include <QtSql>
#include <QtDebug>
#include "lockstate.h"
#include "lockhistoryrec.h"
#include "lockhistoryset.h"

/**
 * @brief The CTblCodes class
 * @table "codes" fields
 *      int ids; // integer primary key unique,
 *      std::string sequence;    // text,\
 *      int sequence_order;  // integer,
 *      int lock_num;    // integer,
 *      std::string  description;    // text,
 *      std::string  code1;  // text,
 *      std::string  code2;  // text,\
 *      datetime     starttime;   // DATETIME,
 *      datetime     endtime; // DATETIME,
 *      std::string  status;  // text,
 *      int  access_count;   // integer,\
 *      int  retry_count; // integer,
 *      int  max_access; // integer,
 *      int  max_retry;  // integer)
 *      datetime    access_time;
 *      bool admin_notification_sent;
 *      std::string email_notification;
 *      bool user_notification_sent;
 */
class CTblCodeHistory
{
    const QString TABLENAME = "codesHistory";

    const char *datetimeFormat = "yyyy-MM-dd HH:mm:ss";
    const char *timeFormatShort = "HH:mm";
    const char *timeFormat = "HH:mm:ss";

    const char *fids = "ids";            // Record id // integer primary key unique, (if -1 then this is a new record)
    const char *fsequence = "sequence";    // Sequence // text,
    const char *fsequence_order = "sequence_order";  // integer,
    const char *flocknum = "locknums";    // text,
    const char *fdescription = "description";    // text,
    const char *fcode1 = "code1";  // text,
    const char *fcode2 = "code2";  // text,
    const char *fstarttime = "starttime";   // DATETIME,
    const char *fendtime = "endtime"; // DATETIME,
    const char *fstatus = "status";  // text,
    const char *faccess_count = "access_count";   // integer,
    const char *fretry_count = "retry_count"; // integer,
    const char *fmax_access = "max_access"; // integer,
    const char *fmax_retry = "max_retry";  // integer)
    const char *faccess_time = "access_time";
    const char *fadmin_notification_sent = "admin_notification_sent";
    const char *femail_notification = "email_notification";
    const char *fuser_notification_sent = "user_notification_sent";

    const char *fmodified = "modified";
    const char *fmarktodelete = "delete";

private:
    QSqlDatabase *_pDB;
    QString _sCodeOne;
    QString _sCodeTwo;

    void createTable();
    bool tableExists();
    void createColumn(QString column, QString fieldType);
    bool columnExists(QString column);
    
    void initialize();

    bool readTestDefault();
    bool createTestDefault();
    bool updateLockCodeHistory(CLockHistoryRec &rec);
public:
    CTblCodeHistory(QSqlDatabase *db) {
        qDebug() << "CTblCodeHistory constructor\n";
        setDatabase(db);
        // make sure the table is created if it does not exist.
        initialize();
    }

    void setDatabase(QSqlDatabase *db) 
    {
        _pDB = db;
    }

    bool addLockCodeHistory(CLockHistoryRec &lockHistoryRec);

    bool addLockCodeHistory(QString locknums, QString code1, QString code2,
                       QDateTime starttime, QDateTime endtime, int maxAccess,
                       QString status, QString desc, QString sequence, int sequenceNum,
                       int maxRetry, QDateTime accesstime,
                       bool adminNotificationSent, QString userNotificationEmail,
                       bool userNotificationSent);

    bool addLockCodeHistoryWithAnswers(CLockHistoryRec &lockHistoryRec, QString answer1, QString answer2, QString answer3);

    bool addLockCodeHistoryWithAnswers(QString locknums, QString code1, QString code2,
                       QDateTime starttime, QDateTime endtime, int maxAccess,
                       QString status, QString desc, QString sequence, int sequenceNum,
                       int maxRetry, QDateTime accesstime,
                       bool adminNotificationSent, QString userNotificationEmail,
				       bool userNotificationSent, QString answer1, QString answer2, QString answer3);

    void currentTimeFormat(QString format, QString strBuffer, int nExpectedLength);

    bool deleteLockCodeHistory(CLockHistoryRec &rec);
    bool deleteLockCodeHistory(QString locknums, QDateTime starttime, QDateTime endtime);

    bool deleteLockCodeHistory(QString locknums, QString code1, QString code2,
                               QDateTime starttime, QDateTime endtime, QDateTime accesstime);
    bool deleteLockCodeHistory(QDateTime accesstime);
    bool deleteLockCodeHistory(QDateTime starttime, QDateTime endtime);

    bool updateRecord(CLockHistoryRec &rec);
    void selectLockCodeHistorySet(QString &LockNums, QDateTime start, QDateTime end, CLockHistorySet **pLockHistorySet);
    bool updateLockCodeHistorySet(CLockHistorySet &lockHistorySet);
    bool updateLockCodeHistorySet(QJsonObject &jsonObj);
    void selectLastLockCodeHistorySet(QString &LockNums, QDateTime &start, QDateTime &end, CLockHistorySet **pLockHistorySet);
};

#endif // CTBLCODEHISTORY_H
