#include <QtGlobal>
#include <QTime>
#include <QDateTime>
#include <QDebug>
#include "tblcodehistory.h"
#include "encryption.h"
#include "lockhistoryrec.h"

/**
 * @brief CTblCodeHistory::selectCodeSet
 * @param code - encrypted
 */
void CTblCodeHistory::selectLockCodeHistorySet(QString &LockNums, QDateTime start, QDateTime end, CLockHistorySet **pLockHistorySet)
{
    qDebug() << "CTblCodes::selectLockCodeHistorySet()";
    // hold on to the code
    CLockHistoryRec  *pLockHistory;
    *pLockHistorySet = 0;

    if( _pDB && _pDB->isOpen() ) 
    {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknums, description, "
                      "code1, code2, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry,"
                      " access_time, admin_notification_sent,"
                      " user_notification_email, user_notification_sent, answer1, answer2, answer3"
                      " from " + TABLENAME +
                      " WHERE ";
        if(LockNums != "" ) 
        {
            sql += " locknums = :lockNums and ";
        }
        sql += "(access_time >= :stime and access_time <= :etime)";

        qDebug() << ">>SQL:" << sql;
        qDebug() << ">>Start:" << start.toString("yyyy-MM-dd HH:mm:ss") << "  End:" << end.toString("yyyy-MM-dd HH:mm:ss");

        if( !qry.prepare(sql) ) 
        {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        if(LockNums != "" ) 
        {
            qry.bindValue(":lockNums", LockNums);
        }
        qry.bindValue(":stime", start.toString("yyyy-MM-dd HH:mm:ss"));
        qry.bindValue(":etime", end.toString("yyyy-MM-dd HH:mm:ss"));

        QMap<QString, QVariant> mapVals = qry.boundValues();
        qDebug() << "Mapped count:" << mapVals.count();
        QMap<QString, QVariant>::Iterator   itor;
        for (itor = mapVals.begin(); itor != mapVals.end(); itor++)
        {
            qDebug() << " : " << (*itor).typeName() << " value:" << (*itor).toString();
        }

        QString sCode1;
        QString sCode2;
        int nCount = 0; //
        // Selection
        if(qry.exec())
        {
            qDebug() << "exec() ok ";
            int fldID = qry.record().indexOf("ids");
            int fldSeq = qry.record().indexOf("sequence");
            int fldSeqOrder = qry.record().indexOf("sequence_order");
            int fldLockNo = qry.record().indexOf("locknums");
            int fldDesc = qry.record().indexOf("description");
            int fldCode1No = qry.record().indexOf("code1");
            int fldCode2No = qry.record().indexOf("code2");
            int fldStart = qry.record().indexOf("starttime");
            int fldEnd = qry.record().indexOf("endtime");
            int fldStatus = qry.record().indexOf("status");
            int fldAccessCount = qry.record().indexOf("access_count");
            int fldRetryCount = qry.record().indexOf("retry_count");
            int fldMaxAccess = qry.record().indexOf("max_access");
            int fldMaxRetry = qry.record().indexOf("max_retry");
            int fldAccessTime = qry.record().indexOf("access_time");
            int fldAdminNotificationSent = qry.record().indexOf("admin_notification_sent");
            int fldUserNotificationEmail = qry.record().indexOf("user_notification_email");
            int fldUserNotificationSent = qry.record().indexOf("user_notification_sent");
            int fldQuestion1 = qry.record().indexOf("answer1");
            int fldQuestion2 = qry.record().indexOf("answer2");
            int fldQuestion3 = qry.record().indexOf("answer3");

            qDebug() << "fldLockNo:" << fldLockNo << "  fldCode1No:" << fldCode1No << "  fldCode2No:" << fldCode2No;
            qDebug() << "Count: " << qry.size();

            *pLockHistorySet = new CLockHistorySet();

            while(qry.next())
            {
                pLockHistory = new CLockHistoryRec();
                // Check how many we have.
                // If only one then see if it requires a second code.
                //   - If second code then signal that the second code is required.
                //   - If no second required then signal Ok to open door
                nCount++;
                sCode1 = qry.value(fldCode1No).toString();
                sCode2 = qry.value(fldCode2No).toString();

                LockNums = qry.value(fldLockNo).toString();

                pLockHistory->setID(qry.value(fldID).toInt());
                pLockHistory->setSequence(qry.value(fldSeq).toString());
                pLockHistory->setSequenceOrder(qry.value(fldSeqOrder).toInt());
                pLockHistory->setLockNums(qry.value(fldLockNo).toString());
                pLockHistory->setDescription(qry.value(fldDesc).toString());
                pLockHistory->setCode1(sCode1);    // unencrypted
                pLockHistory->setCode2(sCode2);    // unencrypted
                pLockHistory->setStartTime(qry.value(fldStart).toDateTime());
                pLockHistory->setEndTime(qry.value(fldEnd).toDateTime());
                pLockHistory->setStatus(qry.value(fldStatus).toString());
                pLockHistory->setAccessCount(qry.value(fldAccessCount).toInt());
                pLockHistory->setRetryCount(qry.value(fldRetryCount).toInt());
                pLockHistory->setMaxAccess(qry.value(fldMaxAccess).toInt());
                pLockHistory->setMaxRetry(qry.value(fldMaxRetry).toInt());

                pLockHistory->setAccessTime(qry.value(fldAccessTime).toDateTime());
                pLockHistory->setAdminNotificationSent(qry.value(fldAdminNotificationSent).toBool());
                pLockHistory->setUserNotificationEmail(qry.value(fldUserNotificationEmail).toString());
                pLockHistory->setUserNotificationSent(qry.value(fldUserNotificationSent).toBool());

                pLockHistory->setQuestion1(qry.value(fldQuestion1).toString());
                pLockHistory->setQuestion2(qry.value(fldQuestion2).toString());
                pLockHistory->setQuestion3(qry.value(fldQuestion3).toString());

                (*pLockHistorySet)->addToSet(*pLockHistory);
            }
        }
        else 
        {
            qDebug() << "query.exec() failed." << qry.lastError();
            Q_ASSERT_X(FALSE, "CTblCodeHistory::selectLockCodeHistorySet", "SQL Query Failed");
        }
    }

    Q_ASSERT_X(*pLockHistorySet != nullptr, "CTblCodeHistory::selectLockCodeHistorySet", "pLockHistorySet is null");
}

/**
 * @brief CTblCodeHistory::selectLastLockCodeHistorySet
 * @param code - encrypted
 */
void CTblCodeHistory::selectLastLockCodeHistorySet(QString &LockNums, QDateTime &start, QDateTime &end, CLockHistorySet **pLockHistorySet)
{
    qDebug() << "CTblCodes::selectLastLockCodeHistorySet(..)";
    // hold on to the code
    CLockHistoryRec  *pLockHistory = NULL;
    *pLockHistorySet = 0;

    if( _pDB && _pDB->isOpen() ) {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknums, description, "
                      "code1, code2, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry,"
                      " access_time, admin_notification_sent,"
                      " user_notification_email, user_notification_sent, answer1, answer2, answer3"
                      " from " + TABLENAME +
                      " WHERE ";
        if(LockNums != "" ) 
        {
            sql += " locknums = :lockNum and ";
        }
        sql += "(access_time >= :stime and access_time <= :etime)";
        sql += " ORDER BY access_time";

        qDebug() << ">>SQL:" << sql;
        qDebug() << ">>Start:" << start.toString("yyyy-MM-dd HH:mm:ss") << "  End:" << end.toString("yyyy-MM-dd HH:mm:ss");

        if( !qry.prepare(sql) ) {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        if(LockNums != "" ) {
            qry.bindValue(":lockNums", LockNums);
        }
        qry.bindValue(":stime", start.toString("yyyy-MM-dd HH:mm:ss"));
        qry.bindValue(":etime", end.toString("yyyy-MM-dd HH:mm:ss"));

        QMap<QString, QVariant> mapVals = qry.boundValues();
        qDebug() << "Mapped count:" << mapVals.count();
        QMap<QString, QVariant>::Iterator   itor;
        for (itor = mapVals.begin(); itor != mapVals.end(); itor++)
        {
            qDebug() << " : " << (*itor).typeName() << " value:" << (*itor).toString();
        }

        QString sCode1;
        QString sCode2;
        int nCount = 0; //
        // Selection
        if(qry.exec())
        {
            qDebug() << "exec() ok ";
            int fldID = qry.record().indexOf("ids");
            int fldSeq = qry.record().indexOf("sequence");
            int fldSeqOrder = qry.record().indexOf("sequence_order");
            int fldLockNo = qry.record().indexOf("locknums");
            int fldDesc = qry.record().indexOf("description");
            int fldCode1No = qry.record().indexOf("code1");
            int fldCode2No = qry.record().indexOf("code2");
            int fldStart = qry.record().indexOf("starttime");
            int fldEnd = qry.record().indexOf("endtime");
            int fldStatus = qry.record().indexOf("status");
            int fldAccessCount = qry.record().indexOf("access_count");
            int fldRetryCount = qry.record().indexOf("retry_count");
            int fldMaxAccess = qry.record().indexOf("max_access");
            int fldMaxRetry = qry.record().indexOf("max_retry");
            int fldAccessTime = qry.record().indexOf("access_time");
            int fldAdminNotificationSent = qry.record().indexOf("admin_notification_sent");
            int fldUserNotificationEmail = qry.record().indexOf("user_notification_email");
            int fldUserNotificationSent = qry.record().indexOf("user_notification_sent");
            int fldQuestion1 = qry.record().indexOf("answer1");
            int fldQuestion2 = qry.record().indexOf("answer2");
            int fldQuestion3 = qry.record().indexOf("answer3");

            qDebug() << "fldLockNo:" << fldLockNo << "  fldCode1No:" << fldCode1No << "  fldCode2No:" << fldCode2No;
            qDebug() << "Count: " << qry.size();

            *pLockHistorySet = new CLockHistorySet();

            while(qry.next())
            {
                pLockHistory = new CLockHistoryRec();
                // Check how many we have.
                // If only one then see if it requires a second code.
                //   - If second code then signal that the second code is required.
                //   - If no second required then signal Ok to open door
                nCount++;
                sCode1 = qry.value(fldCode1No).toString();
                sCode2 = qry.value(fldCode2No).toString();
                sCode1 = CEncryption::decryptString(sCode1);
                sCode2 = CEncryption::decryptString(sCode2);

                LockNums = qry.value(fldLockNo).toString();

                pLockHistory->setID(qry.value(fldID).toInt());
                pLockHistory->setSequence(qry.value(fldSeq).toString());
                pLockHistory->setSequenceOrder(qry.value(fldSeqOrder).toInt());
                pLockHistory->setLockNums(qry.value(fldLockNo).toString());
                pLockHistory->setDescription(qry.value(fldDesc).toString());
                pLockHistory->setCode1(sCode1);    // unencrypted
                pLockHistory->setCode2(sCode2);    // unencrypted
                pLockHistory->setStartTime(qry.value(fldStart).toDateTime());
                pLockHistory->setEndTime(qry.value(fldEnd).toDateTime());
                pLockHistory->setStatus(qry.value(fldStatus).toString());
                pLockHistory->setAccessCount(qry.value(fldAccessCount).toInt());
                pLockHistory->setRetryCount(qry.value(fldRetryCount).toInt());
                pLockHistory->setMaxAccess(qry.value(fldMaxAccess).toInt());
                pLockHistory->setMaxRetry(qry.value(fldMaxRetry).toInt());

                pLockHistory->setAccessTime(qry.value(fldAccessTime).toDateTime());
                pLockHistory->setAdminNotificationSent(qry.value(fldAdminNotificationSent).toBool());
                pLockHistory->setUserNotificationEmail(qry.value(fldUserNotificationEmail).toString());
                pLockHistory->setUserNotificationSent(qry.value(fldUserNotificationSent).toBool());

                pLockHistory->setQuestion1(qry.value(fldQuestion1).toString());
                pLockHistory->setQuestion2(qry.value(fldQuestion2).toString());
                pLockHistory->setQuestion3(qry.value(fldQuestion3).toString());
            }

            if( pLockHistory ) 
            {
                (*pLockHistorySet)->addToSet(*pLockHistory);
            }
        }
        else {
            qDebug() << "query.exec() failed." << qry.lastError();
        }
    }
}


bool CTblCodeHistory::tableExists()
{
    QStringList tables = _pDB->tables();

    foreach (auto table, tables)
    {
        if (table == TABLENAME)
        {
            return true;
        }
    }

    return false;
}

bool CTblCodeHistory::columnExists(QString column)
{
    qDebug() << "CTblCodeHistory::columnExists()";
    QStringList::iterator  itor;
    QSqlQuery qry(*_pDB);
    bool foundColumn = false;

    if( _pDB && _pDB->isOpen() )
    {
        QString sql("PRAGMA TABLE_INFO(codes);");

        qry.prepare( sql );

        if( qry.exec() )
        {
            while( qry.next() )
            {
                if( qry.value(1).toString().compare(column) == 0 )
                {
                    qDebug() << "CTblCodeHistory::columnExists(), found column: " << column;

                    foundColumn = true;
                    break;
                }
            }
        }
        else
            qDebug() << qry.lastError();

    } else {
        std::cout << "Either _pDB is NULL or _pDB is not open\n";
    }
    return foundColumn;
}

void CTblCodeHistory::createColumn(QString column, QString fieldType)
{
    qDebug() << "CTblCodeHistory::createColumn\n";
    if( _pDB && _pDB->isOpen() ) {
        std::cout << "Creating table \n";
        QSqlQuery qry(*_pDB);

        QString sql("ALTER TABLE  ");
        sql += TABLENAME;
        sql += " ADD ";
        sql += column;
        sql += " ";
        sql += fieldType;

        qry.prepare( sql );

        if( !qry.exec() )
            qDebug() << qry.lastError();
        else
            qDebug() << "Table altered!";
    } else {
        std::cout << "Either _pDB is NULL or _pDB is not open\n";
    }
}

void CTblCodeHistory::initialize()
{
    if(!tableExists())
    {
        qDebug() << "Table does not Exist";
        createTable();
    }
    if(!tableExists())
    {
        qDebug() << "Failed to cretae CodeHistory table.";
    }

    QString column1 = "answer1";
    QString column2 = "answer2";
    QString column3 = "answer3";

    if(!columnExists(column1))
        createColumn(column1, "text");
    if(!columnExists(column2))
        createColumn(column2, "text");
    if(!columnExists(column3))
        createColumn(column3, "text");
}

void CTblCodeHistory::createTable()
{
    std::cout << "CTblCodeHistory::createTable\n";
    if( _pDB && _pDB->isOpen() ) {
        std::cout << "Creating table \n";
        QSqlQuery qry(*_pDB);

        QString sql("CREATE TABLE IF NOT EXISTS ");
        sql += TABLENAME;
        sql += "(ids integer primary key unique, sequence text,"
               "sequence_order integer, locknums integer, description text, "
               "code1 text, code2 text,"
               " starttime DATETIME, endtime DATETIME, status text, access_count integer,"
               " retry_count integer, max_access integer, max_retry integer,"
               " access_time DATETIME, admin_notification_sent BOOL,"
                " user_notification_email text, user_notification_sent BOOL)";

        qry.prepare( sql );

        if( !qry.exec() )
            qDebug() << qry.lastError();
        else
            qDebug() << "Table created!";
    } else {
        std::cout << "Either _pDB is NULL or _pDB is not open\n";
    }

}

bool CTblCodeHistory::addLockCodeHistory(CLockHistoryRec &lockHistoryRec)
{
    return addLockCodeHistory(lockHistoryRec.getLockNums(), lockHistoryRec.getCode1(), lockHistoryRec.getCode2(),
                              lockHistoryRec.getStartTime(), lockHistoryRec.getEndTime(), lockHistoryRec.getMaxAccess(),
                              lockHistoryRec.getStatus(), lockHistoryRec.getDescription(),
                              lockHistoryRec.getSequence(), lockHistoryRec.getSequenceOrder(),
                              lockHistoryRec.getMaxRetry(), lockHistoryRec.getAccessTime(),
                              lockHistoryRec.getAdminNotificationSent(), lockHistoryRec.getUserNotificationEmail(),
                              lockHistoryRec.getUserNotificationSent());
}

/**
 * @brief CTblCodeHistory::addLockCodeHistory
 * @param locknum
 * @param code1
 * @param code2
 * @param starttime
 * @param endtime
 * @param status
 * @param desc
 * @param sequence
 * @param sequenceNum
 * @param maxAccess
 * @param maxRetry
 * @return
 */
bool CTblCodeHistory::addLockCodeHistory(QString locknums, QString code1, QString code2,
                                         QDateTime starttime, QDateTime endtime, int maxAccess,
                                         QString status, QString desc, QString sequence, int sequenceNum,
                                         int maxRetry, QDateTime accesstime,
                                         bool adminNotificationSent, QString userNotificationEmail,
                                         bool userNotificationSent)
{
    qDebug() << "CTblCodeHistory::addLockCodeHistory()";

    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + TABLENAME +
                QString(" (sequence, sequence_order, "
                        "locknums, description, code1, "
                        "code2, starttime, endtime, status, access_count,"
                        "retry_count, max_access, max_retry,"
                        "access_time, admin_notification_sent,"
                        "user_notification_email, user_notification_sent)"
                        " VALUES (:seqDesc, :seqOrder, :lockNum, :desc, :codeOne, "
                        " :codeTwo, "
                        " :start, :end, :stat, 0, 0, :maxAccess, :maxRetry, "
                        " :accessTime, :adminNotificationSent, "
                        " :userEmail, :userNotificationSent)" ));

    qDebug() << "Query:" << qry.lastQuery();

    qry.bindValue(":seqDesc", sequence);
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNums", locknums);
    qry.bindValue(":desc", desc);
    qry.bindValue(":codeOne", code1);
    qry.bindValue(":codeTwo", code2);
    qry.bindValue(":start", starttime.toString(datetimeFormat));
    qry.bindValue(":end", endtime.toString(datetimeFormat));
    qry.bindValue(":stat", status);
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessTime", accesstime.toString(datetimeFormat));
    qry.bindValue(":adminNotificationSent", QVariant(adminNotificationSent));
    qry.bindValue(":userEmail", userNotificationEmail);
    qry.bindValue(":userNotificationSent", QVariant(userNotificationSent));

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) {
        qDebug() << "CTblCodeHistory::addLockCodeHistory():" << qry.lastError();
        qDebug() << "Query After:" << qry.lastQuery();
        return false;
    }
    else {
        qDebug( "Inserted!" );
        return true;
    }
}

