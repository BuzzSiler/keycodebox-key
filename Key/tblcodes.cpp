#include <QTime>
#include <QDateTime>
#include <QString>
//#include <string>
#include "tblcodes.h"
#include "encryption.h"
#include "kcbcommon.h"

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

QString CTblCodes::checkCodeOne(QString code, bool &bSecondCodeRequired, bool &bFingerprintRequired, QString &LockNums )
{
    qDebug() << "CTblCodes::checkCodeOne()";
    // get the time.
    QDateTime time = QDateTime::currentDateTime();
    // hold on to the code    
    code = CEncryption::decryptString(code);
    _sCodeOne = code;   // Unencrypted
    LockNums = "";

    if( _pDB && _pDB->isOpen() ) 
    {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknums, description, "
                      "       code1, code2, fingerprint1, fingerprint2, "
                      "       starttime, endtime, status, access_count, retry_count, max_access, max_retry, access_type "
                      "  FROM " + TABLENAME +
                      " WHERE ((starttime = :startNone and endtime = :endNone) "
                      "    OR (starttime <= :time and endtime >= :timeend))";

        qDebug() << "SQL:" << sql;

        if( !qry.prepare(sql) ) 
        {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        //qDebug() << "startNone/endNone" << _DATENONE_STR;
        //qDebug() << "time" << time.toString("yyyy-MM-dd HH:mm:ss") << "timeend" << time.toString("yyyy-MM-dd HH:mm:ss");

        qry.bindValue(":startNone", _DATENONE_STR);
        qry.bindValue(":endNone", _DATENONE_STR);
        qry.bindValue(":time", time.toString("yyyy-MM-dd HH:mm:ss"));
        qry.bindValue(":timeend", time.toString("yyyy-MM-dd HH:mm:ss"));

        //qDebug() << "SQL:" << sql;

        LockNums = "";
        bSecondCodeRequired = true;
        int nSecondCode = 0;
        QString sCode1;
        QString sCode2;
        int nCount = 0; //
        int ids;    // temporarily hold the ids of last one
        int access_type;
        int max_access;
        int access_count;

        _lastIDS = -1;

        // Selection
        if(qry.exec())
        {
            qDebug() << "exec() ok ";

            int fldIds = qry.record().indexOf("ids");
            int fldLockNo = qry.record().indexOf("locknums");
            int fldCode1No = qry.record().indexOf("code1");
            int fldCode2No = qry.record().indexOf("code2");
            int fldFingerprint1No = qry.record().indexOf("fingerprint1");
            qDebug() << "fldLockNo:" << fldLockNo << "  fldCode1No:" << fldCode1No << "  fldCode2No:" << fldCode2No;
            qDebug() << "Count: " << qry.size() << " Fingerprint1: " << fldFingerprint1No;

            int fldAccessType = qry.record().indexOf("access_type");            
            int fldMaxAccess = qry.record().indexOf("max_access");
            int fldAccessCount = qry.record().indexOf("access_count");

            while(qry.next())
            {
                // Check how many we have.
                // If only one then see if it requires a second code.
                //   - If second code then signal that the second code is required.
                //   - If no second required then signal Ok to open door
                nCount++;
                ids = qry.value(fldIds).toInt();
                sCode1 = qry.value(fldCode1No).toString();
                sCode1 = CEncryption::decryptString(sCode1);


                //qDebug() << "sCode1" << sCode1 << "Code" << code;

                /// first one in the set to match unencrypted
                if( sCode1 == code ) 
                {
                    //qDebug() << "Code1:" << sCode1 << " == code:" << code;

                    /* Check for expiration */
                    access_type = qry.value(fldAccessType).toInt();
                    access_count = qry.value(fldAccessCount).toInt();
                    max_access = qry.value(fldMaxAccess).toInt();
                    if (isExpired(access_type, access_count, max_access))
                    {
                        return LockNums;
                    }                    

                    LockNums = qry.value(fldLockNo).toString();
                    if(qry.value(fldCode2No).isNull()) 
                    {
                        sCode2 = "";
                    } 
                    else 
                    {
                        sCode2 = qry.value(fldCode2No).toString();
                        sCode2 = CEncryption::decryptString(sCode2);
                    }
                    //qDebug() << "Code2:>" << sCode2 << "< and LockNums=" << LockNums;
                    if(!sCode2.trimmed().isEmpty())
                    {
                        nSecondCode++;
                        _lastIDS = 0;
                    } 
                    else 
                    {
                        // Should be successful
                        _lastIDS = ids;

                        /* Increment access_count */
                        incrementAccessCount(ids);
                    }
                    if(nSecondCode > 0 ) 
                    {
                        bSecondCodeRequired = true;
                    } 
                    else 
                    {
                        bSecondCodeRequired = false;
                    }

                    bFingerprintRequired = qry.value(fldFingerprint1No).toInt() == 1 ? true : false;

                    return LockNums;
                }
            }
            if(nSecondCode > 0 ) {
                bSecondCodeRequired = true;
            } else {
                bSecondCodeRequired = false;
            }

            //qDebug() << "CTblCodes::checkCodeOne LockNums" << LockNums;
            return LockNums;
        }
        else {
            qDebug() << "query.exec() failed." << qry.lastError();
        }
    }
    return LockNums;
}

bool CTblCodes::isWhiteSpace(const QString &str)
{
    return QRegExp("\\s*").exactMatch(str);
}

QString CTblCodes::checkCodeTwo(QString code, 
                            bool &bFingerprintRequired, 
                            bool &bQuestionsRequired, 
                            QString &codeOne, 
                            QString &LockNums, 
                            bool &bAskQuestions, 
                            QString &question1, 
                            QString &question2, 
                            QString &question3)
{
    // Make sure both the first code and the second code match
    // use _sCodeOne

    qDebug() << "CTblCodes::checkCodeTwo()";
    qDebug() << " code2:" << code;
    // get the time.
    QDateTime time = QDateTime::currentDateTime();
    // hold on to the code
    code = CEncryption::decryptString(code);
    _sCodeTwo = code;
    LockNums = "";

    if( _pDB && _pDB->isOpen() ) 
    {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknums, description, "
                      "code1, code2, fingerprint2, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry, lockbox_state, "
                      " ask_questions, question1, question2, question3, access_type"
                      " from " + TABLENAME +
                      " WHERE ((starttime = :startNone and endtime = :endNone) "
                      " or (starttime <= :time and endtime >= :timeend))";

        qDebug() << "SQL:" << sql;

        if( !qry.prepare(sql) ) {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        qry.bindValue(":startNone", _DATENONE_STR);
        qry.bindValue(":endNone", _DATENONE_STR);
        qry.bindValue(":time", time.toString("yyyy-MM-dd HH:mm:ss"));
        qry.bindValue(":timeend", time.toString("yyyy-MM-dd HH:mm:ss"));

        LockNums = "";
        QString sCode1;
        QString sCode2;
        int nCount = 0; //
        _lastIDS = -1;
        // Selection

        qry.setForwardOnly(true);

        if(qry.exec())
        {
            qDebug() << "exec() ok 2";

            int fldIDS = qry.record().indexOf("ids");
            int fldLockNo = qry.record().indexOf("locknums");
            int fldCode1No = qry.record().indexOf("code1");
            int fldCode2No = qry.record().indexOf("code2");
            int fldFingerprint2 = qry.record().indexOf("fingerprint2");
            int fldLockboxStatus = qry.record().indexOf("lockbox_state");
            int fldQuestion1 = qry.record().indexOf("question1");
            int fldQuestion2 = qry.record().indexOf("question2");
            int fldQuestion3 = qry.record().indexOf("question3");
            int fldAccessType = qry.record().indexOf("access_type");
            int fldAccessCount = qry.record().indexOf("access_count");
            int fldMaxAccess = qry.record().indexOf("max_access");
            qDebug() << "fldLockNo:" << fldLockNo << "  fldCode1No:" << fldCode1No << "  fldCode2No:" << fldCode2No;
            qDebug() << "Count: " << qry.size() << "   fldFingerprint2:" << fldFingerprint2 << "   fldLockboxStatus:" << fldLockboxStatus;

            /* I think we need to loop over all records and collect all matching records
                while (qry.next())
                {
                    // get code1 and code2 and decrypt
                    // if code1 and code2 match then
                    //     evaluate whether the lock should be added to the list
                    //     if codes not expired add to lock list
                    //     else add to lock list
                }

            */

            if(qry.first())
            {
                qDebug() << "qry.first()";
                do {
                    qDebug() << "qry.next()";
                    // Check how many we have.
                    // If only one then see if it requires a second code.
                    //   - If second code then signal that the second code is required.
                    //   - If no second required then signal Ok to open door
                    nCount++;
                    sCode1 = qry.value(fldCode1No).toString();
                    sCode1 = CEncryption::decryptString(sCode1);
                    sCode2 = qry.value(fldCode2No).toString();
                    sCode2 = CEncryption::decryptString(sCode2);

                    //qDebug() << "Code1:" << sCode1 << " == _sCodeOne:" << _sCodeOne;
                    //qDebug() << "Code2:" << sCode2 << " == _sCodeTwo:" << _sCodeTwo;

                    // first one in the set to match unencrypted
                    // Both codes must match
                    if( sCode1 == _sCodeOne && sCode2 == code)
                    {
                        _lastIDS = qry.value(fldIDS).toInt();
                        //qDebug() << "_lastIDS: " << QString::number(_lastIDS) << " Code2:" << sCode2 << " == code:" << code;


                        int access_type = qry.value(fldAccessType).toInt();
                        int access_count = qry.value(fldAccessCount).toInt();
                        int max_access = qry.value(fldMaxAccess).toInt();
                        /* Check for expiration */
                        if (isExpired(access_type, access_count, max_access))
                        {
                            return LockNums;
                        }                    

                        LockNums = qry.value(fldLockNo).toInt();

                        bFingerprintRequired = qry.value(fldFingerprint2).toInt() == 1 ? true : false;
                        if( bFingerprintRequired )
                        {
                            codeOne = sCode1;
                        }


                        // if lockbox status == 0, then item is IN
                        //            status == 1, then ttem is OUT

                        //qDebug() << "LOCKBOX STATE: " << QString::number(qry.value(fldLockboxStatus).toInt());

                        if( qry.value(fldLockboxStatus).toInt() == 0)
                        {
                            // item is being taken out

                            bQuestionsRequired = false;
                            updateLockboxState(_lastIDS, true);
                        }
                        else
                        {
                            question1 = qry.value(fldQuestion1).toString();
                            question2 = qry.value(fldQuestion2).toString();
                            question3 = qry.value(fldQuestion3).toString();

                            qDebug() << "ASK_QUESTIONS: " << QString::number(bAskQuestions);
                            qDebug() << "QUESTION1: " << question1;
                            qDebug() << "QUESTION2: " << question2;
                            qDebug() << "QUESTION3: " << question3;
                            QString emptyQString = "";

                            if( !question1.isEmpty() )
                            {
                                bQuestionsRequired = true;
                            }

                            if( !question2.isEmpty() )
                            {
                                bQuestionsRequired = true;
                            }

                            if( !question3.isEmpty() )
                            {
                                bQuestionsRequired = true;
                            }

                            updateLockboxState(_lastIDS, false);
                        }

                        return LockNums;
                    }
                } while(qry.next());
            } else {
                qDebug() << "Error:" << qry.lastError();
            }
        }
        else {
            qDebug() << "query.exec() failed." << qry.lastError();
        }
    }
    return LockNums;
}

void CTblCodes::selectCodeSet(QString &LockNums, QDateTime start, QDateTime end, CLockSet **pLockSet)
{
    qDebug() << "CTblCodes::selectCodeSet(LockNums" << LockNums << ", start" << start.toString() << ", end" << end.toString() << ", pLockSet)";
    // hold on to the code
    CLockState  *pLock;
    *pLockSet = 0;

    if( _pDB && _pDB->isOpen() ) 
    {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknums, description, "
                      "code1, code2, fingerprint1, fingerprint2, ask_questions, question1, question2, question3, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry, access_type"
                      " from " + TABLENAME +
                      " WHERE ";
        if(LockNums != "" ) 
        {
            sql += " locknums = :lockNums and ";
        }
        sql += " ((starttime = :startNone and endtime = :endNone) "
               " or (starttime >= :stime and endtime <= :etime))";

        qDebug() << "SQL:" << sql;

        if( !qry.prepare(sql) ) 
        {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        if(LockNums != "" ) 
        {
            qry.bindValue(":lockNums", LockNums);
        }
        qry.bindValue(":startNone", _DATENONE_STR);
        qry.bindValue(":endNone", _DATENONE_STR);
        qry.bindValue(":stime", start);
        qry.bindValue(":etime", end);

        QMap<QString, QVariant> mapVals = qry.boundValues();
        qDebug() << "Mapped count:" << mapVals.count();
        QMap<QString, QVariant>::Iterator   itor;
        for (itor = mapVals.begin(); itor != mapVals.end(); itor++)
        {
            qDebug() << " : " << (*itor).typeName() << " value:" << (*itor).toString();
        }

        QString sCode1;
        QString sCode2;
        int nCount = 0;
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
            int fldFingerprint1 = qry.record().indexOf("fingerprint1");
            int fldFingerprint2 = qry.record().indexOf("fingerprint2");
            int fldAskQuestions = qry.record().indexOf("ask_questions");
            int fldQuestion1 = qry.record().indexOf("question1");
            int fldQuestion2 = qry.record().indexOf("question2");
            int fldQuestion3 = qry.record().indexOf("question3");
            int fldAccessType = qry.record().indexOf("access_type");

            qDebug() << "fldLockNo:" << fldLockNo << "  fldCode1No:" << fldCode1No << "  fldCode2No:" << fldCode2No;
            qDebug() << "Count: " << qry.size() << "   fingerprint1: " << QString::number(fldFingerprint1) << "   fingerprint2: ";
            qDebug() << QString::number(fldFingerprint2);

            *pLockSet = new CLockSet();

            if( qry.first() )
            {
                qDebug() << "Retrieving at least first record that was found!";
                do
                {
                    pLock = new CLockState();
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

                    pLock->setID(qry.value(fldID).toInt());
                    pLock->setSequence(qry.value(fldSeq).toString());
                    pLock->setSequenceOrder(qry.value(fldSeqOrder).toInt());
                    pLock->setLockNums(qry.value(fldLockNo).toString());
                    pLock->setDescription(qry.value(fldDesc).toString());
                    pLock->setCode1(sCode1);    // unencrypted
                    pLock->setCode2(sCode2);    // unencrypted
                    pLock->setStartTime(qry.value(fldStart).toDateTime());
                    pLock->setEndTime(qry.value(fldEnd).toDateTime());
                    pLock->setStatus(qry.value(fldStatus).toString());
                    pLock->setAccessCount(qry.value(fldAccessCount).toInt());
                    pLock->setRetryCount(qry.value(fldRetryCount).toInt());
                    pLock->setMaxAccess(qry.value(fldMaxAccess).toInt());
                    pLock->setMaxRetry(qry.value(fldMaxRetry).toInt());

                    /* Note: The conversio from qry.value to bool had odd behavior which caused
                       the bool to become true even when it displayed as false.  I introduced
                       intermediate variables to store the conversion plus also biased the
                       conversion to be false unless explicitly true.
                    */
                    bool fp1 = qry.value(fldFingerprint1).toInt() == 1 ? true : false;
                    bool fp2 = qry.value(fldFingerprint2).toInt() == 1 ? true : false;

                    fp1 == true ? pLock->setFingerprint1() : pLock->clearFingerprint1();
                    fp2 == true ? pLock->setFingerprint2() : pLock->clearFingerprint2();

                    bool aq = qry.value(fldAskQuestions).toInt() == 1 ? true : false;
                    pLock->setAskQuestions(aq);

                    pLock->setAskQuestions(qry.value(fldAskQuestions).toInt());
                    pLock->setQuestion1(qry.value(fldQuestion1).toString());
                    pLock->setQuestion2(qry.value(fldQuestion2).toString());
                    pLock->setQuestion3(qry.value(fldQuestion3).toString());

                    pLock->setAccessType(qry.value(fldAccessType).toInt());

                    (*pLockSet)->addToSet(pLock);
                } while(qry.next());
            } 
            else 
            {
                qDebug() << "No FIRST record found!";
            }
        }
        else 
        {
            qDebug() << "query.exec() failed." << qry.lastError();
        }
    }
}

void CTblCodes::selectCodeSet(int ids, CLockSet **pLockSet)
{
    qDebug() << "CTblCodes::selectCodeSet(ids)";
    // hold on to the code
    CLockState  *pLock;
    *pLockSet = 0;

    if( _pDB && _pDB->isOpen() ) {
        QSqlQuery qry(*_pDB);
        QString sql = "SELECT ids, sequence, sequence_order, locknums, description, "
                      "code1, code2, fingerprint1, fingerprint2, ask_questions, question1, question2, question3, "
                      " starttime, endtime, status, access_count, retry_count, max_access, max_retry, access_type"
                      " from " + TABLENAME +
                      " WHERE ids = :id";

        qDebug() << "SQL:" << sql;

        if( !qry.prepare(sql) ) {
            qDebug() << "qry.prepare fails!" << qry.lastError();
        }

        qry.bindValue(":id", ids);

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
            int fldFingerprint1 = qry.record().indexOf("fingerprint1");
            int fldFingerprint2 = qry.record().indexOf("fingerprint2");
            int fldAskQuestions = qry.record().indexOf("ask_questions");
            int fldQuestion1 = qry.record().indexOf("question1");
            int fldQuestion2 = qry.record().indexOf("question2");
            int fldQuestion3 = qry.record().indexOf("question3");
            int fldAccessType = qry.record().indexOf("access_type");

            *pLockSet = new CLockSet();

            if( qry.first() )
            {
                qDebug() << "Retrieving at least first record that was found!";
                do
                {
                    pLock = new CLockState();
                    // Check how many we have.
                    // If only one then see if it requires a second code.
                    //   - If second code then signal that the second code is required.
                    //   - If no second required then signal Ok to open door
                    nCount++;
                    sCode1 = qry.value(fldCode1No).toString();
                    sCode2 = qry.value(fldCode2No).toString();
                    sCode1 = CEncryption::decryptString(sCode1);
                    sCode2 = CEncryption::decryptString(sCode2);

                    //LockNums = qry.value(fldLockNo).toString();

                    pLock->setID(qry.value(fldID).toInt());
                    pLock->setSequence(qry.value(fldSeq).toString());
                    pLock->setSequenceOrder(qry.value(fldSeqOrder).toInt());
                    pLock->setLockNums(qry.value(fldLockNo).toString());
                    pLock->setDescription(qry.value(fldDesc).toString());
                    pLock->setCode1(sCode1);    // unencrypted
                    pLock->setCode2(sCode2);    // unencrypted
                    pLock->setStartTime(qry.value(fldStart).toDateTime());
                    pLock->setEndTime(qry.value(fldEnd).toDateTime());
                    pLock->setStatus(qry.value(fldStatus).toString());
                    pLock->setAccessCount(qry.value(fldAccessCount).toInt());
                    pLock->setRetryCount(qry.value(fldRetryCount).toInt());
                    pLock->setMaxAccess(qry.value(fldMaxAccess).toInt());
                    pLock->setMaxRetry(qry.value(fldMaxRetry).toInt());

                    /* Note: The conversio from qry.value to bool had odd behavior which caused
                       the bool to become true even when it displayed as false.  I introduced
                       intermediate variables to store the conversion plus also biased the
                       conversion to be false unless explicitly true.
                    */
                    bool fp1 = qry.value(fldFingerprint1).toInt() == 1 ? true : false;
                    bool fp2 = qry.value(fldFingerprint2).toInt() == 1 ? true : false;

                    fp1 == true ? pLock->setFingerprint1() : pLock->clearFingerprint1();
                    fp2 == true ? pLock->setFingerprint2() : pLock->clearFingerprint2();

                    bool aq = qry.value(fldAskQuestions).toInt() == 1 ? true : false;
                    pLock->setAskQuestions(aq);

                    pLock->setQuestion1(qry.value(fldQuestion1).toString());
                    pLock->setQuestion2(qry.value(fldQuestion2).toString());
                    pLock->setQuestion3(qry.value(fldQuestion3).toString());

                    pLock->setAccessType(qry.value(fldAccessType).toInt());

                    (*pLockSet)->addToSet(pLock);
                } while(qry.next());
            } else {
                qDebug() << "No FIRST record found!";
            }
        }
        else {
            qDebug() << "query.exec() failed." << qry.lastError();
        }
    }
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

    // QStringList::iterator  itor;

    // for (itor = lstTables.begin(); itor != lstTables.end(); itor++)
    // {
    //     if((*itor).toStdString() == TABLENAME) {
    //         //
    //         return true;
    //     }
    // }
    // return false;
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
        std::cout << "Table does not Exist\n";
        createTable();
    }

    if(!columnExists(column))
        createColumn(column, "integer");
    if(!columnExists(column1))
        createColumn(column1, "integer");
    if(!columnExists(column2))
        createColumn(column2, "integer");
    if(!columnExists(column3))
        createColumn(column3, "integer");
    if(!columnExists(column4))
        createColumn(column4, "text");
    if(!columnExists(column5))
        createColumn(column5, "text");
    if(!columnExists(column6))
        createColumn(column6, "text");
}

