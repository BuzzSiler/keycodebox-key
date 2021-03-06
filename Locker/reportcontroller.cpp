#include "reportcontroller.h"
#include <exception>
#include <QDir>
#include <QTextStream>
#include "encryption.h"
#include "unistd.h"


CReportController::CReportController(QObject *parent) : QObject(parent)
{
    _padminInfo = 0;
    initializeConnections();
}

void CReportController::initializeConnections()
{
}

void CReportController::setCodeHistoryTable(CTblCodeHistory *pTable) {
    _ptblCodeHistory = pTable;
}



void CReportController::start()
{
    qDebug() << "CReportController::start()";
    _reportCheckTimer.setInterval(60000);   // Check every minute

    connect(&_reportCheckTimer, SIGNAL(timeout()), this, SLOT(OnCheckIfReportingTimeEvent()));
}


void CReportController::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    //
    _padminInfo = adminInfo;
    _bCurrentAdminRetrieved = true;

    qDebug() << "CReportController::OnRequestedCurrentAdmin(CAdminRec*) -> emit __OnRequestedCurrentAdmin(CAdminRec*)";
}

QString CReportController::createNewFileName()
{
    QDateTime   now = QDateTime::currentDateTime();
    QString strFileName = "SafePak_" + now.toString("yyyy-MM-dd-HH_mm_ss");
    return strFileName;
}

void CReportController::buildReportFile(CLockHistorySet *pLockHistorySet, CAdminRec *adminInfo, QFile **ppFile)
{
    qDebug() << "CReportController::buildReportFile()";

    // Predictive - so history rec
    CLockHistoryRec *plockHistoryRec;
    QString strFileName = createNewFileName();
    QDir    dir(adminInfo->getReportDirectory().c_str());
    *ppFile = new QFile(dir.absoluteFilePath(strFileName));

    if(!(*ppFile)) {
        qDebug() << "CReportController::buildReportFile() ... no file:" << dir.absoluteFilePath(strFileName);

        return;
    }
    // Check for directory "SafePakReports"
    // Create if it doesn't exist
    if(!dir.exists())
    {
        qDebug() << " Path doesn't exist. Creating entire path";
        dir.mkpath(adminInfo->getReportDirectory().c_str());
    }
    if((*ppFile)->open(QIODevice::ReadWrite))
    {
        qDebug() << "CReportController::buildReportFile() ... file open!:" << dir.absoluteFilePath(strFileName);

        QDateTime dtAccess;
        QString sDesc, body;
        int nLockNum;
	QString question1, question2, question3;
        QTextStream stream( *ppFile );
        stream << "SafePak Access History." << endl;

        for(CLockHistorySet::Iterator itor = pLockHistorySet->begin(); itor != pLockHistorySet->end(); itor++) {
            plockHistoryRec = itor.value();
            //
            body = "";

            dtAccess = plockHistoryRec->getAccessTime();
            sDesc = plockHistoryRec->getDescription().c_str();
            nLockNum = plockHistoryRec->getLockNum();
	    
            body = QString("Lock #") + QVariant(nLockNum).toString();

            if( sDesc.size() > 0 )
            {
                body += " [" + sDesc + "]";
            }
            body += " accessed:" + dtAccess.toString("MM/dd/yyyy HH:mm:ss");
            std::string code_one = plockHistoryRec->getCode1();
            std::string code_two = plockHistoryRec->getCode2();
	    std::string question1 = plockHistoryRec->getQuestion1();
	    std::string question2 = plockHistoryRec->getQuestion2();
	    std::string question3 = plockHistoryRec->getQuestion3();
	    int lockbox_state = plockHistoryRec->getLockboxState();
	    
            body += " code #1:" + QString(code_one.c_str());

	    if(!lockbox_state)
	      body += " state: IN";
	    else
	      body += " state: OUT";
	    
            stream << body << endl;
        }
        stream.flush();
        (*ppFile)->close();
    } else {
        qDebug() << "can't open file: " << (*ppFile)->errorString();
        qDebug() << "CReportController::buildReportFile() ... can't open file!:" << dir.absoluteFilePath(strFileName);
        delete *ppFile;
        *ppFile = NULL;
    }
}

