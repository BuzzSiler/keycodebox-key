#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>

#include <QString>
#include <QAbstractSocket>
#include <QMetaEnum>
#include <iostream>
#include <sstream>
#include "systemcontroller.h"
#include "frmusercode.h"
#include "encryption.h"
#include "usbcontroller.h"
#include "lockcontroller.h"
#include "lockhistoryrec.h"
#include "lockstate.h"
#include "version.h"
#include "usbprovider.h"
#include "frmselectlocks.h"
#include "kcbcommon.h"

CSystemController::CSystemController(QObject *parent) :
    QObject(parent)
{
    Q_UNUSED(parent);

    qRegisterMetaType<QVector<int> >("QVector<int>");
}

CSystemController::~CSystemController()
{
    if(_pmagTekReader)
    {
        delete _pmagTekReader;
    }
    if(_phidReader)
    {
        delete _phidReader;
    }
    if(_fingerprintReader)
    {
        delete _fingerprintReader;
    }
}

void CSystemController::initialize(QThread *pthread)
{
    _pInitThread = pthread;
    _ptimer = 0;

    _systemState = ETimeoutScreen;
    _systemStateDisplay = ENone;
    _pfUsercode = 0;

    _fingerprintReader = 0;

    if( _LCDGraphicsController.isLCDAttached() ) 
    {
        qDebug() << "CSystemController::initialize moveToThread.";
        _LCDGraphicsController.setBrightness(75);
    }


    qDebug() << "Starting up KeyCodeBox Alpha " << VERSION;

    UsbProvider::Initialize();
    initializeSecurityConnections();
    initializeLockController();
    initializeReportController();
    initializeReaders();

}

void CSystemController::TrigEnrollFingerprint(QString sCode)
{
    qDebug() << "CSystemController::TrigEnrollFingerprint(), code: " << sCode;
    if(_fingerprintReader)
    {
        _fingerprintReader->initEnrollment(sCode);
    }
    startTimeoutTimer(15000);
}

void CSystemController::EnrollFingerprintDialogCancel()
{
    qDebug() << "CSystemController::EnrollFingerprintCancel()";
    if(_fingerprintReader)
    {
        _fingerprintReader->cancelEnrollment();
    }
    startTimeoutTimer(1000);
}

void CSystemController::EnrollFingerprintResetStageCount()
{
    qDebug() << "CSystemController::EnrollFingerprintResetStageCount()";
    if(_fingerprintReader)
    {
        _fingerprintReader->resetEnrollmentStage();
    }
}

void CSystemController::OnVerifyFingerprint()
{
    qDebug() << "CSystemController::OnVerifyFingerprint()";

    startTimeoutTimer(10000);
    if(_fingerprintReader)
    {
        _fingerprintReader->initVerify();
    }
}

void CSystemController::OnVerifyFingerprintDialogCancel()
{
    qDebug() << "CSystemController::OnFingerprintFingerprintCancel()";
    if(_fingerprintReader)
    {
        _fingerprintReader->cancelVerify();
    }
    startTimeoutTimer(1000);
}

void CSystemController::OnFingerprintVerifyComplete(bool result, QString message) 
{ 
    emit __onUpdateVerifyFingerprintDialog(result, message); 
}

void CSystemController::OnReadLockSet(QString LockNums, QDateTime start, QDateTime end) 
{ 
    qDebug() << "SLOT: System Controller -> OnReadLockSet";
    emit __OnReadLockSet(LockNums, start, end); 
}

void CSystemController::OnLockSet(CLockSet *pSet) 
{ 
    qDebug() << "SLOT: System Controller -> OnLockSet";
    emit __OnLockSet(pSet); 
}

void CSystemController::TrigQuestionUser(QString lockNums, QString question1, QString question2, QString question3)
{
    Q_UNUSED(lockNums);
    Q_UNUSED(question1);
    Q_UNUSED(question2);
    Q_UNUSED(question3);
    qDebug() << "CSystemController::TrigQuestionUser()";
    stopTimeoutTimer();
}

void CSystemController::AnswerUserSave(QString lockNums, QString question1, QString question2, QString question3)
{
    qDebug() << "CSystemController::QuestionUserSave()";
    //emit signal to security model/controller here
    qDebug() << "CSystemController::AnswerUserSave(), emitting AnswerUserSave(): " << lockNums << ", " << question1 << ", " << question2 << ", " << question3;
    emit __onQuestionUserAnswers(lockNums, question1, question2, question3);
}

