#include "tblcodehistory.h"

#include <QtGlobal>
#include <QTime>
#include <QDateTime>
#include <QDebug>

#include "lockhistoryrec.h"
#include "kcbcommon.h"
#include "kcbapplication.h"

CTblCodeHistory::CTblCodeHistory(QSqlDatabase *db) 
{
    // KCB_DEBUG_ENTRY;
    _pDB = db;
    initialize();
    // KCB_DEBUG_EXIT;
}

QSqlQuery CTblCodeHistory::createQuery(QStringList column_list,
                                 QString table, 
                                 // Note: While table is a constant in this class, it is envisioned
                                 // a common class/module will exist that can handle all SQL query
                                 // creation and execution.  This is just a reminder of what's is
                                 // planned.
                                 QString condition)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");

    QSqlQuery query(*_pDB);
    QString sql;
    
    query.setForwardOnly(true);

    auto select = QString("SELECT %1").arg(column_list.join(","));
    sql += QString("%1").arg(select);
    auto from = QString("FROM %1").arg(table);
    sql += QString(" %1").arg(from);
    if (!condition.isEmpty())
    {
        auto where = QString("WHERE %1").arg(condition);
        sql += QString(" %1").arg(where);
    }

    qDebug() << "SQL:" << sql;

    if( !query.prepare(sql) )
    {
        KCB_WARNING_TRACE("prepare failed" << query.lastError());
    }

    // KCB_DEBUG_EXIT;

    return query;
}

void CTblCodeHistory::execSelectCodeHistorySetQuery(QSqlQuery& qry, CLockHistorySet **pLockHistorySet)
{
    if (!qry.exec())
    {
        KCB_DEBUG_TRACE("Exec Failed: " << qry.lastError().text() << qry.lastQuery());
        return;
    }

    if (!qry.isActive() || !qry.isSelect())
    {
        KCB_DEBUG_TRACE("SQL Query Failure: " << "Active" << qry.isActive() << "Select" << qry.isSelect());
        return;
    }

    if (!qry.first())
    {
        KCB_WARNING_TRACE("First Failed: " << qry.lastError().text() << qry.lastQuery());
        return;
    }
    
    // KCB_DEBUG_TRACE("Retrieving at least first record that was found!");

    *pLockHistorySet = new CLockHistorySet();

    do
    {                    
        auto ids = QUERY_VALUE(qry, "ids").toInt();
        auto seq = QUERY_VALUE(qry, "sequence").toString();
        auto seq_order = QUERY_VALUE(qry, "sequence_order").toInt();
        auto lock_nums = QUERY_VALUE(qry, "locknums").toString();
        auto desc = QUERY_VALUE(qry, "description").toString();
        auto sCode1 = QUERY_VALUE(qry, "code1").toString();
        auto sCode2 = QUERY_VALUE(qry, "code2").toString();
        auto sAccessSelection = QUERY_VALUE(qry, "access_selection").toString();
        auto startDT = QUERY_VALUE(qry, "starttime").toDateTime();
        auto endDT = QUERY_VALUE(qry, "endtime").toDateTime();
        auto status = QUERY_VALUE(qry, "status").toString();
        auto access_count = QUERY_VALUE(qry, "access_count").toInt();
        auto retry_count = QUERY_VALUE(qry, "retry_count").toInt();
        auto max_access = QUERY_VALUE(qry, "max_access").toInt();
        auto max_retry = QUERY_VALUE(qry, "max_retry").toInt();
        auto access_time = QUERY_VALUE(qry, "access_time").toDateTime();
        auto admin_notif_sent = QUERY_VALUE(qry, "admin_notification_sent").toBool();
        auto user_notif_email = QUERY_VALUE(qry, "user_notification_email").toString();
        auto user_notif_sent = QUERY_VALUE(qry, "user_notification_sent").toBool();
        auto answer1 = QUERY_VALUE(qry, "answer1").toString();
        auto answer2 = QUERY_VALUE(qry, "answer2").toString();
        auto answer3 = QUERY_VALUE(qry, "answer3").toString();
		auto image = QUERY_VALUE(qry, "image").toByteArray();

        CLockHistoryRec *pLockHistoryRec = new CLockHistoryRec();

        pLockHistoryRec->setID(ids);
        pLockHistoryRec->setSequence(seq);
        pLockHistoryRec->setSequenceOrder(seq_order);
        pLockHistoryRec->setLockNums(lock_nums);
        pLockHistoryRec->setDescription(desc);
        pLockHistoryRec->setCode1(sCode1);
        pLockHistoryRec->setCode2(sCode2);        
        pLockHistoryRec->setAccessSelection(sAccessSelection);
        pLockHistoryRec->setStartTime(startDT);
        pLockHistoryRec->setEndTime(endDT);
        pLockHistoryRec->setStatus(status);
        pLockHistoryRec->setAccessCount(access_count);
        pLockHistoryRec->setRetryCount(retry_count);
        pLockHistoryRec->setMaxAccess(max_access);
        pLockHistoryRec->setMaxRetry(max_retry);

        pLockHistoryRec->setAccessTime(access_time);
        pLockHistoryRec->setAdminNotificationSent(admin_notif_sent);
        pLockHistoryRec->setUserNotificationEmail(user_notif_email);
        pLockHistoryRec->setUserNotificationSent(user_notif_sent);

        pLockHistoryRec->setQuestion1(answer1);
        pLockHistoryRec->setQuestion2(answer2);
        pLockHistoryRec->setQuestion3(answer3);

		pLockHistoryRec->setImage(image);
        (*pLockHistorySet)->addToSet(*pLockHistoryRec);
        
    } while (qry.next());
}

