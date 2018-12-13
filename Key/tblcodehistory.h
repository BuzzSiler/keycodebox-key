#ifndef CTBLCODEHISTORY_H
#define CTBLCODEHISTORY_H

#include <QtSql>
#include <QtDebug>
#include <QByteArray>
#include "lockstate.h"
#include "lockhistoryrec.h"
#include "lockhistoryset.h"

class CTblCodeHistory
{
    const QString TABLENAME = "codesHistory";

    public:
        CTblCodeHistory(QSqlDatabase *db);

        bool addLockCodeHistory(CLockHistoryRec &lockHistoryRec);


        bool addLockCodeHistoryWithAnswers(CLockHistoryRec &lockHistoryRec, QString answer1, QString answer2, QString answer3);


        void currentTimeFormat(QString format, QString strBuffer, int nExpectedLength);

        bool deleteLockCodeHistory(CLockHistoryRec &rec);

        bool updateRecord(CLockHistoryRec &rec);
        void selectLockCodeHistorySet(QString LockNums, QDateTime start, QDateTime end, CLockHistorySet **pLockHistorySet);
        bool updateLockCodeHistorySet(CLockHistorySet &lockHistorySet);
        bool updateLockCodeHistorySet(QJsonObject &jsonObj);
        void selectLastLockCodeHistorySet(QString &LockNums, QDateTime &start, QDateTime &end, CLockHistorySet **pLockHistorySet);


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

        QSqlQuery createQuery(QStringList column_list,
                            QString table, 
                            // Note: While table is a constant in this class, it is envisioned
                            // a common class/module will exist that can handle all SQL query
                            // creation and execution.  This is just a reminder of what's is
                            // planned.
                            QString condition);

        void execSelectCodeHistorySetQuery(QSqlQuery& qry, CLockHistorySet **pLockHistorySet);

        bool addLockCodeHistory(QString locknums, QString code1, QString code2,
                        QString accessSelection,
                        QDateTime starttime, QDateTime endtime, int maxAccess,
                        QString status, QString desc, QString sequence, int sequenceNum,
                        int maxRetry, QDateTime accesstime,
                        bool adminNotificationSent, QString userNotificationEmail,
                        bool userNotificationSent, QByteArray image);
        bool addLockCodeHistoryWithAnswers(QString locknums, QString code1, QString code2,
                        QString accessSelection,
                        QDateTime starttime, QDateTime endtime, int maxAccess,
                        QString status, QString desc, QString sequence, int sequenceNum,
                        int maxRetry, QDateTime accesstime,
                        bool adminNotificationSent, QString userNotificationEmail,
                        bool userNotificationSent, QString answer1, QString answer2, QString answer3, QByteArray image);
};

#endif // CTBLCODEHISTORY_H