void CSystemController::QuestionUserCancel()
{
    qDebug() << "CSystemController::QuestionUserCancel()";
    startTimeoutTimer(1000);
    emit __onQuestionUserCancel();
}

void CSystemController::initializeReaders()
{
    // MagTek Reader
    _pmagTekReader = new CMagTekCardReader();
    if(_pmagTekReader->initMagTekCardReader()) // hardcoded VID & PID
    {
        connect(_pmagTekReader, SIGNAL(__onCardSwipe(QString,QString)), this, SLOT(OnCardSwipe(QString,QString)));
        _pmagTekReader->moveToThread(&_threadCardReader);

        connect(&_threadCardReader, SIGNAL(started()), _pmagTekReader, SLOT(start()));
        _threadCardReader.start();
        qDebug() << "MagTekReader found and started";
    }
    else {
        qDebug() << "No MagTekReader found";
    }

    // Original HID Reader
    bool hid_reader_found = false;
    _phidReader = new CHWKeyboardReader();
    if( _phidReader->initHIDReader(0x04d8, 0x0055) )
    {
        hid_reader_found = true;
        qDebug() << "RF 0x04d8:0x0055 HID Reader Found and started";
    }
    else if( _phidReader->initHIDReader(0x076b, 0x5428) )
    {
        hid_reader_found = true;
        qDebug() << "RF 0x076b:0x5428 HID Reader Found and started";
    }
    else
    {
        qDebug() << "No RF HID Reader found";
    }

    if (hid_reader_found)
    {
        connect(_phidReader, SIGNAL(__onHIDSwipeCodes(QString,QString)), this, SLOT(OnHIDCard(QString,QString)));
        _phidReader->moveToThread(&_threadHID);
        connect(&_threadHID, SIGNAL(started()), _phidReader, SLOT(start()));
        _threadHID.start();
    }

    // Fingerprint Reader
    _fingerprintReader = new CFingerprintReader();

    if( _fingerprintReader->initFingerprintReader())
    {
        qDebug() << "Fingerprint reader found and started";
        connect(_fingerprintReader, SIGNAL(__onFingerprintStageComplete(int, int, QString)), this, SLOT(OnFingerprintStageComplete(int, int, QString)));
        connect(_fingerprintReader, SIGNAL(__onVerifyFingerprintComplete(bool, QString)), this, SLOT(OnFingerprintVerifyComplete(bool, QString)));
        connect(&_securityController, SIGNAL(__TrigEnrollFingerprint(QString)), this, SLOT(TrigEnrollFingerprint(QString)));
        connect(&_securityController, SIGNAL(__TrigEnrollFingerprintDialog(QString)), this, SLOT(TrigEnrollFingerprintDialog(QString)));
        connect(_fingerprintReader, SIGNAL(__onIdentifiedFingerprint(QString,QString)), this, SLOT(OnIdentifiedFingerprint(QString,QString)));
    }
    else
    {
        _fingerprintReader = 0;
    }

    connect(&_securityController, SIGNAL(__TrigQuestionUserDialog(QString,QString,QString,QString)), this, SLOT(TrigQuestionUserDialog(QString,QString,QString,QString)));
    connect(&_securityController, SIGNAL(__TrigQuestionUser(QString,QString,QString,QString)), this, SLOT(TrigQuestionUser(QString,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUser(QString,QString,QString,QString)), this, SLOT(TrigQuestionUserDialog(QString,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUserAnswers(QString,QString,QString,QString)), &_securityController, SLOT(OnQuestionUserAnswers(QString,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUserCancel()), &_securityController, SLOT(OnQuestionUserCancel()));
}

// On a card swipe. If the first code is empty, then use the 2nd code.
QString CSystemController::getCodeToUse(QString code1, QString code2) 
{
    code1 = code1.trimmed();

    if(code1.length() == 0) 
    {
        return code2.trimmed();
    } 
    else 
    {
        return code1;
    }
}

void CSystemController::OnCardSwipe(QString sCode1, QString sCode2)
{
    QString sCodeToUse = getCodeToUse(sCode1, sCode2);

    qDebug() << "CSystemController::OnCardSwipe(" << sCodeToUse << ")";
    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) 
    {
        qDebug() << "...ETimeoutScreen || EUserCodeOne:" << sCodeToUse;
        emit __onUserCodeOne(sCodeToUse);
    } 
    else if( _systemState == EUserCodeTwo) 
    {
        qDebug() << "... EUserCodeTwo: " << sCode2;
        emit __onUserCodeTwo(sCode2);
    }
    // Always emit this -- s.b.going to the AdminInfo screen only.
    emit __onUserCodes(sCodeToUse, sCode2);
}

void CSystemController::OnFingerSwipe(QString sCode1, QString sCode2)
{
    QString sCodeToUse = getCodeToUse(sCode1, sCode2);

    qDebug() << "CSystemController::OnFingerSwipe(" << sCodeToUse << ")";
    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) {
        qDebug() << "...ETimeoutScreen || EUserCodeOne:" << sCodeToUse;
        emit __onUserFingerprintCodeOne(sCodeToUse);
    } else if( _systemState == EUserCodeTwo) {
        qDebug() << "... EUserCodeTwo: " << sCode2;
        emit __onUserFingerprintCodeTwo(sCode2);
    }
    // Always emit this -- s.b.going to the AdminInfo screen only.
    emit __onUserCodes(sCodeToUse, sCode2);
}

void CSystemController::OnHIDCard(QString sCode1, QString sCode2)
{
    qDebug() << "CSystemController::OnHIDCard(" << sCode1 << ", " << sCode2 << ")";

    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) {
        qDebug() << "...ETimeoutScreen || EUserCodeOne:" << sCode1;
        emit __onUserCodeOne(sCode1);
    } else if( _systemState == EUserCodeTwo) {
        qDebug() << "... EUserCodeTwo: " << sCode2;
        emit __onUserCodeTwo(sCode2);
    }
    // Always emit the two -- s.b.going to the AdminInfo screen only
    emit __onUserCodes(sCode1, sCode2);
}

