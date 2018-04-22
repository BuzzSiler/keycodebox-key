#include "tbladmin.h"
#include <QtSql>
#include <QtDebug>
#include <sqlite3.h>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include "encryption.h"
#include <exception>
#include "kcbcommon.h"

const char *fid = "ids";
const char *fname = "admin_name";
const char *femail = "admin_email";
const char *fphone = "admin_phone";
const char *freportActive = "email_report_active";
const char *ffreq = "default_report_freq";
const char *fstart = "default_report_start";
const char *fpassword = "password";
const char *faccesscd = "access_code";
const char *fassistpassword = "assist_password";
const char *fassistcode = "assist_code";
const char *fshowFingerprint = "show_fingerprint";
const char *fshowPassword = "show_password";
const char *fpredictivecode = "use_predictive_access_code";
const char *fpredkey = "predictive_key";
const char *fpredres = "predictive_resolution";
const char *fmaxlocks = "max_locks";

const char *fsmtpserver = "smtp_server";
const char *fsmtpport = "smtp_port";
const char *fsmtptype = "smtp_type";
const char *fsmtpusername = "smtp_username";
const char *fsmtppassword = "smtp_password";

// remote desktop
const char *fvncport = "vnc_port";
const char *fvncpassword = "vnc_password";

const char *freportviaemail = "report_via_email";
const char *freporttofile = "report_to_file";
const char *freportdirectory = "report_directory";
const char *fdisplaypowerdown = "display_power_down_timeout";

CAdminRec::CAdminRec() : 
    QObject(nullptr),
    default_report_freq(*new QDateTime(QDate(), QTime(24, 0, 0))),
    default_report_start(*new QDateTime(QDate(2016, 6, 1), QTime(00, 00)))
{
    ids = -1;
    admin_name = "";
    admin_email = "";
    admin_phone = "";

    //default_report_freq = QDateTime(QDate(), QTime(24, 0, 0)); // TIME, Daily
    //default_report_start = QDateTime(QDate(2016, 6, 1), QTime(00, 00));    // DATETIME,
    password = "";
    access_code_old = "";
    access_code = "";
    assist_code = "";
    assist_password = "";
    use_predictive_access_code = false;
    pred_key = "KEYCODEBOXTEST";
    pred_resolution = 10;
    max_locks = 16;

    show_fingerprint = false;
    show_password = false;
    smtp_server = "";
    smtp_port = 0;
    smtp_type = 0; // 0=TCP, 1=SSL, 2=TSL
    smtp_username = "";
    smtp_password = "";
    vnc_server = "";
    vnc_port = 0;
    vnc_type = 0; // 0=TCP, 1=SSL, 2=TSL
    vnc_username = "";
    vnc_password = "";

    report_via_email = false;
    report_save_to_file = false;
    report_directory = "";

    display_power_down_timeout = 0;
}

CAdminRec &CAdminRec::operator=(CAdminRec &newRec) 
{
    ids = newRec.ids;
    admin_name = newRec.admin_name;
    admin_email = newRec.admin_email;
    admin_phone = newRec.admin_phone;
    email_report_active = newRec.email_report_active;
    default_report_freq = newRec.default_report_freq;
    default_report_start = newRec.default_report_start;
    password = newRec.password;
    access_code_old = newRec.access_code_old;
    access_code = newRec.access_code;
    assist_code = newRec.assist_code;
    assist_password = newRec.assist_password;
    use_predictive_access_code = newRec.use_predictive_access_code;
    pred_key = newRec.pred_key;
    pred_resolution = newRec.pred_resolution;
    max_locks = newRec.max_locks;

    show_fingerprint = newRec.show_fingerprint;
    show_password = newRec.show_password;
    smtp_server = newRec.smtp_server;
    smtp_port = newRec.smtp_port;
    smtp_type = newRec.smtp_type;
    smtp_username = newRec.smtp_username;
    smtp_password = newRec.smtp_password;

    vnc_port = newRec.vnc_port;
    vnc_password = newRec.vnc_password;

    report_via_email = newRec.report_via_email;
    report_save_to_file = newRec.report_save_to_file;
    report_directory = newRec.report_directory;

    display_power_down_timeout = newRec.display_power_down_timeout;

    return *this;
}


