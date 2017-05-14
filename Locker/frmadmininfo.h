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
#include "dlgfullkeyboard.h"
#include "currentedit.h"
#include "dlgsmtp.h"
#include "dlgvnc.h"
#include "frmcodeedit.h"

namespace Ui {
class CFrmAdminInfo;
}

#define COL_LOCKNUM 0
#define COL_DESC    1
#define COL_CODE1   2
#define COL_CODE2   3
#define COL_START   4
#define COL_END     5

class CFrmAdminInfo : public QDialog
{
    Q_OBJECT

public:
    explicit CFrmAdminInfo(QWidget *parent = 0);
    ~CFrmAdminInfo();

    void setSystemController(CSystemController *psysController); // { _psysController = psysController; }
    void getSysControllerLockSettings();

    void checkAndCreateCurrentLabelEdit();
    
    CLockState* createNewLockState();

    void editCodeByRow(int row);

    void checkAndCreateCodeEditForm();

    void resizeTreeViewColumns();

private:
    Ui::CFrmAdminInfo   *ui;
    CSystemController   *_psysController;
    bool                _bClose;
    QMenu               *_pTableMenu, *_pTableMenuAdd;
    int                 _nRowSelected = 0;
    bool                _bAddingCode = false;

    const char *fNever = "Never";
    const char *fEach = "Each Activity";
    const char *fHourly = "Hourly";
    const char *fEvery12Hours = "Every 12 Hours";
    const char *fDaily = "Daily";
    const char *fWeekly = "Weekly";
    const char *fMonthly = "Monthly";

    const int fLockNum = 0;
    const int fDesc = 1;
    const int fCode1 = 2;
    const int fCode2 = 3;
    const int fStart = 4;
    const int fEnd = 5;

    const bool fFingerprint1 = 6;
    const bool fFingerprint2 = 7; 

    const bool fAskQuestions = 8;
    const bool fQuestion1 = 9;
    const bool fQuestion2 = 10;
    const bool fQuestion3 = 11;
    
    QString usbDevice0;
    QString usbDevice1;
    
    QDateTime _currentTime;
    
    QString _DATENONE = QDateTime(QDate(1990,1,1), QTime(0,0,0)).toString("yyyy-MM-dd HH:mm:ss");

    CAdminRec           _tmpAdminRec;
    uint64_t            _un64LockLocks;
    CLocksStatus        *_pLocksStatus;

    CLockState          *_pState = 0;
    CLockSet            *_pworkingSet = 0;
    CLockHistorySet     *_phistoryWorkingSet = 0;

    QFileSystemModel    *_pmodel;
    QFileSystemModel    *_pcopymodel;
    QPoint              _lastTouchPos;

    QTimer              _timerMedia;

    CDlgFullKeyboard    *_pKeyboard = 0;
    CCurrentEdit        *_pcurrentLabelEdit = 0;

    CFrmCodeEdit        *_pFrmCodeEdit = 0;

    QString             _reportDirectory;
    QString             _copyDirectory;

    int                 _tmpDoorOpen;
    bool                _bContinueOpenLoop, _bStopOpen;

    void ExtractCommandOutput(FILE *pf, std::string &rtnStr);
    std::string rtrim(std::string &s);
    
    void initialize();
    bool isInternetTime();
    void onButtonClick(char key);
    void onBackSpace();
    void hideKeyboard(bool bHide);
    void onDelete();
    void populateAvailableDoors();
    void open_single_door(int nLockNum);
    void createLockMenus();

    void initializeConnections();
    int parseLockNumFromObjectName(QString objectName);
    int isLock(uint16_t nLockNum);
    void openAllDoors();
    void displayInTable(CLockSet *pSet);
    void setupCodeTableContextMenu();
    void displayInHistoryTable(CLockHistorySet *pSet);
    void populateBoxListSelection(QComboBox *cbox);
    void populateCodeLockSelection();
    void populateCodeLockHistorySelection();
    void populateFileWidget(QString sDirectory, QString sFilter);
    void startMediaTimer();
    void setTableMenuLocation(QMenu*);
    int nthSubstr(int n, const std::string& s, const std::string& p);
    void getSystemIPAddressAndStatus();
    void populateFileCopyWidget(QString sDirectory, QString sFilter);
    void purgeCodes();

    
    QString loadLockboxCapacity();
    void saveLockboxCapacity(QString capacity);
    
public slots:
    void OnRequestedCurrentAdmin(CAdminRec *adminInfo);
//    void onTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);
    void OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);

