#include <QDebug>
#include "securitycontroller.h"
#include "modelsecurity.h"
#include "encryption.h"


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
    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckSuccess(int)), this, SLOT(OnSecurityCheckSuccess(int)));

    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckSuccessWithAnswers(int,QString,QString,QString)), this, SLOT(OnSecurityCheckSuccessWithAnswers(int,QString,QString,QString)));

    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckedFailed()), this, SLOT(OnSecurityCheckedFailed()));

    connect(&_modelSecurity, SIGNAL(__OnSecurityCheckTimedOut()), this, SLOT(OnSecurityCheckTimedOut()));

    connect(this, SIGNAL(__VerifyCodeOne(QString)), &_modelSecurity, SLOT(OnVerifyCodeOne(QString)));
    connect(&_modelSecurity, SIGNAL(__EnrollFingerprint(QString)), this, SLOT(OnEnrollFingerprint(QString)));
    connect(&_modelSecurity, SIGNAL(__EnrollFingerprintDialog(QString)), this, SLOT(OnEnrollFingerprintDialog(QString)));

    connect(&_modelSecurity, SIGNAL(__QuestionUserDialog(int,QString,QString,QString)), this, SLOT(OnQuestionUserDialog(int,QString,QString,QString)));
    
    connect(this, SIGNAL(__VerifyCodeTwo(QString)), &_modelSecurity, SLOT(OnVerifyCodeTwo(QString)));


    connect(this, SIGNAL(__VerifyFingerprintCodeOne(QString)), &_modelSecurity, SLOT(OnVerifyFingerprintCodeOne(QString)));
    connect(this, SIGNAL(__VerifyFingerprintCodeTwo(QString)), &_modelSecurity, SLOT(OnVerifyFingerprintCodeTwo(QString)));

    connect(this, SIGNAL(__VerifyAdminPassword(QString)), &_modelSecurity, SLOT(OnVerifyAdminPassword(QString)));

    // Data
    connect(this, SIGNAL(__OnRequestCurrentAdmin()), &_modelSecurity, SLOT(OnRequestCurrentAdmin()));
    connect(&_modelSecurity, SIGNAL(__OnRequestedCurrentAdmin(CAdminRec*)), this, SLOT(OnRequestedCurrentAdmin(CAdminRec*)));

    connect(this, SIGNAL(__UpdateCurrentAdmin(CAdminRec*)), &_modelSecurity, SLOT(OnUpdateCurrentAdmin(CAdminRec*)));
    connect(&_modelSecurity, SIGNAL(__OnUpdatedCurrentAdmin(bool)), this, SLOT(OnUpdatedCurrentAdmin(bool)));

    connect(this, SIGNAL(__OnReadLockSet(int,QDateTime,QDateTime)), &_modelSecurity, SLOT(OnReadLockSet(int,QDateTime,QDateTime)));
    connect(&_modelSecurity, SIGNAL(__OnLockSet(CLockSet*)), this, SLOT(OnLockSet(CLockSet*)));

    connect(this, SIGNAL(__OnReadLockHistorySet(int,QDateTime,QDateTime)), &_modelSecurity, SLOT(OnReadLockHistorySet(int,QDateTime,QDateTime)));
    connect(&_modelSecurity, SIGNAL(__OnLockHistorySet(CLockHistorySet*)), this, SLOT(OnLockHistorySet(CLockHistorySet*)));

    connect(this, SIGNAL(__OnCreateHistoryRecordFromLastSuccessfulLogin()), &_modelSecurity, SLOT(OnCreateHistoryRecordFromLastSuccessfulLogin()));
    connect(this, SIGNAL(__OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString,QString,QString)), &_modelSecurity, SLOT(OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString,QString,QString)));
    
    connect(&_modelSecurity, SIGNAL(__OnCreateHistoryRecordFromLastPredictiveLogin(int, QString)), &_modelSecurity, SLOT(OnCreateHistoryRecordFromLastPredictiveLogin(int, QString)));
    connect(&_modelSecurity, SIGNAL(__OnLastSuccessfulLogin(CLockHistoryRec*)), this, SLOT(OnLastSuccessfulLoginRequest(CLockHistoryRec*)));
    connect(this, SIGNAL( __RequestLastSuccessfulLogin()), &_modelSecurity, SLOT(RequestLastSuccessfulLogin()));

    connect(&_modelSecurity, SIGNAL(__OnCodeHistoryForDateRange(QDateTime,QDateTime,CLockHistorySet*)),
            this, SLOT(OnCodeHistoryForDateRange(QDateTime,QDateTime,CLockHistorySet*)));
    connect(this, SIGNAL( __RequestCodeHistoryForDateRange(QDateTime,QDateTime)), &_modelSecurity, SLOT(OnRequestCodeHistoryForDateRange(QDateTime,QDateTime)));

    connect(this, SIGNAL(__OnUpdateCodeState(CLockState*)), &_modelSecurity, SLOT(OnUpdateCodeState(CLockState*)));
    connect(&_modelSecurity, SIGNAL(__OnUpdatedCodeState(bool)), this, SLOT(OnUpdatedCodeState(bool)));

    connect(this, SIGNAL(__OnSuccessfulQuestionUsersAnswers(int,QString,QString,QString)), &_modelSecurity, SLOT(OnSuccessfulQuestionUsersAnswers(int,QString,QString,QString)));
    connect(this, SIGNAL(__OnQuestionUserCancelled()), &_modelSecurity, SLOT(OnQuestionUserCancelled()));
}