QJsonObject& CAdminRec::jsonRecord(QJsonObject &json)
{
    json.insert(fid, QJsonValue(ids));
    json.insert(fname, QJsonValue(admin_name));
    json.insert(femail, QJsonValue(admin_email)); // text,
    json.insert(fphone, QJsonValue(admin_phone)); // text,
    json.insert(freportActive, QJsonValue(email_report_active));
    json.insert(ffreq, QJsonValue(default_report_freq.toString("yyyy-MM-dd HH:mm:ss")));
    json.insert(fstart, QJsonValue(default_report_start.toString("yyyy-MM-dd HH:mm:ss")));    // DATETIME,
    json.insert(fpassword, QJsonValue(password));    // text,
    json.insert(faccesscd, QJsonValue(access_code)); // text
    json.insert(fassistpassword, QJsonValue(assist_password)); //text
    json.insert(fassistcode, QJsonValue(assist_code)); //text
    json.insert(fshowFingerprint, QJsonValue(show_fingerprint));
    json.insert(fshowPassword, QJsonValue(show_password));
    json.insert(fpredictivecode, QJsonValue(use_predictive_access_code));
    json.insert(fpredkey, QJsonValue(pred_key));
    json.insert(fpredres, QJsonValue(pred_resolution));
    json.insert(fmaxlocks, QJsonValue((int)max_locks));

    // remote desktop
    json.insert(fvncport, QJsonValue(vnc_port));
    json.insert(fvncpassword, QJsonValue(vnc_password));

    json.insert(fsmtpserver, QJsonValue(smtp_server));
    json.insert(fsmtpport, QJsonValue(smtp_port));
    json.insert(fsmtptype, QJsonValue(smtp_type));
    json.insert(fsmtpusername, QJsonValue(smtp_username));
    json.insert(fsmtppassword, QJsonValue(smtp_password));

    json.insert(freportviaemail, QJsonValue(report_via_email));
    json.insert(freporttofile, QJsonValue(report_save_to_file));
    json.insert(freportdirectory, QJsonValue(report_directory));

    return json;
}

// QString CAdminRec::jsonRecordAsString()
// {
//     QJsonObject jsonObj;
//     jsonObj = this->jsonRecord(jsonObj);
//     QJsonDocument doc(jsonObj);
//     QString str(doc.toJson(QJsonDocument::Compact));
//     return str;
// }

bool CAdminRec::setFromJsonObject(QJsonObject jsonObj)
{
    try {
    //    ids = jsonObj.value(fid).toInt();
        if(!jsonObj.value(fname).isUndefined())
        {
            admin_name = jsonObj.value(fname).toString();
        }
        if(!jsonObj.value(femail).isUndefined())
        {
            admin_email = jsonObj.value(femail).toString();
        }
        if(!jsonObj.value(fphone).isUndefined())
        {
            admin_phone = jsonObj.value(fphone).toString();
        }
        if(!jsonObj.value(freportActive).isUndefined())
        {
            email_report_active = jsonObj.value(freportActive).toBool();
        }
        if(!jsonObj.value(ffreq).isUndefined())
        {            
            default_report_freq.fromString(jsonObj.value(ffreq).toString(), "yyyy-MM-dd HH:mm:ss");
        }        
        if(!jsonObj.value(fstart).isUndefined())
        {            
            default_report_start.fromString(jsonObj.value(fstart).toString(), "yyyy-MM-dd HH:mm:ss");
        }        
        if(!jsonObj.value(fpassword).isUndefined())
        {            
            password = jsonObj.value(fpassword).toString();
        }        
        if(!jsonObj.value(faccesscd).isUndefined())
        {            
            access_code = jsonObj.value(faccesscd).toString();
        }
        if(!jsonObj.value(fassistpassword).isUndefined())
        {            
            assist_password = jsonObj.value(fassistpassword).toString();
        }        
        if(!jsonObj.value(fassistcode).isUndefined())
        {            
            assist_code = jsonObj.value(fassistcode).toString();
        }        
        if(!jsonObj.value(fshowFingerprint).isUndefined())
        {            
            show_fingerprint = jsonObj.value(fshowFingerprint).toBool();
        }        
        if(!jsonObj.value(fshowPassword).isUndefined())
        {            
            show_password = jsonObj.value(fshowPassword).toBool();
        }        
        if(!jsonObj.value(fpredictivecode).isUndefined())
        {            
            use_predictive_access_code = jsonObj.value(fpredictivecode).toBool();
        }        
        if(!jsonObj.value(fpredkey).isUndefined())
        {            
            pred_key = jsonObj.value(fpredkey).toString();
        }       
        if(!jsonObj.value(fpredres).isUndefined())
        {            
            pred_resolution = jsonObj.value(fpredres).toInt();
        }        
        if(!jsonObj.value(fmaxlocks).isUndefined())
        {            
            max_locks = jsonObj.value(fmaxlocks).toInt();
        }        
        if(!jsonObj.value(fsmtpserver).isUndefined())
        {            
            smtp_server = jsonObj.value(fsmtpserver).toString();
        }        
        if(!jsonObj.value(fsmtpport).isUndefined())
        {            
            smtp_port = jsonObj.value(fsmtpport).toInt();
        }        
        if(!jsonObj.value(fsmtpusername).isUndefined())
        {            
            smtp_username = jsonObj.value(fsmtpusername).toString();
        }        
        if(!jsonObj.value(fsmtppassword).isUndefined())
        {            
            smtp_password = jsonObj.value(fsmtppassword).toString();
        }        
        if(!jsonObj.value(fvncport).isUndefined())
        {            
            vnc_port = jsonObj.value(fvncport).toInt();
        }        
        if(!jsonObj.value(fvncpassword).isUndefined())
        {        
            vnc_password = jsonObj.value(fvncpassword).toString();
        }        
        if(!jsonObj.value(freportviaemail).isUndefined())
        {
            report_via_email = jsonObj.value(freportviaemail).toBool();
        }        
        if(!jsonObj.value(freporttofile).isUndefined())
        {            
            report_save_to_file = jsonObj.value(freporttofile).toBool();
        }        
        if(!jsonObj.value(freportdirectory).isUndefined())
        {
            report_directory = jsonObj.value(freportdirectory).toString();
        }
    } catch(std::exception &e)
    {
        qDebug() << "CAdminRec::setFromJsonObject()" << e.what();
    }

    return true;
}

