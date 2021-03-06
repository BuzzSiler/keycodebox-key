#include "systemcontroller.h"

// #include <iostream>
#include <sstream>

#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <QString>
#include <QAbstractSocket>
#include <QMetaEnum>

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
#include "kcbapplication.h"
#include "omnikey5427ckreader.h"
#include "keycodeboxsettings.h"
#include "kcbsystem.h"
#include "autocodegenstatic.h"

static bool fleetwave_enabled;

static const QString DEFAULT_SMTP_SERVER = "smtpout.secureserver.net";
static const int DEFAULT_SMTP_PORT = 465;
static const int DEFAULT_SMTP_SECURITY = 1;
static const QString DEFAULT_SMTP_USER = "kcb@keycodebox.com";
static const QString DEFAULT_SMTP_PW = "keycodebox";
static const QString DEFAULT_SMTP_FROM = "kcb@keycodebox.com";

CSystemController::CSystemController(QObject *parent) :
    QObject(parent),
    _fingerprintReader(0),
    _systemState(ETimeoutScreen),
    _systemStateDisplay(ENone),
    _pfUsercode(0),
    _pdFingerprintVerify(0),
    _ptimer(0),
    _autoCodeTimer(* new QTimer(this))
{
    // KCB_DEBUG_ENTRY;
    fleetwave_enabled = KeyCodeBoxSettings::isFleetwaveEnabled();
    // KCB_DEBUG_EXIT;
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
    // KCB_DEBUG_ENTRY;
    _pInitThread = pthread;

    if( _LCDGraphicsController.isLCDAttached() ) 
    {
        _LCDGraphicsController.setBrightness(75);
    }

    KCB_DEBUG_TRACE("Starting up KeyCodeBox Alpha " << VERSION);

    UsbProvider::Initialize();
    initializeSecurityConnections();
    initializeLockController();
    initializeReportController();
    initializeReaders();

    connect(this, &CSystemController::__OnAutoCodeEmail, this, &CSystemController::OnAutoCodeEmail);


    _autoCodeTimer.setSingleShot(true);
    connect(&_autoCodeTimer, &QTimer::timeout, this, &CSystemController::OnAutoCodeTimeout);
    _autoCodeTimer.start(1000);
    // KCB_DEBUG_EXIT;
}

void CSystemController::TrigEnrollFingerprint(QString sCode)
{
    if(_fingerprintReader)
    {
        _fingerprintReader->initEnrollment(sCode);
    }
    startTimeoutTimer(15000);
}

void CSystemController::EnrollFingerprintDialogCancel()
{
    // KCB_DEBUG_ENTRY;
    if(_fingerprintReader)
    {
        _fingerprintReader->cancelEnrollment();
    }
    startTimeoutTimer(1000);
}

void CSystemController::EnrollFingerprintResetStageCount()
{
    // KCB_DEBUG_ENTRY;
    if(_fingerprintReader)
    {
        _fingerprintReader->resetEnrollmentStage();
    }
}

