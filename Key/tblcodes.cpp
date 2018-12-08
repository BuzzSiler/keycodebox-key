#include <QObject>
#include <QTime>
#include <QDateTime>
#include <QString>
#include "tblcodes.h"
#include "encryption.h"
#include "kcbcommon.h"

CTblCodes::CTblCodes(QSqlDatabase *db)
{
    KCB_DEBUG_ENTRY;
    _pDB = db;
    initialize();
    KCB_DEBUG_EXIT;
}

bool CTblCodes::isExpired(int access_type, int access_count, int max_access)
{
    if (access_type == 2 /* ACCESS_LIMITED_USE */)
    {
        if (max_access > 0)
        {
            if (access_count == max_access)
            {
                return true;
            }
        }
    }

    return false;
}

void CTblCodes::setLastCodeOne(QString code)
{
    _sCodeOne = code;
}

QSqlQuery CTblCodes::createQuery(QStringList column_list,
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

    //qDebug() << "SQL:" << sql;

    if ( !query.prepare(sql) )
    {
        KCB_WARNING_TRACE("prepare failed" << query.lastError());
    }

    // KCB_DEBUG_EXIT;

    return query;
}


int CTblCodes::checkCodeOne(QString code, 
                            bool &bSecondCodeRequired, 
                            bool &bFingerprintRequired, 
                            QString &lockNums )
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE("code1" << code);

    QDateTime time = QDateTime::currentDateTime();
    code = CEncryption::decryptString(code);
    _sCodeOne = code;
    lockNums = "";
    _lastIDS = -1;
    bSecondCodeRequired = false;

    QStringList column_list;
    column_list << "ids" << "locknums" << "code1" << "code2";
    column_list << "fingerprint1" << "fingerprint2";
    column_list << "starttime" << "endtime";
    column_list << "access_count" << "retry_count" << "max_access" << "max_retry" << "access_type";
    QString condition = "((access_type = 0 or access_type = 2) OR (starttime <= :time and :time <= endtime))";
    auto qry = createQuery(column_list, TABLENAME, condition);

    qry.bindValue(":time", time.toString("yyyy-MM-dd HH:mm:ss"));



    if (!qry.exec())
    {
        qDebug() << qry.lastError().text() << qry.lastQuery();
        return KCB_FAILED;
    }

    // Note: Duplicate code1 entries are not allowed.  So, we can return on the first match

    KCB_DEBUG_TRACE("Active" << qry.isActive() << "Select" << qry.isSelect());

    while (qry.next())
    {

        KCB_DEBUG_TRACE("qry.next");
        
        auto ids = QUERY_VALUE(qry, "ids").toInt();
        auto sCode1 = QUERY_VALUE(qry, "code1").toString();
        auto sCode2 = QUERY_VALUE(qry, "code2").toString();
        lockNums = QUERY_VALUE(qry, "locknums").toString();
        auto access_type = QUERY_VALUE(qry, "access_type").toInt();
        auto access_count = QUERY_VALUE(qry, "access_count").toInt();
        auto max_access = QUERY_VALUE(qry, "max_access").toInt();
        bFingerprintRequired = QUERY_VALUE(qry, "fingerprint1").toBool();
        
        sCode1 = CEncryption::decryptString(sCode1);

        KCB_DEBUG_TRACE("Code1" << sCode1 << "Code" << code << "Locks" << lockNums);

        if( sCode1 == code ) 
        {
            KCB_DEBUG_TRACE("codes match");
            
            /* Check for expiration */
            KCB_DEBUG_TRACE("Access Type" << access_type << "Access Count" << access_count << "Max Accesss" << max_access);
            if (isExpired(access_type, access_count, max_access))
            {
                KCB_DEBUG_TRACE("expired" << lockNums);
                KCB_DEBUG_EXIT;
                return KCB_FAILED;
            }

            // code2 may or may not be present, but must be decrypted first to make the determination
            sCode2 = CEncryption::decryptString(sCode2);

            // if code2 is present (and valid after decryption) then we need to ask for code2
            // if code2 is not present, then we only have a single code - successful
            auto code2_present = !(sCode2.isNull() || sCode2.isEmpty());

            if (code2_present)
            {
                bSecondCodeRequired = true;
                _lastIDS = 0;
            }
            else
            {
                _lastIDS = ids;
                incrementAccessCount(ids);
            }

            KCB_DEBUG_EXIT;
            return KCB_SUCCESS;
        }
    }

    KCB_DEBUG_TRACE("failed - no match found");
    KCB_DEBUG_EXIT;
    return KCB_FAILED;
}

bool CTblCodes::isWhiteSpace(const QString &str)
{
    return QRegExp("\\s*").exactMatch(str);
}