void CTblCodes::createTable()
{
    std::cout << "CTblCodes::createTable\n";
    if( _pDB && _pDB->isOpen() ) {
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
            qDebug() << qry.lastError();
        else
            qDebug() << "Table created!";
    } else {
        std::cout << "Either _pDB is NULL or _pDB is not open\n";
    }

}

void CTblCodes::createColumn(QString column, QString fieldType)
{
    qDebug() << "CTblCodes::createColumn\n";
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

/**
 * @brief CTblCodes::addLockCodeClear
 * @param locknum
 * @param code1 - unencrypted?
 * @param code2 - unencrypted?
 * @param starttime
 * @param endtime
 * @param fingerprint1
 * @param fingerprint2
 * @param status
 * @param desc
 * @param sequenceNum
 * @param maxAccess
 * @param maxRetry
 */
int CTblCodes::addLockCodeClear(QString locknums, QString code1, QString code2,
                                QDateTime starttime, QDateTime endtime, bool fingerprint1, bool fingerprint2,
                                bool askQuestions, QString question1, QString question2, QString question3,
                                QString status, QString desc,
                                QString sequence, int sequenceNum,
                                int maxAccess, int maxRetry, int accessType, int accessCount)
{
    qDebug() << "CTblCodes::addLockCodeClear()";
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

// int CTblCodes::addLockCode(QString locknums, QString code1, QString code2,
//                            QDateTime starttime, QDateTime endtime,
//                            bool fingerprint1, bool fingerprint2,
//                            bool askQuestions, QString question1, QString question2, QString question3,
//                            QString status, QString desc,
//                            QString sequence, int sequenceNum,
//                            int maxAccess, int maxRetry, int accessType, int accessCount)
// {
//     qDebug() << "CTblCodes::addLockCode()";

//     QSqlQuery qry(*_pDB);
//     qry.prepare(QString("INSERT INTO ") + TABLENAME +
//                 QString(" (sequence, sequence_order, "
//                         "locknums, description, code1, "
//                         "code2, starttime, endtime, fingerprint1, fingerprint2, ask_questions, question1, question2, question3, status, access_count,"
//                         "retry_count, max_access, max_retry, lockbox_state, access_type)"
//                         " VALUES (:seqDesc, :seqOrder, :lockNums, :desc, :codeOne, :codeTwo,"
//                         " :start, :end, :fingerprint1, :fingerprint2, :ask_questions, :question1, :question2, :question3,"
//                         " :stat, :access_count, 0, :maxAccess, :maxRetry, 0, :accessType)"));

//     qDebug() << "Query:" << qry.lastQuery();

//     code1 = CEncryption::encryptString(code1);
//     code2 = CEncryption::encryptString(code2);

//     qry.bindValue(":seqDesc", sequence);
//     qry.bindValue(":seqOrder", sequenceNum);
//     qry.bindValue(":lockNums", locknums);
//     qry.bindValue(":desc", desc);
//     qry.bindValue(":codeOne", code1);
//     qry.bindValue(":codeTwo", code2);
//     qry.bindValue(":start", starttime.toString(datetimeFormat));
//     qry.bindValue(":end", endtime.toString(datetimeFormat));
//     qry.bindValue(":stat", status);
//     qry.bindValue(":fingerprint1", (int) fingerprint1);
//     qry.bindValue(":fingerprint2", (int) fingerprint2);
//     qry.bindValue(":ask_questions", (int) askQuestions);
//     qry.bindValue(":question1", question1);
//     qry.bindValue(":question2", question2);
//     qry.bindValue(":question3", question3);
//     qry.bindValue(":maxAccess", maxAccess);
//     qry.bindValue(":maxRetry", maxRetry);
//     qry.bindValue(":accessType", accessType);
//     qry.bindValue(":access_count", accessCount);

//     QMap<QString, QVariant> mapVals = qry.boundValues();
//     qDebug() << "Mapped count:" << mapVals.count();

//     if( !qry.exec() ) {
//         qDebug() << "CTblCodes::addLockCode():" << qry.lastError();
//         qDebug() << "Query After:" << qry.lastQuery();
//         return -1;
//     }
//     else {
//         qDebug( "Inserted!" );
//         QVariant var = qry.lastInsertId();
//         if(var.isValid())
//         {    int nId = var.toInt();
//             return nId;
//         } else {
//             return -1;
//         }
//     }
// }

int CTblCodes::addLockCode(QString locknums, QString code1, QString code2,
                           QDateTime starttime, QDateTime endtime,
                           bool fingerprint1, bool fingerprint2,
                           bool askQuestions, QString question1, QString question2, QString question3,
                           QString status, QString desc,
                           QString sequence, int sequenceNum,
                           int maxAccess, int maxRetry, int accessType, int accessCount)
{
    qDebug() << "CTblCodes::addLockCode()";

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
    qDebug() << "CTblCodes::createTestDefault()";
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
    qry.bindValue(":start", _DATENONE_STR);
    qry.bindValue(":end", _DATENONE_STR);
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

/**
 * @brief CTblCodes::currentTimeFormat
 * @param format
 * @param strBuffer
 * @param nExpectedLength = expected length of the return string
 */
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
    qDebug( )<< "CTblCodes::readTestDefault()";

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

/**
 * @brief CTblCodes::deleteCode
 *  For now will just delete based on the ids field
 * @param rec
 * @return
 */
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
    qDebug() << "CTblCodes::resetCodeLimitedUse(CLockState)";

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
    qDebug() << "CTblCodes::updateLockboxState()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("lockbox_state=:lockbox_state, access_count = access_count + 1 "
                              " WHERE ids=:fids");

    qDebug() << "CTblCodes::updateLockboxState(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(":lockbox_state", lockstate);

    if(qry.exec()) {
        qDebug() << "CTblCodes::updateLockboxState() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::updateLockboxState() failed";
        return false;
    }
}

bool CTblCodes::updateAskQuestions(int fids, bool askQuestions)
{
    qDebug() << "CTblAdmin::updateAskQuestions()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("ask_questions=:ask_questions"
                              " WHERE ids=:fids");

    qDebug() << "CTblAdmin::updateAskQuestions(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(":ask_questions", askQuestions);

    if(qry.exec()) {
        qDebug() << "CTblCodes::updateAskQuestions() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::updateAskQuestions() failed";
        return false;
    }
}