signals:
    
    void __OnDoneSave(int nRow, int nId, int nLockNum, QString sAccessCode, QString sSecondCode, QString sDescription, QDateTime dtStart, QDateTime dtEnd, bool fingerprint1, bool fingerprint2, bool askQuestions, QString question1, QString question2, QString question3);
    void __OnCloseFrmAdmin();
    void __TextEntered(QString sText);
    void __KeyPressed(char key);
    void __OnCloseWidgetEdit();

    void __OnRequestCurrentAdmin();

    void __UpdateCurrentAdmin(CAdminRec *adminInfo);
    void __OnUpdatedCurrentAdmin(bool bSuccess);

    void __OnOpenLockRequest(int nLockNum);

    void __OnReadDoorLocksState();

    void __OnBrightnessChanged(int nValue);

    void __OnImmediateReportRequest(QDateTime dtReportStart, QDateTime dtReportEnd, int nLockNum);
    void __OnAdminInfoCodes(QString code1, QString code2);

signals:
    void __LocalOnReadLockSet(int nLockNum, QDateTime start, QDateTime end);
    void __LocalOnReadLockHistorySet(int nLockNum, QDateTime start, QDateTime end);
    void __OnReadLockSet(int nLockNum, QDateTime start, QDateTime end);
    void __OnLockSet(CLockSet *pLockSet);
public slots:
    void OnReadLockSet(int nLockNum, QDateTime start, QDateTime end) { emit __OnReadLockSet(nLockNum, start, end); }
    void OnLockSet(CLockSet *pSet);

signals:
    void __OnReadLockHistorySet(int nLockNum, QDateTime start, QDateTime end);
    void __OnLockHistorySet(CLockHistorySet *pLockSet);

    void __OnUpdateCodeState(CLockState *rec);

public slots:
    void OnReadLockHistorySet(int nLockNum, QDateTime start, QDateTime end) { emit __OnReadLockHistorySet(nLockNum, start, end); }
    void OnLockHistorySet(CLockHistorySet *pSet);

public slots:
    void OnUpdateCurrentAdmin(CAdminRec *adminInfo) {emit __UpdateCurrentAdmin(adminInfo);}
    void OnUpdatedCurrentAdmin(bool bSuccess);

    void OnUpdateCodeState(CLockState *rec) { emit __OnUpdateCodeState(rec); }
    void OnUpdatedCodeState(bool bSuccess);

    void OnFoundNewStorageDevice(QString device0, QString device1);
    
    void onStartEditLabel(QLabel* pLabel, QString sLabelText);

    void onStopEdit();
    void OnCodeEditDoneSave(int nRow, int nId, int nLockNum, QString sAccessCode, QString sSecondCode, QString sDescription, QDateTime dtStart, QDateTime dtEnd, bool fingerprint1, bool fingerprint2, bool askQuestions, QString question1, QString question2, QString question3);
    void OnCodeEditClose();
protected:
    void touchEvent(QTouchEvent *ev);
    bool eventFilter(QObject *target, QEvent *event);