void CSystemController::initializeReportController()
{
    connect(this, SIGNAL(__OnSendEmail(QString,int,int,QString,QString,QString,QString,QString,QString,const QFile*)),
            &_ReportController, SLOT(OnSendEmail(QString,int,int,QString,QString,QString,QString,QString,QString,const QFile*)));
    _ReportController.moveToThread(&_threadReport);
    _threadReport.start();

    connect(this, SIGNAL(__RequestLastSuccessfulLogin()), &_securityController, SLOT(RequestLastSuccessfulLogin()));
    connect(&_securityController, SIGNAL(__OnLastSuccessfulLogin(CLockHistoryRec*)), this, SLOT(OnLastSuccessfulLoginRequest(CLockHistoryRec*)));
}

void CSystemController::initializeSecurityConnections()
{
    qDebug() << "CSystemController::initializeSecurityConnections moveToThread.";
    //    _securityController.moveToThread(_pSecurityThread);

    connect(&_securityController, SIGNAL(__OnRequireAdminPassword()), this, SLOT(OnRequireAdminPassword()));
    connect(&_securityController, SIGNAL(__OnRequireCodeTwo()), this, SLOT(OnRequireCodeTwo()));
    connect(&_securityController, SIGNAL(__OnAdminSecurityCheckOk(QString)), this, SLOT(OnAdminSecurityCheckOk(QString)));
    connect(&_securityController, SIGNAL(__OnAdminSecurityCheckFailed()), this, SLOT(OnAdminSecurityCheckFailed()));
    connect(&_securityController, SIGNAL(__OnSecurityCheckSuccess(QString)), this, SLOT(OnSecurityCheckSuccess(QString)));
    connect(&_securityController, SIGNAL(__OnSecurityCheckedFailed()), this, SLOT(OnSecurityCheckedFailed()));
    connect(&_securityController, SIGNAL(__OnSecurityCheckTimedOut()), this, SLOT(OnSecurityCheckTimedOut()));

    connect(&_securityController, SIGNAL(__OnRequestedCurrentAdmin(CAdminRec*)), this, SLOT(OnRequestedCurrentAdmin(CAdminRec*)));

    connect(this, SIGNAL(__UpdateCurrentAdmin(CAdminRec*)), &_securityController, SLOT(OnUpdateCurrentAdmin(CAdminRec*)));
    connect(&_securityController, SIGNAL(__OnUpdatedCurrentAdmin(bool)), this, SLOT(OnUpdatedCurrentAdmin(bool)));

    connect(this, SIGNAL(__OnReadLockSet(QString,QDateTime,QDateTime)), &_securityController, SLOT(OnReadLockSet(QString,QDateTime,QDateTime)));
    connect(&_securityController, SIGNAL(__OnLockSet(CLockSet*)), this, SLOT(OnLockSet(CLockSet*)));

    connect(this, SIGNAL(__OnReadLockHistorySet(QString,QDateTime,QDateTime)), &_securityController, SLOT(OnReadLockHistorySet(QString,QDateTime,QDateTime)));
    connect(&_securityController, SIGNAL(__OnLockHistorySet(CLockHistorySet*)), this, SLOT(OnLockHistorySet(CLockHistorySet*)));

    connect(this, SIGNAL(__OnRequestCurrentAdmin()), &_securityController, SLOT(OnRequestCurrentAdmin()));

    connect(&_securityController, SIGNAL(__OnRequestedCurrentAdmin(CAdminRec*)), &_ReportController, SLOT(OnRequestedCurrentAdmin(CAdminRec*)));
    connect(&_ReportController, SIGNAL(__OnRequestCurrentAdmin()), &_securityController, SLOT(OnRequestCurrentAdmin()));

    connect(&_ReportController, SIGNAL(__RequestCodeHistoryForDateRange(QDateTime,QDateTime)),
            &_securityController, SLOT(OnRequestCodeHistoryForDateRange(QDateTime,QDateTime)));
    connect(&_securityController, SIGNAL(__OnCodeHistoryForDateRange(QDateTime,QDateTime,CLockHistorySet*)),
            &_ReportController, SLOT(OnCodeHistoryForDateRange(QDateTime,QDateTime,CLockHistorySet*)));

    connect(this, SIGNAL(__OnUpdateCodeState(CLockState*)), &_securityController, SLOT(OnUpdateCodeState(CLockState*)));
    connect(&_securityController, SIGNAL(__OnUpdatedCodeState(bool)), this, SLOT(OnUpdatedCodeState(bool)));


    emit __OnRequestCurrentAdmin();
}