int CTblCodes::checkCodeTwo(QString code, 
                            bool &bFingerprintRequired, 
                            QString &codeOne, 
                            QString &lockNums, 
                            bool &bAskQuestions, 
                            QString &question1, 
                            QString &question2, 
                            QString &question3)
{
    KCB_DEBUG_ENTRY;
    
    KCB_DEBUG_TRACE("code1" << codeOne << "code2" << code);

    QDateTime time = QDateTime::currentDateTime();
    code = CEncryption::decryptString(code);
    _sCodeTwo = code;
    lockNums = "";
    _lastIDS = -1;


    QStringList column_list;
    column_list << "ids" << "locknums" << "code1" << "code2" << "fingerprint2";
    column_list << "starttime" << "endtime" << "access_count" << "retry_count" << "max_access" << "max_retry" << "access_type";
    column_list << "ask_questions" << "question1" << "question2" << "question3" << "lockbox_state";
    QString condition = "((access_type = 0 or access_type = 2) OR (starttime <= :time and :time <= endtime))";
    auto qry = createQuery(column_list, TABLENAME, condition);

    qry.bindValue(":time", time.toString("yyyy-MM-dd HH:mm:ss"));

    if(!qry.exec())
    {
        qDebug() << qry.lastError().text() << qry.lastQuery();
        return KCB_FAILED;
    }

    KCB_DEBUG_TRACE("Active" << qry.isActive() << "Select" << qry.isSelect());

    if(!qry.first())
    {
        KCB_WARNING_TRACE(qry.lastError().text() << qry.lastQuery());
        return KCB_FAILED;
    }

    do
    {
        auto sCode1 = QUERY_VALUE(qry, "code1").toString();
        auto sCode2 = QUERY_VALUE(qry, "code2").toString();
        lockNums = QUERY_VALUE(qry, "locknums").toString();
        auto lockboxState = QUERY_VALUE(qry, "lockbox_state").toInt();
        auto access_type = QUERY_VALUE(qry, "access_type").toInt();
        auto access_count = QUERY_VALUE(qry, "access_count").toInt();
        auto max_access = QUERY_VALUE(qry, "max_access").toInt();
        auto ids = QUERY_VALUE(qry, "ids").toInt();
        bFingerprintRequired = QUERY_VALUE(qry, "fingerprint2").toBool();
        bAskQuestions = QUERY_VALUE(qry, "ask_questions").toBool();
        question1 = QUERY_VALUE(qry, "question1").toString();
        question2 = QUERY_VALUE(qry, "question2").toString();
        question3 = QUERY_VALUE(qry, "question3").toString();

        KCB_DEBUG_TRACE("Code1" << sCode1 << "Code2" << sCode2 << "Locks" << lockNums);

        sCode1 = CEncryption::decryptString(sCode1);
        sCode2 = CEncryption::decryptString(sCode2);

        if( sCode1 == _sCodeOne && sCode2 == code)
        {
            KCB_DEBUG_TRACE("codes match");

            _lastIDS = ids;

            /* Check for expiration */
            if (isExpired(access_type, access_count, max_access))
            {
                KCB_DEBUG_TRACE("expired" << lockNums);
                KCB_DEBUG_EXIT;
                return KCB_FAILED;
            }

            if( bFingerprintRequired )
            {
                codeOne = sCode1;
            }

            if( lockboxState == 0)
            {
                updateLockboxState(_lastIDS, true);
            }
            else
            {
                updateLockboxState(_lastIDS, false);
            }

            return KCB_SUCCESS;
        }
    } while(qry.next());

    return KCB_FAILED;
}

bool CTblCodes::containsMatchingEntries(const QStringList& s1, const QStringList& s2)
{
    /* Note: I intended to use regular expressions to do the matching, i.e., 1$|,1$|^1,|,1,
     * Running QRegularExpression example application, this pattern worked perfectly, but
     * it did not work in this application.  I don't know why.  It would match on 10 or 13 or 21 --
     * anything that contained a '1'.  Ultimately, settled for a double loop.
     */
    if (s1.count() > 0)
    {
        foreach (auto s1_str, s1)
        {
            foreach (auto s2_str, s2)
            {
                if (s1_str == s2_str)
                {
                    return true;
                }
            }
        }
        return false;
    }

    return true;
}

