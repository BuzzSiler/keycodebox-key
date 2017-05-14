#ifndef CSYSTEMCONTROLLER_H
#define CSYSTEMCONTROLLER_H

#include <QMainWindow>
#include <QObject>
#include <QFile>
#include "lcdgraphicscontroller.h"
#include "securitycontroller.h"
#include "lockcontroller.h"
#include "frmusercode.h"
#include "tbladmin.h"
#include "reportcontroller.h"
#include "magtekcardreader.h"
#include "hidreader.h"
#include "fingerprintreader.h"

class CSystemController : public QObject
{
    Q_OBJECT
public:
    explicit CSystemController(QObject *parent = 0);
    ~CSystemController();

    enum SystemState {ENone, ETimeoutScreen, EUserCodeOne, EUserCodeTwo, EAdminPassword, EPasswordFailed,
                      EPasswordTimeout, EAdminPasswordFailed, EThankYou, EAdminMain};

private:
    QThread                 *_pInitThread;
    QThread                 _threadReport;
    QThread                 _threadHID, _threadCardReader;
    CSecurityController     _securityController;
    CLCDGraphicsController  _LCDGraphicsController;
    CUSBController          _serialCtrl;
    CLockController         _LockController;
    CReportController       _ReportController;
    QMainWindow             *_pmainWindow;

    CMagTekCardReader   *_pmagTekReader = 0;
    CHWKeyboardReader          *_phidReader = 0;
    CFingerprintReader *_fingerprintReader = 0;
    
    uint32_t        _lockNum;
    SystemState     _systemState;
    SystemState     _systemStateDisplay;
    bool            _bCurrentAdminRetrieved;
    CAdminRec       *_padminInfo;

    CFrmUserCode *_pfUsercode;

    QTimer      *_ptimer;
    uint64_t    _un64Locks;

    void initializeSecurityConnections();
    void initializeLockController();

    CFrmUserCode *getUserCodeOne();
    void initializeReportController();
    void startTimeoutTimer(int duration);
    void stopTimeoutTimer();
    void initializeReaders();
    QString getCodeToUse(QString code1, QString code2);
public:
    void initialize(QThread *pthread);
    SystemState getSystemState() { return _systemState; }
    SystemState getSystemStateDisplay() { return _systemStateDisplay; }

    void setMainWindow(QMainWindow *mw) { _pmainWindow = mw; }

    void looprun();

    const CLockController &getLockController() { return _LockController; }

    void reportActivity();
signals:
    void __verifyUserAccess(QString sCode1);
    void __verifyUserAccessTwo(QString sCode1, QString sCode2);
    void __verifyAdminAccess(QString sCode1, QString sCode2);

    void __openDoor(int nDoor);

    // For Entry dialogs
    void __OnDisplayTimeoutScreen();
    void __OnNewMessage(QString sMsg);      //
    void __OnCodeMessage(QString sMsg); // <>
    void __OnClearEntry();
    void __OnEnableKeypad(bool bEnable);
    void __AdminSecurityCheckFailed();
    void __UserSecurityCheckOneFailed();
    void __UserSecurityCheckTwoFailed();

    void __OnDisplayCodeDialog(QObject *obj);
    void __OnDisplayUserCodeTwoDialog(QObject *obj);
    void __OnDisplayThankYouDialog(QObject *obj);
    void __OnDisplayAdminPasswordDialog(QObject *obj);
    void __OnDisplayAdminMainDialog(QObject *obj);

    void __OnRequestCurrentAdmin();
    void __OnRequestedCurrentAdmin(CAdminRec *adminInfo);
    void __UpdateCurrentAdmin(CAdminRec *adminInfo);
    void __OnUpdatedCurrentAdmin(bool bSuccess);
    void __OnUpdateCodeState(CLockState *rec);
    void __OnUpdatedCodeState(bool bSuccess);

    void __OnFoundNewStorageDevice(QString device0, QString device1);
    
    void __OnLockStatusUpdated(CLocksStatus *locksStatus);

    void __onUserCodeOne(QString sCode1);
    void __onUserCodeTwo(QString sCode2);
    
    void __onUserFingerprintCodeOne(QString sCode1);
    void __onUserFingerprintCodeTwo(QString sCode2);

    void __onUserCodes(QString sCode1, QString sCode2);
    
signals:
    void __OnReadLockSet(int nLockNum, QDateTime start, QDateTime end);
    void __OnLockSet(CLockSet *pLockSet);
    void __OnSendEmail(const QString SMTPServer, const int &SMTPPort, const int &SMTPType,
                       const QString &SMTPUsername, const QString &SMTPPassword,
                       const QString &from, const QString &to, const QString &subject, const QString &body, const QFile *fileAttach );

