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
    CLockSet    *pLockSet;
    // Build the lock set for this lock num
    qDebug() << "CModelSecurity::OnReadLockSet";

    _ptblCodes->selectCodeSet(LockNums, start, end, &pLockSet);
    qDebug() << "Selected locks:" << LockNums;
    emit __OnLockSet(pLockSet);
}

void CModelSecurity::OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end)
{
    CLockHistorySet *pLockHistorySet;
    // Build the lock history set for this locknum and date range
    _ptblCodeHistory->selectLockCodeHistorySet(LockNums, start, end, &pLockHistorySet);

    Q_ASSERT_X(pLockHistorySet != nullptr, "CModelSecurity::OnReadLockHistorySet", "pLockHistorySet is null");

    emit __OnLockHistorySet(pLockHistorySet);
}

/**
 * @brief CModelSecurity::OnUpdateCurrentAdmin
 * @param adminInfo
 * Password and AccessCode are required to be clear text here.
 */
void CModelSecurity::OnUpdateCurrentAdmin(CAdminRec *adminInfo)
{
    bool bSuccess = _ptblAdmin->updateAdminClear(adminInfo->getAdminName(), adminInfo->getAdminEmail(), adminInfo->getAdminPhone(),
                                                 adminInfo->getEmailReportActive(), adminInfo->getDefaultReportFreq(),
                                                 adminInfo->getDefaultReportStart(), adminInfo->getPassword(), adminInfo->getAccessCode(),
                                                 adminInfo->getAssistPassword(), adminInfo->getAssistCode(),
                                                 adminInfo->getDisplayFingerprintButton(), adminInfo->getDisplayShowHideButton(),
                                                 adminInfo->getUsePredictiveAccessCode(), adminInfo->getPredictiveKey(), adminInfo->getPredictiveResolution(),
                                                 adminInfo->getMaxLocks(),
                                                 adminInfo->getSMTPServer(), adminInfo->getSMTPPort(), adminInfo->getSMTPType(),
                                                 adminInfo->getSMTPUsername(), adminInfo->getSMTPPassword(),
                                                 adminInfo->getVNCPort(), adminInfo->getVNCPassword(),
                                                 adminInfo->getReportViaEmail(), adminInfo->getReportToFile(), adminInfo->getReportDirectory());
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
    if(_ptblCodes) {
        delete _ptblCodes;
    }
    if( _ptblCodeHistory) {
        delete _ptblCodeHistory;
    }
    if( _ptblAdmin) {
        delete _ptblAdmin;
    }
}

void CModelSecurity::OnTimeout()
{
    // Timeout waiting on security code?
    if( _type == "Admin" ) {
        _type = "";
        emit __OnAdminSecurityCheckFailed();
    } else {
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
        else if( 0 /*TBD - _ptblAdmin->getCurrentAdmin().getUseAnyAccessCode() */ )
        {
            // Store AnyAccessCode = code
            // Enter Secondary code - Verify against DB (second access code?)            
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
    bool bQuestionsRequired = false;
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
                                             bQuestionsRequired,
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

            if( bQuestionsRequired == true)
            {
                qDebug() << "ASK_QUESTIONS: " << QString::number(bAskQuestions);
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
    qDebug() << "CModelSecurity::OnSuccessfulQuestionUsersAnswers()";
    emit __OnSecurityCheckSuccessWithAnswers(lockNums, answer1, answer2, answer3);
}

void CModelSecurity::OnQuestionUserCancelled()
{
    qDebug() << "CModelSecurity::OnQuestionUserCancelled()";
    //emit __OnSecurityCheckedFailed();
    _ptblCodes->updateLockboxState(_ptblCodes->_lastIDS, true);
}

void CModelSecurity::OnVerifyFingerprintCodeTwo(QString code)
{
    //    _timer.stop();
    bool bFingerprintRequired = false;
    bool bQuestionsRequired = false;
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
                                              bQuestionsRequired,
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
            _ptblCodeHistory->addLockCodeHistoryWithAnswers(lockHistoryRec, answer1, answer2, answer3);
        }
        if(nVal > 1) {
            KCB_DEBUG_TRACE("Error has more than 1 record");
        }
    }
    KCB_DEBUG_EXIT;
    
}

void CModelSecurity::RequestLastSuccessfulLogin()
{
    KCB_DEBUG_ENTRY;
    if( !_ptblAdmin->getCurrentAdmin().getUsePredictiveAccessCode() )
    {
        int ids = _ptblCodes->getLastSuccessfulIDS();
        if(ids != -1)
        {
            qDebug() << "--------lastSuccessfulIDS ids != -1";
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
                pState = itor.value();
                plockHistoryRec = new CLockHistoryRec();
                plockHistoryRec->setFromLockState(*pState);
                //
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
    else 
    {
        qDebug() << "RequestLastSuccessfulLogin( Predictive )";
        CLockHistorySet *_pHistorySet = NULL;

        QDateTime   time, now;
        time = time.currentDateTime().addSecs(-5);  // 5 Seconds ago
        now = now.currentDateTime();
        QString LockNums;

        qDebug() << ">>SENDING >>> Start:" << time.toString("yyyy-MM-dd HH:mm:ss") << "  End:" << now.toString("yyyy-MM-dd HH:mm:ss");

        _ptblCodeHistory->selectLastLockCodeHistorySet(LockNums, time, now, &_pHistorySet);

        // Predictive - so history rec
        CLockHistoryRec *plockHistoryRec;

        for(CLockHistorySet::Iterator itor = _pHistorySet->begin(); itor != _pHistorySet->end(); itor++) 
        {
            plockHistoryRec = itor.value();
            //
            qDebug() << "  Found last successful login";
            emit __OnLastSuccessfulLogin(plockHistoryRec);
        }
    }

    KCB_DEBUG_EXIT;
}

void CModelSecurity::OnRequestCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd)
{
    CLockHistorySet     *pLockHistorySet = new CLockHistorySet();
    // Get the data
    QString LockNums;
    _ptblCodeHistory->selectLockCodeHistorySet(LockNums, dtStart, dtEnd, &pLockHistorySet);
    // Send it back
    emit __OnCodeHistoryForDateRange(dtStart, dtEnd, pLockHistorySet);
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