void CTblCodes::execSelectCodeSetQuery(QStringList lockNumsList, QSqlQuery& qry, CLockSet **pLockSet, bool clear_or_encrypted)
{
    CLockState *pLock;
    
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

    *pLockSet = new CLockSet();

    qDebug() << "Retrieving at least first record that was found!";
    do
    {

        auto lock_nums = QUERY_VALUE(qry, "locknums").toString();
        if (!containsMatchingEntries(lockNumsList, lock_nums.split(",")))
        {
            continue;
        }

        auto ids = QUERY_VALUE(qry, "ids").toInt();
        auto seq = QUERY_VALUE(qry, "sequence").toString();
        auto seq_order = QUERY_VALUE(qry, "sequence_order").toInt();
        auto desc = QUERY_VALUE(qry, "description").toString();
        auto sCode1 = QUERY_VALUE(qry, "code1").toString();
        auto sCode2 = QUERY_VALUE(qry, "code2").toString();                    
        auto startDT = QUERY_VALUE(qry, "starttime").toDateTime();
        auto endDT = QUERY_VALUE(qry, "endtime").toDateTime();
        auto status = QUERY_VALUE(qry, "status").toString();
        auto access_count = QUERY_VALUE(qry, "access_count").toInt();
        auto retry_count = QUERY_VALUE(qry, "retry_count").toInt();
        auto max_access = QUERY_VALUE(qry, "max_access").toInt();
        auto max_retry = QUERY_VALUE(qry, "max_retry").toInt();
        /* Note: The conversion from qry.value to bool had odd behavior which caused
            the bool to become true even when it displayed as false.  I introduced
            intermediate variables to store the conversion plus also biased the
            conversion to be false unless explicitly true.
        */
        auto fp1 = QUERY_VALUE(qry, "fingerprint1").toInt() == 1 ? true : false;
        auto fp2 = QUERY_VALUE(qry, "fingerprint2").toInt() == 1 ? true : false;
        auto aq = QUERY_VALUE(qry, "ask_questions").toInt() == 1 ? true : false;
        auto question1 = QUERY_VALUE(qry, "question1").toString();
        auto question2 = QUERY_VALUE(qry, "question2").toString();
        auto question3 = QUERY_VALUE(qry, "question3").toString();
        auto access_type = QUERY_VALUE(qry, "access_type").toInt();


        pLock = new CLockState();

        pLock->setID(ids);
        pLock->setSequence(seq);
        pLock->setSequenceOrder(seq_order);
        pLock->setLockNums(lock_nums);
        pLock->setDescription(desc);
        if (clear_or_encrypted)
        {
            sCode1 = CEncryption::decryptString(sCode1);
            sCode2 = CEncryption::decryptString(sCode2);
        }
        pLock->setCode1(sCode1);
        pLock->setCode2(sCode2);
        pLock->setStartTime(startDT);
        pLock->setEndTime(endDT);
        pLock->setStatus(status);
        pLock->setAccessCount(access_count);
        pLock->setRetryCount(retry_count);
        pLock->setMaxAccess(max_access);
        pLock->setMaxRetry(max_retry);
        // Set or clear fingerprint based on fingerprint state, i.e., fp1 or fp2
        fp1 == true ? pLock->setFingerprint1() : pLock->clearFingerprint1();
        fp2 == true ? pLock->setFingerprint2() : pLock->clearFingerprint2();
        pLock->setAskQuestions(aq);
        pLock->setQuestion1(question1);
        pLock->setQuestion2(question2);
        pLock->setQuestion3(question3);
        pLock->setAccessType(access_type);

        (*pLockSet)->addToSet(pLock);

    } while (qry.next());
}