void CSystemController::initializeLockController()
{
    qDebug() << "CSystemController::initializeLockController moveToThread.";

    _LockController.initController();
    
    emit __OnLockStatusUpdated(_LockController.getLockStatus());
}

void CSystemController::OnIdentifiedFingerprint(QString sCode, QString sCode2)
{
    qDebug() << "SystemController::OnIdentifiedFingerprint()";
    qDebug() << "OnCardSwipe() -> " << sCode << "," << sCode2;
    this->OnFingerSwipe(sCode, sCode2);
}

void CSystemController::OnCodeEntered(QString sCode)
{
    KCB_DEBUG_TRACE(sCode);

    //force enrollment step count reset
    EnrollFingerprintResetStageCount();

    _securityController.CheckAccessCodeOne(sCode);
}

void CSystemController::OnCodeEnteredTwo(QString sCode)
{
    KCB_DEBUG_TRACE(sCode);

    //if( _securityController.CheckAccessCodeTwoFingerprint() )
    //  check to see if the directory exists with fingerprintreader class
    //  if it doesn't:
    //  emit fingerprint dialog for enrollment, else do nothing
    // else

    _securityController.CheckAccessCodeTwo(sCode);
}

void CSystemController::OnFingerprintCodeEntered(QString sCode)
{
    qDebug() << "SystemController::OnFingerprintCodeEntered:" << sCode;

    //if( _securityController.CheckAccessCodeOneFingerprint() )
    //  check to see ifi the directory exists with fingerprintreader class
    //  if it doesn't:
    //  emit fingerprint dialog for enrollment, else do nothing
    // else

    _securityController.CheckFingerprintAccessCodeOne(sCode);
}

void CSystemController::OnFingerprintCodeEnteredTwo(QString sCode)
{
    qDebug() << "SystemController::OnFingerprintCodeEnteredTwo:" << sCode;

    //if( _securityController.CheckAccessCodeTwoFingerprint() )
    //  check to see if the directory exists with fingerprintreader class
    //  if it doesn't:
    //  emit fingerprint dialog for enrollment, else do nothing
    // else

    _securityController.CheckFingerprintAccessCodeTwo(sCode);
}

void CSystemController::OnAdminPasswordEntered(QString sPW)
{
    qDebug() << "SystemController::OnAdminPasswordEntered";
    _securityController.CheckAdminPassword(sPW);
}

void CSystemController::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    _padminInfo = adminInfo;
    _bCurrentAdminRetrieved = true;

    qDebug() << "CSystemController::OnRequestedCurrentAdmin(CAdminRec*) -> emit __OnRequestedCurrentAdmin(CAdminRec*)";
    emit __OnRequestedCurrentAdmin(adminInfo);
}

void CSystemController::OnAdminDialogClosed()
{
    qDebug() << "CSystemController::OnAdminDialogClosed()";
    _systemState = EUserCodeOne;
}

