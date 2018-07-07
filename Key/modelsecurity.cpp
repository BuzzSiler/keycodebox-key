#include "modelsecurity.h"
#include <sqlite3.h>
#include <sqlite3ext.h>
#include "encryption.h"

#include <QtGlobal>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDateTime>
#include "kcbcommon.h"
#include "kcbapplication.h"


void CModelSecurity::openDatabase()
{
    _DB = QSqlDatabase::addDatabase( "QSQLITE", "SQLITE" );
    // Get the app directory
    QString sPath = "/home/pi/run/Alpha.db";
    qDebug() << "Path:" << sPath;
    _DB.setDatabaseName(sPath);

    if( !_DB.open() )
    {
        qDebug() << _DB.lastError();
        qFatal( "Failed to connect." );
    }
}

CModelSecurity::CModelSecurity(QObject *parent) : QObject(parent)
{
    initialize();

    qDebug( "Connected!" );
}

CModelSecurity::~CModelSecurity()
{
    removeCreatedObjects();
}

void CModelSecurity::OnReadLockSet(QString LockNums, QDateTime start, QDateTime end)
{
    // KCB_DEBUG_ENTRY;
    CLockSet    *pLockSet;

    _ptblCodes->selectCodeSet(LockNums, start, end, &pLockSet);
    // qDebug() << "Selected locks:" << LockNums;
    emit __OnLockSet(pLockSet);
}

void CModelSecurity::OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end)
{

    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE("Locks" << LockNums);
    CLockHistorySet *pLockHistorySet;
    _ptblCodeHistory->selectLockCodeHistorySet(LockNums, start, end, &pLockHistorySet);
    Q_ASSERT_X(pLockHistorySet != nullptr, Q_FUNC_INFO, "pLockHistorySet is null");

    if (pLockHistorySet)
    {
        emit __OnLockHistorySet(pLockHistorySet);
    }

    // KCB_DEBUG_EXIT;
}

void CModelSecurity::OnUpdateCurrentAdmin(CAdminRec *adminInfo)
{
    bool bSuccess = _ptblAdmin->updateAdminClear(adminInfo->getAdminName(), adminInfo->getAdminEmail(), adminInfo->getAdminPhone(),
                                                 adminInfo->getDefaultReportFreq(),
                                                 adminInfo->getDefaultReportStart(), adminInfo->getPassword(),
                                                 adminInfo->getAssistPassword(),
                                                 adminInfo->getDisplayFingerprintButton(), adminInfo->getDisplayShowHideButton(), adminInfo->getUsePredictiveAccessCode(), adminInfo->getPredictiveKey(), adminInfo->getPredictiveResolution(),
                                                 adminInfo->getMaxLocks(),
                                                 adminInfo->getSMTPServer(), adminInfo->getSMTPPort(), adminInfo->getSMTPType(),
                                                 adminInfo->getSMTPUsername(), adminInfo->getSMTPPassword(),
                                                 adminInfo->getVNCPort(), adminInfo->getVNCPassword(),
                                                 adminInfo->getReportViaEmail(), adminInfo->getReportToFile(), adminInfo->getReportDirectory(),
                                                 adminInfo->getDisplayPowerDownTimeout(),
                                                 adminInfo->getDefaultReportDeleteFreq(),
                                                 adminInfo->getDisplayTakeReturnButtons());
    emit __OnUpdatedCurrentAdmin(bSuccess);
}

void CModelSecurity::OnUpdateCodeState(CLockState *rec)
{
    qDebug() << "CModelSecurity::OnUpdateCodeState";
    bool bSuccess = _ptblCodes->updateCode(rec);

    emit __OnUpdatedCodeState(bSuccess);
}

void CModelSecurity::initialize()
{
    _ptblCodes = 0;
    _ptblCodeHistory = 0;
    _ptblAdmin = 0;

    openDatabase();
    setupTimer();
    setupTables();
}

void CModelSecurity::removeCreatedObjects()
{
    if(_ptblCodes) 
    {
        delete _ptblCodes;
    }
    if( _ptblCodeHistory) 
    {
        delete _ptblCodeHistory;
    }
    if( _ptblAdmin) 
    {
        delete _ptblAdmin;
    }
}

