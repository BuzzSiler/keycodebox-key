#include <QDebug>
#include "securitycontroller.h"
#include "modelsecurity.h"
#include "encryption.h"
#include "kcbcommon.h"


CSecurityController::CSecurityController(QObject *parent) : QObject(parent),
    _pSecurityUser(0)
{
    initializeSignals();

    // In own thread
    qDebug() << "CSecurityController(): moveToThread.";
    _modelSecurity.moveToThread(&_securityControlThread);
    _securityControlThread.start();
}

void CSecurityController::initializeSignals()
{
    connect(&_modelSecurity, SIGNAL(__OnRequireAdminPassword()), this, SLOT(OnRequireAdminPassword()));
    connect(&_modelSecurity, SIGNAL(__OnRequireCodeTwo()), this, SLOT(OnRequireCodeTwo()));
    connect(&_modelSecurity, SIGNAL(__OnAdminSecurityCheckOk(QString)), this, SLOT(OnAdminSecurityCheckOk(QString)));
    connect(&_modelSecurity, SIGNAL(__OnAdminSecurityCheckFailed()), this, SLOT(OnAdminSecurityCheckFailed()));
    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckSuccess(QString)), this, SLOT(OnSecurityCheckSuccess(QString)));

    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckSuccessWithAnswers(QString,QString,QString,QString)), this, SLOT(OnSecurityCheckSuccessWithAnswers(QString,QString,QString,QString)));

    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckedFailed()), this, SLOT(OnSecurityCheckedFailed()));

    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckTimedOut()), this, SLOT(OnSecurityCheckTimedOut()));

    connect(this, SIGNAL(__VerifyCodeOne(QString)), &_modelSecurity, SLOT(OnVerifyCodeOne(QString)));
    connect(&_modelSecurity, SIGNAL(__EnrollFingerprint(QString)), this, SLOT(OnEnrollFingerprint(QString)));
    connect(&_modelSecurity, SIGNAL(__EnrollFingerprintDialog(QString)), this, SLOT(OnEnrollFingerprintDialog(QString)));

    connect(&_modelSecurity, SIGNAL(__QuestionUserDialog(QString,QString,QString,QString)), this, SLOT(OnQuestionUserDialog(QString,QString,QString,QString)));
    
    connect(this, SIGNAL(__VerifyCodeTwo(QString)), &_modelSecurity, SLOT(OnVerifyCodeTwo(QString)));


    connect(this, SIGNAL(__VerifyFingerprintCodeOne(QString)), &_modelSecurity, SLOT(OnVerifyFingerprintCodeOne(QString)));
    connect(this, SIGNAL(__VerifyFingerprintCodeTwo(QString)), &_modelSecurity, SLOT(OnVerifyFingerprintCodeTwo(QString)));

    connect(this, SIGNAL(__VerifyAdminPassword(QString)), &_modelSecurity, SLOT(OnVerifyAdminPassword(QString)));

    // Data
    connect(this, SIGNAL(__OnRequestCurrentAdmin()), &_modelSecurity, SLOT(OnRequestCurrentAdmin()));
    connect(&_modelSecurity, SIGNAL(__OnRequestedCurrentAdmin(CAdminRec*)), this, SLOT(OnRequestedCurrentAdmin(CAdminRec*)));

    connect(this, SIGNAL(__UpdateCurrentAdmin(CAdminRec*)), &_modelSecurity, SLOT(OnUpdateCurrentAdmin(CAdminRec*)));
    connect(&_modelSecurity, SIGNAL(__OnUpdatedCurrentAdmin(bool)), this, SLOT(OnUpdatedCurrentAdmin(bool)));

    connect(this, SIGNAL(__OnReadLockSet(QString,QDateTime,QDateTime)), &_modelSecurity, SLOT(OnReadLockSet(QString,QDateTime,QDateTime)));
    connect(&_modelSecurity, SIGNAL(__OnLockSet(CLockSet*)), this, SLOT(OnLockSet(CLockSet*)));

    connect(this, SIGNAL(__OnReadLockHistorySet(QString,QDateTime,QDateTime)), &_modelSecurity, SLOT(OnReadLockHistorySet(QString,QDateTime,QDateTime)));
    connect(&_modelSecurity, SIGNAL(__OnLockHistorySet(CLockHistorySet*)), this, SLOT(OnLockHistorySet(CLockHistorySet*)));

    connect(this, SIGNAL(__OnCreateHistoryRecordFromLastSuccessfulLogin()), &_modelSecurity, SLOT(OnCreateHistoryRecordFromLastSuccessfulLogin()));
    connect(this, SIGNAL(__OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString,QString,QString)), &_modelSecurity, SLOT(OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString,QString,QString)));
    
    connect(&_modelSecurity, SIGNAL(__OnCreateHistoryRecordFromLastPredictiveLogin(QString, QString)), &_modelSecurity, SLOT(OnCreateHistoryRecordFromLastPredictiveLogin(QString, QString)));
    connect(&_modelSecurity, SIGNAL(__OnLastSuccessfulLogin(CLockHistoryRec*)), this, SLOT(OnLastSuccessfulLoginRequest(CLockHistoryRec*)));
    connect(this, SIGNAL( __RequestLastSuccessfulLogin(QString, QString, QString, QString)), &_modelSecurity, SLOT(RequestLastSuccessfulLogin(QString, QString, QString, QString)));

    connect(&_modelSecurity, SIGNAL(__OnCodeHistoryForDateRange(QDateTime,QDateTime,CLockHistorySet*)),
            this, SLOT(OnCodeHistoryForDateRange(QDateTime,QDateTime,CLockHistorySet*)));
    connect(this, SIGNAL( __RequestCodeHistoryForDateRange(QDateTime,QDateTime)), &_modelSecurity, SLOT(OnRequestCodeHistoryForDateRange(QDateTime,QDateTime)));

    connect(this, SIGNAL(__OnUpdateCodeState(CLockState*)), &_modelSecurity, SLOT(OnUpdateCodeState(CLockState*)));
    connect(&_modelSecurity, SIGNAL(__OnUpdatedCodeState(bool)), this, SLOT(OnUpdatedCodeState(bool)));

    connect(this, SIGNAL(__OnSuccessfulQuestionUsersAnswers(QString,QString,QString,QString)), &_modelSecurity, SLOT(OnSuccessfulQuestionUsersAnswers(QString,QString,QString,QString)));
    connect(this, SIGNAL(__OnQuestionUserCancelled()), &_modelSecurity, SLOT(OnQuestionUserCancelled()));
}