void CReportController::processImmediateReport(QDateTime dtReportStart, QDateTime dtReportEnd, int nLockNum)
{
    requestCurrentAdminRecord();

    //
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
        processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd, nLockNum);
        if(pHistorySet) delete pHistorySet;
        if(pFile) delete pFile;
    }
}

void CReportController::processLockCodeHistoryReport(CLockHistorySet *_pHistorySet, QFile *pFile, QDateTime &dtReportStart, QDateTime &dtReportEnd, int nLockNum)
{
    qDebug() << "CReportController::processLockCodeHistoryReport()";

    _pFile = pFile;
    _dtReportStart = dtReportStart;
    _dtReportEnd = dtReportEnd;

    assembleAndSendCodeRecordsForReportDate(dtReportStart, dtReportEnd);
}


void CReportController::assembleAndSendCodeRecordsForReportDate(QDateTime dtStart, QDateTime dtEnd)
{
    emit __RequestCodeHistoryForDateRange(dtStart, dtEnd);
}

void CReportController::OnCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd, CLockHistorySet *pLockHistorySet)
{
    // Build the file
    qDebug() << "CReportController::OnCodeHistoryForDateRange() >> buildReportFile(...)";
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

        if( _padminInfo->getReportViaEmail() ) {
            qDebug() << "CReportController::OnCodeHistoryForDateRange() >> PrepareToSendEmail(...)";
            PrepareToSendEmail(_padminInfo, _pFile);
        }
        if( _padminInfo->getReportToFile()) {
            qDebug() << "CReportController::OnCodeHistoryForDateRange() >> Save to file";
            // Save it
            // Already saved...
        } else {
            qDebug() << "CReportController::OnCodeHistoryForDateRange() >> DON'T Save to file. Deleting file.";
            // Remove it
            if(_pFile) {
                _pFile->remove();
            }
        }
    }
}


bool CReportController::timetoSendReport(QDateTime date, QDateTime &dtReportStart, QDateTime &dtReportEnd)
{
    if( !_ptblCodeHistory) {
        qDebug() << "CReportController::timetoSendReport(). Requires TblCodeHistory to be set.";
        return false;
    }

    QDateTime dtFreq = _padminInfo->getDefaultReportFreq();

    if(dtFreq == QDateTime(QDate(), QTime(0,0)) ||
            dtFreq == QDateTime(QDate(1,1,1), QTime(0,0)))   // Represents admin wants each access sent
    {   // Never or each event are not sent from this process
        return false;
    }

    QDateTime   tmpDate;
    QFile       *pFile;
    CLockHistorySet *pHistorySet = NULL;
    if(dtFreq == QDateTime(QDate(1,1,1), QTime(1,0)))
    { // Hourly
        tmpDate = _dtLastReportDate.addSecs(3600);  // Add an hour in seconds

        if(_dtLastReportDate.isNull() || (tmpDate >= _dtLastReportDate) )
        {
            dtReportStart = date.addSecs(-3600);
            dtReportEnd = date.currentDateTime();
            int         nLockNum = -1;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd, nLockNum);
        }
    } else if(dtFreq == QDateTime(QDate(1,1,1), QTime(12,0)))
    { // Every 12 hours
        tmpDate = _dtLastReportDate.addSecs(12 * 3600); // 12 hours
        if(_dtLastReportDate.isNull() || (tmpDate >= _dtLastReportDate) )
        {
            dtReportStart = date.addSecs(-3600 * 12);
            dtReportEnd = date.currentDateTime();
            int         nLockNum = -1;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd, nLockNum);
        }
    } else if(dtFreq == QDateTime(QDate(1,1,1), QTime(23,59)))
    {  // Daily
        tmpDate = _dtLastReportDate.addDays(1);
        if(_dtLastReportDate.isNull() || (tmpDate  >= _dtLastReportDate) )
        {
            dtReportStart = date.addDays(-1);
            dtReportEnd = date.currentDateTime();
            int         nLockNum = -1;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd, nLockNum);
        }
    } else if(dtFreq == QDateTime(QDate(1,1,7), QTime(0,0)))
    {   // Weekly
        tmpDate = _dtLastReportDate.addDays(7);
        if(_dtLastReportDate.isNull() || (tmpDate  >= _dtLastReportDate) )
        {
            dtReportStart = date.addDays(-7);
            dtReportEnd = date.currentDateTime();
            int         nLockNum = -1;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd, nLockNum);
        }
    } else if(dtFreq == QDateTime(QDate(1,12,1), QTime(0,0)))
    {   // Monthly
        tmpDate = _dtLastReportDate.addMonths(1);
        if(_dtLastReportDate.isNull() || (tmpDate  >= _dtLastReportDate) )
        {
            dtReportStart = date.addMonths(-1);
            dtReportEnd = date.currentDateTime();
            int         nLockNum = -1;
            // Accumulate and send
            processLockCodeHistoryReport(pHistorySet, pFile, dtReportStart, dtReportEnd, nLockNum);
        }
    }
    if(pHistorySet)
        delete pHistorySet;
    if(pFile) {
        delete pFile;
        return true;
    }
    else {
        return false;
    }
}