void CTblCodes::selectCodeSet(QString &lockNums, QDateTime start, QDateTime end, CLockSet **pLockSet, bool clear_or_encrypted)
{
    KCB_DEBUG_TRACE("lockNums" << lockNums << "start" << start.toString() << "end" << end.toString() << "pLockSet");

    *pLockSet = 0;

    QStringList column_list;
    column_list << "ids" << "sequence" << "sequence_order" << "locknums" << "description";
    column_list << "code1" << "code2" << "fingerprint1" << "fingerprint2";
    column_list << "ask_questions" << "question1" << "question2" << "question3";
    column_list << "starttime" << "endtime" << "status";
    column_list << "access_count" << "retry_count" << "max_access" << "max_retry" << "access_type";
    QString condition = "";
    bool locknums_but_not_all = lockNums != "" && lockNums != "*";
    /* The goal is to query for those codes whose locknums field contains the specified lock.  For example,
     *     lock = 2
     * Existing Codes and associated locks:
     *     code = xxx, locks = 1,2,3
     *     code = yyy, locks = 3,20,21
     *     code = zzz, locks = 2,7,10
     * We should only selected codes xxx and zzz.
     *
     * The locknums field contains a string of one or more lock numbers.  If a single lock number is specified
     * it will be one or more characters, e.g. 1 or 10 or 100.  If multiple lock numbers are specified, they
     * will be one or more characters separated by commas, e.g., 1,10,100
     *
     * Note: No spaces are used and the locks are sorted (before they were added to the database)
     *
     * When we are searching the database for matching codes, we need to do the following:
     *     1. if the lock number is a single value, then find all of the entries with that single value
     *     2. if the lock number is multiple values, then split on the comma and do a search to match
     *        each individual value
     *
     * Note: SQLite does support REGEXP, but not natively and there are apparently issues connecting C++ (Qt)
     *       to C-based SQLite.  So, regular expression as part of the database query is out.
     *
     * Without regular expression support in SQLite, we will need to pull all possible matches from the database,
     * e.g., 1,2,3 would match 8,9,10 (because of the 1).  When we loop over the records we will perform
     * pattern matching and select the relevant codes.
     */

    QStringList sl = lockNums.split(',');
    if (sl[0] == "*")
    {
        sl.clear();
    }

    if(locknums_but_not_all)
    {
        /* locknums is a string with either a single value or comma-separated values.
            * Entries in the database may be single values or comma-separated values.
            * Find all entries with all matching values.
            */

        condition += " ( ";
        if (sl.count() > 1)
        {
            int count = sl.count();
            foreach (auto s, sl)
            {
                condition += QString("instr(locknums, %1) > 0").arg(s);
                if (count > 1)
                {
                    condition += " or ";
                }
                count--;
            }
        }
        else
        {
            condition += "instr(locknums, :lockNums) > 0";
        }
        condition += " ) and ";
    }
    condition += " ( (access_type = 0 or access_type = 2) or"
                 " ((starttime >= :stime and starttime <= :etime) or"
                 " (endtime >= :stime and endtime <= :etime)))";
    // Select codes where the start/end access doesn't matter (always and limited user) OR
    // codes where there is an overlap between start/end range and the code start/end
    //
    // Display Window Date/time:
    //                                          |-----  window  -----|
    //                                         ST                    ET
    // Code Window Date/Time:     ^         ^       ^  ^          ^       ^  ^         ^
    //                            |-Ignored-|       |  |-Selected-|       |  |-Ignored-|
    //                           ST         ET      | ST          ET      | ST         ET
    //                            |----Selected-----|  |                  |
    //                           ST                 ET |-----Selected-----|
    //                                                ST                  ET

    auto qry = createQuery(column_list, TABLENAME, condition);

    if (locknums_but_not_all)
    {
        qry.bindValue(":lockNums", sl[0]);
    }
    qry.bindValue(":stime", start);
    qry.bindValue(":etime", end);

    execSelectCodeSetQuery(sl, qry, pLockSet, clear_or_encrypted);
}

void CTblCodes::selectCodeSet(int ids, CLockSet **pLockSet, bool clear_or_encrypted)
{
    KCB_DEBUG_ENTRY;

    *pLockSet = 0;

    QStringList column_list;
    column_list << "ids" << "sequence" << "sequence_order" << "locknums" << "description";
    column_list << "code1" << "code2" << "fingerprint1" << "fingerprint2";
    column_list << "ask_questions" << "question1" << "question2" << "question3";
    column_list << "starttime" << "endtime" << "status";
    column_list << "access_count" << "retry_count" << "max_access" << "max_retry" << "access_type";
    QString condition = "ids = :id";

    auto qry = createQuery(column_list, TABLENAME, condition);

    qry.bindValue(":id", ids);

    execSelectCodeSetQuery(QStringList(), qry, pLockSet, clear_or_encrypted);
}

bool CTblCodes::tableExists()
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