void CTblCodeHistory::selectLockCodeHistorySet(QString lockNums, QDateTime start, QDateTime end, CLockHistorySet **pLockHistorySet)
{
    // KCB_DEBUG_ENTRY;
    *pLockHistorySet = 0;

    // KCB_DEBUG_TRACE(lockNums);

    QStringList columns_list;
    columns_list << "ids" << "sequence" << "sequence_order";
    columns_list << "locknums" << "description" << "code1" << "code2";
    columns_list << "access_selection";
    columns_list << "starttime" << "endtime" << "status";
    columns_list << "access_count" << "retry_count" << "max_access" << "max_retry" << "access_time";
    columns_list << "admin_notification_sent" << "user_notification_email" << "user_notification_sent";
    columns_list << "answer1" << "answer2" << "answer3";
	columns_list << "image";
    QString condition = "";
    if(lockNums != "" && lockNums != "*")
    {
        /* Lock nums is a string with either a single value or comma-separated values.
            * Entries in the database may be single values or comma-separated values.
            * Find all entries with all matching values.
            */

        condition += " ( ";
        if (lockNums.contains(','))
        {
            QStringList sl = lockNums.split(',');
            foreach (auto s, sl)
            {
                condition += QString("instr(locknums, %1) > 0 and ").arg(s);
            }
        }
        // Note: This handles the case of a single number.  It is redundant for
        // comma-separated values, but eliminates an 'else'
        condition += "instr(locknums, :lockNums) > 0";
        condition += " ) and ";
    }
    condition += "(access_time >= :stime and access_time <= :etime) ";
    condition += "ORDER BY access_time DESC";

    auto qry = createQuery(columns_list, TABLENAME, condition);

    if (lockNums != "" && lockNums != "*")
    {
        qry.bindValue(":lockNums", lockNums);
    }
    qry.bindValue(":stime", start.toString(DATETIME_FORMAT));
    qry.bindValue(":etime", end.toString(DATETIME_FORMAT));

    execSelectCodeHistorySetQuery(qry, pLockHistorySet);
}

void CTblCodeHistory::selectLastLockCodeHistorySet(QString &lockNums, QDateTime &start, QDateTime &end, CLockHistorySet **pLockHistorySet)
{
    selectLockCodeHistorySet(lockNums, start, end, pLockHistorySet);
}

bool CTblCodeHistory::tableExists()
{
    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    
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
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    
    QStringList::iterator  itor;
    QSqlQuery qry(*_pDB);
    bool foundColumn = false;

    // KCB_DEBUG_TRACE("Opening Database" << _pDB << _pDB->isOpen());

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");

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

    } 
    else 
    {
        KCB_DEBUG_TRACE("Either _pDB is NULL or _pDB is not open");
    }
    return foundColumn;
}