void CReportController::requestCurrentAdminRecord()
{
    _bCurrentAdminRetrieved = false;
    emit __OnRequestCurrentAdmin();
}


void CReportController::OnCheckIfReportingTimeEvent()
{
    // Check if we need to send a report.
    QDateTime   dtNow = QDateTime::currentDateTime();
    QDateTime dtReportStart, dtReportEnd;
    if(!timetoSendReport(dtNow, dtReportStart, dtReportEnd)) {
        return;
    } else {
        _dtLastReportDate = QDateTime::currentDateTime();
    }

    assembleAndSendCodeRecordsForReportDate(dtReportStart, dtReportEnd);

}


void CReportController::PrepareToSendEmail(CAdminRec *admin, QFile *pFile)
{
    qDebug() << " Reporting via email";

    qDebug() << "Sending email";

    QString SMTPSvr = admin->getSMTPServer().c_str();
    int SMTPPort = admin->getSMTPPort();
    int SMTPType = admin->getSMTPType();

    QString SMTPUser = admin->getSMTPUsername().c_str();
    QString SMTPPW = admin->getSMTPPassword().c_str();

    QString from = admin->getAdminEmail().c_str();
    QString to = admin->getAdminEmail().c_str();
    QString subject = "Lock Box History Report";

    QString body = "SafePak report :" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    OnSendEmail(SMTPSvr, SMTPPort, SMTPType, SMTPUser, SMTPPW, from, to, subject, body, pFile );
}



void CReportController::OnSendEmail(const QString SMTPServer, const int &SMTPPort, const int &SMTPType,
                                    const QString &SMTPUsername,
                                    const QString &SMTPPassword, const QString &from, const QString &to,
                                    const QString &subject, const QString &body, const QFile *pfileAttach)
{
    qDebug() << "Sending Email:" << SMTPServer;
    qDebug() << "  Port:" << QVariant(SMTPPort).toString();
    qDebug() << "  Type:" << QVariant(SMTPType).toString();
    qDebug() << "  User:" << SMTPUsername;
    qDebug() << "  PW  :" << SMTPPassword;
    qDebug() << "  from:" << from;
    qDebug() << "  to  :" << to;
    qDebug() << "  subject:" << subject;
    qDebug() << "  body  :"  << body;
    if(pfileAttach)
        qDebug() << "filename:" << pfileAttach->fileName();

    SmtpClient::ConnectionType nConnType  = SmtpClient::TcpConnection;
    if(SMTPType == SmtpClient::TcpConnection || SMTPType == SmtpClient::SslConnection
            || SmtpClient::TlsConnection)
    {
        nConnType = (SmtpClient::ConnectionType)SMTPType;
    }

    qDebug() << "SmtpClient smtp";
    SmtpClient smtp(SMTPServer, SMTPPort, nConnType);

    try {
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
        } else {
            qDebug() << "<<<<<<<<<< Email: login";
            smtp.login();

            qDebug() << "<<<<<<<<<< Sending Email";
            smtp.sendMail(message);

            qDebug() << "<<<<<<<<<< Quit Email";
            smtp.quit();
        }
    } catch (std::exception &e) {
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