private slots:
    void OnCodes(QString code1, QString code2);
    void LocalReadLockSet(int nLock, QDateTime dtStart, QDateTime dtEnd);
    void LocalReadLockHistorySet(int nLock, QDateTime dtStart, QDateTime dtEnd);

    void menuSelection(QAction *action);
    void OnLockStatusUpdated(CLocksStatus *pLocksStatus);

    void codeDeleteSelection();
    void codeAddNew();
    void codeInitNew();
    void codeEditSelection();

    void on_btnDone_clicked();
    void on_btnSaveSettings_clicked();
    
    void on_lblName_clicked();
    void on_lblEmail_clicked();
    void on_lblPhone_clicked();
    void on_lblAccessCode_clicked();
    void on_lblPassword_clicked();
    void on_lblKey_clicked();

    void setTime();
    void setTimeZone();
    void on_cbInternetTime_clicked();
    void on_btnSetTime_clicked();
    
    void on_btnOpenDoor_clicked();

    void on_cbBox1_clicked();
    void on_cbBox2_clicked();
    void on_cbBox3_clicked();
    void on_cbBox4_clicked();
    void on_cbBox5_clicked();
    void on_cbBox6_clicked();
    void on_cbBox7_clicked();
    void on_cbBox8_clicked();
    void on_cbBox9_clicked();
    void on_cbBox10_clicked();
    void on_cbBox11_clicked();
    void on_cbBox12_clicked();
    void on_cbBox13_clicked();
    void on_cbBox14_clicked();
    void on_cbBox15_clicked();
    void on_cbBox16_clicked();
    void on_cbBox17_clicked();
    void on_cbBox18_clicked();
    void on_cbBox19_clicked();
    void on_cbBox20_clicked();
    void on_cbBox21_clicked();
    void on_cbBox22_clicked();
    void on_cbBox23_clicked();
    void on_cbBox24_clicked();
    void on_cbBox25_clicked();
    void on_cbBox26_clicked();
    void on_cbBox27_clicked();
    void on_cbBox28_clicked();
    void on_cbBox29_clicked();
    void on_cbBox30_clicked();
    void on_cbBox31_clicked();
    void on_cbBox32_clicked();
    void on_cbBox33_clicked();
    void on_cbBox34_clicked();
    void on_cbBox35_clicked();
    void on_cbBox36_clicked();
    void on_cbBox37_clicked();
    void on_cbBox38_clicked();
    void on_cbBox39_clicked();
    void on_cbBox40_clicked();
    void on_cbBox41_clicked();
    void on_cbBox42_clicked();
    void on_cbBox43_clicked();
    void on_cbBox44_clicked();
    void on_cbBox45_clicked();
    void on_cbBox46_clicked();
    void on_cbBox47_clicked();
    void on_cbBox48_clicked();
    void on_cbBox49_clicked();
    void on_cbBox50_clicked();
    void on_cbBox51_clicked();
    void on_cbBox52_clicked();
    void on_cbBox53_clicked();
    void on_cbBox54_clicked();
    void on_cbBox55_clicked();
    void on_cbBox56_clicked();
    void on_cbBox57_clicked();
    void on_cbBox58_clicked();
    void on_cbBox59_clicked();
    void on_cbBox60_clicked();
    void on_cbBox61_clicked();
    void on_cbBox62_clicked();
    void on_cbBox63_clicked();
    void on_cbBox64_clicked();


    void on_dialBright_valueChanged(int value);
    void on_btnReadCodes_clicked();
    void codeTableCellSelected(int nRow, int nCol);
    void codeHistoryTableCellSelected(int nRow, int nCol);

    void codeCellSelected(int row, int col);
    void on_btnReadCodes_2_clicked();
    void on_tblCodesList_clicked(const QModelIndex &index);
    void on_btnSetupSMTP_clicked();
    void onSMTPDialogComplete(CDlgSMTP *dlg);

    //remote desktop server settings
    void on_btnSetupVNC_clicked();
    void onVNCDialogComplete(CDlgVNC *dlg);

    //timezone info
    void populateTimeZoneSelection(QComboBox *cbox);
    
    void OnCloseAdmin();
    void on_btnPrintReport_clicked();
    void on_treeView_clicked(const QModelIndex &index);
    void OnMediaCheckTimeout();
    void onModelDirectoryLoaded(QString path);
    void onRootPathChanged(QString path);
    void on_btnOpenAllDoors_2_clicked(bool checked);
    void OpenDoorTimer();
    void on_btnReadDoorLocks_2_clicked();
    void on_btnToggleSource_clicked(bool checked);
    void on_tblCodesList_doubleClicked(const QModelIndex &index);
    void on_tblCodesList_cellDoubleClicked(int row, int column);
    void on_tblCodesList_cellClicked(int row, int column);
    void OnRowSelected(int row, int column);
    void deleteCodeByRow(int row);
    void addCodeByRow(int row);
    void OnHeaderSelected(int nHeader);
    void on_treeViewCopy_clicked(const QModelIndex &index);
    void onCopyRootPathChanged(QString path);
    void onCopyModelDirectoryLoaded(QString path);
    void on_btnCopyFile_clicked();

    void printElementNames(xmlNode *aNode);
    void on_btnCopyFileLoadCodes_clicked();
    void on_btnCopyFileBrandingImage_clicked();
    void on_btnCopyFileBrandingImageReset_clicked();

    void on_btnCopyToggleSource_clicked(bool checked);
    void on_btnRebootSystem_clicked();
    void on_btnPurgeCodes_clicked();
};

#endif // FRMADMININFO_H
