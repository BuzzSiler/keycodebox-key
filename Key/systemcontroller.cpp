
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>

#include "systemcontroller.h"
#include "frmusercode.h"
#include "encryption.h"
#include "usbcontroller.h"
#include "lockcontroller.h"
#include "lockhistoryrec.h"
#include "lockstate.h"


CSystemController::CSystemController(QObject *parent)
{
}

CSystemController::~CSystemController()
{
    if(_pmagTekReader)
      delete _pmagTekReader;
    if(_phidReader)
      delete _phidReader;
    if(_fingerprintReader)
      delete _fingerprintReader;
}

void CSystemController::initialize(QThread *pthread)
{
    _pInitThread = pthread;
    _ptimer = 0;

    _systemState = ETimeoutScreen;
    _systemStateDisplay = ENone;
    _pfUsercode = 0;

    _fingerprintReader = 0;
    
    if( _LCDGraphicsController.isLCDAttached() ) {
        qDebug() << "CSystemController::initialize moveToThread.";
//        _LCDGraphicsController.moveToThread(_pInitThread);
        _LCDGraphicsController.setBrightness(75);
    }

    
    qDebug() << "Starting up KeyCodeBox Alpha v1.2b";
    
    initializeSecurityConnections();
    initializeLockController();
    initializeReportController();
    initializeReaders();

}


void CSystemController::TrigEnrollFingerprint(QString sCode){

  qDebug() << "CSystemController::TrigEnrollFingerprint(), code: " << sCode;
  if(_fingerprintReader)
    _fingerprintReader->initEnrollment(sCode);
  startTimeoutTimer(15000);
}


void CSystemController::EnrollFingerprintDialogCancel()
{
  qDebug() << "CSystemController::EnrollFingerprintCancel()";
  if(_fingerprintReader)
    _fingerprintReader->cancelEnrollment();
  startTimeoutTimer(1000);
}

void CSystemController::EnrollFingerprintResetStageCount()
{
  qDebug() << "CSystemController::EnrollFingerprintResetStageCount()";
  if(_fingerprintReader)
    _fingerprintReader->resetEnrollmentStage();
}

void CSystemController::OnVerifyFingerprint()
{
  qDebug() << "CSystemController::OnVerifyFingerprint()";
  
  startTimeoutTimer(10000);
  if(_fingerprintReader)
    _fingerprintReader->initVerify();
}

void CSystemController::OnVerifyFingerprintDialogCancel()
{
  qDebug() << "CSystemController::OnFingerprintFingerprintCancel()";
  if(_fingerprintReader)
    _fingerprintReader->cancelVerify();
  startTimeoutTimer(1000);
}

void CSystemController::TrigQuestionUser(int doorNum, QString question1, QString question2, QString question3)
{
  qDebug() << "CSystemController::TrigQuestionUser()";
  stopTimeoutTimer();
}