bool CTblCodes::columnExists(QString column)
{
    qDebug() << "CTblCodes::fingerprintColumnExists()";
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
                    qDebug() << "CTblCodes::fingerprintColumnExists(), found column: " << column;

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

void CTblCodes::initialize()
{
    QString column = "fingerprint1";
    QString column1 = "fingerprint2";
    QString column2 = "lockbox_state";

    QString column3 = "ask_questions";
    QString column4 = "question1";
    QString column5 = "question2";
    QString column6 = "question3";

    qDebug() << columnExists(column);

    if(!tableExists())
    {
        qDebug() << "Table does not Exist";
        createTable();
    }

    if(!columnExists(column))
    {
        createColumn(column, "integer");
    }
    if(!columnExists(column1))
    {
        createColumn(column1, "integer");
    }
    if(!columnExists(column2))
    {
        createColumn(column2, "integer");
    }
    if(!columnExists(column3))
    {
        createColumn(column3, "integer");
    }
    if(!columnExists(column4))
    {
        createColumn(column4, "text");
    }
    if(!columnExists(column5))
    {
        createColumn(column5, "text");
    }
    if(!columnExists(column6))
    {
        createColumn(column6, "text");
    }
}

void CTblCodes::createTable()
{
    std::cout << "CTblCodes::createTable\n";
    if( _pDB && _pDB->isOpen() ) 
    {
        std::cout << "Creating table \n";
        QSqlQuery qry(*_pDB);


        QString sql("CREATE TABLE IF NOT EXISTS ");
        sql += TABLENAME;
        sql += "(ids integer primary key unique, sequence text,"
               "sequence_order integer, locknums text, description text, "
               "code1 text, code2 text,"
               " starttime DATETIME, endtime DATETIME, fingerprint1 integer, fingerprint2 integer, status text, access_count integer,"
               " retry_count integer, max_access integer, max_retry integer, lockbox_state integer, ask_questions integer,"
               " question1 text, question2 text, question3 text, access_type integer)";

        qry.prepare( sql );

        if( !qry.exec() )
        {
            qDebug() << qry.lastError();
        }
        else
        {
            qDebug() << "Table created!";
        }
    } 
    else 
    {
        std::cout << "Either _pDB is NULL or _pDB is not open\n";
    }

}

void CTblCodes::createColumn(QString column, QString fieldType)
{
    qDebug() << "CTblCodes::createColumn\n";
    if( _pDB && _pDB->isOpen() ) 
    {
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
        {
            qDebug() << qry.lastError();
        }
        else
        {
            qDebug() << "Table altered!";
        }
    } 
    else 
    {
        std::cout << "Either _pDB is NULL or _pDB is not open\n";
    }
}

void CTblCodes::addJSONCodes(const CLockState *prec)
{
    Q_UNUSED(prec);
}

void CTblCodes::addJSONCodes(const CLockSet *pcodeSet)
{
    Q_UNUSED(pcodeSet);
}

// JSON format file
void CTblCodes::addJSONCodes(std::iostream iofile)
{
    Q_UNUSED(iofile);
}

void CTblCodes::addJSONCodes(QString jsonCodes)
{
    Q_UNUSED(jsonCodes);
}

int CTblCodes::addLockCodeClear(QString locknums, QString code1, QString code2,
                                QDateTime starttime, QDateTime endtime, bool fingerprint1, bool fingerprint2,
                                bool askQuestions, QString question1, QString question2, QString question3,
                                QString status, QString desc,
                                QString sequence, int sequenceNum,
                                int maxAccess, int maxRetry, int accessType, int accessCount)
{
    KCB_DEBUG_ENTRY;
    QString     encCode1, encCode2;
    if(code1.length() > 0)
    {
        encCode1 = CEncryption::decryptString(code1);
    }

    if(code2.length() > 0) 
    {
        encCode2 = CEncryption::decryptString(code2);
    }

    return addLockCode(locknums, encCode1, encCode2,
                       starttime, endtime,
                       fingerprint1, fingerprint2,
                       askQuestions, question1, question2, question3,
                       status, desc,
                       sequence, sequenceNum,
                       maxAccess, maxRetry, accessType, accessCount);
}

int CTblCodes::addLockCode(QString locknums, QString code1, QString code2,
                           QDateTime starttime, QDateTime endtime,
                           bool fingerprint1, bool fingerprint2,
                           bool askQuestions, QString question1, QString question2, QString question3,
                           QString status, QString desc,
                           QString sequence, int sequenceNum,
                           int maxAccess, int maxRetry, int accessType, int accessCount)
{
    KCB_DEBUG_ENTRY;


    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT INTO ") + TABLENAME +
                QString(" (sequence, sequence_order, "
                        "locknums, description, code1, "
                        "code2, starttime, endtime, fingerprint1, fingerprint2, ask_questions, question1, question2, question3, status, access_count,"
                        "retry_count, max_access, max_retry, lockbox_state, access_type)"
                        " VALUES (:seqDesc, :seqOrder, :lockNums, :desc, :codeOne, :codeTwo,"
                        " :start, :end, :fingerprint1, :fingerprint2, :ask_questions, :question1, :question2, :question3,"
                        " :stat, :access_count, 0, :maxAccess, :maxRetry, 0, :accessType)"));

    qDebug() << "Query:" << qry.lastQuery();

    code1 = CEncryption::encryptString(code1);
    code2 = CEncryption::encryptString(code2);

    qry.bindValue(":seqDesc", sequence);
    qry.bindValue(":seqOrder", sequenceNum);
    qry.bindValue(":lockNums", locknums);
    qry.bindValue(":desc", desc);
    qry.bindValue(":codeOne", code1);
    qry.bindValue(":codeTwo", code2);
    qry.bindValue(":start", starttime.toString(datetimeFormat));
    qry.bindValue(":end", endtime.toString(datetimeFormat));
    qry.bindValue(":stat", status);
    qry.bindValue(":fingerprint1", (int) fingerprint1);
    qry.bindValue(":fingerprint2", (int) fingerprint2);
    qry.bindValue(":ask_questions", (int) askQuestions);
    qry.bindValue(":question1", question1);
    qry.bindValue(":question2", question2);
    qry.bindValue(":question3", question3);
    qry.bindValue(":maxAccess", maxAccess);
    qry.bindValue(":maxRetry", maxRetry);
    qry.bindValue(":accessType", accessType);
    qry.bindValue(":access_count", accessCount);

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) {
        qDebug() << "CTblCodes::addLockCode():" << qry.lastError();
        qDebug() << "Query After:" << qry.lastQuery();
        return -1;
    }
    else {
        qDebug( "Inserted!" );
        QVariant var = qry.lastInsertId();
        if(var.isValid())
        {    int nId = var.toInt();
            return nId;
        } else {
            return -1;
        }
    }
}