void CSystemController::OnVerifyFingerprint()
{
    // KCB_DEBUG_ENTRY;

    startTimeoutTimer(10000);
    if(_fingerprintReader)
    {
        _fingerprintReader->initVerify();
    }
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnVerifyFingerprintDialogCancel()
{
    // KCB_DEBUG_ENTRY;
    if(_fingerprintReader)
    {
        _fingerprintReader->cancelVerify();
    }
    if (_pdFingerprintVerify)
    {
        _pdFingerprintVerify->hide();
    }    
    
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnFingerprintVerifyComplete(bool result, QString message) 
{ 
    // KCB_DEBUG_ENTRY;

    emit __onUpdateVerifyFingerprintDialog(result, message); 

    // KCB_DEBUG_EXIT;
}

void CSystemController::OnReadLockSet(QString LockNums, QDateTime start, QDateTime end) 
{ 
    // KCB_DEBUG_ENTRY;
    emit __OnReadLockSet(LockNums, start, end); 
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnLockSet(CLockSet *pSet) 
{ 
    // KCB_DEBUG_ENTRY;
    emit __OnLockSet(pSet); 
    // KCB_DEBUG_EXIT;
}

void CSystemController::TrigQuestionUser(QString lockNums, QString question1, QString question2, QString question3)
{
    Q_UNUSED(lockNums);
    Q_UNUSED(question1);
    Q_UNUSED(question2);
    Q_UNUSED(question3);
    // KCB_DEBUG_ENTRY;
    stopTimeoutTimer();
    // KCB_DEBUG_EXIT;
}

void CSystemController::AnswerUserSave(QString lockNums, QString question1, QString question2, QString question3)
{
    // KCB_DEBUG_ENTRY;
    _answer1 = question1;
    _answer2 = question2;
    _answer3 = question3;
    _answers_provided = true;
    emit __onQuestionUserAnswers(lockNums, question1, question2, question3);
    // KCB_DEBUG_EXIT;
}

void CSystemController::QuestionUserCancel()
{
    startTimeoutTimer(1000);
    emit __onQuestionUserCancel();
}

void CSystemController::initializeReaders()
{

    _pmagTekReader = new CMagTekCardReader();
    if(_pmagTekReader->initMagTekCardReader())
    {
        connect(_pmagTekReader, SIGNAL(__onCardSwipe(QString,QString)), this, SLOT(OnCardSwipe(QString,QString)));
        _pmagTekReader->moveToThread(&_threadCardReader);

        connect(&_threadCardReader, SIGNAL(started()), _pmagTekReader, SLOT(start()));
        _threadCardReader.start();
    }
    else 
    {
        KCB_DEBUG_TRACE("No MagTekReader found");
    }

    bool hid_reader_found = false;
    _phidReader = new CHWKeyboardReader();
    if( _phidReader->initHIDReader(0x04d8, 0x0055) )
    {
        hid_reader_found = true;
        KCB_DEBUG_TRACE("RF 0x04d8:0x0055 HID Reader Found and started");
    }
    else if( _phidReader->initHIDReader(0x076b, 0x5428) )
    {
        hid_reader_found = true;
        KCB_DEBUG_TRACE("RF 0x076b:0x5428 HID Reader Found and started");
    }
    else
    {
        KCB_DEBUG_TRACE("No RF HID Reader found");
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
        KCB_DEBUG_TRACE("Fingerprint reader found and started");
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

    if( !_pdFingerprintVerify )
    {
        _pdFingerprintVerify = new CDlgFingerprintVerify();
        connect(_pdFingerprintVerify, SIGNAL(__onVerifyFingerprintDialogCancel()), this, SLOT(OnVerifyFingerprintDialogCancel()));
        connect(this, SIGNAL(__onUpdateVerifyFingerprintDialog(bool, QString)), _pdFingerprintVerify, SLOT(OnUpdateVerifyFingerprintDialog(bool, QString)));
        _pdFingerprintVerify->hide();
    }
    
    if (fleetwave_enabled)
    {
        _omnikey5427CKReader = new Omnikey5427CKReader();
        connect(_omnikey5427CKReader, SIGNAL(__onHIDSwipeCodes(QString,QString)), this, SLOT(OnHIDCard(QString,QString)));
        _omnikey5427CKReader->Start();
    }

    connect(&_securityController, SIGNAL(__TrigQuestionUserDialog(QString,QString,QString,QString)), this, SLOT(TrigQuestionUserDialog(QString,QString,QString,QString)));
    connect(&_securityController, SIGNAL(__TrigQuestionUser(QString,QString,QString,QString)), this, SLOT(TrigQuestionUser(QString,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUser(QString,QString,QString,QString)), this, SLOT(TrigQuestionUserDialog(QString,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUserAnswers(QString,QString,QString,QString)), &_securityController, SLOT(OnQuestionUserAnswers(QString,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUserCancel()), &_securityController, SLOT(OnQuestionUserCancel()));

}

void CSystemController::OnVerifyFingerprintDialog()
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pdFingerprintVerify != nullptr, Q_FUNC_INFO, "FPVerify is null");
    
    if (_pdFingerprintVerify)
    {
        _pdFingerprintVerify->show();
        _pdFingerprintVerify->setMessage("");
    }
    // KCB_DEBUG_EXIT;
}

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

    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) 
    {
        KCB_DEBUG_TRACE("Code 1 entry");
        emit __onUserCodeOne(sCodeToUse);
    } 
    else if( _systemState == EUserCodeTwo) 
    {
        KCB_DEBUG_TRACE("Code 2 entry");
        emit __onUserCodeTwo(sCode2);
    }

    // Always emit this -- s.b.going to the AdminInfo screen only.
    emit __onUserCodes(sCodeToUse, sCode2);
}

void CSystemController::OnFingerSwipe(QString sCode1, QString sCode2)
{
    QString sCodeToUse = getCodeToUse(sCode1, sCode2);

    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) 
    {
        KCB_DEBUG_TRACE("Code 1 entry");
        emit __onUserFingerprintCodeOne(sCodeToUse);
    } 
    else if( _systemState == EUserCodeTwo) 
    {
        KCB_DEBUG_TRACE("Code 2 entry");
        emit __onUserFingerprintCodeTwo(sCode2);
    }
    // Always emit this -- s.b.going to the AdminInfo screen only.
    emit __onUserCodes(sCodeToUse, sCode2);
}

void CSystemController::OnHIDCard(QString sCode1, QString sCode2)
{
    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) 
    {
        KCB_DEBUG_TRACE("Code 1 entry");
        emit __onUserCodeOne(sCode1);
    } 
    else if( _systemState == EUserCodeTwo) 
    {
        KCB_DEBUG_TRACE("Code 2 entry");
        emit __onUserCodeTwo(sCode2);
    }
    emit __onUserCodes(sCode1, sCode2);
}

void CSystemController::initializeReportController()
{
    connect(this, SIGNAL(__OnSendEmail(QString,int,int,QString,QString,QString,QString,QString,QString,const QFile*)),
            &_ReportController, SLOT(OnSendEmail(QString,int,int,QString,QString,QString,QString,QString,QString,const QFile*)));
    _ReportController.moveToThread(&_threadReport);
    _threadReport.start();

    connect(this, SIGNAL(__RequestLastSuccessfulLogin(QString)), 
            &_securityController, SLOT(RequestLastSuccessfulLogin(QString)));
    connect(this, SIGNAL(__RequestLastSuccessfulLoginWithAnswers(QString, QString, QString, QString)), 
            &_securityController, SLOT(RequestLastSuccessfulLoginWithAnswers(QString, QString, QString, QString)));
    connect(&_securityController, SIGNAL(__OnLastSuccessfulLogin(CLockHistoryRec*)), 
            this, SLOT(OnLastSuccessfulLoginRequest(CLockHistoryRec*)));
}


void CSystemController::initializeSecurityConnections()
{
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
    connect(&_securityController, SIGNAL(__OnCodeHistoryForDateRange(CLockHistorySet*)),
            &_ReportController, SLOT(OnCodeHistoryForDateRange(CLockHistorySet*)));

    connect(this, SIGNAL(__OnUpdateCodeState(CLockState*)), &_securityController, SLOT(OnUpdateCodeState(CLockState*)));
    connect(&_securityController, SIGNAL(__OnUpdatedCodeState(bool)), this, SLOT(OnUpdatedCodeState(bool)));

    emit __OnRequestCurrentAdmin();
}

void CSystemController::initializeLockController()
{
    // KCB_DEBUG_ENTRY;
    connect(&_LockController, &CLockController::DiscoverHardwareProgressUpdate, this, &CSystemController::DiscoverHardwareProgressUpdate);
    _LockController.initController();
    _LockController.detectHardware();
    emit __OnNotifyDetectHardwareComplete();
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnIdentifiedFingerprint(QString sCode, QString sCode2)
{
    // KCB_DEBUG_ENTRY;
    this->OnFingerSwipe(sCode, sCode2);
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnCodeEntered(QString sCode)
{
    EnrollFingerprintResetStageCount();
    KCB_DEBUG_ENTRY;
    _securityController.CheckAccessCodeOne(sCode);
}

void CSystemController::OnCodeEnteredTwo(QString sCode)
{
    KCB_DEBUG_ENTRY;
    _securityController.CheckAccessCodeTwo(sCode);
}

void CSystemController::OnFingerprintCodeEntered(QString sCode)
{
    KCB_DEBUG_ENTRY;
    _securityController.CheckFingerprintAccessCodeOne(sCode);
}

void CSystemController::OnFingerprintCodeEnteredTwo(QString sCode)
{
    KCB_DEBUG_ENTRY;
    _securityController.CheckFingerprintAccessCodeTwo(sCode);
}

void CSystemController::OnAdminPasswordEntered(QString sPW)
{
    KCB_DEBUG_ENTRY;
    _securityController.CheckAdminPassword(sPW);
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    // KCB_DEBUG_ENTRY;
    _padminInfo = adminInfo;
    _bCurrentAdminRetrieved = true;

    emit __OnRequestedCurrentAdmin(adminInfo);
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnAdminDialogClosed()
{
    // KCB_DEBUG_ENTRY;
    _systemState = EUserCodeOne;
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnRequireAdminPassword()
{
    // KCB_DEBUG_ENTRY;
    _systemState = EAdminPassword;
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnRequireCodeTwo()
{
    // KCB_DEBUG_ENTRY;
    _systemState = EUserCodeTwo;
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnAdminSecurityCheckOk(QString type)
{
    // KCB_DEBUG_ENTRY;

    // Note: If take/return is enabled, then we have to select either take or return
    // to get access to enter the admin code in order to access the admin features.
    // Selecting take or return, sets the application state to 'take' or 'return',
    // respectively.  So, if we end of here, we know we can safely reset the application
    // state.
    kcb::Application::clearAccessSelection();

    if(type == "Assist")
    {
        _systemState = EAssistMain;
    }
    if(type == "Admin")
    {
        _systemState = EAdminMain;
    }
    emit __OnClearEntry();
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnAdminSecurityCheckFailed()
{
    KCB_DEBUG_ENTRY;
    emit __OnCodeMessage(tr("Incorrect Password"));
    emit __OnClearEntry();
    emit __AdminSecurityCheckFailed();
}

void CSystemController::OnAdminPasswordCancel()
{
    // KCB_DEBUG_ENTRY;
    _systemState = EUserCodeOne;
    emit __OnClearEntry();
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnUserCodeCancel()
{
    KCB_DEBUG_ENTRY;

    emit __OnCodeMessage(tr("Cancelling ..."));
    stopTimeoutTimer();

    _systemState = EPasswordTimeout;
    _systemStateDisplay = ENone;

    startTimeoutTimer(1000);
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnOpenLockRequest(QString lockNum, bool takePicture)
{
    KCB_DEBUG_TRACE(lockNum);
    if (takePicture)
    {
        kcb::TakeAndStorePicture();
    }
    _LockController.openLocks(lockNum);
}

void CSystemController::reportActivity(QString locknums)
{
    _bCurrentAdminRetrieved = false;
    emit __OnRequestCurrentAdmin();

    if (_answers_provided)
    {
        // KCB_DEBUG_TRACE("Answer1" << _answer1 << "Answer2" << _answer2 << "Answer3" << _answer3);
        emit __RequestLastSuccessfulLoginWithAnswers(locknums, _answer1, _answer2, _answer3);
        _answers_provided = false;
        _answer1 = "";
        _answer2 = "";
        _answer3 = "";
    }
    else
    {
        emit __RequestLastSuccessfulLogin(locknums);
    }
}

void CSystemController::OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd)
{
    _ReportController.processImmediateReport(dtReportStart, dtReportEnd);
}

void CSystemController::OnSecurityCheckSuccess(QString locks)
{    
    // KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE(locks);

    bool take_picture = true;

    if (_pdFingerprintVerify)
    {
        _pdFingerprintVerify->hide();
    }

    emit __OnCodeMessage(tr("Opening Locks ..."));

    if (locks.contains(','))
    {
        // KCB_DEBUG_TRACE("Multi Lock");

        CFrmSelectLocks sl;

        sl.setLocks(locks);
        if (sl.exec())
        {
            _locks = sl.getLocks();

            QStringList sl = _locks.split(",");

            foreach (auto s, sl)
            {
                OnOpenLockRequest(s, take_picture);
            }
        }
        else
        {
            OnUserCodeCancel();
            return;
        }
    }
    else
    {
        // KCB_DEBUG_TRACE("Single Lock");
        _locks = locks;
        OnOpenLockRequest(locks, take_picture);
    }

    reportActivity(_locks);
    _systemState = EThankYou;
    // KCB_DEBUG_EXIT;
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
    // KCB_DEBUG_ENTRY;
    if(_systemState == EUserCodeOne) 
    {
        QString prompt = fleetwave_enabled ? USER_CODE_FLEETWAVE_PROMPT : USER_CODE_PROMPT;
        emit __OnCodeMessage(prompt);
    } 
    else if(_systemState == EUserCodeTwo) 
    {
        emit __OnCodeMessage(USER_CODE_CODE2_PROMPT);
    }
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnSecurityCheckTimedOut()
{
    _systemState = EPasswordTimeout;
    emit __OnCodeMessage(tr("Password Timeout"));
    emit __OnClearEntry();
}

void CSystemController::resetToTimeoutScreen()
{
    // KCB_DEBUG_ENTRY;
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
    if (_pdFingerprintVerify)
    {
        _pdFingerprintVerify->hide();
    }
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnRequestCurrentAdmin()
{
    // KCB_DEBUG_ENTRY;
    _securityController.OnRequestCurrentAdmin();
    // KCB_DEBUG_EXIT;
}

bool CSystemController::getDisplayFingerprintButton()
{
    return _padminInfo->getDisplayFingerprintButton();
}

bool CSystemController::getDisplayShowHideButton()
{
    return _padminInfo->getDisplayShowHideButton();
}

bool CSystemController::getDisplayTakeReturnButtons()
{
    return _padminInfo->getDisplayTakeReturnButtons();
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
    static int lastFoundDeviceCount = 0;
    
    // Consider moving the following code that lists folders in /media/pi 
    // parses the output into kcbsystem.
    // The function should return a list of folder strings
    std::string listCmd = "ls /media/pi/";
    FILE *cmdF;
    std::string sOutput;


    cmdF = popen(listCmd.c_str(), "r");
    if(!cmdF)
    {
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
        // KCB_DEBUG_TRACE("Existing Device Count" << existingDeviceCount);
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

    // Handles the case when USB drives are removed and the device count is 0
    if (foundDeviceCount == 0 && foundDeviceCount != lastFoundDeviceCount)
    {
        lastFoundDeviceCount = foundDeviceCount;
        refreshAdminDeviceList = true;
    }

    std::string dev0 = mountedDevices[0];
    std::string dev1 = mountedDevices[1];

    QString device0 = QString::fromStdString(dev0);
    QString device1 = QString::fromStdString(dev1);

    //KCB_DEBUG_TRACE("dev0" << device0 << "dev1" << device1);

    // The purpose of this logic is to ensure clients are notified when there is a 
    // change to the mounted USB drives, i.e., 
    //  - If a drive is added
    //  - If a drive is removed
    bool device_added = foundDeviceCount > lastFoundDeviceCount;
    bool device_removed = foundDeviceCount < lastFoundDeviceCount;
    lastFoundDeviceCount = foundDeviceCount;

    //KCB_DEBUG_TRACE("da" << device_added << "dr" << device_removed << "nd" << foundDeviceCount << "lfdc" << lastFoundDeviceCount);

    if (device_added || device_removed)
    {
        refreshAdminDeviceList = true;
    }

    if ( refreshAdminDeviceList )
    {
        KCB_DEBUG_ENTRY;
        emit __OnFoundNewStorageDevice(device0, device1);
    }

    return mountedDeviceCount;
}

void CSystemController::start()
{
    // KCB_DEBUG_ENTRY;
    initialize(QThread::currentThread());
    char mountedDevices[2][40] = {};
    int mountedDeviceCount = 0;
    int count = 0;
    while(1)
    {
        looprun();

        QCoreApplication::processEvents();

        if( _fingerprintReader )
        {
            _fingerprintReader->handleEvents();
        }

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
            if (_pdFingerprintVerify)
            {
                _pdFingerprintVerify->hide();
            }            

            emit __OnCodeMessage("Code 1");
            emit __OnDisplayCodeDialog(this);

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
            QString str = QString("<%1>").arg(tr("Please Enter Second Code"));
            emit __OnCodeMessage(str);

            startTimeoutTimer(20000);
        }
    }
    else if (_systemState == EAdminPassword) 
    {
        if (_systemStateDisplay != EAdminPassword) 
        {
            _systemStateDisplay = EAdminPassword;
            stopTimeoutTimer();
            QString str = QString("<%1>").arg(tr("Enter Admin Password"));
            emit __OnCodeMessage(str);
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

            QString str = QString("%1 %2 %3.").arg(tr("Thank You!"), locks_str, tr("open"));
            emit __OnClearEntry();
            emit __OnCodeMessage(str);

            startTimeoutTimer(2000);
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
    // KCB_DEBUG_ENTRY;
    if(_ptimer) 
    {
        if(_ptimer->isActive())
        {
            _ptimer->stop();
        }
        delete _ptimer;
        _ptimer = 0;
    }
    // KCB_DEBUG_TRACE("Time:" << QTime::currentTime().toString());
    // KCB_DEBUG_EXIT;
}

void CSystemController::startTimeoutTimer(int duration)
{
    // KCB_DEBUG_TRACE("SingleShot timer " << QVariant(duration).toString());
    // KCB_DEBUG_TRACE("Time:" << QTime::currentTime().toString());
    
    if(!_ptimer) 
    {
        _ptimer = new QTimer();
        connect(_ptimer, SIGNAL(timeout()), this, SLOT(resetToTimeoutScreen()));
    }
    _ptimer->stop();
    _ptimer->start(duration);
}

void CSystemController::RequestLastSuccessfulLogin(QString locknums)
{
    emit __RequestLastSuccessfulLogin(locknums);
}

CSystemController::EmailParts CSystemController::CreateReportEmail(QDateTime access, QString desc, QString lockNums)
{
    QString subject = tr("Lock Box Event");

    QString body = QString("%1 #%2").arg(tr("Lock")).arg(lockNums);

    if( desc.size() > 0 )
    {
        body += QString(" [%1]").arg(desc);
    }

    body += QString(" %1 %2").arg(tr("was accessed at")).arg(access.toString("MM/dd/yyyy HH:mm:ss"));

    return CSystemController::EmailParts{subject, body};
}

CSystemController::EmailParts CSystemController::CreateAutoCodeEmail(const QString& key)
{
    QString body = QString("KeyCodeBox has generated new codes."
                           "\n\n"
                           "%1").arg(key);
    return CSystemController::EmailParts{tr("KeyCodeBox AutoCode Update"), body};
}

void CSystemController::OnAutoCodeEmail(const QString& key)
{
    // KCB_DEBUG_ENTRY;
    auto parts = CreateAutoCodeEmail(key);
    sendEmail(parts.subject, parts.body);
    // KCB_DEBUG_EXIT;
}

void CSystemController::sendEmail(const QString& subject, const QString& body)
{
    // KCB_DEBUG_ENTRY;

    QString SMTPSvr = DEFAULT_SMTP_SERVER;
    int SMTPPort = DEFAULT_SMTP_PORT;
    int SMTPType = DEFAULT_SMTP_SECURITY;
    QString SMTPUser = DEFAULT_SMTP_USER;
    QString SMTPPW = DEFAULT_SMTP_PW;
    QString from = DEFAULT_SMTP_FROM;

    bool valid_server = _padminInfo->getSMTPServer().length() > 0;
    bool valid_port = _padminInfo->getSMTPPort() != 0;
    bool valid_security = _padminInfo->getSMTPType() >= 0 && _padminInfo->getSMTPType() <= 2;
    bool valid_user = _padminInfo->getSMTPUsername().length() > 0;

    if (valid_server && valid_port && valid_security && valid_user)
    {
        SMTPSvr = _padminInfo->getSMTPServer();
        SMTPPort = _padminInfo->getSMTPPort();
        SMTPType = _padminInfo->getSMTPType();
        SMTPUser = _padminInfo->getSMTPUsername();
        SMTPPW = _padminInfo->getSMTPPassword();
        from = _padminInfo->getSMTPUsername();
    }

    QString to = _padminInfo->getAdminEmail();

    emit __OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, NULL );
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory)
{
    // KCB_DEBUG_ENTRY;
    // KCB_DEBUG_TRACE(pLockHistory->getLockNums());
    
    int nCount = 0;
    while(!_bCurrentAdminRetrieved && nCount < 25) 
    {
        usleep(100000);
        QCoreApplication::processEvents();
        nCount++;
    }
    if(_bCurrentAdminRetrieved && pLockHistory)
    {

        QDateTime dtAccess = pLockHistory->getAccessTime();
        QString sDesc = pLockHistory->getDescription();
        QString LockNums = pLockHistory->getLockNums();
        QDateTime dtFreq = _padminInfo->getDefaultReportFreq();

        if (IS_EVERY_ACTIVITY(dtFreq))
        {
            if (_padminInfo->getReportViaEmail())
            {
                auto email = CreateReportEmail(dtAccess, sDesc, LockNums);
                //sendEmailReport(dtAccess, sDesc, LockNums);
                sendEmail(email.subject, email.body);
            }

            if (_padminInfo->getReportToFile())
            {
                _ReportController.processEveryActivityReport(dtFreq);
            }
        }

        delete pLockHistory;
    } 
    else 
    {
        KCB_DEBUG_TRACE("No admin record yet!");
    }
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnBrightnessChanged(int nValue)
{
    if( _LCDGraphicsController.isLCDAttached() ) 
    {
        _LCDGraphicsController.setBrightness(nValue);
    }
}

void CSystemController::OnSendTestEmail(int select_type)
{
    // KCB_DEBUG_TRACE("Sending Test Email (" << select_type << ")");

    QString SMTPSvr;
    int SMTPPort;
    int SMTPType;
    QString SMTPUser;
    QString SMTPPW;
    QString from;
    QString to;
    QString subject;

    QString body;
    
    if (select_type == EMAIL_ADMIN_SEND)
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
    else if (select_type == EMAIL_ADMIN_RECV)
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
        KCB_DEBUG_TRACE("Invalid test type" << select_type);
        return;
    }

    emit __OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, NULL );
    
}

void CSystemController::getAllCodes1(QStringList& codes1)
{
    _securityController.getAllCodes1(codes1);
}

void CSystemController::readAllCodes(CLockSet **lockset, bool clear_or_encrypted)
{
    _securityController.readAllCodes(lockset, clear_or_encrypted);
}

void CSystemController::DiscoverHardware()
{
    KCB_DEBUG_ENTRY;
    _LockController.detectHardware();
    KCB_DEBUG_EXIT;
}

void CSystemController::UpdateLockRanges()
{
    // KCB_DEBUG_ENTRY;
    _LockController.setLockRanges();
    // KCB_DEBUG_EXIT;
}

void CSystemController::clearAllCodes()
{
    _securityController.clearAllCodes();
}

void CSystemController::deleteAllCode1OnlyCodes()
{
    _securityController.deleteAllCode1OnlyCodes();
}

void CSystemController::clearLockAndCode2ForAllCodes()
{
    _securityController.clearLockAndCode2ForAllCodes();
}

void CSystemController::addCode(CLockState& state)
{
    _securityController.addCode(state);
}

void CSystemController::OnAutoCodeTimeout()
{
    if (AutoCodeGeneratorStatic::IsCommitted())
    {
        if (AutoCodeGeneratorStatic::IsNextGenDateTime(QDateTime::currentDateTime()))
        {
            KCB_DEBUG_TRACE("generating new codes");
            bool is_code1_mode = AutoCodeGeneratorStatic::IsCode1Mode();
            bool is_code2_mode = AutoCodeGeneratorStatic::IsCode2Mode();

            AutoCodeGenerator::CodeMap map = AutoCodeGeneratorStatic::GenerateCodeMap();
            QMapIterator<QString, QString> i(map);
            while (i.hasNext())
            {
                i.next();
                if (is_code1_mode)
                {
                    _securityController.updateCode1(i.key(), i.value());
                }
                else if (is_code2_mode)
                {
                    _securityController.updateCode2(i.key(), i.value());
                }
                else
                {
                    KCB_DEBUG_TRACE("Invalid code mode");
                }
            }

            emit __OnUpdateCodes();
            OnAutoCodeEmail(QString(KeyCodeBoxSettings::GetAutoCodeKey()));
        }

        int msecs = AutoCodeGeneratorStatic::MsecsToNextGen(QDateTime::currentDateTime());

        KCB_DEBUG_TRACE("secs/msecs to next gen" << msecs/1000 << msecs%1000);
        _autoCodeTimer.start( qMin(msecs, 60000) );
    }
    else
    {
        KCB_DEBUG_TRACE("AutoCode is disabled");
    }
}

void CSystemController::OnNotifyAutoCodeEnabled()
{
    // KCB_DEBUG_ENTRY;
    _autoCodeTimer.stop();
    OnAutoCodeTimeout();
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnNotifyAutoCodeDisabled()
{
    // KCB_DEBUG_ENTRY;
    _securityController.clearAutoCodeForAllCodes();
    _autoCodeTimer.stop();
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnNotifyAdminRequested()
{
    // KCB_DEBUG_ENTRY;
    _ptimer->stop();
    // KCB_DEBUG_EXIT;
}

void CSystemController::OnNotifyAdminCancelled()
{
    // KCB_DEBUG_ENTRY;
    _ptimer->start();
    // KCB_DEBUG_EXIT;
}


void CSystemController::updateCodeSet(CLockSet& codeSet)
{
    _securityController.updateCodeSet(codeSet);
}