void CSystemController::OnRequireAdminPassword()
{
    qDebug() << "SystemController::OnRequireAdminPassword";
    _systemState = EAdminPassword;
}

void CSystemController::OnRequireCodeTwo()
{
    qDebug() << "SystemController::OnRequireCodeTwo()";
    _systemState = EUserCodeTwo;
}

void CSystemController::OnAdminSecurityCheckOk(QString type)
{
    qDebug() << "SystemController.OnAdminSecuritCheckOk()";
    if(type == "Assist")
    {
        _systemState = EAssistMain;
    }
    if(type == "Admin")
    {
        _systemState = EAdminMain;
    }
    emit __OnClearEntry();
}

void CSystemController::OnAdminSecurityCheckFailed()
{
    emit __OnCodeMessage("Incorrect Password");
    emit __OnClearEntry();
    emit __AdminSecurityCheckFailed();
}

void CSystemController::OnAdminPasswordCancel()
{
    qDebug() << "CSystemController::OnAdminPasswordCancel()";

    _systemState = EUserCodeOne;
    emit __OnClearEntry();
}

void CSystemController::OnUserCodeCancel()
{
    qDebug() << "CSystemController::OnUserCodeCancel()";
    _systemState = EUserCodeOne;
    emit __OnClearEntry();
}

void CSystemController::OnOpenLockRequest(QString LockNums, bool is_user)
{
    KCB_DEBUG_TRACE(LockNums << is_user);
    // Open the lock
    qDebug() << "Lock Open";
    emit __OnCodeMessage(tr("Lock Open"));
    _LockController.openLocks(LockNums);
    if (is_user)
    {
        qDebug() << "SystemController: reportActivity";
        reportActivity();
    }
}

void CSystemController::reportActivity()
{
    // Check the frequency & send and email if it's each event
    _bCurrentAdminRetrieved = false;
    emit __OnRequestCurrentAdmin();
    emit __RequestLastSuccessfulLogin();
}

void CSystemController::OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd)
{
    _ReportController.processImmediateReport(dtReportStart, dtReportEnd);
}

void CSystemController::OnReadLockStatus()
{
    _un64Locks = _LockController.inquireLockStatus(4);

    emit __OnLockStatusUpdated(_LockController.getLockStatus());
}

void CSystemController::OnSecurityCheckSuccess(QString locks)
{    
    KCB_DEBUG_TRACE(locks);
    _locks = locks;

     if (!locks.contains(','))
     {
         OnOpenLockRequest(locks, true);
         _systemState = EThankYou;
     }
     else
     {
         CFrmSelectLocks dlg;

         dlg.setLocks(locks);
         if (dlg.exec())
         {
             // We get here, when open finished and calls done(Accepted)
             // What is our system state here?
             _locks = dlg.getLocks();
             qDebug() << "Selected locks - " << _locks;

             QStringList sl = _locks.split(",");

             foreach (auto s, sl)
             {
                OnOpenLockRequest(s, true);
             }

             _systemState = EThankYou;
         }
         else
         {
             // We get here, if close is clicked
             // What is our system state here?
             _systemState = ETimeoutScreen;
         }
     }
}

void CSystemController::OnSecurityCheckedFailed()
{
    //    _systemState = EPasswordFailed;
    emit __OnCodeMessage(tr("Incorrect Code"));
    emit __OnClearEntry();

    QTimer::singleShot(4000, this, SLOT(resetCodeMessage()));

}

void CSystemController::resetCodeMessage()
{
    if(_systemState == EUserCodeOne) 
    {
        emit __OnCodeMessage(QString("<%1 #1>").arg(tr("Please Enter Code")));
    } 
    else if(_systemState == EUserCodeTwo) 
    {
        emit __OnCodeMessage(QString("<%1>").arg(tr("Please Enter Second Code")));
    }
}

void CSystemController::OnSecurityCheckTimedOut()
{
    _systemState = EPasswordTimeout;
    emit __OnCodeMessage(tr("Password Timeout"));
    emit __OnClearEntry();
}

void CSystemController::resetToTimeoutScreen()
{
    qDebug() << "CSystemController::resetToTimeoutScreen()";
    stopTimeoutTimer();
    _systemState = ETimeoutScreen;

    if(_fingerprintReader)
    {
        _fingerprintReader->cancelEnrollment();
    }
    if(_fingerprintReader)
    {
        _fingerprintReader->cancelVerify();
    }
}