void CSystemController::AnswerUserSave(int doorNum, QString question1, QString question2, QString question3)
{
  qDebug() << "CSystemController::QuestionUserSave()";
  //emit signal to security model/controller here
  qDebug() << "CSystemController::AnswerUserSave(), emitting AnswerUserSave(): " << QString::number(doorNum) << ", " << question1 << ", " << question2 << ", " << question3;
  emit __onQuestionUserAnswers(doorNum, question1, question2, question3);
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
    _phidReader = new CHWKeyboardReader();
    if( _phidReader->initHIDReader(0x04d8, 0x0055) )
    {
        connect(_phidReader, SIGNAL(__onHIDSwipeCodes(QString,QString)), this, SLOT(OnHIDCard(QString,QString)));
        _phidReader->moveToThread(&_threadHID);
        connect(&_threadHID, SIGNAL(started()), _phidReader, SLOT(start()));
        _threadHID.start();
        qDebug() << "RF 0x04d8:0x0055 HID Reader Found and started";
    }
    else
    {
        if( _phidReader->initHIDReader(0x076b, 0x5428) )
        {
            connect(_phidReader, SIGNAL(__onHIDSwipeCodes(QString,QString)), this, SLOT(OnHIDCard(QString,QString)));
            _phidReader->moveToThread(&_threadHID);
            connect(&_threadHID, SIGNAL(started()), _phidReader, SLOT(start()));
            _threadHID.start();
            qDebug() << "RF 0x076b:0x5428 HID Reader Found and started";
        } else {
            qDebug() << "No RF HID Reader found";
        }
    }

    // Fingerprint Reader
    _fingerprintReader = new CFingerprintReader();

    if( _fingerprintReader->initFingerprintReader() )
      {
	qDebug() << "Fingerprint reader found and started";
	connect(_fingerprintReader, SIGNAL(__onFingerprintStageComplete(int, int, QString)), this, SLOT(OnFingerprintStageComplete(int, int, QString)));
	connect(_fingerprintReader, SIGNAL(__onVerifyFingerprintComplete(bool, QString)), this, SLOT(OnFingerprintVerifyComplete(bool, QString)));
	connect(&_securityController, SIGNAL(__TrigEnrollFingerprint(QString)), this, SLOT(TrigEnrollFingerprint(QString)));
 	connect(&_securityController, SIGNAL(__TrigEnrollFingerprintDialog(QString)), this, SLOT(TrigEnrollFingerprintDialog(QString)));
	connect(_fingerprintReader, SIGNAL(__onIdentifiedFingerprint(QString,QString)), this, SLOT(OnIdentifiedFingerprint(QString,QString)));
      }
    else
      _fingerprintReader = 0;
	
    connect(&_securityController, SIGNAL(__TrigQuestionUserDialog(int,QString,QString,QString)), this, SLOT(TrigQuestionUserDialog(int,QString,QString,QString)));
    connect(&_securityController, SIGNAL(__TrigQuestionUser(int,QString,QString,QString)), this, SLOT(TrigQuestionUser(int,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUser(int,QString,QString,QString)), this, SLOT(TrigQuestionUserDialog(int,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUserAnswers(int,QString,QString,QString)), &_securityController, SLOT(OnQuestionUserAnswers(int,QString,QString,QString)));
    connect(this, SIGNAL(__onQuestionUserCancel()), &_securityController, SLOT(OnQuestionUserCancel()));
}

// On a card swipe. If the first code is empty, then use the 2nd code.
QString CSystemController::getCodeToUse(QString code1, QString code2) {
    code1 = code1.trimmed();

    if(code1.length() == 0) {
        return code2.trimmed();
    } else {
        return code1;
    }
}

void CSystemController::OnCardSwipe(QString sCode1, QString sCode2)
{
    QString sCodeToUse = getCodeToUse(sCode1, sCode2);

    qDebug() << "CSystemController::OnCardSwipe(" << sCodeToUse << ")";
    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) {
        qDebug() << "...ETimeoutScreen || EUserCodeOne:" << sCodeToUse;
        emit __onUserCodeOne(sCodeToUse);
    } else if( _systemState == EUserCodeTwo) {
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
    qDebug() << "CSystemController::OnHIDCard(" << sCode1 << ")";

    if(_systemState == ETimeoutScreen || _systemState == EUserCodeOne) {
        qDebug() << "...ETimeoutScreen || EUserCodeOne:" << sCode1;
        emit __onUserCodeOne(sCode1);
    } else if( _systemState == EUserCodeTwo) {
        qDebug() << "... EUserCodeTwo: " << sCode1;
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
    connect(&_securityController, SIGNAL(__OnAdminSecurityCheckOk()), this, SLOT(OnAdminSecurityCheckOk()));
    connect(&_securityController, SIGNAL(__OnAdminSecurityCheckFailed()), this, SLOT(OnAdminSecurityCheckFailed()));
    connect(&_securityController, SIGNAL(__OnSecurityCheckSuccess(int)), this, SLOT(OnSecurityCheckSuccess(int)));
    connect(&_securityController, SIGNAL(__OnSecurityCheckedFailed()), this, SLOT(OnSecurityCheckedFailed()));
    connect(&_securityController, SIGNAL(__OnSecurityCheckTimedOut()), this, SLOT(OnSecurityCheckTimedOut()));

    connect(&_securityController, SIGNAL(__OnRequestedCurrentAdmin(CAdminRec*)), this, SLOT(OnRequestedCurrentAdmin(CAdminRec*)));

    connect(this, SIGNAL(__UpdateCurrentAdmin(CAdminRec*)), &_securityController, SLOT(OnUpdateCurrentAdmin(CAdminRec*)));
    connect(&_securityController, SIGNAL(__OnUpdatedCurrentAdmin(bool)), this, SLOT(OnUpdatedCurrentAdmin(bool)));

    connect(this, SIGNAL(__OnReadLockSet(int,QDateTime,QDateTime)), &_securityController, SLOT(OnReadLockSet(int,QDateTime,QDateTime)));
    connect(&_securityController, SIGNAL(__OnLockSet(CLockSet*)), this, SLOT(OnLockSet(CLockSet*)));

    connect(this, SIGNAL(__OnReadLockHistorySet(int,QDateTime,QDateTime)), &_securityController, SLOT(OnReadLockHistorySet(int,QDateTime,QDateTime)));
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

//    _serialCtrl.moveToThread(_pInitThread);
//    _LockController.moveToThread(_pInitThread);

    _LockController.setUSBController(_serialCtrl);
//    _un64Locks = _LockController.inquireLockStatus(4);
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
    qDebug() << "SystemController::OnCodeEntered:" << sCode;

    //if( _securityController.CheckAccessCodeOneFingerprint() )
    //  check to see ifi the directory exists with fingerprintreader class
    //  if it doesn't:
    //  emit fingerprint dialog for enrollment, else do nothing
    // else
    
    //force enrollment step count reset
    EnrollFingerprintResetStageCount();
	
    _securityController.CheckAccessCodeOne(sCode);
}

void CSystemController::OnCodeEnteredTwo(QString sCode)
{
    qDebug() << "SystemController::OnCodeEnteredTwo:" << sCode;

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
    //
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

void CSystemController::OnAdminSecurityCheckOk()
{
    qDebug() << "SystemController.OnAdminSecuritCheckOk()";
    _systemState = EAdminMain;
    emit __OnClearEntry();
}

void CSystemController::OnAdminSecurityCheckFailed()
{
//    _systemState = EAdminPasswordFailed;
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

/**
 * @brief CSystemController::OnOpenLockRequest
 * This is the administrator screen open lock selection.
 * Kept separate from the user open lock process so
 * separate reporting may occur.
 * @param nLockNum
 */
void CSystemController::OnOpenLockRequest(int nLockNum)
{
    int nRC = _LockController.isLock(nLockNum);
    qDebug() << "LockController.isLock() returns: " << QVariant(nRC).toString();
    if( nRC == 1 )
    {
        // Open the lock
        qDebug() << "Lock Open";
        emit __OnCodeMessage("Lock Open");
//        _LockController.openLockWithPulse(nLockNum, );
        _LockController.openLock(nLockNum);
        qDebug() << "SystemController: reportActivity";
    } else {
        qDebug() << QString(nLockNum) << " is not a connected lock.";
    }
}

void CSystemController::reportActivity()
{
    // Check the frequency & send and email if it's each event
    _bCurrentAdminRetrieved = false;
    emit __OnRequestCurrentAdmin();
    emit __RequestLastSuccessfulLogin();
}

void CSystemController::OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd, int nLockNum)
{
    _ReportController.processImmediateReport(dtReportStart, dtReportEnd, nLockNum);
}


void CSystemController::OnReadLockStatus()
{
    _un64Locks = _LockController.inquireLockStatus(4);

    emit __OnLockStatusUpdated(_LockController.getLockStatus());
}



void CSystemController::OnSecurityCheckSuccess(int lockNum)
{
    _systemState = EThankYou;
    _lockNum = lockNum;
    qDebug() << "Opening Lock:" << QVariant(lockNum).toString();
    if( _LockController.isLock(lockNum) )
    {
        // Open the lock
        qDebug() << "Lock Open";
        emit __OnCodeMessage("Lock Open");
        _LockController.openLock(lockNum);
        reportActivity();
    }
}

void CSystemController::OnSecurityCheckedFailed()
{
//    _systemState = EPasswordFailed;
    emit __OnCodeMessage("Incorrect Code");
    emit __OnClearEntry();

    QTimer::singleShot(4000, this, SLOT(resetCodeMessage()));

}

void CSystemController::resetCodeMessage()
{
    if(_systemState == EUserCodeOne) {
        emit __OnCodeMessage("<Please Enter Code #1>");
    } else if(_systemState == EUserCodeTwo) {
        emit __OnCodeMessage("<Please Enter Second Code>");
    }
}

void CSystemController::OnSecurityCheckTimedOut()
{
    _systemState = EPasswordTimeout;
    emit __OnCodeMessage("Password Timeout");
    emit __OnClearEntry();
}


void CSystemController::resetToTimeoutScreen()
{
    qDebug() << "CSystemController::resetToTimeoutScreen()";
    stopTimeoutTimer();
    _systemState = ETimeoutScreen;
    
    if(_fingerprintReader)
      _fingerprintReader->cancelEnrollment();
    if(_fingerprintReader)
    _fingerprintReader->cancelVerify();
}

void CSystemController::OnRequestCurrentAdmin()
{
    qDebug() << "CSystemController::OnRequestCurrentAdmin() -> call _securityController.OnRequestCurrentAdmin()";
    _securityController.OnRequestCurrentAdmin();
}


CFrmUserCode* CSystemController::getUserCodeOne() {
    if(!_pfUsercode) {
//        _pfUsercode = new CFrmUserCode(_pmainWindow);
//        connect(this, SIGNAL(__OnNewMessage(QString)), _pfUsercode, SLOT(OnNewMessage(QString)));
//        connect(this, SIGNAL(__OnClearEntry()), _pfUsercode, SLOT(OnClearCodeDisplay()));
//        connect(this, SIGNAL(__OnEnableKeypad(bool)), _pfUsercode, SLOT(OnEnableKeyboard(bool)));
//        connect(this, SIGNAL(__OnCodeMessage(QString)), _pfUsercode, SLOT(OnNewCodeMessage(QString)));
    }
//    disconnect(_pfUsercode, SIGNAL(__CodeEntered(QString)), this, 0);
//    connect(_pfUsercode, SIGNAL(__CodeEntered(QString)), this, SLOT(OnCodeEntered(QString)));

//    _pfUsercode->__EnableKeyboard(true);
//    _pfUsercode->show();

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
	if( foundDeviceCount > 4 )
	  break;
	else
	  {
	    strcpy(foundDevices[foundDeviceCount], ssOutput.c_str());
	    foundDeviceCount++;
	  }
    }

  bool refreshAdminDeviceList = false;
  bool oldDeviceFound[2] = {false};
  
  int i,j;
  int existingDeviceCount = 0;  
  for(i=0; i<2; i++)
    for(j=0; j<foundDeviceCount; j++)
      {
	if( strcmp(mountedDevices[i],foundDevices[j]) == 0 )
	  {
	    qDebug() << "CSystemController::watchUSBStorageDevices(), found old device";
	    oldDeviceFound[i] = true;
	    existingDeviceCount++;
	  }
      }

  if( !oldDeviceFound[0] )
    strcpy(mountedDevices[0], "");
  if( !oldDeviceFound[1] )
    strcpy(mountedDevices[1], "");
  
  if( (existingDeviceCount > 1) )
    return existingDeviceCount;
  
  for(i=0; i<foundDeviceCount; i++)
    {
      qDebug() << "CSystemController::watchUSBStorageDevices(), found device " << QString::number(i) << " ";
      
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

void CSystemController::OnTouchScreenTouched() {
    _systemState = EUserCodeOne;
}

void CSystemController::looprun()
{
    if( _systemState == ETimeoutScreen ) {
        if( _systemStateDisplay != ETimeoutScreen ) {
            _systemStateDisplay = ETimeoutScreen;
            emit __OnDisplayTimeoutScreen();
        }
    }
    if( _systemState == EUserCodeOne ) {
        if( _systemStateDisplay != EUserCodeOne ) {
            _systemStateDisplay = EUserCodeOne;

            stopTimeoutTimer();

	    qDebug() << "CSystemController::EnrollFingerprintCancel()";
	    if(_fingerprintReader)
	      _fingerprintReader->cancelEnrollment();
	    if(_fingerprintReader)
	      _fingerprintReader->cancelVerify();
	    
            emit __OnCodeMessage("<Please Enter Code #1>");
            emit __OnDisplayCodeDialog(this);
            emit __OnNewMessage("Enter Code #1");
            emit __OnEnableKeypad(true);

            startTimeoutTimer(30000);
        }
    } else if( _systemState == EUserCodeTwo) {
        if( _systemStateDisplay != EUserCodeTwo ) {
            _systemStateDisplay = EUserCodeTwo;

            stopTimeoutTimer();
            emit __OnCodeMessage("");
            emit __OnClearEntry();
            emit __OnDisplayUserCodeTwoDialog(this);
            QThread::msleep(200);
            emit __OnCodeMessage("<Please Enter Second Code>");
            emit __OnNewMessage("Enter Second Code");

            startTimeoutTimer(20000);
        }
    }
    else if( _systemState == EAdminPassword) {
        if( _systemStateDisplay != EAdminPassword ) {
            _systemStateDisplay = EAdminPassword;
            stopTimeoutTimer();
            emit __OnCodeMessage("<Enter Admin Password>");
            emit __OnNewMessage("Enter Admin Password");
            emit __OnClearEntry();
            emit __OnDisplayAdminPasswordDialog(this);

            startTimeoutTimer(30000);
        }
    }
    else if( _systemState == EThankYou) {
        if( _systemStateDisplay != EThankYou) {
            _systemStateDisplay = EThankYou;
            stopTimeoutTimer();

            QString str = "Thank you! Lock #" + QVariant(_lockNum).toString() + " open.";
            emit __OnClearEntry();
            emit __OnCodeMessage(str);
            emit __OnNewMessage("");

            startTimeoutTimer(5000);
        }
    }
    else if( _systemState == EAdminMain) {
        if( _systemStateDisplay != EAdminMain) {
            _systemStateDisplay = EAdminMain;
            stopTimeoutTimer();
            emit __OnDisplayAdminMainDialog(this);
        }
    }
}


void CSystemController::stopTimeoutTimer()
{
    if(_ptimer) {
        if(_ptimer->isActive())
            _ptimer->stop();
        delete _ptimer;
        _ptimer = 0;
    }
}

void CSystemController::startTimeoutTimer(int duration)
{
    // TODO:Set timer to return to Code #1 entry screen
    //
    qDebug() << "SingleShot timer " << QVariant(duration).toString();
    if(!_ptimer) {
        _ptimer = new QTimer();
        connect(_ptimer, SIGNAL(timeout()), this, SLOT(resetToTimeoutScreen()));
    }
    //_ptimer->setInterval(duration);
    _ptimer->start(duration);
    //_ptimer->singleShot(duration, this, SLOT(resetToTimeoutScreen()));
    //qDebug() << "Timer id(started):" << _ptimer->timerId();
}

void CSystemController::RequestLastSuccessfulLogin()
{
    emit __RequestLastSuccessfulLogin();
}

void CSystemController::OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory)
{
    //
    int nCount = 0;
    while(!_bCurrentAdminRetrieved && nCount < 25) {
        usleep(100000);
        QCoreApplication::processEvents();
        nCount++;
    }
    if(_bCurrentAdminRetrieved && pLockHistory)
    {
        qDebug() << "OnLastSuccessfulLoginRequest()";

        QDateTime dtFreq = _padminInfo->getDefaultReportFreq();

        if( _padminInfo->getReportViaEmail()) {
            qDebug() << " Reporting via email";
            qDebug() << " Freq: " << dtFreq.toString("yyyy-MM-dd HH:mm:ss");

            if(dtFreq == QDateTime(QDate(1,1,1), QTime(0,0)))   // Represents admin wants each access sent
            {
                qDebug() << "Sending email";

                QString SMTPSvr = _padminInfo->getSMTPServer().c_str();
                int SMTPPort = _padminInfo->getSMTPPort();
                int SMTPType = _padminInfo->getSMTPType();

                QString SMTPUser = _padminInfo->getSMTPUsername().c_str();
                QString SMTPPW = _padminInfo->getSMTPPassword().c_str();

                QString from = _padminInfo->getAdminEmail().c_str();
                QString to = _padminInfo->getAdminEmail().c_str();
                QString subject = "Lock Box Event";

                QDateTime dtAccess = pLockHistory->getAccessTime();

                QString sDesc = pLockHistory->getDescription().c_str();
                int nLockNum = pLockHistory->getLockNum();

                QString body = QString("Lock #") + QVariant(nLockNum).toString();

                if( sDesc.size() > 0 )
                {
                    body += " [" + sDesc + "]";
                }
                body += " was accessed at " + dtAccess.toString("MM/dd/yyyy HH:mm:ss");

                qDebug() << "Calling __OnSendEmail() from:" << from << endl << " to:" << to << endl << " subject:" << subject << endl << " body:" << body;
                emit __OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, NULL );            
            }
        }
        delete pLockHistory;
    } else {
        qDebug() << "OnLastSuccessfulLoginRequest() No admin record yet!";
    }
}

void CSystemController::OnBrightnessChanged(int nValue)
{
    if( _LCDGraphicsController.isLCDAttached() ) {
        _LCDGraphicsController.setBrightness(nValue);
    }
}
