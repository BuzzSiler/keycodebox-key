#include "reportcontroller.h"
#include <exception>
#include <QDir>
#include <QTextStream>
#include "encryption.h"
#include "unistd.h"
#include "kcbcommon.h"

#define REPORT_FILE "KeyCodeBox_%1.txt"
#define REPORT_FILE_FORMAT "yyyy-MM-dd-HH_mm_ss"

#define IS_HOURLY(dt) (dt == HOURLY)
#define IS_DAILY(dt) (dt == DAILY)
#define IS_WEEKLY(dt) (dt == WEEKLY)
#define IS_EVERY_12_HOURS(dt) (dt == EVERY_12_HOURS)
#define IS_BIWEEKLY(dt) (dt == BIWEEKLY)
#define IS_MONTHLY(dt) (dt == MONTHLY)
#define DAYS_IN_DAY (1)
#define DAYS_IN_WEEK (7)
#define DAYS_IN_TWOWEEKS (2*DAYS_IN_WEEK)
#define DAYS_IN_MONTH(dt) (dt.daysInMonth())


CReportController::CReportController(QObject *parent) : QObject(parent)
{
    _padminInfo = 0;
    start();

    _dtLastReportDate = QDateTime::currentDateTime();
    _dtLastDeleteDate = QDateTime::currentDateTime();
}

void CReportController::setCodeHistoryTable(CTblCodeHistory *pTable)
{
    _ptblCodeHistory = pTable;
}

void CReportController::start()
{
    KCB_DEBUG_ENTRY;
    _reportCheckTimer.setInterval(60000);

    connect(&_reportCheckTimer, SIGNAL(timeout()), this, SLOT(OnCheckIfReportingTimeEvent()));
    _reportCheckTimer.start();
}


void CReportController::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    KCB_DEBUG_ENTRY;
    _padminInfo = adminInfo;
    _bCurrentAdminRetrieved = true;
}

QString CReportController::createNewFileName()
{
    return QString(REPORT_FILE).arg(QDateTime::currentDateTime().toString(REPORT_FILE_FORMAT));
}

void CReportController::buildReportFile(CLockHistorySet *pLockHistorySet, CAdminRec *adminInfo, QFile **ppFile)
{
    KCB_DEBUG_ENTRY;

    CLockHistoryRec *plockHistoryRec;
    QString         strFileName = createNewFileName();
    QDir            dir(adminInfo->getReportDirectory());
    
    *ppFile = new QFile(dir.absoluteFilePath(strFileName));

    if(!(*ppFile)) 
    {
        KCB_DEBUG_TRACE("no file:" << dir.absoluteFilePath(strFileName));
        return;
    }

    if((*ppFile)->open(QIODevice::ReadWrite))
    {
        KCB_DEBUG_TRACE("file open!:" << dir.absoluteFilePath(strFileName));

        QDateTime dtAccess;
        QString sDesc;
        QString body;
        QString LockNums;
	    QString question1, question2, question3;
        QTextStream stream( *ppFile );
        stream << "KeyCodeBox Access History." << endl;

        auto itor = pLockHistorySet->getIterator();

        while (itor.hasNext())
        {
            plockHistoryRec = itor.next();
            
            body = "";

            dtAccess = plockHistoryRec->getAccessTime();
            sDesc = plockHistoryRec->getDescription();
            LockNums = plockHistoryRec->getLockNums();
	    
            body = QString("%1 #%2").arg(tr("Lock"), QVariant(LockNums).toString());

            if( sDesc.size() > 0 )
            {
                body += QString(" %1: [%2]").arg(tr("user"), sDesc);
            }
            body += QString(" %1: %2").arg(tr("accessed"), dtAccess.toString("MM/dd/yyyy HH:mm:ss"));

            QString code_one = plockHistoryRec->getCode1();
            QString code_two = plockHistoryRec->getCode2();
            QString question1 = plockHistoryRec->getQuestion1();
            QString question2 = plockHistoryRec->getQuestion2();
            QString question3 = plockHistoryRec->getQuestion3();

            body += QString(" %1 #1:%2").arg(tr("code")).arg(code_one);
            body += QString(" %1 #2:%2").arg(tr("code")).arg(code_two);
            body += QString(" %1 #1:%2").arg(tr("question")).arg(question1);
            body += QString(" %1 #2:%2").arg(tr("question")).arg(question2);
            body += QString(" %1 #3:%2").arg(tr("question")).arg(question3);
            stream << body << endl;
        }
        stream.flush();
        (*ppFile)->close();
    } 
    else 
    {
        KCB_DEBUG_TRACE("can't open file!:" << (*ppFile)->errorString() << dir.absoluteFilePath(strFileName));
        delete *ppFile;
        *ppFile = NULL;
    }
    KCB_DEBUG_ENTRY;
}