void CSystemController::OnRequestCurrentAdmin()
{
    qDebug() << "CSystemController::OnRequestCurrentAdmin() -> call _securityController.OnRequestCurrentAdmin()";
    _securityController.OnRequestCurrentAdmin();
}

bool CSystemController::getDisplayFingerprintButton()
{
    return _padminInfo->getDisplayFingerprintButton();
}

bool CSystemController::getDisplayShowHideButton()
{
    return _padminInfo->getDisplayShowHideButton();
}

CFrmUserCode* CSystemController::getUserCodeOne()
{
    if(!_pfUsercode)
    {
    }
    return (CFrmUserCode *)NULL;
}

void CSystemController::ExtractCommandOutput(FILE *pF, std::string &rtnStr)
{
    char cChar = '\0';

    while(!feof(pF))
    {
        cChar = fgetc(pF);
        rtnStr += cChar;
    }
}

int CSystemController::watchUSBStorageDevices(char mountedDevices[2][40], int mountedDeviceCount)
{
    std::string listCmd = "ls /media/pi/";
    FILE *cmdF;
    std::string sOutput;


    cmdF = popen(listCmd.c_str(), "r");
    if(!cmdF)
    {
        qDebug() << "failed to open popen(listCmd, 'r');\n";
        return -1;
    }

    ExtractCommandOutput(cmdF, sOutput);
    fclose(cmdF);

    std::stringstream ss(sOutput);
    std::string ssOutput;

    char foundDevices[4][40] = {};
    int foundDeviceCount = 0;

    while(std::getline(ss,ssOutput,'\n'))
    {
        if( ssOutput.length() > 1 )
        {
            if( foundDeviceCount > 4 )
            {
                break;
            }
            else
            {
                strcpy(foundDevices[foundDeviceCount], ssOutput.c_str());
                foundDeviceCount++;
            }
        }
    }

    bool refreshAdminDeviceList = false;
    bool oldDeviceFound[2] = {false};

    int i,j;
    int existingDeviceCount = 0;
    for(i=0; i<2; i++)
    {
        for(j=0; j<foundDeviceCount; j++)
        {
            if( strcmp(mountedDevices[i],foundDevices[j]) == 0 )
            {
                oldDeviceFound[i] = true;
                existingDeviceCount++;
            }
        }
    }

    if( !oldDeviceFound[0] )
    {
        strcpy(mountedDevices[0], "");
    }
    if( !oldDeviceFound[1] )
    {
        strcpy(mountedDevices[1], "");
    }

    if( (existingDeviceCount > 1) )
    {
        return existingDeviceCount;
    }

    for(i=0; i<foundDeviceCount; i++)
    {
        if( (strcmp(mountedDevices[0],foundDevices[i]) != 0) &&
                (strcmp(mountedDevices[1],foundDevices[i]) != 0) )
        {
            strcpy(mountedDevices[i],foundDevices[i]);
            refreshAdminDeviceList = true;
        }
    }
    std::string dev0 = mountedDevices[0];
    std::string dev1 = mountedDevices[1];

    QString device0 = QString::fromStdString(dev0);
    QString device1 = QString::fromStdString(dev1);

    if( refreshAdminDeviceList )
    {
        emit __OnFoundNewStorageDevice(device0, device1);
    }

    return mountedDeviceCount;
}

void CSystemController::start()
{
    initialize(QThread::currentThread());
    qDebug() << "CSystemController::start()";
    char mountedDevices[2][40] = {};
    int mountedDeviceCount = 0;
    int count = 0;
    while(1)
    {
        looprun();

        QCoreApplication::processEvents();

        if( _fingerprintReader )
            _fingerprintReader->handleEvents();

        // dirty hack for events that need to be triggered after the
        //   admin has been setup (signals need to be connected there before
        //   emitting them or won't trigger anything)
        if( _systemState == EAdminMain )
        {
            if( count == 200000 )
            {
                mountedDeviceCount = watchUSBStorageDevices(mountedDevices, mountedDeviceCount);
                count = 0;
            }
            count++;
        }
    }
}

void CSystemController::OnTouchScreenTouched() 
{
    _systemState = EUserCodeOne;    
}