bool CTblCodes::updateQuestion1(int fids, QString question)
{
    qDebug() << "CTblCodes::updateQuestion1()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("question1=:question1"
                              " WHERE ids=:fids");

    qDebug() << "CTblCodes::updateQuestion1(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(":question1", question);

    if(qry.exec()) {
        qDebug() << "CTblCodes::updateQuestion1() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::updateQuestion1() failed";
        return false;
    }
}

bool CTblCodes::updateQuestion2(int fids, QString question)
{
    qDebug() << "CTblCodes::updateQuestio2()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("question2=:question2"
                              " WHERE ids=:fids");

    qDebug() << "CTblCodes::updateQuestion2(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(":question2", question);

    if(qry.exec()) {
        qDebug() << "CTblCodes::updateQuestion2() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::updateQuestion2() failed";
        return false;
    }
}

bool CTblCodes::updateQuestion3(int fids, QString question)
{
    qDebug() << "CTblCodes::updateQuestion3()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("question3=:question3"
                              " WHERE ids=:fids");

    qDebug() << "CTblCodes::updateQuestion3(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);
    qry.bindValue(":question3", question);

    if(qry.exec()) {
        qDebug() << "CTblCodes::updateQuestion3() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::updateQuestion3() failed";
        return false;
    }
}

bool CTblCodes::updateRecord(CLockState &rec)
{
    qDebug() << "CTblAdmin::updateRecord()";

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
    qDebug() << "CTblCodes::updateCode";
    // update
    if(prec->isMarkedForDeletion()) 
    {
        qDebug() << "CTblCodes::deleteCode";
        return deleteCode(*prec);
    } 
    else if (prec->isMarkedForReset())
    {        
        qDebug() << "CTblCodes::resetCodeLimitedUse";
        return resetCodeLimitedUse(*prec);        
    }
    else 
    {
        if(prec->getID() == -1 ) 
        {
            qDebug() << "CTblCodes::addLockCode";
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
        if(nRC == -1) {
            bRC = false;
        }
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

bool CTblCodes::updateCodes(QJsonObject &jsonObj)
{
    CLockSet    lockSet;
    if(!lockSet.setFromJsonObject(jsonObj))
    {
        qDebug() << "CTblCodes::updateCodes(): invalid JSON Object Codeset";
    }
    // Valid set
    return updateCodeSet(lockSet);
}

bool CTblCodes::incrementAccessCount(int fids)
{
    qDebug() << "CTblCodes::incrementAccessCount()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
            " SET " + QString("access_count = access_count + 1 "
                              " WHERE ids=:fids");

    qDebug() << "CTblCodes::incrementAccessCount(), query: " << sql;

    qry.prepare(sql);

    qry.bindValue(":fids", fids);

    if(qry.exec()) {
        qDebug() << "CTblCodes::incrementAccessCount() succeeded";
        return true;
    } else {
        qDebug() << "CTblCodes::incrementAccessCount() failed";
        return false;
    }
    
}