/**
 * @brief CAdminRec::setFromJsonString
 * @param strJson - must be an object "{}"
 * @return
 */
bool CAdminRec::setFromJsonString(QString strJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8());
    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            setFromJsonObject(doc.object());
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
            return false;
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << strJson << endl;
        return false;
    }

    return true;
}

CTblAdmin::CTblAdmin(QSqlDatabase *db)
{
    KCB_DEBUG_ENTRY;
    _pDB = db;
    initialize();
    KCB_DEBUG_EXIT;
}

QString CTblAdmin::isAccessCode(QString code)
{
    QString access_code = _currentAdmin.getAccessCode();  // already unencrypted
    QString assist_code = _currentAdmin.getAssistCode();  // already unencrypted
    QString access_code_old = _currentAdmin.getAccessCodeOld();

    if (code == access_code || code == access_code_old)
    {
        return "Admin";
    }
    if (code == assist_code)
    {
        return "Assist";
    }
    else
    {
        return "User";
    }
}

bool CTblAdmin::isPassword(QString password, QString type)
{
    // unencrypt
    QString adminPassword   = _currentAdmin.getPassword();          // Already unencrypted in currentAdmin
    QString assistPassword  = _currentAdmin.getAssistPassword();    // Already unencrypted in currentAdmin
    QString enteredPassword = CEncryption::decryptString(password);

    if(type == QStringLiteral("Assist") && (assistPassword == enteredPassword))
    {
        return true;
    }

    if(type == QStringLiteral("Admin") && (adminPassword == enteredPassword))
    {
        return true;
    }

    return false;
}

bool CTblAdmin::createTable()
{
    if( _pDB && _pDB->isOpen() ) {
        QSqlQuery qry(*_pDB);
        QString sql = "CREATE TABLE IF NOT EXISTS " + TABLENAME +
                "(ids integer primary key unique, admin_name text,"
                " admin_email text, admin_phone text, email_report_active bool,"
                " default_report_freq DATETIME,"
                " default_report_start DATETIME, password text, "
                " access_code text,"
                " assist_password text, assist_code text, show_fingerprint bool,"
                " use_predictive_access_code bool, "
                " predictive_key text, predictive_resolution integer,"
                " max_locks,"
                " smtp_server, smtp_port, smtp_type, smtp_username, smtp_password,"
                " vnc_port, vnc_password,"
                " report_via_email, report_to_file, report_directory, display_power_down_timeout)";

        qry.prepare( sql );

        if( !qry.exec() ) {
            qDebug() << qry.lastError();
            return false;
        }
        else {
            qDebug() << "Table created!";
            return true;
        }
    }
    return false;
}

/**
 * @brief CTblAdmin::currentTimeFormat
 * @param format
 * @param strBuffer
 * @param nExpectedLength = expected length of the return string
 */
void CTblAdmin::currentTimeFormat(QString format, QString strBuffer, int nExpectedLength)
{
    time_t rawtime;
    struct tm *currentTime;
    time ( &rawtime );
    currentTime = gmtime( &rawtime );
    char buffer [nExpectedLength+1];

    strftime(buffer, nExpectedLength, format.toStdString().c_str(), currentTime);

    strBuffer = QString::fromStdString(buffer);
}