void CTblCodeHistory::createColumn(QString column, QString fieldType)
{
    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE("Opening Database" << _pDB << _pDB->isOpen());

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");
    
    if( _pDB && _pDB->isOpen() ) 
    {
        // KCB_DEBUG_TRACE("Creating table");
        QSqlQuery qry(*_pDB);

        QString sql("ALTER TABLE  ");
        sql += TABLENAME;
        sql += " ADD ";
        sql += column;
        sql += " ";
        sql += fieldType;

        qry.prepare( sql );

        if( !qry.exec() )
        {
            KCB_DEBUG_TRACE(qry.lastError());
        }
    } 
    else 
    {
        KCB_DEBUG_TRACE("Either _pDB is NULL or _pDB is not open");
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

    QString image_column = "image";

    if(!columnExists(image_column))
    {
        createColumn(image_column, "BLOB");
    }            
}

void CTblCodeHistory::createTable()
{
    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE("Opening Database" << _pDB << _pDB->isOpen());

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");
    
    if( _pDB && _pDB->isOpen() ) 
    {
        // KCB_DEBUG_TRACE("Creating table");
        QSqlQuery qry(*_pDB);

        QString sql("CREATE TABLE IF NOT EXISTS ");
        sql += TABLENAME;
        sql += "(ids integer primary key unique, sequence text,"
               "sequence_order integer, locknums integer, description text, "
               "code1 text, code2 text, access_selection text, "
               " starttime DATETIME, endtime DATETIME, status text, access_count integer,"
               " retry_count integer, max_access integer, max_retry integer,"
               " access_time DATETIME, admin_notification_sent BOOL,"
                " user_notification_email text, user_notification_sent BOOL,"
                "answer1 text, answer2 text, answer3 text,"
                "image BLOB)";

        qry.prepare( sql );

        if( !qry.exec() )
        {
            qDebug() << qry.lastError();
        }
    } 
    else 
    {
        KCB_DEBUG_TRACE("Either _pDB is NULL or _pDB is not open");
    }

}

bool CTblCodeHistory::addLockCodeHistory(CLockHistoryRec &lockHistoryRec)
{
    return addLockCodeHistory(lockHistoryRec.getLockNums(), lockHistoryRec.getCode1(), lockHistoryRec.getCode2(),
                              lockHistoryRec.getAccessSelection(),
                              lockHistoryRec.getStartTime(), lockHistoryRec.getEndTime(), lockHistoryRec.getMaxAccess(),
                              lockHistoryRec.getStatus(), lockHistoryRec.getDescription(),
                              lockHistoryRec.getSequence(), lockHistoryRec.getSequenceOrder(),
                              lockHistoryRec.getMaxRetry(), lockHistoryRec.getAccessTime(),
                              lockHistoryRec.getAdminNotificationSent(), lockHistoryRec.getUserNotificationEmail(),
                              lockHistoryRec.getUserNotificationSent(),
                              lockHistoryRec.getImage());
}

bool CTblCodeHistory::addLockCodeHistory(QString locknums, QString code1, QString code2,
                                         QString accessSelection,
                                         QDateTime starttime, QDateTime endtime, int maxAccess,
                                         QString status, QString desc, QString sequence, int sequenceNum,
                                         int maxRetry, QDateTime accesstime,
                                         bool adminNotificationSent, QString userNotificationEmail,
                                         bool userNotificationSent, QByteArray image)
{
    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE("Adding Locks to History" << locknums);
    // KCB_DEBUG_TRACE("image size" << image.count());
    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + TABLENAME +
                QString(" (sequence, sequence_order, "
                        "locknums, description, code1, code2, "
                        "access_selection, "
                        "starttime, endtime, status, access_count,"
                        "retry_count, max_access, max_retry,"
                        "access_time, admin_notification_sent,"
                        "user_notification_email, user_notification_sent,"
					    "image)"
                        " VALUES (:seqDesc, :seqOrder, :lockNums, :desc, :codeOne, "
                        " :codeTwo, :accessSelection, "
                        " :start, :end, :stat, 0, 0, :maxAccess, :maxRetry, "
                        " :accessTime, :adminNotificationSent, "
                        " :userEmail, :userNotificationSent,"
						" :image)" ));

    qDebug() << "Query:" << qry.lastQuery();

    qry.bindValue(":seqDesc", sequence);
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNums", locknums);
    qry.bindValue(":desc", desc);
    qry.bindValue(":codeOne", code1);
    qry.bindValue(":codeTwo", code2);
    qry.bindValue(":accessSelection", accessSelection);
    qry.bindValue(":start", starttime.toString(DATETIME_FORMAT));
    qry.bindValue(":end", endtime.toString(DATETIME_FORMAT));
    qry.bindValue(":stat", status);
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessTime", accesstime.toString(DATETIME_FORMAT));
    qry.bindValue(":adminNotificationSent", QVariant(adminNotificationSent));
    qry.bindValue(":userEmail", userNotificationEmail);
    qry.bindValue(":userNotificationSent", QVariant(userNotificationSent));
	
    qry.bindValue(":image", QVariant(image));

    if( !qry.exec() ) 
    {
        KCB_DEBUG_TRACE("error" << qry.lastError() << "query" << qry.lastQuery());
        return false;
    }
    else 
    {
        return true;
    }
}

