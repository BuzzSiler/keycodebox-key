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
#include "dlgfingerprintverify.h"
#include "frmselectlocks.h"
#include "kcbcommon.h"
#include "keycodeboxsettings.h"


class Omnikey5427CKReader;

class CSystemController : public QObject
{
    Q_OBJECT

    public:
        enum SystemState
        {
            ENone, ETimeoutScreen, EUserCodeOne, EUserCodeTwo,
            EAdminPassword, EPasswordFailed, EPasswordTimeout,
            EAdminPasswordFailed, EThankYou, EAdminMain, EAssistMain
        };

        explicit CSystemController(QObject *parent = 0);
        ~CSystemController();

        void initialize(QThread *pthread);
        SystemState getSystemState() { return _systemState; }
        SystemState getSystemStateDisplay() { return _systemStateDisplay; }
        bool getDisplayFingerprintButton();
        bool getDisplayShowHideButton();
        bool getDisplayTakeReturnButtons();
        QString getAdminType() { return _adminType; }

        void setMainWindow(QMainWindow *mw) { _pmainWindow = mw; }

        void looprun();

        const CLockController &getLockController() { return _LockController; }

        void reportActivity(QString locknums);

        void getAllCodes1(QStringList& codes1);
        void readAllCodes(CLockSet **lockset, bool clear_or_encrypted);

        void DiscoverHardware();
        void UpdateLockRanges();

    private:
        QThread                 *_pInitThread;
        QThread                 _threadReport;
        QThread                 _threadHID, _threadCardReader;
        CSecurityController     _securityController;
        CLCDGraphicsController  _LCDGraphicsController;
        CLockController         _LockController;
        CReportController       _ReportController;
        QMainWindow             *_pmainWindow;
        QString                 _adminType;

        CMagTekCardReader       *_pmagTekReader = 0;
        CHWKeyboardReader       *_phidReader = 0;
        CFingerprintReader      *_fingerprintReader = 0;

        int             _lockNum;
        QString         _locks;
        SystemState     _systemState;
        SystemState     _systemStateDisplay;
        bool            _bCurrentAdminRetrieved;
        CAdminRec       *_padminInfo;

        CFrmUserCode *_pfUsercode;
        CDlgFingerprintVerify *_pdFingerprintVerify;

        Omnikey5427CKReader* _omnikey5427CKReader;
        


        QTimer      *_ptimer;
        uint64_t    _un64Locks;

        QString _answer1;
        QString _answer2;
        QString _answer3;
        bool _answers_provided;
        

        void initializeSecurityConnections();
        void initializeLockController();

        void initializeReportController();
        void startTimeoutTimer(int duration);
        void stopTimeoutTimer();
        void initializeReaders();
        QString getCodeToUse(QString code1, QString code2);
        void sendEmailReport(QDateTime access, QString desc, QString lockNums);
        

    signals:
        void __verifyUserAccess(QString sCode1);
        void __verifyUserAccessTwo(QString sCode1, QString sCode2);
        void __verifyAdminAccess(QString sCode1, QString sCode2);

        void __openDoor(QString lockNums);

        // For Entry dialogs
        void __OnDisplayTimeoutScreen();
        void __OnCodeMessage(QString sMsg);
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
        void __OnDisplayFingerprintButton(bool);
        void __OnDisplayShowHideButton(bool);
        void __OnHideShowPassword(bool);
        void __OnDisplayTakeReturnButtons(bool);

        void __onUserCodeOne(QString sCode1);
        void __onUserCodeTwo(QString sCode2);

        void __onUserFingerprintCodeOne(QString sCode1);
        void __onUserFingerprintCodeTwo(QString sCode2);

        void __onUserCodes(QString sCode1, QString sCode2);

        void __OnReadLockSet(QString LockNums, QDateTime start, QDateTime end);
        void __OnLockSet(CLockSet *pLockSet);
        void __OnSendEmail(const QString SMTPServer, const int &SMTPPort, const int &SMTPType,
                        const QString &SMTPUsername, const QString &SMTPPassword,
                        const QString &from, const QString &to, const QString &subject, const QString &body, const QFile *fileAttach );

        void __onUpdateEnrollFingerprintDialog(int current, int total, QString message);
        void __onUpdateVerifyFingerprintDialog(bool result, QString message);

