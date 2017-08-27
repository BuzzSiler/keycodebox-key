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
void CTblCodeHistory::selectLockCodeHistorySet(int &nLockNum, QDateTime start, QDateTime end, CLockHistorySet **pLockHistorySet)
{
    qDebug() << "CTblCodes::selectLockCodeHistorySet()";
    // hold on to the code
    CLockHistoryRec  *pLockHistory;
    *pLockHistorySet = 0;

    if( _pDB && _pDB->isOpen() ) {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknum, description, "
                      "code1, code2, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry,"
                      " access_time, admin_notification_sent,"
                      " user_notification_email, user_notification_sent, answer1, answer2, answer3"
                      " from " + QString(TABLENAME.c_str()) +
                      " WHERE ";
        if(nLockNum != -1 ) {
            sql += " locknum = :lockNum and ";
        }
        sql += "(access_time >= :stime and access_time <= :etime)";

        qDebug() << ">>SQL:" << sql;
        qDebug() << ">>Start:" << start.toString("yyyy-MM-dd HH:mm:ss") << "  End:" << end.toString("yyyy-MM-dd HH:mm:ss");

        if( !qry.prepare(sql) ) {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        if(nLockNum != -1 ) {
            qry.bindValue(":lockNum", nLockNum);
        }
        qry.bindValue(":stime", start.toString("yyyy-MM-dd HH:mm:ss"));   // .toStdString().c_str());
        qry.bindValue(":etime", end.toString("yyyy-MM-dd HH:mm:ss"));    //.toStdString().c_str());

        QMap<QString, QVariant> mapVals = qry.boundValues();
        qDebug() << "Mapped count:" << mapVals.count();
        QMap<QString, QVariant>::Iterator   itor;
        for (itor = mapVals.begin(); itor != mapVals.end(); itor++)
        {
            qDebug() << " : " << (*itor).typeName() << " value:" << (*itor).toString();
        }

        std::string sCode1;
        std::string sCode2;
        int nCount = 0; //
        // Selection
        if(qry.exec())
        {
            qDebug() << "exec() ok ";
            int fldID = qry.record().indexOf("ids");
            int fldSeq = qry.record().indexOf("sequence");
            int fldSeqOrder = qry.record().indexOf("sequence_order");
            int fldLockNo = qry.record().indexOf("locknum");
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
                sCode1 = qry.value(fldCode1No).toString().toStdString();
                sCode2 = qry.value(fldCode2No).toString().toStdString();

                nLockNum = qry.value(fldLockNo).toInt();

                pLockHistory->setID(qry.value(fldID).toInt());
                pLockHistory->setSequence(qry.value(fldSeq).toString().toStdString());
                pLockHistory->setSequenceOrder(qry.value(fldSeqOrder).toInt());
                pLockHistory->setLockNum(qry.value(fldLockNo).toInt());
                pLockHistory->setDescription(qry.value(fldDesc).toString().toStdString());
                pLockHistory->setCode1(sCode1);    // unencrypted
                pLockHistory->setCode2(sCode2);    // unencrypted
                pLockHistory->setStartTime(qry.value(fldStart).toDateTime());
                pLockHistory->setEndTime(qry.value(fldEnd).toDateTime());
                pLockHistory->setStatus(qry.value(fldStatus).toString().toStdString());
                pLockHistory->setAccessCount(qry.value(fldAccessCount).toInt());
                pLockHistory->setRetryCount(qry.value(fldRetryCount).toInt());
                pLockHistory->setMaxAccess(qry.value(fldMaxAccess).toInt());
                pLockHistory->setMaxRetry(qry.value(fldMaxRetry).toInt());

                pLockHistory->setAccessTime(qry.value(fldAccessTime).toDateTime());
                pLockHistory->setAdminNotificationSent(qry.value(fldAdminNotificationSent).toBool());
                pLockHistory->setUserNotificationEmail(qry.value(fldUserNotificationEmail).toString().toStdString());
                pLockHistory->setUserNotificationSent(qry.value(fldUserNotificationSent).toBool());

                pLockHistory->setQuestion1(qry.value(fldQuestion1).toString().toStdString());
                pLockHistory->setQuestion2(qry.value(fldQuestion2).toString().toStdString());
                pLockHistory->setQuestion3(qry.value(fldQuestion3).toString().toStdString());

                (*pLockHistorySet)->addToSet(*pLockHistory);
            }
        }
        else {
            qDebug() << "query.exec() failed." << qry.lastError();
        }
    }
}

/**
 * @brief CTblCodeHistory::selectLastLockCodeHistorySet
 * @param code - encrypted
 */
