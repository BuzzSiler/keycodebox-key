#ifndef CREPORTCONTROLLER_H
#define CREPORTCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include "smtp/SmtpMime"
#include "tbladmin.h"
#include "tblcodehistory.h"
#include "lockhistoryset.h"

class CReportController : public QObject
{
    Q_OBJECT

    public:
        explicit CReportController(QObject *parent = 0);
    public:
        void start();
        void setCodeHistoryTable(CTblCodeHistory *pTable);
        QString createNewFileName();
        void processLockCodeHistoryReport(QFile *pFile, QDateTime dtReportStart, QDateTime dtReportEnd);    
        void processImmediateReport(QDateTime dtReportStart, QDateTime dtReportEnd);

    signals:
        void __EmailFailed();
        void __EmailSucceeded();
        void __OnRequestCurrentAdmin();
        void __RequestCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd);

    public slots:
        void OnCodeHistoryForDateRange(CLockHistorySet *pLockHistorySet);
        void OnSendEmail(const QString SMTPServer, const int &SMTPPort, const int &SMTPType,
                         const QString &SMTPUsername,
                         const QString &SMTPPassword, const QString &from, const QString &to,
                         const QString &subject, const QString &body, const QFile *pfileAttach );
        void OnStop();
        void OnStart();

    private slots:
        void OnCheckIfReportingTimeEvent();
        void OnRequestedCurrentAdmin(CAdminRec *adminInfo);

    private:
        QTimer  _reportCheckTimer;
        CAdminRec *_padminInfo;
        CTblCodeHistory     *_ptblCodeHistory = NULL;
        bool _bCurrentAdminRetrieved = false;
        QDateTime   _dtLastReportDate;
        QDateTime   _dtLastDeleteDate;
        QFile *_pFile;
        QDateTime _dtReportStart;
        QDateTime _dtReportEnd;

        void requestCurrentAdminRecord();

        bool timetoSendReport(QDateTime date, QDateTime &dtReportStart, QDateTime &dtReportEnd);
        bool timetoDeleteOldReports(QDateTime dtNow, QDateTime& dtDelete);
        void assembleAndSendCodeRecordsForReportDate(QDateTime dtStart, QDateTime dtEnd);
        void buildReportFile(CLockHistorySet *pLockHistorySet, CAdminRec *adminInfo, QFile **pFile);
        void PrepareToSendEmail(CAdminRec *admin, QFile *pFile);
        void RemoveOldReports(QDateTime dtDelete);
};

#endif // CREPORTCONTROLLER_H