void CSystemController::looprun()
{
    if(_systemState == ETimeoutScreen) 
    {
        if(_systemStateDisplay != ETimeoutScreen) 
        {
            _systemStateDisplay = ETimeoutScreen;
            emit __OnDisplayTimeoutScreen();
        }
    }

    if(_systemState == EUserCodeOne) 
    {
        if(_systemStateDisplay != EUserCodeOne) 
        {
            _systemStateDisplay = EUserCodeOne;

            stopTimeoutTimer();

            if(_fingerprintReader)
            {
                _fingerprintReader->cancelEnrollment();
            }
            if(_fingerprintReader)
            {
                _fingerprintReader->cancelVerify();
            }

            emit __OnCodeMessage(QString("<%1 #1>").arg(tr("Please Enter Code")));
            emit __OnDisplayCodeDialog(this);
            emit __OnNewMessage(QString("%1 #1").arg(tr("Enter Code")));
            emit __OnEnableKeypad(true);

            startTimeoutTimer(30000);
        }
    }
    else if (_systemState == EUserCodeTwo)
    {
        if(_systemStateDisplay != EUserCodeTwo) 
        {
            _systemStateDisplay = EUserCodeTwo;

            stopTimeoutTimer();
            emit __OnCodeMessage("");
            emit __OnClearEntry();
            emit __OnDisplayUserCodeTwoDialog(this);
            QThread::msleep(200);
            emit __OnCodeMessage(QString("<%1>").arg(tr("Please Enter Second Code")));
            emit __OnNewMessage(tr("Enter Second Code"));

            startTimeoutTimer(20000);
        }
    }
    else if (_systemState == EAdminPassword) 
    {
        if (_systemStateDisplay != EAdminPassword) 
        {
            _systemStateDisplay = EAdminPassword;
            stopTimeoutTimer();
            emit __OnCodeMessage(QString("<%1>").arg(tr("Enter Admin Password")));
            emit __OnNewMessage(tr("Enter Admin Password"));
            emit __OnClearEntry();
            emit __OnDisplayAdminPasswordDialog(this);

            startTimeoutTimer(30000);
        }
    }
    else if(_systemState == EThankYou) 
    {
        if(_systemStateDisplay != EThankYou) 
        {
            _systemStateDisplay = EThankYou;
            stopTimeoutTimer();

            // Convert the list of locks into a string
            QString locks_str;
            if (!_locks.contains(','))
            {
                locks_str = QString("%1 #%2").arg(tr("Lock")).arg(_locks);
            }
            else
            {
                QStringList locks_list = _locks.split(',');
                locks_str = QString("%1 #'s %2").arg(tr("Lock")).arg(locks_list.join(","));
            }
            _locks.clear();

            QString str = QString("%1 %2 %3.").arg(tr("Thank you!"), locks_str, tr("open"));
            emit __OnClearEntry();
            emit __OnCodeMessage(str);
            emit __OnNewMessage("");

            startTimeoutTimer(5000);
        }
    }
    else if(_systemState == EAdminMain) 
    {
        if(_systemStateDisplay != EAdminMain) 
        {
            _systemStateDisplay = EAdminMain;
            _adminType = "Admin";
            stopTimeoutTimer();
            emit __OnDisplayAdminMainDialog(this);
        }
    }
    else if(_systemState == EAssistMain) 
    {
        if(_systemStateDisplay != EAssistMain) 
        {
            _systemStateDisplay = EAssistMain;
            _adminType = "Assist";
            stopTimeoutTimer();
            emit __OnDisplayAdminMainDialog(this);
        }
    }
}

void CSystemController::stopTimeoutTimer()
{
    if(_ptimer) 
    {
        if(_ptimer->isActive())
        {
            _ptimer->stop();
        }
        delete _ptimer;
        _ptimer = 0;
    }
}

void CSystemController::startTimeoutTimer(int duration)
{
    // TODO:Set timer to return to Code #1 entry screen
    //
    qDebug() << "SingleShot timer " << QVariant(duration).toString();
    if(!_ptimer) 
    {
        _ptimer = new QTimer();
        connect(_ptimer, SIGNAL(timeout()), this, SLOT(resetToTimeoutScreen()));
    }
    _ptimer->start(duration);
}

void CSystemController::RequestLastSuccessfulLogin()
{
    emit __RequestLastSuccessfulLogin();
}