void CReportController::processImmediateReport(QDateTime dtReportStart, QDateTime dtReportEnd)
{
    KCB_DEBUG_ENTRY;
    
    requestCurrentAdminRecord();

    int nCount = 0;

    // 3 seconds to get Admin rec?
    while(!_bCurrentAdminRetrieved && nCount++ < 30)
    {
        usleep(100000);
        QCoreApplication::processEvents();
    }
    if(_bCurrentAdminRetrieved)
    {
        QFile       *pFile = NULL;
        CLockHistorySet *pHistorySet = NULL;
        processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd);
        if (pHistorySet) 
        {
            delete pHistorySet;
        }
        if (pFile) 
        {
            delete pFile;
        }
    }
    KCB_DEBUG_EXIT;
    
}

void CReportController::processLockCodeHistoryReport(CLockHistorySet *_pHistorySet, 
                                                     QFile *pFile, 
                                                     QDateTime &dtReportStart, 
                                                     QDateTime &dtReportEnd)
{
    KCB_DEBUG_ENTRY;

    Q_UNUSED(_pHistorySet);

    _pFile = pFile;
    _dtReportStart = dtReportStart;
    _dtReportEnd = dtReportEnd;

    assembleAndSendCodeRecordsForReportDate(dtReportStart, dtReportEnd);
    KCB_DEBUG_EXIT;
}


void CReportController::assembleAndSendCodeRecordsForReportDate(QDateTime dtStart, QDateTime dtEnd)
{
    KCB_DEBUG_ENTRY;
    emit __RequestCodeHistoryForDateRange(dtStart, dtEnd);
    KCB_DEBUG_EXIT;
    
}

void CReportController::OnCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd, CLockHistorySet *pLockHistorySet)
{
    Q_UNUSED(dtStart);
    Q_UNUSED(dtEnd);

    KCB_DEBUG_ENTRY;

    requestCurrentAdminRecord();
    int nCount = 0;
    // 3 seconds to get Admin rec?
    while(!_bCurrentAdminRetrieved && nCount++ < 30)
    {
        usleep(100000);
        QCoreApplication::processEvents();
    }
    if(_bCurrentAdminRetrieved)
    {
        buildReportFile(pLockHistorySet, _padminInfo, &_pFile);

        if( _padminInfo->getReportViaEmail() ) 
        {
            KCB_DEBUG_TRACE("calling PrepareToSendEmail");
            PrepareToSendEmail(_padminInfo, _pFile);
        }
        if( _padminInfo->getReportToFile()) 
        {
            KCB_DEBUG_TRACE("Save to file");
        } 
        else 
        {
            KCB_DEBUG_TRACE("DON'T Save to file. Deleting file.");
            if(_pFile) 
            {
                _pFile->remove();
            }
        }
    }
    KCB_DEBUG_EXIT;
}

void CReportController::requestCurrentAdminRecord()
{
    _bCurrentAdminRetrieved = false;
    emit __OnRequestCurrentAdmin();
}

