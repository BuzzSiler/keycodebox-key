#ifndef FRMADMININFO_H
#define FRMADMININFO_H

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <QDialog>
#include <QDebug>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QDirModel>
#include <QFileSystemModel>
#include <QPointF>
#include "systemcontroller.h"
#include "clickablelabel.h"
#include "frmcodeeditmulti.h"
#include "codelistingelement.h"
#include "kcbcommon.h"

namespace Ui {
class CFrmAdminInfo;
}

#define COL_LOCKNUM 0
#define COL_DESC    1
#define COL_CODE1   2
#define COL_CODE2   3
#define COL_START   4
#define COL_END     5

class SelectLocksWidget;
class ReportControlWidget;


class CFrmAdminInfo : public QDialog
{
    Q_OBJECT

    public:
        explicit CFrmAdminInfo(QWidget *parent = 0);
        ~CFrmAdminInfo();

        void setSystemController(CSystemController *psysController);
        void getSysControllerLockSettings();

        CLockState* createNewLockState();

        void editCodeByRow(int row);
        void checkAndCreateCodeEditForm();
        void show();
        int getDisplayPowerDownTimeout();

    signals:
        void __OnCloseFrmAdmin();
        void __TextEntered(QString sText);
        void __KeyPressed(char key);
        void __OnCloseWidgetEdit();
        void __OnRequestCurrentAdmin();
        void __UpdateCurrentAdmin(CAdminRec *adminInfo);
        void __OnUpdatedCurrentAdmin(bool bSuccess);
        void __OnOpenLockRequest(QString LockNums);
        void __OnReadDoorLocksState();
        void __OnBrightnessChanged(int nValue);
        void __OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd);
        void __OnAdminInfoCodes(QString code1, QString code2);

        void __OnReadLockSet(QString LockNums, QDateTime start, QDateTime end);
        void __OnLockSet(CLockSet *pLockSet);
        void __OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end);
        void __OnLockHistorySet(CLockHistorySet *pLockSet);
        void __OnUpdateCodeState(CLockState *rec);
        void __OnSendTestEmail(int test_type);
        void __OnDisplayFingerprintButton(bool state);
        void __OnDisplayShowHideButton(bool state);
        void __OnDisplayTakeReturnButtons(bool state);

    
    public slots:
        void OnRequestedCurrentAdmin(CAdminRec *adminInfo);
        void OnReadLockSet(QString LockNums, QDateTime start, QDateTime end) { emit __OnReadLockSet(LockNums, start, end); }
        void OnLockSet(CLockSet *pSet);
        void OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end) { emit __OnReadLockHistorySet(LockNums, start, end); }
        void OnLockHistorySet(CLockHistorySet *pSet);
        void OnUpdateCurrentAdmin(CAdminRec *adminInfo) {emit __UpdateCurrentAdmin(adminInfo);}
        void OnUpdatedCurrentAdmin(bool bSuccess);

        void OnUpdateCodeState(CLockState *rec) { emit __OnUpdateCodeState(rec); }
        void OnUpdatedCodeState(bool bSuccess);

        void OnFoundNewStorageDevice(QString device0, QString device1);


        void onStopEdit();
        void OnCodeEditClose();
        void OnTabSelected(int index);
        void OnDisplayFingerprintButton(bool);
        void OnDisplayShowHideButton(bool);
        void OnOpenLockRequest(QString lock, bool is_user);
        void OnNotifyGenerateReport();    
        void OnDisplayTakeReturnButtons(bool);

    private slots:
        void OnCodes(QString code1, QString code2);
        void OnLockStatusUpdated(CLocksStatus *pLocksStatus);

        void codeDeleteSelection();
        void codeAddNew();
        void codeInitNew();
        void codeEnableAll();
        void codeEditSelection();

        void on_btnDone_clicked();

        void on_lblName_clicked();
        void on_lblEmail_clicked();
        void on_lblPhone_clicked();
        void on_lblPassword_clicked();
        void on_lblAssistPassword_clicked();

        void setTime();
        void setTimeZone();
        void on_cbInternetTime_clicked();
        void on_btnSetTime_clicked();

        void on_btnReadCodes_clicked();

        void codeCellSelected(int row, int col);

        //timezone info
        void populateTimeZoneSelection(QComboBox *cbox);

        void OnCloseAdmin();
        void OnMediaCheckTimeout();
        void onRootPathChanged(QString path);
        void OnRowSelected(int row, int column);
        void deleteCodeByRow(int row);
        void addCodeByRow();
        void OnHeaderSelected(int nHeader);
        void on_treeViewCopy_clicked(const QModelIndex &index);
        void onCopyRootPathChanged(QString path);
        void onCopyModelDirectoryLoaded(QString path);
        void on_btnCopyFile_clicked();

        void on_btnCopyFileBrandingImage_clicked();
        void on_btnCopyFileBrandingImageReset_clicked();

        void on_btnRebootSystem_clicked();
        void on_btnPurgeCodes_clicked();
        void on_btnRead_clicked();

        void on_btnTestEmail_clicked();
        void on_btnTestUserEmail_clicked();

        void OnCodeEditReject();
        void OnCodeEditAccept();

        void on_pbNetworkSettings_clicked();
		void codeHistoryTableCellSelected( int row, int col);

        void on_cbActionsSelect_currentIndexChanged(int index);
        void on_btnActionExecute_clicked();
        void on_cbUsbDrives_currentIndexChanged(const QString &arg1);
        void on_cbFileFormat_currentIndexChanged(const QString &arg1);
        void on_pbUtilUnmountDrive_clicked();

    private:
        Ui::CFrmAdminInfo   *ui;
        CSystemController   *_psysController;
        bool                _bClose;
        QMenu               *_pTableMenu, *_pTableMenuAdd;
        int                 _nRowSelected = 0;
        bool                _bAddingCode = false;

        QString usbDevice0;
        QString usbDevice1;

        QDateTime _currentTime;

        CAdminRec           _tmpAdminRec;
        uint64_t            _un64LockLocks;
        CLocksStatus        *_pLocksStatus;

        CLockState          *_pState = 0;
        CLockSet            *_pworkingSet = 0;
        CLockHistorySet     *_phistoryWorkingSet = 0;

        QFileSystemModel    *_pcopymodel;
        QPoint              _lastTouchPos;

        QTimer              _timerMedia;

        FrmCodeEditMulti    *_pFrmCodeEditMulti = 0;

        QString             _copyDirectory;

        EMAIL_ADMIN_SELECT  _testEmail;

        SelectLocksWidget&  m_select_locks;
        QStringList         _codesInUse;
        ReportControlWidget& m_report;
        QStringList         m_file_filter;

        typedef enum { 
            UTIL_ACTION_INSTALL_APP,    
            UTIL_ACTION_SET_BRANDING_IMAGE,
            UTIL_ACTION_DEFAULT_BRANDING_IMAGE,
            UTIL_ACTION_IMPORT_CODES,
            UTIL_ACTION_EXPORT_CODES,
            UTIL_ACTION_EXPORT_LOGS } UTIL_ACTION_TYPE;

        UTIL_ACTION_TYPE    m_util_action;        

        void ExtractCommandOutput(FILE *pf, std::string &rtnStr);

        void initialize();
        bool isInternetTime();
        void onButtonClick(char key);
        void onBackSpace();
        void hideKeyboard(bool bHide);

        void initializeConnections();
        void createCodeTableHeader();
        void displayInTable(CLockSet *pSet);
        void setupCodeTableContextMenu();
        void displayInHistoryTable(CLockHistorySet *pSet);
        void startMediaTimer();
        void setTableMenuLocation(QMenu*);
        int nthSubstr(int n, const std::string& s, const std::string& p);
        void getSystemIPAddressAndStatus();
        void populateFileCopyWidget(QString sDirectory, QStringList sFilter={});
        void purgeCodes();

        void HandleCodeUpdate();
        void setPStateValues(QString lockNums,
                            QString sAccessCode,
                            QString sSecondCode,
                            QString sUsername,
                            QDateTime dtStart,
                            QDateTime dtEnd,
                            bool fingerprint1,
                            bool fingerprint2,
                            bool askQuestions,
                            QString question1,
                            QString question2,
                            QString question3,
                            int access_type);
        void RunKeyboard(QString& text, bool numbersOnly = false);
        void OnNotifyUsbDrive(QStringList list);
        void setFileFilterFromFormatSelection(const QString filter);
                         
        void insertCodes(CodeListing& codeListing);
        void updateTmpAdminRec();
        void updateAdminForEmail(EMAIL_ADMIN_SELECT email_select);

    protected:
        void touchEvent(QTouchEvent *ev);
        bool eventFilter(QObject *target, QEvent *event);
};

#endif // FRMADMININFO_H