void CModelSecurity::OnTimeout()
{
    // Timeout waiting on security code?
    if( _type == "Admin" ) 
    {
        _type = "";
        emit __OnAdminSecurityCheckFailed();
    } 
    else 
    {
        _type = "";
        emit __OnSecurityCheckedFailed();
    }
    emit __OnSecurityCheckTimedOut();
}

void CModelSecurity::setupTimer()
{
    connect(&_timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
    _timer.setInterval(30000);
    _timer.setSingleShot(true);
}

void CModelSecurity::setupTables()
{
    if(_DB.isOpen())
    {
        _ptblCodes = new CTblCodes(&_DB);
        _ptblCodeHistory = new CTblCodeHistory(&_DB);
        _ptblAdmin = new CTblAdmin(&_DB);
    }
}

bool CModelSecurity::CheckPredictiveAccessCode(QString code, int &nLockNum)
{
    nLockNum = 0;
    QDateTime datetime = QDateTime::currentDateTime();
    QString skey;
    std::string outEncrypt = "";
    QString sTmp;
    QString filename="Data.txt";
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        datetime = CEncryption::roundDateTime(_ptblAdmin->getCurrentAdmin().getPredictiveResolution(), datetime);
        skey = _ptblAdmin->getCurrentAdmin().getPredictiveKey();

        for (nLockNum=1; nLockNum<256; nLockNum++) 
        {
            // Round the current date up to the resolution specified
            sTmp = "Predictive Matching:" + QVariant(nLockNum).toString();
            CEncryption::calculatePredictiveCodeOld(nLockNum, skey.toStdString().c_str(), datetime, &outEncrypt, 5 /*max results length*/, &sTmp);

            stream << sTmp << endl;

            if( outEncrypt == code.toStdString()) 
            {
                stream << "MATCH: Predict for Lock# " << QVariant(nLockNum).toString() << " calculated:" << outEncrypt.c_str() << " code to match:" << code << endl;
                return true;
            }
        }
        stream << "NO MATCH: Predict calculated: " << outEncrypt.c_str() << " code to match:" << code << endl;
    }

    emit __OnSecurityCheckedFailed();
    return false;
}

