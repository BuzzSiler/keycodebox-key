#ifndef CMODELSECURITY_H
#define CMODELSECURITY_H

#include <QObject>
#include <QtSql>
#include <QDebug>
#include <QFileInfo>
#include <QTimer>
#include "tblcodes.h"
#include "tblcodehistory.h"
#include "tbladmin.h"

class CModelSecurity : public QObject
{
    Q_OBJECT

private:
    QSqlDatabase        _DB;
    CTblCodes           *_ptblCodes;
    CTblCodeHistory     *_ptblCodeHistory;
    CTblAdmin           *_ptblAdmin;

    QTimer          _timer;
    QString         _type;
    QTime           _time;

    bool            _wasLastSecurityCheckPredictive;

    void setupTables();
    void setupTimer();
    void initialize();
    void openDatabase();

    void removeCreatedObjects();
    bool CheckPredictiveAccessCode(QString code, int &nLockNum);
    //std::string encryptDecrypt(int nVal, std::string toEncrypt, std::string key);
    QDateTime &roundDateTime(int res, QDateTime &datetime);

    bool _updateCodeLockboxState = false;
    int _updateCodeLockboxStateIds = -1;
public:
    explicit CModelSecurity(QObject *parent = 0);    
    ~CModelSecurity();

signals:
    void __QuestionUserDialog(QString doorNums, QString question1, QString question2, QString question3);
    
    // Parallels signal flow to model security
    void __EnrollFingerprint(QString sCode);
    void __EnrollFingerprintDialog(QString sCode);

    void __onVerifyFingerprint(QString sCode);
    void __onVerifyFingerprintDialog(QString sCode);
    
    void __OnRequireAdminPassword();
    void __OnRequireCodeTwo();
    void __OnAdminSecurityCheckOk(QString type);
    void __OnAdminSecurityCheckFailed();
    void __OnSecurityCheckSuccess(QString locks);
    void __OnSecurityCheckSuccessWithAnswers(QString doorNums, QString answer1, QString answer2, QString answer3);
    void __OnSecurityCheckedFailed();

    void __OnSecurityCheckTimedOut();

    void __OnRequestedCurrentAdmin(CAdminRec *adminInfo);
    void __OnCreateHistoryRecordFromLastPredictiveLogin(QString LockNums, QString code);
signals:
    void __OnUpdatedCurrentAdmin(bool bSuccess);
    void __OnUpdatedCodeState(bool bSuccess);

signals:
    void __OnReadLockSet(QString LockNums);
    void __OnLockSet(CLockSet *pLockSet);
public slots:
    void OnReadLockSet(QString LockNums, QDateTime start, QDateTime end);

signals:
    void __OnReadLockHistorySet(QString LockNums, QDateTime start, QDateTime end);
    void __OnLockHistorySet(CLockHistorySet *pLockSet);
signals:
    void __OnLastSuccessfulLogin(CLockHistoryRec *);
public slots:
    void RequestLastSuccessfulLogin();

signals:
    void __OnCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd, CLockHistorySet *pLockHistorySet);
public slots:
    void OnRequestCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd);

public slots:
    void OnReadLockHistorySet(QString nLockNum, QDateTime start, QDateTime end);

public slots:
    void OnUpdateCurrentAdmin(CAdminRec *adminInfo);
    void OnUpdatedCurrentAdmin(bool bSuccess) { emit __OnUpdatedCurrentAdmin(bSuccess); }

    void OnUpdateCodeState(CLockState *rec);

private slots:
    void OnTimeout();

    void OnCreateHistoryRecordFromLastSuccessfulLogin();
    void OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString answer1, QString answer2, QString answer3);
    void OnCreateHistoryRecordFromLastPredictiveLogin(QString LockNums, QString code);

public slots:
    void OnVerifyCodeOne(QString code);
    void OnVerifyCodeTwo(QString code);

    void OnVerifyFingerprintCodeOne(QString code);
    void OnVerifyFingerprintCodeTwo(QString code);

    void OnSuccessfulQuestionUsersAnswers(QString doorNums, QString answer1, QString answer2, QString answer3);
    void OnQuestionUserCancelled();
    
    void OnVerifyAdminPassword(QString code);

    void OnRequestCurrentAdmin();
};

#endif // CMODELSECURITY_H