bool CTblCodeHistory::addLockCodeHistoryWithAnswers(CLockHistoryRec &lockHistoryRec, QString answer1, QString answer2, QString answer3)
{
    return addLockCodeHistoryWithAnswers(lockHistoryRec.getLockNums(), lockHistoryRec.getCode1(), lockHistoryRec.getCode2(),
                                         lockHistoryRec.getAccessSelection(),
                                         lockHistoryRec.getStartTime(), lockHistoryRec.getEndTime(), lockHistoryRec.getMaxAccess(),
                                         lockHistoryRec.getStatus(), lockHistoryRec.getDescription(),
                                         lockHistoryRec.getSequence(), lockHistoryRec.getSequenceOrder(),
                                         lockHistoryRec.getMaxRetry(), lockHistoryRec.getAccessTime(),
                                         lockHistoryRec.getAdminNotificationSent(), lockHistoryRec.getUserNotificationEmail(),
                                         lockHistoryRec.getUserNotificationSent(), answer1, answer2, answer3, lockHistoryRec.getImage());
}

bool CTblCodeHistory::addLockCodeHistoryWithAnswers(QString locknums, QString code1, QString code2,
                                                    QString accessSelection,
                                                    QDateTime starttime, QDateTime endtime, int maxAccess,
                                                    QString status, QString desc, QString sequence, int sequenceNum,
                                                    int maxRetry, QDateTime accesstime,
                                                    bool adminNotificationSent, QString userNotificationEmail,
                                                    bool userNotificationSent, QString answer1, QString answer2, QString answer3,
                                                    QByteArray image)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");

    // KCB_DEBUG_TRACE("Adding Locks w/ Answers to History" << locknums);
    // KCB_DEBUG_TRACE("Answer1" << answer1 << "Answer2" << answer2 << "Answer3" << answer3);

    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + TABLENAME +
                QString(" (sequence, sequence_order, "
                        "locknums, description, code1, "
                        "code2, access_selection, starttime, endtime, status, access_count,"
                        "retry_count, max_access, max_retry,"
                        "access_time, admin_notification_sent,"
                        "user_notification_email, user_notification_sent,"
                        "answer1, answer2, answer3)"
                        " VALUES (:seqDesc, :seqOrder, :lockNums, :desc, :codeOne, "
                        " :codeTwo, :accessSelection, "
                        " :start, :end, :stat, 0, 0, :maxAccess, :maxRetry, "
                        " :accessTime, :adminNotificationSent, "
                        " :userEmail, :userNotificationSent, "
                        " :answer1, :answer2, :answer3,"
						" :image)" ));

    qDebug() << "Query:" << qry.lastQuery();

    qry.bindValue(":seqDesc", sequence);
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNums", locknums);
    qry.bindValue(":desc", desc);
    qry.bindValue(":codeOne", code1);
    qry.bindValue(":codeTwo", code2);
    qry.bindValue(":accessSelection", accessSelection);
    qry.bindValue(":start", starttime.toString(DATETIME_FORMAT));
    qry.bindValue(":end", endtime.toString(DATETIME_FORMAT));
    qry.bindValue(":stat", status);
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessTime", accesstime.toString(DATETIME_FORMAT));
    qry.bindValue(":adminNotificationSent", QVariant(adminNotificationSent));
    qry.bindValue(":userEmail", userNotificationEmail);
    qry.bindValue(":userNotificationSent", QVariant(userNotificationSent));
    qry.bindValue(":answer1", answer1);
    qry.bindValue(":answer2", answer2);
    qry.bindValue(":answer3", answer3);
    qry.bindValue(":image", image);

    if( !qry.exec() ) 
    {
        KCB_DEBUG_TRACE("error" << qry.lastError() << "query" << qry.lastQuery());
        return false;
    }
    else 
    {
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
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");

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

    if( query.exec(sql)) 
    {
        auto fldCode1 = query.record().indexOf("code1");
        auto fldCode2 = query.record().indexOf("code2");
        auto fldStart = query.record().indexOf("starttime");
        auto fldEnd = query.record().indexOf("endtime");

        if (query.next())
        {
            // it exists
            struct tm tm;

            QString code1 = query.value(fldCode1).toString();
            QString code2 = query.value(fldCode2).toString();

            strptime(query.value(fldStart).toDateTime().toString(DATETIME_FORMAT).toStdString().c_str(), "yyyy-MM-dd %H:%M:%S", &tm);
            strptime(query.value(fldEnd).toDateTime().toString(DATETIME_FORMAT).toStdString().c_str(), "yyyy-MM-dd %H:%M:%S", &tm);

            return true;
        }
    }
    return false;
}