bool CTblAdmin::createAdminDefault()
{
    qDebug() << "CTblAdmin::createAdminDefault()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("INSERT OR IGNORE INTO ") + TABLENAME +
                    QString("(admin_name,"\
                            "admin_email, admin_phone, email_report_active, default_report_freq, "\
                            "default_report_start, password, access_code, "
                            "assist_password, assist_code, show_fingerprint, show_password,"
                            "use_predictive_access_code, "
                            "predictive_key, predictive_resolution, max_locks, "
                            "smtp_server, smtp_port, smtp_type, smtp_username, smtp_password, "
                            "report_via_email, report_to_file, report_directory, "
                            "vnc_port, vnc_password, display_power_down_timeout)"
                  " VALUES ('admin', 'admin@email.com', '555.555.5555', 1, :freq, "\
                            ":start, :pw, :code, :assistpw, :assistCode, :showFingerprint, "
                            ":use_pred, :pred_key, :pred_res, 32, "
                            "'', 0, 0, '', '', "
                            "1, 0, '', "
                            "5900, 'vnc_password', 0)");
    qry.prepare(sql);

    QString sTime;
    sTime = QDateTime(QDate(1,1,1), QTime(12,0)).toString("yyyy-MM-dd HH:mm:ss");
    qDebug() << "createAdminDefault(): freq:" << sTime;
    qry.bindValue(":freq", sTime);

    QString sStart;
    QDateTime   dtTime = QDateTime::currentDateTime();
    sStart = dtTime.toString("yyyy-MM-dd HH:mm:ss");
    qDebug() << "createAdminDefault(): start:" << sStart;
    qry.bindValue(":start", sStart);

    QString encPW = CEncryption::encryptString("");
    QString encCode = CEncryption::encryptString("112233");
    QString encAssistPw = CEncryption::encryptString("");
    QString encAssistCode = CEncryption::encryptString("332211");
    QString encSMTPPW = CEncryption::encryptString("keycodebox");
    QString encVNCPW = CEncryption::encryptString("keycodebox");

    qDebug() << "pw:" << encPW << " code:" << encCode;

    qry.bindValue(":pw", encPW);
    qry.bindValue(":code", encCode);
    qry.bindValue(":assistpw", encAssistPw);
    qry.bindValue(":assistCode", encAssistCode);
    qry.bindValue(":showFingerprint", QVariant(false));
    qry.bindValue(":showPassword", QVariant(false));
    qry.bindValue(":smtp_server", "smtpout.secureserver.net");
    qry.bindValue(":smtp_port", 465);
    qry.bindValue(":smtp_type", 1);
    qry.bindValue(":smtp_username", "kcb@keycodebox.com");
    qry.bindValue(":smtp_password", encSMTPPW);
    qry.bindValue(":vnc_password", encVNCPW);

    qry.bindValue(":use_pred", QVariant(true));
    qry.bindValue(":pred_key", QVariant("\*phzNZ2'od:9g\"J]Yc%1_m6Y51NpHImY8dz3,VVXU|jp7B]HG8@SxuC\od9;_>"));
    qry.bindValue(":pred_res", QVariant(10));

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) {
        qDebug() << "CTblAdmin::createAdminDefault():" << qry.lastError();
        return false;
    }
    else {
        qDebug( "Inserted!" );
        return true;
    }
}