bool CTblCodes::createTestDefault()
{
    KCB_DEBUG_ENTRY;
    QString s = CEncryption::encryptString("192837");
    QString     encCode1(s);
    s = CEncryption::encryptString("2837465");
    QString     encCode2(s);

    QSqlQuery qry(*_pDB);
    qry.prepare(QString("INSERT OR IGNORE INTO ") + TABLENAME +
                QString(" (sequence, sequence_order, "
                        "locknums, description, code1, "
                        "code2, starttime, endtime, fingerprint1, fingerprint2, status, access_count,"
                        "retry_count, max_access, max_retry, lockbox_state, access_type"
                        " VALUES ('', 0, 3, 'test lock', :codeOne, "
                        " :codeTwo, "
                        " :start, :end, :fingerprint1, :fingerprint2, 'ok', 0, 0, 0, 0)" ));

    qDebug() << "Query:" << qry.lastQuery();

    qDebug() << "code1:" << encCode1 << " code2:" << encCode2;

    qry.bindValue(":codeOne", encCode1);
    qry.bindValue(":codeTwo", encCode2);
    qry.bindValue(":start", DEFAULT_DATETIME_STR);
    qry.bindValue(":end", DEFAULT_DATETIME_STR);
    qry.bindValue(":fingerprint1", false);
    qry.bindValue(":fingerprint2", false);

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) {
        qDebug() << "CTblCodes::createTestDefault():" << qry.lastError();
        qDebug() << "Query After:" << qry.lastQuery();
        return false;
    }
    else {
        qDebug( "Inserted!" );
        return true;
    }
}

void CTblCodes::currentTimeFormat(QString format, QString strBuffer, int nExpectedLength)
{
    time_t rawtime;
    struct tm *currentTime;
    time ( &rawtime );
    currentTime = gmtime( &rawtime );
    char buffer [nExpectedLength+1];

    strftime(buffer, nExpectedLength, format.toStdString().c_str(), currentTime);

    strBuffer = buffer;
}

bool CTblCodes::readTestDefault()
{
    KCB_DEBUG_ENTRY;

    QSqlQuery query(*_pDB);
    QString sql = "SELECT sequence, sequence_order, "\
                  "locknums, description, code1,"\
                  "code2, starttime, endtime, status, access_count,"\
                  "retry_count, max_access, max_retry, access_type"
                  " FROM ";
    sql += TABLENAME;
    sql += QString(" WHERE description = 'test lock'");

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

            qDebug() << "CTblCodes::readTestDefault(): Code1:" << code1 << " len:" << code1.size();
            qDebug() << "CTblCodes::readTestDefault(): Code2:" << code2 << " len:" << code2.size();

            return true;
        }
    }
    return false;
}


bool CTblCodes::deleteCode(QString locknums, QString code1, QString code2,
                           QDateTime starttime, QDateTime endtime)
{
    Q_UNUSED(locknums);
    Q_UNUSED(code1);
    Q_UNUSED(code2);
    Q_UNUSED(starttime);
    Q_UNUSED(endtime);

    return false;
}

bool CTblCodes::deleteCode(CLockState &rec)
{
    qDebug( )<< "CTblCodes::deleteCode(CLockState)";

    if( rec.getID() == -1 ) {
        return false;
    }

    QSqlQuery query(*_pDB);
    QString sql = "DELETE FROM " + TABLENAME +
            QString(" WHERE ids = :fids");

    qDebug() << "CTblCodes::deleteCode sql:" << sql;

    query.prepare(sql);
    query.bindValue(":fids", rec.getID());
    if( query.exec()) {
        qDebug() << "CTblCodes::deleteCode(" << QVariant(rec.getID()).toString() << ") succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::deleteCode(" << QVariant(rec.getID()).toString() << ") failed";
        return false;
    }
}

bool CTblCodes::resetCodeLimitedUse(CLockState &rec)
{
    KCB_DEBUG_ENTRY;

    if (rec.getID() == -1)
    {
        return false;
    }

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("access_count=0, max_access=2 "
                              " WHERE access_type=2 and access_count > 0 and ids=:fids");
    // Limit reset to 'limited use' codes (access type 2)                              

    qDebug() << "CTblCodes::resetCodeLimitedUse(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", rec.getID());

    if(qry.exec()) 
    {
        qDebug() << "CTblCodes::resetCodeLimitedUse() succeeded";
        return true;
    } 
    else 
    {
        qDebug() << "CTblCodes::resetCodeLimitedUse() failed";
        return false;
    }
                              
}