bool CTblCodeHistory::deleteLockCodeHistory(CLockHistoryRec &rec)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");

    if( rec.getID() == -1 ) 
    {
        return false;
    }

    QSqlQuery query(*_pDB);
    QString sql;
    if(rec.getID() == -1 ) 
    {
        sql = "DELETE FROM " + TABLENAME +
                QString(" WHERE access_time = :faccessTime and "
                        " locknums=:lockNums");
        query.prepare(sql);
        query.bindValue("faccessTime", rec.getAccessTime().toString(DATETIME_FORMAT));
        query.bindValue(":lockNums", rec.getLockNums());
    } 
    else 
    {
        sql = "DELETE FROM " + TABLENAME +
                QString(" WHERE ids = :fids");
        query.prepare(sql);
        query.bindValue("fids", rec.getID());
    }

    if( query.exec(sql)) 
    {
        return true;
    } 
    else 
    {
        return false;
    }
}

bool CTblCodeHistory::updateRecord(CLockHistoryRec &rec)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("sequence=:seqDesc, sequence_order=:seqOrder, "
                              "locknums=:lockNums, description=:desc, code1=:codeOne, "
                              "code2=:codeTwo, access_selection=:accessSelection, starttime=:start, endtime=:end, status=:stat, access_count=:accessCount,"
                              "retry_count=:retryCount, max_access=:maxAccess, max_retry=:maxRetry,"
                              "access_time=:accessTime, admin_notification_sent=:adminNotificationSent,"
                              "user_notification_email=:userNotificationEmail, user_notification_sent=:userNotificationSent,"
                              "image=:image"
                              " WHERE ids=:fids");

    qry.prepare(sql);

    qry.bindValue(":seqDesc", rec.getSequence());
    qry.bindValue(":seqOrder", rec.getSequenceOrder());
    qry.bindValue(":lockNums", rec.getLockNums());
    qry.bindValue(":desc", rec.getDescription());
    qry.bindValue(":codeOne", rec.getCode1());
    qry.bindValue(":codeTwo", rec.getCode2());
    qry.bindValue(":accesssSelection", rec.getAccessSelection());
    qry.bindValue(":start", rec.getStartTime().toString(DATETIME_FORMAT));
    qry.bindValue(":end", rec.getEndTime().toString(DATETIME_FORMAT));
    qry.bindValue(":stat", rec.getStatus());
    qry.bindValue(":maxAccess", rec.getMaxAccess());
    qry.bindValue(":maxRetry", rec.getMaxRetry());
    qry.bindValue(":accessTime", rec.getAccessTime().toString(DATETIME_FORMAT) );
    qry.bindValue(":adminNotificationSent", rec.getAdminNotificationSent() );
    qry.bindValue(":userNotificationEmail", rec.getUserNotificationEmail());
    qry.bindValue(":userNotificationSent", rec.getUserNotificationSent() );

    qry.bindValue(":image", rec.getImage() );
    qry.bindValue(":fids", rec.getID());

    if(qry.exec()) 
    {
        return true;
    } 
    else 
    {
        KCB_DEBUG_TRACE("error" << qry.lastError() << "query" << qry.lastQuery());
        return false;
    }
}


//-------------------------------------------------------------------------------------------------
// EOF