bool CTblAdmin::readAdmin()
{
    qDebug( )<< "CTblAdmin::readAdmin()";

    QSqlQuery query(*_pDB);
    QString sql = "SELECT admin_name, admin_email, admin_phone, email_report_active, default_report_freq," \
                  "default_report_start, password, access_code, "
                  "assist_password, assist_code, show_fingerprint, show_password, "
                  "use_predictive_access_code, "
                  "predictive_key, predictive_resolution, max_locks, "
                  "smtp_server, smtp_port, smtp_type, smtp_username, smtp_password, "
                  "vnc_port,vnc_password, "
                  "report_via_email, report_to_file, report_directory, display_power_down_timeout"
                  " FROM ";
    sql += TABLENAME;

    if( query.exec(sql)) 
    {
        int fldAdmin_name= query.record().indexOf(fname);
        int fldAdmin_email = query.record().indexOf(femail);
        int fldAdmin_phone = query.record().indexOf(fphone);
        int fldEmail_report_active = query.record().indexOf(freportActive);
        int fldReport_freq = query.record().indexOf(ffreq);
        int fldReport_start = query.record().indexOf(fstart);
        int fldPassword = query.record().indexOf(fpassword);
        int fldCode = query.record().indexOf(faccesscd);
        int fldAssistPassword = query.record().indexOf(fassistpassword);
        int fldAssistCode = query.record().indexOf(fassistcode);
        int fldShowFingerprint = query.record().indexOf(fshowFingerprint);
        int fldShowPassword = query.record().indexOf(fshowPassword);
        int fldPredictive = query.record().indexOf(fpredictivecode);
        int fldPredKey = query.record().indexOf(fpredkey);
        int fldPredRes = query.record().indexOf(fpredres);
        int fldMaxLocks = query.record().indexOf(fmaxlocks);
        int fldSMTPServer = query.record().indexOf(fsmtpserver);
        int fldSMTPPort = query.record().indexOf(fsmtpport);
        int fldSMTPType = query.record().indexOf(fsmtptype);
        int fldSMTPUsername = query.record().indexOf(fsmtpusername);
        int fldSMTPPassword = query.record().indexOf(fsmtppassword);
        int fldVNCPort = query.record().indexOf(fvncport);
        int fldVNCPassword = query.record().indexOf(fvncpassword);
        int fldReportToEmail = query.record().indexOf(freportviaemail);
        int fldReportToFile = query.record().indexOf(freporttofile);
        int fldReportDirectory = query.record().indexOf(freportdirectory);
        int fldDisplayPowerDownTimeout = query.record().indexOf(fdisplaypowerdown);

        qDebug() << "fldDisplayPowerDownTimeout" << fldDisplayPowerDownTimeout << "fdisplaypowerdown" << fdisplaypowerdown;

        qDebug() << "fdls: Predictive:" << QString::number(fldPredictive) << " PredKey:" << QString::number(fldPredKey) << " PredRes:" << QString::number(fldPredRes);

        if (query.next())
        {
            // it exists
            _currentAdmin.setAdminName(query.value(fldAdmin_name).toString());
            _currentAdmin.setAdminEmail(query.value(fldAdmin_email).toString());
            _currentAdmin.setAdminPhone(query.value(fldAdmin_phone).toString());
            _currentAdmin.setEmailReportActive(query.value(fldEmail_report_active).toBool());
            _currentAdmin.setDefaultReportFreq( QDateTime::fromString(query.value(fldReport_freq).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss") ); // t is now your desired time_t
            _currentAdmin.setDefaultReportStart( QDateTime::fromString(query.value(fldReport_start).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss") );

            QString accessCode = query.value(fldCode).toString();
            std::string accessCodeOld = accessCode.toStdString();
            qDebug() << "CTblAdmin::readAdmin():code before encryptDecrypt():" << accessCode;

            accessCodeOld = CEncryption::encryptDecryptOld(accessCodeOld.size(), accessCodeOld);  // Old version

            accessCode = CEncryption::decryptString(accessCode);

            QString password = query.value(fldPassword).toString();
            password = CEncryption::decryptString(password);

            QString assistCode = query.value(fldAssistCode).toString();
            assistCode = CEncryption::decryptString(assistCode);

            QString assistPassword = query.value(fldAssistPassword).toString();
            assistPassword = CEncryption::decryptString(assistPassword);

            // Unencrypted in currentAdmin
            _currentAdmin.setPassword(password);
            _currentAdmin.setAccessCode(accessCode);
            _currentAdmin.setAssistPassword(assistPassword);
            _currentAdmin.setAssistCode(assistCode);
            _currentAdmin.setAccessCodeOld(QString::fromStdString(accessCodeOld));

            _currentAdmin.setDisplayFingerprintButton(query.value(fldShowFingerprint).toBool());
            _currentAdmin.setDisplayShowHideButton(query.value(fldShowPassword).toBool());
            _currentAdmin.setUsePredictiveAccessCode(query.value(fldPredictive).toBool());
            _currentAdmin.setPredictiveKey(query.value(fldPredKey).toString());
            _currentAdmin.setPredictiveResolution(query.value(fldPredRes).toInt());
            _currentAdmin.setMaxLocks(query.value(fldMaxLocks).toInt());
            _currentAdmin.setSMTPServer(query.value(fldSMTPServer).toString());
            _currentAdmin.setSMTPPort(query.value(fldSMTPPort).toInt());
            _currentAdmin.setSMTPType(query.value(fldSMTPType).toInt());
            _currentAdmin.setSMTPUsername(query.value(fldSMTPUsername).toString());

            QString smtppw = query.value(fldSMTPPassword).toString();
            smtppw = CEncryption::decryptString(smtppw);
            _currentAdmin.setSMTPPassword(smtppw);

            _currentAdmin.setVNCPort(query.value(fldVNCPort).toInt());
            QString vncpw = query.value(fldVNCPassword).toString();
            vncpw = CEncryption::decryptString(vncpw);
            _currentAdmin.setVNCPassword(vncpw);

            _currentAdmin.setReportViaEmail(query.value(fldReportToEmail).toBool());
            _currentAdmin.setReportToFile(query.value(fldReportToFile).toBool());
            _currentAdmin.setReportDirectory(query.value(fldReportDirectory).toString());

            _currentAdmin.setDisplayPowerDownTimeout(query.value(fldDisplayPowerDownTimeout).toInt());

            qDebug() << "CTblAdmin::readAdmin(): vCode:" << query.value(fldCode).toString() << " len:" << query.value(fldCode).toString().size();
            qDebug() << "CTblAdmin::readAdmin(): :access_code:" <<_currentAdmin.getAccessCode() << " len:" << _currentAdmin.getAccessCode().size();
            qDebug() << "CTblAdmin::readAdmin(): Use Pred:" << QVariant(_currentAdmin.getUsePredictiveAccessCode()).toString();
            qDebug() << "    Pred Key:" << _currentAdmin.getPredictiveKey();
            qDebug() << "    Pred Res:" << QString::number(_currentAdmin.getPredictiveResolution());
            qDebug() << "SMTP:" << _currentAdmin.getSMTPServer() << ":" << QVariant(_currentAdmin.getSMTPPort()).toString();
            qDebug() << "DisplayPowerDownTimeout:" << _currentAdmin.getDisplayPowerDownTimeout();

            return true;
        }
    }
    return false;
}

void CTblAdmin::addAdmin()
{

}

bool CTblAdmin::updateAdminClear(QString name, QString email, QString phone,
                bool emailReportActive, QDateTime repFreq, QDateTime startReport,
                QString passwordClear, QString accessCdClear,
                QString assistPasswordClear, QString assistCodeClear,
                bool showFingerprint, bool showPassword,
                bool usePredictive, QString predKey, int predRes,
                uint32_t nMaxLocks,
                QString smtpserver, int smtpport, int smtptype,
                QString smtpusername, QString smtppassword,
                int vncport, QString vncpassword,
                bool bReportToEmail, bool bReportToFile, QString reportDirectory,
                int displayPowerDownTimeout)
{
    QString encPW = CEncryption::encryptString(passwordClear);
    QString encCode = CEncryption::encryptString(accessCdClear);
    QString encAssistPw = CEncryption::encryptString(assistPasswordClear);
    QString encAssistCode = CEncryption::encryptString(assistCodeClear);
    QString encSMTPPW = CEncryption::encryptString(smtppassword);
    qDebug() << "pw:" << encPW << " code:" << encCode;

    QString encVNCPW = CEncryption::encryptString(vncpassword);

    return updateAdmin(name, email, phone, emailReportActive, repFreq, startReport, encPW, encCode,
                       encAssistPw, encAssistCode, showFingerprint, showPassword, usePredictive, predKey, predRes,
                       nMaxLocks, smtpserver, smtpport, smtptype, smtpusername, encSMTPPW, vncport,
                       encVNCPW, bReportToEmail, bReportToFile, reportDirectory, displayPowerDownTimeout);
}

bool CTblAdmin::updateAdmin(QString name, QString email, QString phone,
                bool emailReportActive, QDateTime repFreq, QDateTime startReport,
                QString passwordEnc, QString accessCdEnc,
                QString assistPasswordEnc, QString assistCodeEnc,
                bool showFingerprint, bool showPassword,
                bool usePredictive, QString predKey, int predRes,
                uint32_t nMaxLocks,
                QString smtpserver, int smtpport, int smtptype,
                QString smtpusername, QString smtppassword,
                int vncport, QString vncpassword,
                bool bReportToEmail, bool bReportToFile, QString reportDirectory,
                int displayPowerDownTimeout)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE("power down timeout" << displayPowerDownTimeout);

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
                  " SET " + QString("admin_name=:name,"\
                  "admin_email=:email, admin_phone=:phone, email_report_active=:active, "
                  "default_report_freq=:freq, "
                  "default_report_start=:start, password=:pw, access_code=:code, "
                  "assist_password=:assistpw, assist_code=:assistcode, show_fingerprint=:showFingerprint, show_password=:showPassword, "
                  "use_predictive_access_code=:usePred, predictive_key=:pKey, predictive_resolution=:pRes, "
                  "max_locks=:maxLocks, "
                  "smtp_server=:smtpserver, smtp_port=:smtpport, smtp_type=:smtptype, "
                  "smtp_username=:smtpusername, smtp_password=:smtppassword, "
                  "vnc_port=:vncport, vnc_password=:vncpassword, "
                  "report_via_email=:reportViaEmail, report_to_file=:reportToFile, report_directory=:reportDir, "
                  "display_power_down_timeout=:displayPowerDownTimeout"
                  " WHERE 1;");
    qry.prepare(sql);

    qry.bindValue(":name", name);
    qry.bindValue(":email", email);
    qry.bindValue(":phone", phone);
    qry.bindValue(":active", emailReportActive);
    QString sTime = repFreq.toString("yyyy-MM-dd HH:mm:ss");
    qry.bindValue(":freq", sTime);

    QString sStart  = startReport.toString("yyyy-MM-dd HH:mm:ss");
    qry.bindValue(":start", sStart);

    qry.bindValue(":pw", passwordEnc);
    qry.bindValue(":code", accessCdEnc);
    qry.bindValue(":assistpw", assistPasswordEnc);
    qry.bindValue(":assistcode", assistCodeEnc);
    qry.bindValue(":showFingerprint", showFingerprint);
    qry.bindValue(":showPassword", showPassword);
    qry.bindValue(":usePred", usePredictive);
    qry.bindValue(":pKey", predKey);
    qry.bindValue(":pRes", predRes);
    qry.bindValue(":maxLocks", nMaxLocks);

    qry.bindValue(":smtpserver", smtpserver);
    qry.bindValue(":smtpport", smtpport);
    qry.bindValue(":smtptype", smtptype);
    qry.bindValue(":smtpusername", smtpusername);
    qry.bindValue(":smtppassword", smtppassword);

    qry.bindValue(":vncport", vncport);
    qry.bindValue(":vncpassword", vncpassword);

    qry.bindValue(":reportViaEmail", bReportToEmail);
    qry.bindValue(":reportToFile", bReportToFile);
    qry.bindValue(":reportDir", reportDirectory);

    qry.bindValue(":displayPowerDownTimeout", displayPowerDownTimeout);

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) {
        qDebug() << "CTblAdmin::updateAdmin():" << qry.lastError();
        return false;
    }
    else {
        _pDB->commit();
        qDebug() << "Updated Admin! SMTP Server:" << smtpserver << ":" << QVariant(smtpport).toString();
        readAdmin();
        return true;
    }

}

/**
 * @brief CTblAdmin::updateAdmin
 * @param rec
 * @return
 */
bool CTblAdmin::updateAdmin(CAdminRec &rec)
{
    return updateAdmin(rec.getAdminName(), rec.getAdminEmail(), rec.getAdminPhone(),
                       rec.getEmailReportActive(), rec.getDefaultReportFreq(), rec.getDefaultReportStart(),
                       rec.getPassword(), rec.getAccessCode(),
                       rec.getAssistPassword(), rec.getAssistCode(), rec.getDisplayFingerprintButton(), rec.getDisplayShowHideButton(),
                       rec.getUsePredictiveAccessCode(),
                       rec.getPredictiveKey(), rec.getPredictiveResolution(),
                       rec.getMaxLocks(),
                       rec.getSMTPServer(), rec.getSMTPPort(), rec.getSMTPType(),
                       rec.getSMTPUsername(), rec.getSMTPPassword(),
                       rec.getVNCPort(), rec.getVNCPassword(),
                       rec.getReportViaEmail(), rec.getReportToFile(), rec.getReportDirectory(),
                       rec.getDisplayPowerDownTimeout());
}

/**
 * @brief CTblAdmin::updateAdmin - update the admin record with the new values from JSON Object
 * @param adminObj - json object with given values
 * @return true on success, false otherwise
 */
bool CTblAdmin::updateAdmin(QJsonObject adminObj)
{
    QString name, email, phone;
    bool active;
    QDateTime freq;
    QDateTime start;
    QString pw, accessCd, assistpw, assistCode;
    bool showFingerprint, showPassword, usePredictive;
    QString predKey;
    int predResolution;
    uint32_t unMaxLocks;

    QString smtpserver, smtpusername, smtppassword, vncpassword;
    int smtpport, smtptype, vncport;
    bool bReportToEmail, bReportToFile;
    QString reportDir;
    int displayPowerDown;

    if( adminObj.value(fname).isUndefined() ||
            adminObj.value(femail).isUndefined() ||
            adminObj.value(fphone).isUndefined() ||
            adminObj.value(freportActive).isUndefined() ||
            adminObj.value(ffreq).isUndefined() ||
            adminObj.value(fstart).isUndefined() ||
            adminObj.value(fpassword).isUndefined() ||
            adminObj.value(faccesscd).isUndefined() ||
            adminObj.value(fassistpassword).isUndefined() ||
            adminObj.value(fassistcode).isUndefined() ||
            adminObj.value(fshowFingerprint).isUndefined() ||
            adminObj.value(fshowPassword).isUndefined() ||
            adminObj.value(fpredictivecode).isUndefined() ||
            adminObj.value(fpredkey).isUndefined() ||
            adminObj.value(fpredres).isUndefined() ||
            adminObj.value(fmaxlocks).isUndefined() ||
            adminObj.value(fsmtpserver).isUndefined() ||
            adminObj.value(fsmtpport).isUndefined() ||
            adminObj.value(fsmtptype).isUndefined() ||
            adminObj.value(fsmtpusername).isUndefined() ||
            adminObj.value(fsmtppassword).isUndefined() ||
            adminObj.value(fvncport).isUndefined() ||
            adminObj.value(fvncpassword).isUndefined() ||
            adminObj.value(freportviaemail).isUndefined() ||
            adminObj.value(freporttofile).isUndefined() ||
            adminObj.value(freportdirectory).isUndefined() ||
            adminObj.value(fdisplaypowerdown).isUndefined()
            )
    {
        qDebug() << "Failed to update admin";
        return false;
    }

    name = adminObj.value(fname).toString();
    email = adminObj.value(femail).toString();
    phone = adminObj.value(fphone).toString();
    active = adminObj.value(freportActive).toBool();
    freq.fromString(adminObj.value(ffreq).toString(), "yyyy-MM-dd HH:mm:ss");
    start.fromString(adminObj.value(fstart).toString(), "yyyy-MM-dd HH:mm:ss");
    pw = adminObj.value(fpassword).toString();
    accessCd = adminObj.value(faccesscd).toString();
    assistpw = adminObj.value(fassistpassword).toString();
    assistCode = adminObj.value(fassistcode).toString();
    showFingerprint = adminObj.value(fshowFingerprint).toBool();
    showPassword = adminObj.value(fshowPassword).toBool();
    usePredictive = adminObj.value(fpredictivecode).toBool();
    predKey = adminObj.value(fpredkey).toString();
    predResolution = adminObj.value(fpredres).toInt();
    unMaxLocks = adminObj.value(fmaxlocks).toInt();

    if( adminObj.value(fsmtpserver).isUndefined()) {
        smtpserver = "";
    } else {
        smtpserver = adminObj.value(fsmtpserver).toString();
    }
    if( adminObj.value(fsmtpport).isUndefined()) {
        smtpport = 0;
    } else {
        smtpport = adminObj.value(fsmtpport).toInt();
    }
    if( adminObj.value(fsmtptype).isUndefined()) {
        smtptype = 0;
    } else {
        smtptype = adminObj.value(fsmtptype).toInt();
    }
    if( adminObj.value(fsmtpusername).isUndefined()) {
        smtpusername = "";
    } else {
        smtpusername = adminObj.value(fsmtpusername).toString();
    }
    const char *fsmtppassword = "smtp_password";
    if( adminObj.value(fsmtppassword).isUndefined()) {
        smtppassword = "";
    } else {
        smtppassword = adminObj.value(fsmtppassword).toString();
    }

    if( adminObj.value(fvncport).isUndefined()) {
      vncport = 5900;
    } else {
      vncport = adminObj.value(fvncport).toInt();
    }
    const char *fvncpassword = "vnc_password";
    if( adminObj.value(fvncpassword).isUndefined()) {
        vncpassword = "TEST";
    } else {
        vncpassword = adminObj.value(fvncpassword).toString();
    }

    bReportToEmail = adminObj.value(freportviaemail).toBool();
    bReportToFile= adminObj.value(freporttofile).toBool();
    reportDir = adminObj.value(freportdirectory).toString();

    displayPowerDown = adminObj.value(fdisplaypowerdown).toInt();

    return updateAdminClear(name, email, phone, active, freq, start, pw, accessCd, assistpw, assistCode, showFingerprint,
                            showPassword, usePredictive, predKey, predResolution, unMaxLocks, smtpserver, smtpport, smtptype,
                            smtpusername, smtppassword, vncport, vncpassword, bReportToEmail, bReportToFile, reportDir, displayPowerDown);
}

bool CTblAdmin::tableExists()
{
    QStringList lstTables = _pDB->tables();
    QStringList::iterator  itor;

    for(itor = lstTables.begin(); itor != lstTables.end(); itor++)
    {
        if((*itor)==TABLENAME.toStdString().c_str()) {
            //
            return true;
        }
    }
    return false;
}

void CTblAdmin::initialize()
{
    if(!tableExists())
    {
        createTable();
    }
    if(tableExists())
    {
        if( !readAdmin() ) {
            if( createAdminDefault() ) {
                readAdmin();
            }
        }
    }
}