bool CTblCodes::updateLockboxState(int fids, bool lockstate)
{
    KCB_DEBUG_ENTRY;

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("lockbox_state=:lockbox_state, access_count = access_count + 1 "
                              " WHERE ids=:fids");

    KCB_DEBUG_TRACE("query: " << sql);

    qry.prepare(sql);
    qry.bindValue(":fids", fids);
    qry.bindValue(":lockbox_state", lockstate);

    if(qry.exec()) 
    {
        KCB_DEBUG_TRACE("succeeded");
        return true;
    } 
    else 
    {
        KCB_DEBUG_TRACE("failed");
        return false;
    }
}

bool CTblCodes::updateAskQuestions(int fids, bool askQuestions)
{
    KCB_DEBUG_ENTRY;

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("ask_questions=:ask_questions"
                              " WHERE ids=:fids");

    KCB_DEBUG_TRACE("query: " << sql);

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(":ask_questions", askQuestions);

    if(qry.exec()) 
    {
        KCB_DEBUG_TRACE("succeeded");
        return true;
    } 
    else 
    {
        KCB_DEBUG_TRACE("failed");
        return false;
    }
}

bool CTblCodes::updateQuestion(int fids, QString which_question, QString value)
{
    KCB_DEBUG_ENTRY;

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("%1=:%1 WHERE ids=:fids").arg(which_question);

    KCB_DEBUG_TRACE("query: " << sql);

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(QString(":%1").arg(which_question), value);

    if(qry.exec())
    {
        KCB_DEBUG_TRACE(QString("succeeded (%1)").arg(which_question));
        return true;
    } 
    else 
    {
        KCB_DEBUG_TRACE(QString("failed (%1)").arg(which_question));
        return false;
    }
}

bool CTblCodes::updateQuestion1(int fids, QString question)
{
    KCB_DEBUG_ENTRY;

    bool result = updateQuestion(fids, "question1", question);

    KCB_DEBUG_EXIT;
    return result;
}

bool CTblCodes::updateQuestion2(int fids, QString question)
{
    KCB_DEBUG_ENTRY;

    bool result = updateQuestion(fids, "question2", question);

    KCB_DEBUG_EXIT;
    return result;
}

bool CTblCodes::updateQuestion3(int fids, QString question)
{
    KCB_DEBUG_ENTRY;

    bool result = updateQuestion(fids, "question3", question);

    KCB_DEBUG_EXIT;
    return result;
}

bool CTblCodes::updateRecord(CLockState &rec)
{
    KCB_DEBUG_ENTRY;

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("sequence=:seqDesc, sequence_order=:seqOrder, "
                              "locknums=:lockNums, description=:desc, code1=:codeOne, "
                              "code2=:codeTwo, starttime=:start, endtime=:end, fingerprint1=:fingerprinone, fingerprint2=:fingerprintwo, "
                              "ask_questions=:askQuestions, question1=:question1, question2=:question2, question3=:question3, "
                              "status=:stat, access_count=:accessCount,"
                              "retry_count=:retryCount, max_access=:maxAccess, max_retry=:maxRetry,"
                              " access_type=:accessType"
                              " WHERE ids=:fids");

    qry.prepare(sql);

    QString code1 = CEncryption::encryptString( rec.getCode1());
    QString code2 = CEncryption::encryptString( rec.getCode2());

    qry.bindValue(":codeOne", rec.getCode1());
    qry.bindValue(":codeTwo", rec.getCode2());

    qry.bindValue(":seqDesc", rec.getSequence());
    qry.bindValue(":seqOrder", rec.getSequenceOrder());
    qry.bindValue(":lockNums", rec.getLockNums());
    qry.bindValue(":desc", rec.getDescription());
    qry.bindValue(":codeOne", code1);
    qry.bindValue(":codeTwo", code2);
    qry.bindValue(":start", rec.getStartTime().toString(datetimeFormat));
    qry.bindValue(":end", rec.getEndTime().toString(datetimeFormat));
    qry.bindValue(":fingerprinone", (int) rec.getFingerprint1());
    qry.bindValue(":fingerprintwo", (int) rec.getFingerprint2());
    qry.bindValue(":askQuestions", (int) rec.getAskQuestions());
    qry.bindValue(":question1", rec.getQuestion1());
    qry.bindValue(":question2", rec.getQuestion2());
    qry.bindValue(":question3", rec.getQuestion3());
    qry.bindValue(":stat", rec.getStatus());
    qry.bindValue(":accessCount", rec.getAccessCount());
    qry.bindValue(":maxAccess", rec.getMaxAccess());
    qry.bindValue(":maxRetry", rec.getMaxRetry());
    qry.bindValue(":fids", rec.getID());
    qry.bindValue(":accessType", rec.getAccessType());

    if(qry.exec()) {
        qDebug() << "CTblCodes::updateRecord() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::updateRecord() failed";
        qDebug() << rec.getFingerprint1();
        return false;
    }
}