        void __onEnrollFingerprintDialog(QString sCode);

        void __onQuestionUserDialog(QString lockNums, QString question1, QString question2, QString question3);
        void __onQuestionUser(QString lockNums, QString question1, QString question2, QString question3);
        void __onQuestionUserAnswers(QString lockNums, QString answer1, QString answer2, QString answer3);
        void __onQuestionUserCancel();

        void DiscoverHardwareProgressUpdate(int);

    public slots:

        void TrigQuestionUserDialog(QString lockNums, QString question1, QString question2, QString question3) { emit __onQuestionUserDialog(lockNums, question1, question2, question3);}

        void TrigQuestionUser(QString lockNums, QString question1, QString question2, QString question3);
        void QuestionUserCancel();
        void AnswerUserSave(QString lockNums, QString question1, QString question2, QString question3);

        void TrigEnrollFingerprint(QString sCode);
        void TrigEnrollFingerprintDialog(QString sCode) { emit __onEnrollFingerprintDialog(sCode); }

        void EnrollFingerprintDialogCancel();
        void EnrollFingerprintResetStageCount();
        void OnFingerprintStageComplete(int current, int total, QString message) {  emit __onUpdateEnrollFingerprintDialog(current, total, message); };

        void OnVerifyFingerprint();
        void OnVerifyFingerprintDialogCancel();
        void OnFingerprintVerifyComplete(bool result, QString message);
        void OnReadLockSet(QString LockNums, QDateTime start, QDateTime end);
        void OnLockSet(CLockSet *pSet);
        void OnIdentifiedFingerprint(QString sCode, QString sCode2);

        void OnVerifyFingerprintDialog();


    signals:
        void __OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end);
        void __OnLockHistorySet(CLockHistorySet *pLockSet);

    public slots:
        void OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end) { emit __OnReadLockHistorySet(LockNums, start, end); }
        void OnLockHistorySet(CLockHistorySet *pSet) { emit __OnLockHistorySet(pSet); }

        void OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd);

    signals:
        void __RequestLastSuccessfulLogin(QString locknums);
        void __RequestLastSuccessfulLoginWithAnswers(QString locknums, QString answer1, QString answer2, QString answer3);
        void __OnSendTestEmail(int select_type);
        
    public slots:
        void RequestLastSuccessfulLogin(QString locknums);
        void OnLastSuccessfulLoginRequest(CLockHistoryRec *pLockHistory);

    public slots:
        void OnUpdateCurrentAdmin(CAdminRec *adminInfo) {emit __UpdateCurrentAdmin(adminInfo);}
        void OnUpdatedCurrentAdmin(bool bSuccess) { emit __OnUpdatedCurrentAdmin(bSuccess); }
        void OnBrightnessChanged(int nValue);

        void OnUpdateCodeState(CLockState *rec) { emit __OnUpdateCodeState(rec); }
        void OnUpdatedCodeState(bool bSuccess) { emit __OnUpdatedCodeState(bSuccess); }
        void OnSendTestEmail(int select_type);

    private slots:
        void OnCodeEntered(QString sCode);

        void OnFingerprintCodeEntered(QString sCode);
        void OnFingerprintCodeEnteredTwo(QString sCode);

        void OnCodeEnteredTwo(QString sCode);
        void OnAdminPasswordEntered(QString sPW);
        void OnRequestedCurrentAdmin(CAdminRec *adminInfo);

        void OnAdminDialogClosed();

        void OnUserCodeCancel();

        void OnOpenLockRequest(QString lockNum);
        void OnReadLockStatus();

        void OnTouchScreenTouched();
        void resetCodeMessage();
        void OnCardSwipe(QString sCode1, QString sCode2);
        void OnFingerSwipe(QString sCode1, QString sCode2);
    public slots:
        void OnHIDCard(QString sCode1, QString sCode2);
        void DisplayUserCodeDialog()
        {
        }

        void AccessCodeVerification()
        {
        }      // Just one check at a time right now.

        // Security Controller connections
        void    OnRequireAdminPassword();
        void    OnRequireCodeTwo();
        void    OnAdminSecurityCheckOk(QString type);
        void    OnAdminSecurityCheckFailed();
        void    OnSecurityCheckSuccess(QString locks);
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