void CModelSecurity::OnVerifyCodeOne(QString code)
{
    KCB_DEBUG_TRACE(code);

    _type = "";  // "User" or "Admin" or "Assist" or ""

    _type = _ptblAdmin->isAccessCode(code);
    if(_type == "Admin" || _type == "Assist")
    {
        emit __OnRequireAdminPassword();
    }
    else
    {
        bool bSecondCodeRequired;
        bool bFingerprintRequired = false;
        int nDoorNum;

        if( _ptblAdmin->getCurrentAdmin().getUsePredictiveAccessCode() )
        {
            qDebug() << "CModelSecurity::OnVerifyCode() Predictive. Code:" << code;

            if( CheckPredictiveAccessCode(code, nDoorNum) )
            {
                // Check again to see if we match a second code existing in the codes table
                QString sCodeEnc = CEncryption::encryptString(code);
                QString lockNums;

                int result = _ptblCodes->checkCodeOne(sCodeEnc,
                                                      bSecondCodeRequired,
                                                      bFingerprintRequired,
                                                      lockNums);
                if( result == KCB_SUCCESS && lockNums != "" )
                {
                    // need to check if fingerprint security is enabled
                    // if so, check for existence of existing fingerprints
                    // if fingerprints do not exist, open up enroll dialog
                    // (otherwise let user know fingerprints are enrolled?
                    //  or emit __OnSecurityCheckFailed()

                    // this is probably a good time to mention,
                    // fingerprints for a second code are stored like:
                    //   /home/pi/run/prints/<codeOne>/<codeOne>.<codeTwo>
                    // single code are of course stored like:
                    //   /home/pi/run/prints/<codeOne>/<codeOne>

                    // need to do check again for code 2

                    if( bSecondCodeRequired )
                    {
                        emit __OnRequireCodeTwo();
                    }
                    else
                    {
                        if( bFingerprintRequired == true)
                        {
                            //we need to check if a fingerprint directory already exists,
                            // if they do, do not attempt enrollment
                            if( !QDir("/home/pi/run/prints/" + code).exists() )
                            {
                                emit __EnrollFingerprintDialog(code);
                                emit __EnrollFingerprint(code);
                            }
                            else
                            {
                                emit __OnSecurityCheckedFailed();
                            }
                            return;
                        }

                        emit __OnSecurityCheckSuccess(lockNums);
                        emit __OnCreateHistoryRecordFromLastPredictiveLogin(lockNums, code);
                    }
                }
                else
                {
                    emit __OnSecurityCheckedFailed();
                }
            }
        }
        else
        {
            
            QString sCodeEnc = CEncryption::encryptString(code);
            QString lockNums;

            KCB_DEBUG_TRACE("Encrypted Code" << sCodeEnc);

            int result = _ptblCodes->checkCodeOne(sCodeEnc,
                                                  bSecondCodeRequired,
                                                  bFingerprintRequired,
                                                  lockNums);
            KCB_DEBUG_TRACE("Locks" << lockNums);
            if( result == KCB_SUCCESS && lockNums != "" )
            {
                // need to check if fingerprint security is enabled
                // if so, check for existence of existing fingerprints
                // if fingerprints do not exist, open up enroll dialog
                // (otherwise let user know fingerprints are enrolled?
                //  or emit __OnSecurityCheckFailed()

                // this is probably a good time to mention,
                // fingerprints for a second code are stored like:
                //   /home/pi/run/prints/<codeOne>/<codeOne>.<codeTwo>
                // single code are of course stored like:
                //   /home/pi/run/prints/<codeOne>/<codeOne>

                // need to do check again for code 2

                if( bSecondCodeRequired ) 
                {
                    emit __OnRequireCodeTwo();
                } 
                else 
                {
                    if( bFingerprintRequired == true)
                    {
                        //we need to check if a fingerprint directory already exists,
                        // if they do, do not attempt enrollment
                        if( !QDir("/home/pi/run/prints/" + code).exists() )
                        {
                            emit __EnrollFingerprintDialog(code);
                            emit __EnrollFingerprint(code);
                        }
                        else
                        {
                            emit __OnSecurityCheckedFailed();
                        }
                        KCB_DEBUG_TRACE("fingerprint required");
                        return;
                    }

                    emit __OnSecurityCheckSuccess(lockNums);
                }
            }
            else
            {
                emit __OnSecurityCheckedFailed();
            }
        }
    }

    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnVerifyFingerprintCodeOne(QString code)
{
    // Check the DB
    _type = "";  // "User" or "Admin" or "Assist"

    qDebug() << "CModelSecurity::OnVerifyFingerprintCodeOne. Code:" << code;

    _type = _ptblAdmin->isAccessCode(code);
    if(_type == "Admin" || _type == "Assist") 
    {
        emit __OnRequireAdminPassword();
    }
    else
    {
        bool bSecondCodeRequired;
        bool bFingerprintRequired = false;
        int nDoorNum;

        if( _ptblAdmin->getCurrentAdmin().getUsePredictiveAccessCode() )
        {
            KCB_DEBUG_TRACE("Predictive Code:" << code);
            CheckPredictiveAccessCode(code, nDoorNum);

        } 
        else if( 0 /*TBD - _ptblAdmin->getCurrentAdmin().getUseAnyAccessCode() */ )
        {
            // Store AnyAccessCode = code
            // Enter Secondary code - Verify against DB (second access code?)
        }
        else
        {
            QString sCodeEnc = CEncryption::encryptString(code);
            QString lockNums = "";

            int result = _ptblCodes->checkCodeOne(sCodeEnc,
                                                  bSecondCodeRequired,
                                                  bFingerprintRequired,
                                                  lockNums);
            if( result == KCB_SUCCESS && lockNums != "" )
            {
                // need to check if fingerprint security is enabled
                // if so, check for existence of existing fingerprints
                // if fingerprints do not exist, open up enroll dialog
                // (otherwise let user know fingerprints are enrolled?
                //  or emit __OnSecurityCheckFailed()

                // this is probably a good time to mention,
                // fingerprints for a second code are stored like:
                //   /home/pi/run/prints/<codeOne>/<codeOne>.<codeTwo>
                // single code are of course stored like:
                //   /home/pi/run/prints/<codeOne>/<codeOne>

                // need to do check again for code 2

                if( bSecondCodeRequired ) 
                {
                    emit __OnRequireCodeTwo();
                } 
                else 
                {

                    emit __OnSecurityCheckSuccess(lockNums);
                }
            } 
            else 
            {
                emit __OnSecurityCheckedFailed();
            }
        }
    }
}

void CModelSecurity::OnVerifyCodeTwo(QString code)
{
    KCB_DEBUG_TRACE(code);

    bool bFingerprintRequired = false;
    QString codeOne;
    bool bAskQuestions = false;
    QString question1 = "";
    QString question2 = "";
    QString question3 = "";

    _updateCodeLockboxState = false;
    if(_type == "Admin" || _type == "Assist") 
    {
        if(_ptblAdmin->isPassword(code, _type))
        {
            emit __OnAdminSecurityCheckOk(_type);
        }
        else
        {
            emit __OnAdminSecurityCheckFailed();
        }
    }
    else if( _type == "User" )
    {
        QString lockNums;

        KCB_DEBUG_TRACE("code1" << codeOne << "code2" << code);

        int result = _ptblCodes->checkCodeTwo(code,
                                             bFingerprintRequired,
                                             codeOne,
                                             lockNums,
                                             bAskQuestions,
                                             question1,
                                             question2,
                                             question3);
        KCB_DEBUG_TRACE("Locks" << lockNums);
		if (result == KCB_SUCCESS && lockNums != "")
        {
            KCB_DEBUG_TRACE(lockNums);

            //we need to check if a fingerprint directory already exists,
            // if they do, do not attempt enrollmesnt
            if( bFingerprintRequired == true)
            {
                // this is probably a good time to mention,
                // fingerprints for a second code are stored like:
                //   /home/pi/run/prints/<codeOne>/<codeOne>.<codeTwo>
                // single code are of course stored like:
                //   /home/pi/run/prints/<codeOne>/<codeOne>

                if ( !QDir( QString("%1%2.%3").arg("/home/pi/run/prints/").arg(codeOne).arg(code) ).exists() )
                {
                    KCB_DEBUG_TRACE("fp check success" << codeOne << "." << code);
                    emit __EnrollFingerprintDialog(codeOne + "." + code);
                    emit __EnrollFingerprint(codeOne + "." + code);
                }
                else
                {
                    KCB_DEBUG_TRACE("fp check failed");
                    emit __OnSecurityCheckedFailed();
                }

                return;
            }

            if ( bAskQuestions && kcb::Application::isReturnSelection() )
            {
                qDebug() << "QUESTION1: " << question1;
                qDebug() << "QUESTION2: " << question2;
                qDebug() << "QUESTION3: " << question3;

                emit __QuestionUserDialog(lockNums,question1,question2,question3);
            }
            else
            {
                KCB_DEBUG_TRACE("check success" << lockNums);
                emit __OnSecurityCheckSuccess(lockNums);
            }
        }
        else
        {
            KCB_DEBUG_TRACE("check failed");
            emit __OnSecurityCheckedFailed();
        }
    }

    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnSuccessfulQuestionUsersAnswers(QString lockNums, QString answer1, QString answer2, QString answer3)
{
    KCB_DEBUG_ENTRY;
    emit __OnSecurityCheckSuccessWithAnswers(lockNums, answer1, answer2, answer3);
}

void CModelSecurity::OnQuestionUserCancelled()
{
    KCB_DEBUG_ENTRY;
    _ptblCodes->updateLockboxState(_ptblCodes->_lastIDS, true);
}

void CModelSecurity::OnVerifyFingerprintCodeTwo(QString code)
{
    bool bFingerprintRequired = false;
    QString codeOne;
    bool bAskQuestions = false;
    QString question1 = "";
    QString question2 = "";
    QString question3 = "";

    if(_type == "Admin" || _type == "Assist")
    {
        if(_ptblAdmin->isPassword(code, _type))
        {
            emit __OnAdminSecurityCheckOk(_type);
        }
        else
        {
            emit __OnAdminSecurityCheckFailed();
        }
    }
    else if( _type == "User" )
    {
        QString lockNums;
        int result = _ptblCodes->checkCodeTwo(code,
                                              bFingerprintRequired,
                                              codeOne,
                                              lockNums,
                                              bAskQuestions,
                                              question1,
                                              question2,
                                              question3);
		if (result == KCB_SUCCESS && lockNums != "")
		{
            emit __OnSecurityCheckSuccess(lockNums);
        }
        else
        {
            emit __OnSecurityCheckedFailed();
        }
    }
    // Might have timed out and cleared the _type
}

void CModelSecurity::OnCreateHistoryRecordFromLastPredictiveLogin(QString LockNums, QString code)
{
    KCB_DEBUG_ENTRY;

    CLockHistoryRec lockHistoryRec;
    CLockState  lockState;

    // Build the lock state for this lock num
    lockState.setLockNums(LockNums);
    lockState.setCode1(code);
    lockHistoryRec.setFromLockState(lockState);
    _ptblCodeHistory->addLockCodeHistory(lockHistoryRec);
    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnCreateHistoryRecordFromLastSuccessfulLogin()
{
    KCB_DEBUG_ENTRY;

    int ids = _ptblCodes->getLastSuccessfulIDS();

    KCB_DEBUG_TRACE("Ids" << ids);
    if(ids != -1)
    {
        int         nVal;
        CLockSet    *pLockSet;
        // Build the lock set for this lock num
        _ptblCodes->selectCodeSet(ids, &pLockSet);

        CLockHistoryRec lockHistoryRec;
        CLockState      *pState;
        nVal = 0;
        for(CLockSet::Iterator itor = pLockSet->begin(); itor != pLockSet->end(); itor++) 
        {
            nVal++;
            pState = itor.value();
            lockHistoryRec.setFromLockState(*pState);
            lockHistoryRec.setLockNums(pState->getLockNums());
            lockHistoryRec.setAccessSelection(kcb::Application::getAccessSelection());
            _ptblCodeHistory->addLockCodeHistory(lockHistoryRec);
        }
        if(nVal > 1) 
        {
            KCB_DEBUG_TRACE("Error has more than 1 record");
        }
    }
    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString answer1, QString answer2, QString answer3)
{
    KCB_DEBUG_ENTRY;

    int ids = _ptblCodes->getLastSuccessfulIDS();
    KCB_DEBUG_TRACE("Ids" << ids);

    if(ids != -1)
    {
        int         nVal;
        CLockSet    *pLockSet;
        // Build the lock set for this lock num
        _ptblCodes->selectCodeSet(ids, &pLockSet);

        CLockHistoryRec lockHistoryRec;
        CLockState      *pState;
        nVal = 0;
        for(CLockSet::Iterator itor = pLockSet->begin(); itor != pLockSet->end(); itor++) 
        {
            nVal++;
            pState = itor.value();
            lockHistoryRec.setFromLockState(*pState);
            lockHistoryRec.setLockNums(pState->getLockNums());
            lockHistoryRec.setAccessSelection(kcb::Application::getAccessSelection());
            _ptblCodeHistory->addLockCodeHistoryWithAnswers(lockHistoryRec, answer1, answer2, answer3);
        }
        if(nVal > 1) 
        {
            KCB_DEBUG_TRACE("Error has more than 1 record");
        }
    }
    KCB_DEBUG_EXIT;
    
}

void CModelSecurity::RequestLastSuccessfulLogin(QString locknums, QString answer1, QString answer2, QString answer3)
{
    KCB_DEBUG_ENTRY;
    
    KCB_DEBUG_TRACE(locknums);
    if( _ptblAdmin->getCurrentAdmin().getUsePredictiveAccessCode() )
    {
        qDebug() << "RequestLastSuccessfulLogin( Predictive )";
        CLockHistorySet *_pHistorySet = NULL;

        QDateTime   time, now;
        time = time.currentDateTime().addSecs(-5);  // 5 Seconds ago
        now = now.currentDateTime();
        QString LockNums;

        qDebug() << ">>SENDING >>> Start:" << time.toString(DATETIME_FORMAT) << "  End:" << now.toString(DATETIME_FORMAT);

        _ptblCodeHistory->selectLastLockCodeHistorySet(LockNums, time, now, &_pHistorySet);

    }
    else
    {
        
        int ids = _ptblCodes->getLastSuccessfulIDS();
        KCB_DEBUG_TRACE("Ids" << ids);
        if(ids != -1)
        {
            qDebug() << "lastSuccessfulIDS ids" << ids << "locks" << locknums;
            int         nVal;
            CLockSet    *pLockSet;
            // Build the lock set for this lock num
            _ptblCodes->selectCodeSet(ids, &pLockSet);

            CLockHistoryRec *plockHistoryRec;
            CLockState      *pState;
            nVal = 0;
            for(CLockSet::Iterator itor = pLockSet->begin(); itor != pLockSet->end(); itor++) 
            {
                nVal++;
                qDebug() << "count" << nVal;
                pState = itor.value();
                plockHistoryRec = new CLockHistoryRec();
                plockHistoryRec->setFromLockState(*pState);
                // Note: What we get back from the database is the code entry.  We want to 
                // enter into the history the locks that were actually opened not the locks
                // assigned to the code.  So, we initialize the history record with the default
                // for the code and then override the locks value.
                plockHistoryRec->setLockNums(locknums);

                // Despite the fact that we are in a loop, there is only ever one response from
                // the code table query, i.e., ids is single value associated with a single code
                // So, since we're here, we will add an entry to the lock history for this lock
                // More desirable to know exactly what locks were opened as opposed to the 
                // 'possible' locks that can be opened which is what selectCodeSet gives us.


                // Set the answers if they exist, i.e., not empty
                if (!answer1.isEmpty() || !answer2.isEmpty() || !answer3.isEmpty())
                {
                    _ptblCodeHistory->addLockCodeHistoryWithAnswers(*plockHistoryRec, answer1, answer2, answer3);
                }
                else
                {
                    _ptblCodeHistory->addLockCodeHistory(*plockHistoryRec);
                }

                emit __OnLastSuccessfulLogin(plockHistoryRec);
            }
            if(nVal > 1) 
            {
                KCB_DEBUG_TRACE("Error has more than 1 record");
            }
        } 
        else 
        {
            qDebug() << "ids == -1)";
        }
    }

    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnRequestCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd)
{
    KCB_DEBUG_ENTRY;
    CLockHistorySet *pLockHistorySet;
    _ptblCodeHistory->selectLockCodeHistorySet(QString(""), dtStart, dtEnd, &pLockHistorySet);
    if (pLockHistorySet)
    {
        emit __OnCodeHistoryForDateRange(pLockHistorySet);
    }
    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnVerifyAdminPassword(QString code)
{
    if(_type == "Admin" || _type == "Assist")
    {
        if(_ptblAdmin->isPassword(code, _type))
        {
            emit __OnAdminSecurityCheckOk(_type);
        }
        else
        {
            emit __OnAdminSecurityCheckFailed();
        }
    }
    // Might have timed out and cleared the _type
}

void CModelSecurity::OnRequestCurrentAdmin()
{
    qDebug() << "CModelSecurity::OnRequestCurrentAdmin() : retrieved current admin -> emit __OnRequestedCurrentAdmin(CAdminRec*)";

    emit __OnRequestedCurrentAdmin(&_ptblAdmin->getCurrentAdmin());
}

void CModelSecurity::getAllCodes1(QStringList& codes1)
{
    _ptblCodes->getAllCodes1(codes1);
}