bool CTblCodes::updateCode(CLockState *prec)
{
    KCB_DEBUG_ENTRY;
    // update
    if(prec->isMarkedForDeletion()) 
    {
        KCB_DEBUG_TRACE("deleteCode");
        return deleteCode(*prec);
    } 
    else if (prec->isMarkedForReset())
    {        
        KCB_DEBUG_TRACE("resetCodeLimitedUse");
        return resetCodeLimitedUse(*prec);        
    }
    else 
    {
        if(prec->getID() == -1 ) 
        {
            KCB_DEBUG_TRACE("addLockCode");
            int nId = addLockCode(prec->getLockNums(),prec->getCode1(),prec->getCode2(),
                                  prec->getStartTime(), prec->getEndTime(),
                                  prec->getFingerprint1(), prec->getFingerprint2(),
                                  prec->getAskQuestions(), prec->getQuestion1(), prec->getQuestion2(), prec->getQuestion3(),
                                  prec->getStatus(), prec->getDescription(),
                                  prec->getSequence(), prec->getSequenceOrder(), prec->getMaxAccess(), prec->getMaxRetry(),
                                  prec->getAccessType(), prec->getAccessCount());
            if(nId != -1 )
            {
                return false;
            } 
            else 
            {
                prec->setID(nId);
                return true;
            }
        }

        if (prec->getID() > 0) 
        {
            if (prec->isModified())
            {
                updateRecord(*prec);
            }
        }
    }

    return true;
}

/**
 * @brief CTblCodes::updateCodeSet
 * @param codeSet
 * @return true if all were updated successfully, false if any fail (note: some may have updated if false is returned BAD!)
 */
bool CTblCodes::updateCodeSet(CLockSet &codeSet)
{
    bool    bRC = true;

    _pDB->transaction();

    CLockSet::Iterator  itor;
    // walk the set and update as we go
    for(itor = codeSet.begin(); itor != codeSet.end(); itor++)
    {
        int nRC = updateCode(itor.value());
        if(nRC == -1) 
		{
            bRC = false;
        }
    }

    if( !bRC ) 
    {
        KCB_DEBUG_TRACE("failed!");
        _pDB->rollback();
    } 
    else 
    {
        KCB_DEBUG_TRACE("succeeded. Committing...");
        if ( !_pDB->commit() )
        {
            KCB_DEBUG_TRACE("committed successfully.");
        }
    }
    return bRC;
}

bool CTblCodes::updateCodes(QJsonObject &jsonObj)
{
    CLockSet    lockSet;
    if(!lockSet.setFromJsonObject(jsonObj))
    {
        KCB_DEBUG_TRACE("invalid JSON Object Codeset");
    }
    // Valid set
    return updateCodeSet(lockSet);
}

bool CTblCodes::incrementAccessCount(int fids)
{
    KCB_DEBUG_ENTRY;
    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("access_count = access_count + 1 "
                              " WHERE ids=:fids");

    qDebug() << "CTblCodes::incrementAccessCount(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);

	bool result = qry.exec();
    if (result) 
	{
        KCB_DEBUG_TRACE("succeeded");
    } else 
	{
        KCB_DEBUG_TRACE("failed");
    }
	
	return result;
}

void CTblCodes::getAllCodes1(QStringList& codes1)
{
    // KCB_DEBUG_ENTRY;

    // Create a query to return all code1 entries
    QStringList column_list;
    column_list << "code1";
    QString condition = "";
    auto qry = createQuery(column_list, TABLENAME, condition);

    if (!qry.exec())
    {
        KCB_WARNING_TRACE(qry.lastError().text() << qry.lastQuery());
    }

    KCB_DEBUG_TRACE("Active" << qry.isActive() << "Select" << qry.isSelect());

    if (!qry.first())
    {
        KCB_WARNING_TRACE(qry.lastError().text() << qry.lastQuery());
    }

    KCB_DEBUG_TRACE("Retrieving at least first record that was found!");

    do
    {
        auto code1_enc = QUERY_VALUE(qry, "code1").toString();
        codes1.append(CEncryption::decryptString(code1_enc));
    } while (qry.next());

    // KCB_DEBUG_TRACE(codes1);

    // KCB_DEBUG_EXIT;
}

//-------------------------------------------------------------------------------------------------
// EOF