bool CReportController::timetoSendReport(QDateTime date, QDateTime &dtReportStart, QDateTime &dtReportEnd)
{
    KCB_DEBUG_ENTRY;
    if( !_ptblCodeHistory) 
    {
        KCB_DEBUG_TRACE("Requires TblCodeHistory to be set.");
        return false;
    }

    QDateTime dtFreq = _padminInfo->getDefaultReportFreq();

    // Note: It appears that 'every activity' is not generating a report
    // Is 'every activity' handled elsewhere?

    if(dtFreq == QDateTime(QDate(), QTime(0,0)) ||
            dtFreq == QDateTime(QDate(1,1,1), QTime(0,0)))   // Represents admin wants each access sent
    {   // Never or each event are not sent from this process
        KCB_DEBUG_EXIT;
        return false;
    }

    QDateTime   tmpDate;
    QFile       *pFile = NULL;
    CLockHistorySet *pHistorySet = NULL;
    if (IS_HOURLY(dtFreq))
    {
        tmpDate = _dtLastReportDate.addSecs(3600);  // Add an hour in seconds
        if(tmpDate >= _dtLastReportDate)
        {
            dtReportStart = date.addSecs(-3600);
            dtReportEnd = date.currentDateTime();
            QString         LockNums;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd);
        }
    } 
    else if (IS_EVERY_12_HOURS(dtFreq))
    {
        tmpDate = _dtLastReportDate.addSecs(12 * 3600); // 12 hours
        if(tmpDate >= _dtLastReportDate)
        {
            dtReportStart = date.addSecs(-3600 * 12);
            dtReportEnd = date.currentDateTime();
            QString         LockNums;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd);
        }
    } 
    else if (IS_DAILY(dtFreq))
    {
        tmpDate = _dtLastReportDate.addDays(1);
        if(tmpDate >= _dtLastReportDate)
        {
            dtReportStart = date.addDays(-1);
            dtReportEnd = date.currentDateTime();
            QString         LockNums;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd);
        }
    } 
    else if (IS_WEEKLY(dtFreq))
    {
        tmpDate = _dtLastReportDate.addDays(7);
        if(tmpDate >= _dtLastReportDate)
        {
            dtReportStart = date.addDays(-7);
            dtReportEnd = date.currentDateTime();
            QString         LockNums;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd);
        }
    } 
    else if (IS_MONTHLY(dtFreq))
    {
        tmpDate = _dtLastReportDate.addMonths(1);
        if(tmpDate >= _dtLastReportDate)
        {
            dtReportStart = date.addMonths(-1);
            dtReportEnd = date.currentDateTime();
            QString         LockNums;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd);
        }
    }

    if(pHistorySet)
    {
        delete pHistorySet;
    }

    if(pFile) 
    {
        delete pFile;
        KCB_DEBUG_EXIT;
        return true;
    }

    else 
    {
        KCB_DEBUG_EXIT;
        return false;
    }
    
    KCB_DEBUG_EXIT;
}

bool CReportController::timetoDeleteOldReports(QDateTime dtNow, QDateTime& dtDelete)
{
    KCB_DEBUG_ENTRY;

    QDateTime dtFreq = _padminInfo->getDefaultReportDeleteFreq();
    quint64 numDays = _dtLastDeleteDate.daysTo(dtNow);
    KCB_DEBUG_TRACE(QString("%1 days since last deletion").arg(QString::number(numDays)));

    if (IS_DAILY(dtFreq))
    {
        if (((int)numDays) > DAYS_IN_DAY)
        {
            KCB_DEBUG_TRACE("Deleting reports older than 1 day");
            dtDelete = dtNow;
            KCB_DEBUG_EXIT;
            return true;
        }
    }
    else if (IS_WEEKLY(dtFreq))
    {
        if (((int) numDays) > DAYS_IN_WEEK)
        {
            KCB_DEBUG_TRACE("Deleting reports older than 1 week");
            dtDelete = dtNow;
            KCB_DEBUG_EXIT;
            return true;
        }
    }
    else if (IS_BIWEEKLY(dtFreq))
    {
        if (((int)numDays) > DAYS_IN_TWOWEEKS)
        {
            KCB_DEBUG_TRACE("Deleting reports older than 2 weeks");
            dtDelete = dtNow;
            KCB_DEBUG_EXIT;
            return true;
        }
    }
    else if (IS_MONTHLY(dtFreq))
    {
        if (((int)numDays) > DAYS_IN_MONTH(dtNow.date()))
        {
            KCB_DEBUG_TRACE("Deleting reports older than 1 month");
            dtDelete = dtNow;
            KCB_DEBUG_EXIT;
            return true;
        }
    }
    else
    {
        KCB_DEBUG_TRACE("No matching delete frequency");
    }

    KCB_DEBUG_EXIT;
    return false;
}

void CReportController::RemoveOldReports(QDateTime dtDelete)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE("Deleting files older than" << dtDelete);

    Q_ASSERT_X(_padminInfo != nullptr, Q_FUNC_INFO, "admin is null");
    if (_padminInfo)
    {
        //   Get a list of reports from the report directory sorted by date/time
        QDir dir(_padminInfo->getReportDirectory());
        auto entryList = dir.entryInfoList(QStringList() << "KeyCodeBox*.txt", QDir::Files, QDir::Time);
        if (!entryList.isEmpty())
        {
            foreach (QFileInfo entry, entryList)
            {
                //   Get the 'delete reports older than' value
                //   Determine what files in the list need to be deleted
                if (entry.lastModified() < dtDelete)
                {
                    //   Delete the relevant files
                    QFile file(entry.absoluteFilePath());
                    file.remove();
                }
            }
        }
    }
    KCB_DEBUG_EXIT;
}