bool CTblCodeHistory::addLockCodeHistoryWithAnswers(CLockHistoryRec &lockHistoryRec, QString answer1, QString answer2, QString answer3)
{
    return addLockCodeHistoryWithAnswers(lockHistoryRec.getLockNums(), lockHistoryRec.getCode1(), lockHistoryRec.getCode2(),
                                         lockHistoryRec.getStartTime(), lockHistoryRec.getEndTime(), lockHistoryRec.getMaxAccess(),
                                         lockHistoryRec.getStatus(), lockHistoryRec.getDescription(),
                                         lockHistoryRec.getSequence(), lockHistoryRec.getSequenceOrder(),
                                         lockHistoryRec.getMaxRetry(), lockHistoryRec.getAccessTime(),
                                         lockHistoryRec.getAdminNotificationSent(), lockHistoryRec.getUserNotificationEmail(),
                                         lockHistoryRec.getUserNotificationSent(), answer1, answer2, answer3);
}

bool CTblCodeHistory::addLockCodeHistoryWithAnswers(QString locknums, QString code1, QString code2,
                                                    QDateTime starttime, QDateTime endtime, int maxAccess,
                                                    QString status, QString desc, QString sequence, int sequenceNum,
                                                    int maxRetry, QDateTime accesstime,
                                                    bool adminNotificationSent, QString userNotificationEmail,
                                                    bool userNotificationSent, QString answer1, QString answer2, QString answer3)
{
    qDebug() << "CTblCodeHistory::addLockCodeHistory()";

    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + TABLENAME +
                QString(" (sequence, sequence_order, "
                        "locknums, description, code1, "
                        "code2, starttime, endtime, status, access_count,"
                        "retry_count, max_access, max_retry,"
                        "access_time, admin_notification_sent,"
                        "user_notification_email, user_notification_sent,"
                        "answer1, answer2, answer3)"
                        " VALUES (:seqDesc, :seqOrder, :lockNum, :desc, :codeOne, "
                        " :codeTwo, "
                        " :start, :end, :stat, 0, 0, :maxAccess, :maxRetry, "
                        " :accessTime, :adminNotificationSent, "
                        " :userEmail, :userNotificationSent, "
                        " :answer1, :answer2, :answer3)" ));

    qDebug() << "Query:" << qry.lastQuery();

    qry.bindValue(":seqDesc", sequence);
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNums", locknums);
    qry.bindValue(":desc", desc);
    qry.bindValue(":codeOne", code1);
    qry.bindValue(":codeTwo", code2);
    qry.bindValue(":start", starttime.toString(datetimeFormat));
    qry.bindValue(":end", endtime.toString(datetimeFormat));
    qry.bindValue(":stat", status);
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessTime", accesstime.toString(datetimeFormat));
    qry.bindValue(":adminNotificationSent", QVariant(adminNotificationSent));
    qry.bindValue(":userEmail", userNotificationEmail);
    qry.bindValue(":userNotificationSent", QVariant(userNotificationSent));
    qry.bindValue(":answer1", answer1);
    qry.bindValue(":answer2", answer2);
    qry.bindValue(":answer3", answer3);

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) 
    {
        qDebug() << "CTblCodeHistory::addLockCodeHistory():" << qry.lastError();
        qDebug() << "Query After:" << qry.lastQuery();
        return false;
    }
    else 
    {
        qDebug( "Inserted!" );
        return true;
    }
}