void CTblCodeHistory::selectLastLockCodeHistorySet(int &nLockNum, QDateTime &start, QDateTime &end, CLockHistorySet **pLockHistorySet)
{
    qDebug() << "CTblCodes::selectLastLockCodeHistorySet(..)";
    // hold on to the code
    CLockHistoryRec  *pLockHistory = NULL;
    *pLockHistorySet = 0;

    if( _pDB && _pDB->isOpen() ) {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknum, description, "
                      "code1, code2, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry,"
                      " access_time, admin_notification_sent,"
                      " user_notification_email, user_notification_sent, answer1, answer2, answer3"
                      " from " + QString(TABLENAME.c_str()) +
                      " WHERE ";
        if(nLockNum != -1 ) {
            sql += " locknum = :lockNum and ";
        }
        sql += "(access_time >= :stime and access_time <= :etime)";
        sql += " ORDER BY access_time";

        qDebug() << ">>SQL:" << sql;
        qDebug() << ">>Start:" << start.toString("yyyy-MM-dd HH:mm:ss") << "  End:" << end.toString("yyyy-MM-dd HH:mm:ss");

        if( !qry.prepare(sql) ) {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        if(nLockNum != -1 ) {
            qry.bindValue(":lockNum", nLockNum);
        }
        qry.bindValue(":stime", start.toString("yyyy-MM-dd HH:mm:ss"));   // .toStdString().c_str());
        qry.bindValue(":etime", end.toString("yyyy-MM-dd HH:mm:ss"));    //.toStdString().c_str());

        QMap<QString, QVariant> mapVals = qry.boundValues();
        qDebug() << "Mapped count:" << mapVals.count();
        QMap<QString, QVariant>::Iterator   itor;
        for (itor = mapVals.begin(); itor != mapVals.end(); itor++)
        {
            qDebug() << " : " << (*itor).typeName() << " value:" << (*itor).toString();
        }

        std::string sCode1;
        std::string sCode2;
        int nCount = 0; //
        // Selection
        if(qry.exec())
        {
            qDebug() << "exec() ok ";
            int fldID = qry.record().indexOf("ids");
            int fldSeq = qry.record().indexOf("sequence");
            int fldSeqOrder = qry.record().indexOf("sequence_order");
            int fldLockNo = qry.record().indexOf("locknum");
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
                sCode1 = qry.value(fldCode1No).toString().toStdString();
                sCode2 = qry.value(fldCode2No).toString().toStdString();
                sCode1 = CEncryption::decryptString(sCode1.c_str()).toStdString();
                sCode2 = CEncryption::decryptString(sCode2.c_str()).toStdString();

                nLockNum = qry.value(fldLockNo).toInt();

                pLockHistory->setID(qry.value(fldID).toInt());
                pLockHistory->setSequence(qry.value(fldSeq).toString().toStdString());
                pLockHistory->setSequenceOrder(qry.value(fldSeqOrder).toInt());
                pLockHistory->setLockNum(qry.value(fldLockNo).toInt());
                pLockHistory->setDescription(qry.value(fldDesc).toString().toStdString());
                pLockHistory->setCode1(sCode1);    // unencrypted
                pLockHistory->setCode2(sCode2);    // unencrypted
                pLockHistory->setStartTime(qry.value(fldStart).toDateTime());
                pLockHistory->setEndTime(qry.value(fldEnd).toDateTime());
                pLockHistory->setStatus(qry.value(fldStatus).toString().toStdString());
                pLockHistory->setAccessCount(qry.value(fldAccessCount).toInt());
                pLockHistory->setRetryCount(qry.value(fldRetryCount).toInt());
                pLockHistory->setMaxAccess(qry.value(fldMaxAccess).toInt());
                pLockHistory->setMaxRetry(qry.value(fldMaxRetry).toInt());

                pLockHistory->setAccessTime(qry.value(fldAccessTime).toDateTime());
                pLockHistory->setAdminNotificationSent(qry.value(fldAdminNotificationSent).toBool());
                pLockHistory->setUserNotificationEmail(qry.value(fldUserNotificationEmail).toString().toStdString());
                pLockHistory->setUserNotificationSent(qry.value(fldUserNotificationSent).toBool());

                pLockHistory->setQuestion1(qry.value(fldQuestion1).toString().toStdString());
                pLockHistory->setQuestion2(qry.value(fldQuestion2).toString().toStdString());
                pLockHistory->setQuestion3(qry.value(fldQuestion3).toString().toStdString());
            }
            if( pLockHistory ) {
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
    QStringList lstTables = _pDB->tables();
    QStringList::iterator  itor;

    for(itor = lstTables.begin(); itor != lstTables.end(); itor++)
    {
        if((*itor).toStdString() == TABLENAME) {
            //
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
        sql += QString(TABLENAME.c_str());
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
        sql += QString(TABLENAME.c_str());
        sql += "(ids integer primary key unique, sequence text,"
               "sequence_order integer, locknum integer, description text, "
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
    return addLockCodeHistory(lockHistoryRec.getLockNum(), lockHistoryRec.getCode1(), lockHistoryRec.getCode2(),
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
bool CTblCodeHistory::addLockCodeHistory(int locknum, std::string code1, std::string code2,
                                         QDateTime starttime, QDateTime endtime, int maxAccess,
                                         std::string status, std::string desc, std::string sequence, int sequenceNum,
                                         int maxRetry, QDateTime accesstime,
                                         bool adminNotificationSent, std::string userNotificationEmail,
                                         bool userNotificationSent)
{
    qDebug() << "CTblCodeHistory::addLockCodeHistory()";

    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + QString(TABLENAME.c_str()) +
                QString(" (sequence, sequence_order, "
                        "locknum, description, code1, "
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

    qry.bindValue(":seqDesc", sequence.c_str());
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNum", locknum);
    qry.bindValue(":desc", desc.c_str());
    qry.bindValue(":codeOne", code1.c_str());
    qry.bindValue(":codeTwo", code2.c_str());
    qry.bindValue(":start", starttime.toString(datetimeFormat));
    qry.bindValue(":end", endtime.toString(datetimeFormat));
    qry.bindValue(":stat", status.c_str());
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessTime", accesstime.toString(datetimeFormat));
    qry.bindValue(":adminNotificationSent", QVariant(adminNotificationSent));
    qry.bindValue(":userEmail", userNotificationEmail.c_str());
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
    return addLockCodeHistoryWithAnswers(lockHistoryRec.getLockNum(), lockHistoryRec.getCode1(), lockHistoryRec.getCode2(),
                                         lockHistoryRec.getStartTime(), lockHistoryRec.getEndTime(), lockHistoryRec.getMaxAccess(),
                                         lockHistoryRec.getStatus(), lockHistoryRec.getDescription(),
                                         lockHistoryRec.getSequence(), lockHistoryRec.getSequenceOrder(),
                                         lockHistoryRec.getMaxRetry(), lockHistoryRec.getAccessTime(),
                                         lockHistoryRec.getAdminNotificationSent(), lockHistoryRec.getUserNotificationEmail(),
                                         lockHistoryRec.getUserNotificationSent(), answer1, answer2, answer3);
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
bool CTblCodeHistory::addLockCodeHistoryWithAnswers(int locknum, std::string code1, std::string code2,
                                                    QDateTime starttime, QDateTime endtime, int maxAccess,
                                                    std::string status, std::string desc, std::string sequence, int sequenceNum,
                                                    int maxRetry, QDateTime accesstime,
                                                    bool adminNotificationSent, std::string userNotificationEmail,
                                                    bool userNotificationSent, QString answer1, QString answer2, QString answer3)
{
    qDebug() << "CTblCodeHistory::addLockCodeHistory()";

    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + QString(TABLENAME.c_str()) +
                QString(" (sequence, sequence_order, "
                        "locknum, description, code1, "
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

    qry.bindValue(":seqDesc", sequence.c_str());
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNum", locknum);
    qry.bindValue(":desc", desc.c_str());
    qry.bindValue(":codeOne", code1.c_str());
    qry.bindValue(":codeTwo", code2.c_str());
    qry.bindValue(":start", starttime.toString(datetimeFormat));
    qry.bindValue(":end", endtime.toString(datetimeFormat));
    qry.bindValue(":stat", status.c_str());
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessTime", accesstime.toString(datetimeFormat));
    qry.bindValue(":adminNotificationSent", QVariant(adminNotificationSent));
    qry.bindValue(":userEmail", userNotificationEmail.c_str());
    qry.bindValue(":userNotificationSent", QVariant(userNotificationSent));
    qry.bindValue(":answer1", answer1.toStdString().c_str());
    qry.bindValue(":answer2", answer2.toStdString().c_str());
    qry.bindValue(":answer3", answer3.toStdString().c_str());

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

/**
 * @brief CTblCodeHistory::currentTimeFormat
 * @param format
 * @param strBuffer
 * @param nExpectedLength = expected length of the return string
 */
void CTblCodeHistory::currentTimeFormat(std::string format, std::string strBuffer, int nExpectedLength)
{
    time_t rawtime;
    struct tm *currentTime;
    time ( &rawtime );
    currentTime = gmtime( &rawtime );
    char buffer [nExpectedLength+1];

    strftime(buffer, nExpectedLength, format.c_str(), currentTime);

    strBuffer = buffer;
}

bool CTblCodeHistory::readTestDefault()
{
    qDebug( )<< "CTblCodeHistory::readTestDefault()";

    QSqlQuery query(*_pDB);
    QString sql = "SELECT sequence, sequence_order, "\
                  "locknum, description, code1,"\
                  "code2, starttime, endtime, status, access_count,"\
                  "retry_count, max_access, max_retry,"
                  "access_time, admin_notification_sent,"
                  "user_notification_email, user_notification_sent"
                  " FROM ";
    sql += QString(TABLENAME.c_str());
    sql += QString(" WHERE description = 'test history lock'");

    if( query.exec(sql)) {
        int fldLock= query.record().indexOf("locknum");
        int fldDescription = query.record().indexOf("description");
        int fldCode1 = query.record().indexOf("code1");
        int fldCode2 = query.record().indexOf("code2");
        int fldSequence = query.record().indexOf("sequence");
        int fldSequenceOrder = query.record().indexOf("sequence_order");
        int fldStart = query.record().indexOf("starttime");
        int fldEnd = query.record().indexOf("endtime");
        int fldStatus = query.record().indexOf("status");
        int fldAccessCount = query.record().indexOf("access_count");
        int fldRetryCount = query.record().indexOf("retry_count");
        int fldMaxAccess = query.record().indexOf("max_access");
        int fldMaxRetry = query.record().indexOf("max_retry");
        int fldAccessTime = query.record().indexOf("access_time");
        int fldAdminNotificationSent = query.record().indexOf("admin_notification_sent");
        int fldUserNotificationEmail = query.record().indexOf("user_notification_email");
        int fldUserNotificationSent = query.record().indexOf("user_notification_sent");

        if (query.next())
        {
            // it exists
            struct tm tm;

            int nLock = query.value(fldLock).toInt();
            QString code1 = query.value(fldCode1).toString();
            QString code2 = query.value(fldCode2).toString();

            strptime(query.value(fldStart).toDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str(), "yyyy-MM-dd %H:%M:%S", &tm);
            time_t start_time = mktime(&tm);

            strptime(query.value(fldEnd).toDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str(), "yyyy-MM-dd %H:%M:%S", &tm);
            time_t end_time = mktime(&tm);

            qDebug() << "CTblCodeHistory::readTestDefault(): Code1:" << code1 << " len:" << code1.size();
            qDebug() << "CTblCodeHistory::readTestDefault(): Code2:" << code2 << " len:" << code2.size();

            return true;
        }
    }
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(int locknum, QDateTime starttime, QDateTime endtime)
{
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(int locknum, QString code1, QString code2,
                                            QDateTime starttime, QDateTime endtime, QDateTime accesstime)
{
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(QDateTime accesstime)
{
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(QDateTime starttime, QDateTime endtime)
{
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
        sql = "DELETE FROM " + QString(TABLENAME.c_str()) +
                QString(" WHERE access_time = :faccessTime and "
                        " locknum=:lockNum");
        query.prepare(sql);
        query.bindValue("faccessTime", rec.getAccessTime().toString(datetimeFormat));
        query.bindValue(":locknum", rec.getLockNum());
    } else {
        sql = "DELETE FROM " + QString(TABLENAME.c_str()) +
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
    QString sql = QString("UPDATE ") + QString(TABLENAME.c_str()) +
            " SET " + QString("sequence=:seqDesc, sequence_order=:seqOrder, "
                              "locknum=:lockNum, description=:desc, code1=:codeOne, "
                              "code2=:codeTwo, starttime=:start, endtime=:end, status=:stat, access_count=:accessCount,"
                              "retry_count=:retryCount, max_access=:maxAccess, max_retry=:maxRetry,"
                              "access_time=:accessTime, admin_notification_sent=:adminNotificationSent,"
                              "user_notification_email=:userNotificationEmail, user_notification_sent=:userNotificationSent"
                              " WHERE ids=:fids");

    qry.prepare(sql);

    qry.bindValue(":seqDesc", rec.getSequence().c_str());
    qry.bindValue(":seqOrder", rec.getSequenceOrder());
    qry.bindValue(":lockNum", rec.getLockNum());
    qry.bindValue(":desc", rec.getDescription().c_str());
    qry.bindValue(":codeOne", rec.getCode1().c_str());
    qry.bindValue(":codeTwo", rec.getCode2().c_str());
    qry.bindValue(":start", rec.getStartTime().toString(datetimeFormat));
    qry.bindValue(":end", rec.getEndTime().toString(datetimeFormat));
    qry.bindValue(":stat", rec.getStatus().c_str());
    qry.bindValue(":maxAccess", rec.getMaxAccess());
    qry.bindValue(":maxRetry", rec.getMaxRetry());
    qry.bindValue(":accessTime", rec.getAccessTime().toString((datetimeFormat)) );
    qry.bindValue(":adminNotificationSent", rec.getAdminNotificationSent() );
    qry.bindValue(":userNotificationEmail", rec.getUserNotificationEmail().c_str() );
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