void CReportController::OnCheckIfReportingTimeEvent()
{
    KCB_DEBUG_ENTRY;
    QDateTime   dtNow = QDateTime::currentDateTime();
    QDateTime dtReportStart, dtReportEnd, dtDelete;
    
    if(timetoSendReport(dtNow, dtReportStart, dtReportEnd)) 
    {
        _dtLastReportDate = QDateTime::currentDateTime();
        assembleAndSendCodeRecordsForReportDate(dtReportStart, dtReportEnd);
    }
    else if (timetoDeleteOldReports(dtNow, dtDelete))
    {
        _dtLastDeleteDate = QDateTime::currentDateTime();
        RemoveOldReports(dtDelete);
    }
    KCB_DEBUG_EXIT;
}

void CReportController::PrepareToSendEmail(CAdminRec *admin, QFile *pFile)
{
    KCB_DEBUG_ENTRY;

    QString SMTPSvr = admin->getSMTPServer();
    int SMTPPort = admin->getSMTPPort();
    int SMTPType = admin->getSMTPType();

    QString SMTPUser = admin->getSMTPUsername();
    QString SMTPPW = admin->getSMTPPassword();

    QString from = admin->getAdminEmail();
    QString to = admin->getAdminEmail();
    QString subject = tr("Lock Box History Report");

    QString body = QString("KeyCodeBox %1 :%2").arg(tr("report")).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, pFile );

    KCB_DEBUG_EXIT;
}

void CReportController::OnSendEmail(const QString SMTPServer, const int &SMTPPort, const int &SMTPType,
                                    const QString &SMTPUsername,
                                    const QString &SMTPPassword, const QString &from, const QString &to,
                                    const QString &subject, const QString &body, const QFile *pfileAttach)
{
    qDebug() << "Sending Email:" << SMTPServer;
    qDebug() << "  Port:" << QString::number(SMTPPort);
    qDebug() << "  Type:" << QString::number(SMTPType);
    qDebug() << "  User:" << SMTPUsername;
    qDebug() << "  PW  :" << SMTPPassword;
    qDebug() << "  from:" << from;
    qDebug() << "  to  :" << to;
    qDebug() << "  subject:" << subject;
    qDebug() << "  body  :"  << body;
    if(pfileAttach)
    {
        qDebug() << "filename:" << pfileAttach->fileName();
    }

    SmtpClient::ConnectionType nConnType  = SmtpClient::TcpConnection;
    if ( SMTPType == SmtpClient::TcpConnection || 
         SMTPType == SmtpClient::SslConnection || 
         SmtpClient::TlsConnection)
    {
        nConnType = (SmtpClient::ConnectionType)SMTPType;
    }

    qDebug() << "SmtpClient smtp";
    SmtpClient smtp(SMTPServer, SMTPPort, nConnType);

    try 
    {
        // We need to set the username (your email address) and the password
        // for smtp authentification.

        qDebug() << "SmtpClient setUser";
        smtp.setUser(SMTPUsername);
        qDebug() << "SmtpClient setPassword";
        smtp.setPassword(SMTPPassword);

        // Now we create a MimeMessage object. This will be the email.

        MimeMessage message;

        qDebug() << "SmtpClient message.setSender";

        message.setSender(new EmailAddress(from));
        qDebug() << "SmtpClient message.addRecipient";
        message.addRecipient(new EmailAddress(to));
        message.addRecipient(new EmailAddress("kcb@keycodebox.com"), MimeMessage::RecipientType::Bcc);
        qDebug() << "SmtpClient message.setSubject";
        message.setSubject(subject);

        // Now add some text to the email.
        // First we create a MimeText object.

        MimeText text;

        qDebug() << "SmtpClient text.setText";

        text.setText(body);

        // Now add it to the mail

        message.addPart(&text);        

        if(pfileAttach)
        {
            message.addPart(new MimeAttachment((QFile*)pfileAttach));
        }
        // Now we can send the mail

        qDebug() << "<<<<<<<<<< Email: connectToHost";
        if( !smtp.connectToHost() )
        {
            qDebug() << "<<<<<<<<<<<< Email:: failed to connect to host";
        } 
        else 
        {
            qDebug() << "<<<<<<<<<< Email: login";
            smtp.login();

            qDebug() << "<<<<<<<<<< Sending Email";
            smtp.sendMail(message);

            qDebug() << "<<<<<<<<<< Quit Email";
            smtp.quit();
        }
    } 
    catch (std::exception &e) 
    {
        qDebug() << "SmtpClient::SendMessageTimeoutException";
    }
}

void CReportController::OnStop()
{
    _reportCheckTimer.stop();
}

void CReportController::OnStart()
{
    _reportCheckTimer.start();
}