/**
 * @brief CTblCodeHistory::currentTimeFormat
 * @param format
 * @param strBuffer
 * @param nExpectedLength = expected length of the return string
 */
void CTblCodeHistory::currentTimeFormat(QString format, QString strBuffer, int nExpectedLength)
{
    time_t rawtime;
    struct tm *currentTime;
    time ( &rawtime );
    currentTime = gmtime( &rawtime );
    char buffer [nExpectedLength+1];

    strftime(buffer, nExpectedLength, format.toStdString().c_str(), currentTime);

    strBuffer = QString::fromStdString(buffer);
}

bool CTblCodeHistory::readTestDefault()
{
    qDebug( )<< "CTblCodeHistory::readTestDefault()";

    QSqlQuery query(*_pDB);
    QString sql = "SELECT sequence, sequence_order, "\
                  "locknums, description, code1,"\
                  "code2, starttime, endtime, status, access_count,"\
                  "retry_count, max_access, max_retry,"
                  "access_time, admin_notification_sent,"
                  "user_notification_email, user_notification_sent"
                  " FROM ";
    sql += TABLENAME;
    sql += QString(" WHERE description = 'test history lock'");

    if( query.exec(sql)) {
        int fldCode1 = query.record().indexOf("code1");
        int fldCode2 = query.record().indexOf("code2");
        int fldStart = query.record().indexOf("starttime");
        int fldEnd = query.record().indexOf("endtime");

        if (query.next())
        {
            // it exists
            struct tm tm;

            QString code1 = query.value(fldCode1).toString();
            QString code2 = query.value(fldCode2).toString();

            strptime(query.value(fldStart).toDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str(), "yyyy-MM-dd %H:%M:%S", &tm);
            strptime(query.value(fldEnd).toDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str(), "yyyy-MM-dd %H:%M:%S", &tm);

            qDebug() << "CTblCodeHistory::readTestDefault(): Code1:" << code1 << " len:" << code1.size();
            qDebug() << "CTblCodeHistory::readTestDefault(): Code2:" << code2 << " len:" << code2.size();

            return true;
        }
    }
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(QString locknums, QDateTime starttime, QDateTime endtime)
{
    Q_UNUSED(locknums);
    Q_UNUSED(starttime);
    Q_UNUSED(endtime);
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(QString locknums, QString code1, QString code2,
                                            QDateTime starttime, QDateTime endtime, QDateTime accesstime)
{
    Q_UNUSED(locknums);
    Q_UNUSED(code1);
    Q_UNUSED(code2);
    Q_UNUSED(starttime);
    Q_UNUSED(endtime);
    Q_UNUSED(accesstime);
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(QDateTime accesstime)
{   
    Q_UNUSED(accesstime);
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(QDateTime starttime, QDateTime endtime)
{
    Q_UNUSED(starttime);
    Q_UNUSED(endtime);
    return false;
}

/**
 * @brief CTblCodeHistory::deleteCode
 *  For now will just delete based on the ids field
 * @param rec
 * @return
 */
bool CTblCodeHistory::deleteLockCodeHistory(CLockHistoryRec &rec)
{
    qDebug( )<< "CTblCodeHistory::deleteCode(CLockState)";

    if( rec.getID() == -1 ) {
        return false;
    }

    QSqlQuery query(*_pDB);
    QString sql;
    if(rec.getID() == -1 ) {
        sql = "DELETE FROM " + TABLENAME +
                QString(" WHERE access_time = :faccessTime and "
                        " locknums=:lockNums");
        query.prepare(sql);
        query.bindValue("faccessTime", rec.getAccessTime().toString(datetimeFormat));
        query.bindValue(":locknums", rec.getLockNums());
    } else {
        sql = "DELETE FROM " + TABLENAME +
                QString(" WHERE ids = :fids");
        query.prepare(sql);
        query.bindValue("fids", rec.getID());
    }

    if( query.exec(sql)) {
        return true;
    } else {
        return false;
    }
}

bool CTblCodeHistory::updateRecord(CLockHistoryRec &rec)
{
    qDebug() << "CTblCodeHistory::updateRecord()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("sequence=:seqDesc, sequence_order=:seqOrder, "
                              "locknums=:lockNums, description=:desc, code1=:codeOne, "
                              "code2=:codeTwo, starttime=:start, endtime=:end, status=:stat, access_count=:accessCount,"
                              "retry_count=:retryCount, max_access=:maxAccess, max_retry=:maxRetry,"
                              "access_time=:accessTime, admin_notification_sent=:adminNotificationSent,"
                              "user_notification_email=:userNotificationEmail, user_notification_sent=:userNotificationSent"
                              " WHERE ids=:fids");

    qry.prepare(sql);

    qry.bindValue(":seqDesc", rec.getSequence());
    qry.bindValue(":seqOrder", rec.getSequenceOrder());
    qry.bindValue(":lockNums", rec.getLockNums());
    qry.bindValue(":desc", rec.getDescription());
    qry.bindValue(":codeOne", rec.getCode1());
    qry.bindValue(":codeTwo", rec.getCode2());
    qry.bindValue(":start", rec.getStartTime().toString(datetimeFormat));
    qry.bindValue(":end", rec.getEndTime().toString(datetimeFormat));
    qry.bindValue(":stat", rec.getStatus());
    qry.bindValue(":maxAccess", rec.getMaxAccess());
    qry.bindValue(":maxRetry", rec.getMaxRetry());
    qry.bindValue(":accessTime", rec.getAccessTime().toString((datetimeFormat)) );
    qry.bindValue(":adminNotificationSent", rec.getAdminNotificationSent() );
    qry.bindValue(":userNotificationEmail", rec.getUserNotificationEmail());
    qry.bindValue(":userNotificationSent", rec.getUserNotificationSent() );

    qry.bindValue(":fids", rec.getID());

    if(qry.exec()) {
        return true;
    } else {
        qDebug() << "CTblCodeHistory::updateRecord() failed";
        return false;
    }
}

bool CTblCodeHistory::updateLockCodeHistory(CLockHistoryRec &rec)
{
    Q_UNUSED(rec);
    return false;
}

/**
 * @brief CTblCodeHistory::updateCodeSet
 * @param codeSet
 * @return true if all were updated successfully, false if any fail (note: some may have updated if false is returned BAD!)
 */
bool CTblCodeHistory::updateLockCodeHistorySet(CLockHistorySet &lockHistorySet)
{
    bool    bRC = true;

    _pDB->transaction();

    CLockHistorySet::Iterator  itor;
    // walk the set and update as we go
    for(itor = lockHistorySet.begin(); itor != lockHistorySet.end(); itor++)
    {
        bRC = updateRecord(*(itor.value()));
    }

    if( !bRC ) {
        qDebug() << "CTbleCodes::updateCodeSet() failed!";
        _pDB->rollback();
    } else {
        qDebug() << "CTbleCodes::updateCodeSet() succeeded. Committing...";
        if( !_pDB->commit() )
        {
            qDebug() << "CTbleCodes::updateCodeSet() committed successfully.";
        }
    }
    return bRC;
}

bool CTblCodeHistory::updateLockCodeHistorySet(QJsonObject &jsonObj)
{
    CLockHistorySet    lockHistorySet;
    if(!lockHistorySet.setFromJsonObject(jsonObj))
    {
        qDebug() << "CTblCodeHistory::updateCodes(): invalid JSON Object Codeset";
    }
    // Valid set
    return updateLockCodeHistorySet(lockHistorySet);
}