    void __onUpdateEnrollFingerprintDialog(int current, int total, QString message);
    void __onUpdateVerifyFingerprintDialog(bool result, QString message);

    void __onEnrollFingerprintDialog(QString sCode);

    void __onQuestionUserDialog(int doorNum, QString question1, QString question2, QString question3);
    void __onQuestionUser(int doorNum, QString question1, QString question2, QString question3);
    void __onQuestionUserAnswers(int doorNum, QString answer1, QString answer2, QString answer3);
    void __onQuestionUserCancel();
    
public slots:
  
  void TrigQuestionUserDialog(int doorNum, QString question1, QString question2, QString question3) { emit __onQuestionUserDialog(doorNum, question1, question2, question3);}
    
    void TrigQuestionUser(int doorNum, QString question1, QString question2, QString question3);
    void QuestionUserCancel();
    void AnswerUserSave(int doorNum, QString question1, QString question2, QString question3);
    
    void TrigEnrollFingerprint(QString sCode);
    void TrigEnrollFingerprintDialog(QString sCode) { emit __onEnrollFingerprintDialog(sCode); }
    
    void EnrollFingerprintDialogCancel();
    void EnrollFingerprintResetStageCount();
    void OnFingerprintStageComplete(int current, int total, QString message) {  emit __onUpdateEnrollFingerprintDialog(current, total, message); };

    void OnVerifyFingerprint();
    void OnVerifyFingerprintDialogCancel();
    void OnFingerprintVerifyComplete(bool result, QString message) { emit __onUpdateVerifyFingerprintDialog(result, message); };
    
    void OnReadLockSet(int nLockNum, QDateTime start, QDateTime end) { emit __OnReadLockSet(nLockNum, start, end); }
    void OnLockSet(CLockSet *pSet) { emit __OnLockSet(pSet); }
    void OnIdentifiedFingerprint(QString sCode, QString sCode2);
    
signals:
    void __OnReadLockHistorySet(int nLockNum, QDateTime start, QDateTime end);
    void __OnLockHistorySet(CLockHistorySet *pLockSet);
    
public slots:
    void OnReadLockHistorySet(int nLockNum, QDateTime start, QDateTime end) { emit __OnReadLockHistorySet(nLockNum, start, end); }
    void OnLockHistorySet(CLockHistorySet *pSet) { emit __OnLockHistorySet(pSet); }

    void OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd, int nLockNum);

signals:
    void __RequestLastSuccessfulLogin();
public slots:
    void RequestLastSuccessfulLogin();
    void OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory);

public slots:
    void OnUpdateCurrentAdmin(CAdminRec *adminInfo) {emit __UpdateCurrentAdmin(adminInfo);}
    void OnUpdatedCurrentAdmin(bool bSuccess) { emit __OnUpdatedCurrentAdmin(bSuccess); }
    void OnBrightnessChanged(int nValue);

    void OnUpdateCodeState(CLockState *rec) { emit __OnUpdateCodeState(rec); }
    void OnUpdatedCodeState(bool bSuccess) { emit __OnUpdatedCodeState(bSuccess); }

private slots:
    void OnCodeEntered(QString sCode);

    void OnFingerprintCodeEntered(QString sCode);
    void OnFingerprintCodeEnteredTwo(QString sCode);
         
    void OnCodeEnteredTwo(QString sCode);
    void OnAdminPasswordEntered(QString sPW);
    void OnRequestedCurrentAdmin(CAdminRec *adminInfo);

    void OnAdminDialogClosed();

    void OnUserCodeCancel();

    void OnOpenLockRequest(int nLockNum);
    void OnReadLockStatus();


    void OnTouchScreenTouched();
    void resetCodeMessage();
    void OnCardSwipe(QString sCode1, QString sCode2);
    void OnFingerSwipe(QString sCode1, QString sCode2);
    void OnHIDCard(QString sCode1, QString sCode2);
public slots:
    void DisplayUserCodeDialog()
    {

    }

    void AccessCodeVerification(QString verification)
    {

    }      // Just one check at a time right now.

    // Security Controller connections
    void    OnRequireAdminPassword();
    void    OnRequireCodeTwo();
    void    OnAdminSecurityCheckOk();
    void    OnAdminSecurityCheckFailed();
    void    OnSecurityCheckSuccess(int doorNum);
    void    OnSecurityCheckedFailed();
    void    OnSecurityCheckTimedOut();

    void resetToTimeoutScreen();

    void OnRequestCurrentAdmin();

    void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
    int watchUSBStorageDevices(char mountedDevices[2][40], int mountedDeviceCount);
    
    void start();
    void OnAdminPasswordCancel();
};


#endif // CSYSTEMCONTROLLER_H