void CSecurityController::RequestLastSuccessfulLogin(QString locknums)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE(locknums);
    emit __RequestLastSuccessfulLogin(locknums, "", "", "");
    KCB_DEBUG_EXIT;
}

void CSecurityController::RequestLastSuccessfulLoginWithAnswers(QString locknums, QString answer1, QString answer2, QString answer3)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE("Answer1" << answer1 << "Answer2" << answer2 << "Answer3" << answer3);    
    emit __RequestLastSuccessfulLogin(locknums, answer1, answer2, answer3);
    KCB_DEBUG_EXIT;
}

void CSecurityController::OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE(pLockHistory->getLockNums());
    emit __OnLastSuccessfulLogin(pLockHistory);
    KCB_DEBUG_EXIT;
}

void CSecurityController::OnRequestCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd)
{
    //
    emit __RequestCodeHistoryForDateRange(dtStart, dtEnd);
}

void CSecurityController::OnCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd, CLockHistorySet *pLockHistorySet)
{
    //
    emit __OnCodeHistoryForDateRange(dtStart, dtEnd, pLockHistorySet);
}

void CSecurityController::OnUpdateCodeState(CLockState *rec)
{
    KCB_DEBUG_TRACE("emitting __OnUpdateCodeState");

    emit __OnUpdateCodeState(rec);
}

std::string CSecurityController::GetPredictiveAccessCode(QString code, int nLockNum)
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString skey;
    std::string outEncrypt = "";
    QString  sTmp;

    Q_UNUSED(code);

    if(_pAdminRec) 
    {
        // Round the current date up to the resolution specified
        datetime = CEncryption::roundDateTime(_pAdminRec->getPredictiveResolution(), datetime);
        skey = _pAdminRec->getPredictiveKey();

        CEncryption::calculatePredictiveCodeOld(nLockNum, skey.toStdString().c_str(), datetime, &outEncrypt, 5 /*max results length*/, &sTmp);

        sTmp += QString(" trunc outEncrypt:") + outEncrypt.c_str();
        qDebug() << sTmp;
        return outEncrypt;
    }

    return outEncrypt;
}


void CSecurityController::CheckPredictiveAccessCode(QString code)
{
    uint32_t nLockNum;
    QDateTime datetime = QDateTime::currentDateTime();
    QString skey;
    std::string outEncrypt = "";
    QString sTmp;

    if(_pAdminRec) {
        datetime = CEncryption::roundDateTime(_pAdminRec->getPredictiveResolution(), datetime);
        sTmp = " Date:" + datetime.toString("yyyy-MM-dd HH:mm");

        skey = _pAdminRec->getPredictiveKey();

        for(nLockNum=1; nLockNum<256; nLockNum++) 
        {
            // Round the current date up to the resolution specified

            CEncryption::calculatePredictiveCodeOld(nLockNum, skey.toStdString().c_str(), datetime, &outEncrypt, 5 /*max results length*/, &sTmp);

            sTmp += QString(" trunc outEncrypt:") + outEncrypt.c_str();
            qDebug() << sTmp;

            if( outEncrypt == code.toStdString()) 
            {
                OnSecurityCheckSuccess(QString::number(nLockNum));
                return;
            }
        }
    }
    OnSecurityCheckedFailed();
}