//std::string CSecurityController::encryptDecrypt(uint32_t nVal, std::string toEncrypt, std::string key)
//{
//    std::string skey = key;
//    std::string output = toEncrypt;

//    for (uint i = 0; i < toEncrypt.length(); i++)
//        output[i] = toEncrypt[i] ^ skey[i + nVal % (skey.length())];

//    return output;
//}

void CSecurityController::RequestLastSuccessfulLogin()
{
    emit __RequestLastSuccessfulLogin();
}

void CSecurityController::OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory)
{
    emit __OnLastSuccessfulLogin(pLockHistory);
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
    emit __OnUpdateCodeState(rec);
}


std::string CSecurityController::GetPredictiveAccessCode(QString code, int nLockNum)
{
    QDateTime datetime = QDateTime::currentDateTime();
    std::string skey;
    uint8_t nVal = 0;
    uint32_t un32Sum = 0;
    std::string outEncrypt = "";
    QString  sTmp;

    if(_pAdminRec) {
        // Round the current date up to the resolution specified
        datetime = CEncryption::roundDateTime(_pAdminRec->getPredictiveResolution(), datetime);
        skey = _pAdminRec->getPredictiveKey();

        CEncryption::calculatePredictiveCodeOld(nLockNum, skey, datetime, &outEncrypt, 5 /*max results length*/, &sTmp);

sTmp += QString(" trunc outEncrypt:") + outEncrypt.c_str();
qDebug() << sTmp;
        return outEncrypt;
    }
}


void CSecurityController::CheckPredictiveAccessCode(QString code)
{
    uint32_t nLockNum;
    QDateTime datetime = QDateTime::currentDateTime();
    std::string skey;
    uint8_t nVal = 0;
    uint32_t un32Sum = 0;
    std::string outEncrypt = "";
    QString sTmp;

    if(_pAdminRec) {
        datetime = CEncryption::roundDateTime(_pAdminRec->getPredictiveResolution(), datetime);
sTmp = " Date:" + datetime.toString("yyyy-MM-dd HH:mm");

        skey = _pAdminRec->getPredictiveKey();

        for(nLockNum=1; nLockNum<256; nLockNum++) {
            // Round the current date up to the resolution specified
            un32Sum = 0;

            CEncryption::calculatePredictiveCodeOld(nLockNum, skey, datetime, &outEncrypt, 5 /*max results length*/, &sTmp);

sTmp += QString(" trunc outEncrypt:") + outEncrypt.c_str();
qDebug() << sTmp;

            if( outEncrypt == code.toStdString()) {
                OnSecurityCheckSuccess(nLockNum);
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
    // Use the CModelSecurity class
    _sCode1 = code1;
    _sCode2 = "";       // clear code2
    qDebug() << "CSecurityController::CheckAccessCodeOne:" << code1;

    emit(__VerifyCodeOne(code1));       // This should cross thread to the _modelSecurity object
}

void CSecurityController::CheckAccessCodeTwo(QString code2)
{
    // Use the CModelSecurity class
    _sCode2 = code2;

    qDebug() << "CSecurityController::CheckAccessCodeTwo\n";
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

void CSecurityController::OnSecurityCheckSuccess(int doorNum)
{
    qDebug() << "CSecurityController::OnSecurityCheckSuccess(int nDoorNum)";
    emit __OnSecurityCheckSuccess(doorNum);
    emit __OnCreateHistoryRecordFromLastSuccessfulLogin();
}

void CSecurityController::OnSecurityCheckSuccessWithAnswers(int doorNum, QString answer1, QString answer2, QString answer3)
{
  qDebug() << "CSecurityController::OnSecurityCheckSuccessWithAnswers, " << answer1 << ", " << answer2 << ", " << answer3;
    emit __OnSecurityCheckSuccess(doorNum);
    emit __OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(answer1, answer2, answer3);
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

    //
    qDebug() << "CSecurityController::OnRequestedCurrentAdmin(CAdminRec*) -> emit __OnRequestedCurrentAdmin(CAdminRec*)";
    emit __OnRequestedCurrentAdmin(admin);
}