void CSystemController::OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory)
{
    int nCount = 0;
    while(!_bCurrentAdminRetrieved && nCount < 25) 
    {
        usleep(100000);
        QCoreApplication::processEvents();
        nCount++;
    }
    if(_bCurrentAdminRetrieved && pLockHistory)
    {
        qDebug() << "OnLastSuccessfulLoginRequest()";

        QDateTime dtFreq = _padminInfo->getDefaultReportFreq();

        if( _padminInfo->getReportViaEmail()) 
        {
            qDebug() << " Reporting via email";
            qDebug() << " Freq: " << dtFreq.toString("yyyy-MM-dd HH:mm:ss");

            if(dtFreq == QDateTime(QDate(1,1,1), QTime(0,0)))   // Represents admin wants each access sent
            {
                qDebug() << "Sending email";

                /* Make kcb@keycodebox.com the default */
                QString SMTPSvr = "smtpout.secureserver.net";
                int SMTPPort = 465;
                int SMTPType = 1;
                QString SMTPUser = "kcb@keycodebox.com";
                QString SMTPPW = "keycodebox";
                QString from = "kcb@keycodebox.com";

                if (_padminInfo->getSMTPServer().length() > 0 &&
                    _padminInfo->getSMTPPort() != 0 &&
                    (_padminInfo->getSMTPType() >= 0 && _padminInfo->getSMTPType() <= 2) &&
                    _padminInfo->getSMTPUsername().length() > 0)
                {
                    SMTPSvr = _padminInfo->getSMTPServer();
                    SMTPPort = _padminInfo->getSMTPPort();
                    SMTPType = _padminInfo->getSMTPType();
                    SMTPUser = _padminInfo->getSMTPUsername();
                    SMTPPW = _padminInfo->getSMTPPassword();
                    from = _padminInfo->getSMTPUsername();
                }
                QString to = _padminInfo->getAdminEmail();
                QString subject = tr("Lock Box Event");

                QDateTime dtAccess = pLockHistory->getAccessTime();

                QString sDesc = pLockHistory->getDescription();
                QString LockNums = pLockHistory->getLockNums();

                QString body = QString("%1 #%2").arg(tr("Lock")).arg(LockNums);

                if( sDesc.size() > 0 )
                {
                    body += QString(" [%1]").arg(sDesc);
                }
                //body += " " + tr("was accessed at") + " " + dtAccess.toString("MM/dd/yyyy HH:mm:ss");
                body += QString(" %1 %2").arg(tr("was accessed at"), dtAccess.toString("MM/dd/yyyy HH:mm:ss"));

                qDebug() << "Calling __OnSendEmail() from:" << from << endl << " to:" << to << endl << " subject:" << subject << endl << " body:" << body;
                emit __OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, NULL );
            }
        }
        delete pLockHistory;
    } else 
    {
        qDebug() << "OnLastSuccessfulLoginRequest() No admin record yet!";
    }
}

void CSystemController::OnBrightnessChanged(int nValue)
{
    if( _LCDGraphicsController.isLCDAttached() ) {
        _LCDGraphicsController.setBrightness(nValue);
    }
}

void CSystemController::OnSendTestEmail(int test_type)
{
    qDebug() << "Sending Test Email (" << test_type << ")";

    QString SMTPSvr;
    int SMTPPort;
    int SMTPType;
    QString SMTPUser;
    QString SMTPPW;
    QString from;
    QString to;
    QString subject;

    QString body;
    
    if (test_type == 1 /* ADMIN_SEND */)
    {
        /* When this button is clicked we will send an email from the account configure in the
        email settings to the email configured in the Administrator tab.
        The email will also be BCC'd to kcb@keycodebox.com
        */

        SMTPSvr = _padminInfo->getSMTPServer();
        SMTPPort = _padminInfo->getSMTPPort();
        SMTPType = _padminInfo->getSMTPType();

        SMTPUser = _padminInfo->getSMTPUsername();
        SMTPPW = _padminInfo->getSMTPPassword();

        from = _padminInfo->getSMTPUsername();
        to = _padminInfo->getAdminEmail();
        subject = tr("Testing Email Configuration");

        body = tr("You have successfully configured the email settings!");
    }
    else if (test_type == 2 /* ADMIN_RECV */)
    {
        /* When this button is clicked we will send an email from test@keycodebox.com to the 
        Administrators email.
        */

        SMTPSvr = "smtpout.secureserver.net";
        SMTPPort = 465;
        SMTPType = 1;

        SMTPUser = "test@keycodebox.com";
        SMTPPW = "keycodebox";

        from = "test@keycodebox.com";
        to = _padminInfo->getAdminEmail();
        subject = tr("Testing Administrator Email");

        body = tr("You have successfully configured the Administrator email!");
    }
    else
    {
        qDebug() << "Invalid test type" << test_type;
        return;
    }

    emit __OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, NULL );
    
}

void CSystemController::getAllCodes1(QStringList& codes1)
{
    _securityController.getAllCodes1(codes1);
}