/**
 * @brief CSecurityController::CheckAccessCodeOne
 * @param code1 - encrypted
 */
void CSecurityController::CheckAccessCodeOne(QString code1)
{
    KCB_DEBUG_TRACE(code1);

    // Use the CModelSecurity class
    _sCode1 = code1;
    _sCode2 = "";       // clear code2

    emit(__VerifyCodeOne(code1));       // This should cross thread to the _modelSecurity object
}

void CSecurityController::CheckAccessCodeTwo(QString code2)
{
    KCB_DEBUG_TRACE(code2);

    _sCode2 = code2;

    QString sCodeEnc(CEncryption::encryptString(code2));

    emit(__VerifyCodeTwo(sCodeEnc));       // This should cross thread to the _modelSecurity object
}

/**
 * @brief CSecurityController::CheckFingerprintAccessCodeOne
 * @param code1 - encrypted
 */
void CSecurityController::CheckFingerprintAccessCodeOne(QString code1)
{
    // Use the CModelSecurity class
    _sCode1 = code1;
    _sCode2 = "";       // clear code2
    qDebug() << "CSecurityController::CheckFingerprintAccessCodeOne:" << code1;

    emit(__VerifyFingerprintCodeOne(code1));       // This should cross thread to the _modelSecurity object
}

void CSecurityController::CheckFingerprintAccessCodeTwo(QString code2)
{
    // Use the CModelSecurity class
    _sCode2 = code2;

    qDebug() << "CSecurityController::CheckFingerprintAccessCodeTwo\n";
    QString sCodeEnc(CEncryption::encryptString(code2));

    emit(__VerifyFingerprintCodeTwo(sCodeEnc));       // This should cross thread to the _modelSecurity object
}


void CSecurityController::CheckAdminPassword(QString sPW)
{
    // Use the CModelSecurity class
    _sAdminPW = sPW;

    qDebug() << "CSecurityController::CheckAdminPassword";
    QString sCodeEnc(CEncryption::encryptString(_sAdminPW));

    emit(__VerifyAdminPassword(sCodeEnc));       // This should cross thread to the _modelSecurity object
}

void CSecurityController::OnRequireAdminPassword()
{
    qDebug() << "CSecurityController::OnRequireAdminPassword()";
    emit __OnRequireAdminPassword();
}

void CSecurityController::OnRequireCodeTwo()
{
    qDebug() << "CSecurityController::OnRequireCodeTwo()";
    emit __OnRequireCodeTwo();
}

void CSecurityController::OnAdminSecurityCheckOk(QString type)
{
    qDebug() << "CSecurityController::OnAdminSecurityCheckOk(QString type)";
    emit __OnAdminSecurityCheckOk(type);
}

void CSecurityController::OnAdminSecurityCheckFailed()
{
    qDebug() << "CSecurityController::OnAdminSecurityCheckFailed()";
    emit __OnAdminSecurityCheckFailed();
}

void CSecurityController::OnSecurityCheckSuccess(QString locks)
{
    // KCB_DEBUG_TRACE("locks:" << locks);
    emit __OnSecurityCheckSuccess(locks);
    //emit __OnCreateHistoryRecordFromLastSuccessfulLogin();
}

void CSecurityController::OnSecurityCheckSuccessWithAnswers(QString lockNums, QString answer1, QString answer2, QString answer3)
{
    KCB_DEBUG_TRACE(lockNums << "; " << answer1 << "; " << answer2 << "; " << answer3);
    emit __OnSecurityCheckSuccess(lockNums);
    //emit __OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(answer1, answer2, answer3);
}

void CSecurityController::OnSecurityCheckedFailed()
{
    qDebug() << "CSecurityController::SecurityCheckedFailed()";
    emit __OnSecurityCheckedFailed();
}

void CSecurityController::OnSecurityCheckTimedOut()
{
    qDebug() << "CSecurityController::OnSecurityCheckTimedOut()";
    emit __OnSecurityCheckTimedOut();
}

void CSecurityController::OnRequestCurrentAdmin()
{
    qDebug() << "CSecurityController::OnRequestCurrentAdmin() -> emit __OnRequestCurrentAdmin()";
    emit __OnRequestCurrentAdmin();
}

void CSecurityController::OnRequestedCurrentAdmin(CAdminRec *admin)
{
    qDebug() << "CSecurityController::OnRequestedCurrentAdmin(adminInfo)";
    _pAdminRec = admin;

    qDebug() << "CSecurityController::OnRequestedCurrentAdmin(CAdminRec*) -> emit __OnRequestedCurrentAdmin(CAdminRec*)";
    emit __OnRequestedCurrentAdmin(admin);
}

void CSecurityController::getAllCodes1(QStringList& codes1)
{
    _modelSecurity.getAllCodes1(codes1);
}