#include <ctime>
#include <string>
#include <QtSql>
#include <QtDebug>
#include <sqlite3.h>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <boost/algorithm/string.hpp>
#include "tbladmin.h"
#include "encryption.h"
#include <exception>

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

QJsonObject& CAdminRec::jsonRecord(QJsonObject &json)
{
    json.insert(fid, QJsonValue(ids));
    json.insert(fname, QJsonValue(admin_name.c_str()));
    json.insert(femail, QJsonValue(admin_email.c_str())); // text,
    json.insert(fphone, QJsonValue(admin_phone.c_str())); // text,
    json.insert(freportActive, QJsonValue(email_report_active));
    json.insert(ffreq, QJsonValue(default_report_freq.toString("yyyy-MM-dd HH:mm:ss")));
    json.insert(fstart, QJsonValue(default_report_start.toString("yyyy-MM-dd HH:mm:ss")));    // DATETIME,
    json.insert(fpassword, QJsonValue(password.c_str()));    // text,
    json.insert(faccesscd, QJsonValue(access_code.c_str())); // text
    json.insert(fassistpassword, QJsonValue(assist_password.c_str())); //text
    json.insert(fassistcode, QJsonValue(assist_code.c_str())); //text
    json.insert(fshowFingerprint, QJsonValue(show_fingerprint));
    json.insert(fpredictivecode, QJsonValue(use_predictive_access_code));
    json.insert(fpredkey, QJsonValue(pred_key.c_str()));
    json.insert(fpredres, QJsonValue(pred_resolution));
    json.insert(fmaxlocks, QJsonValue((int)max_locks));

    // remote desktop
    json.insert(fvncport, QJsonValue(vnc_port));
    json.insert(fvncpassword, QJsonValue(vnc_password.c_str()));
    
    json.insert(fsmtpserver, QJsonValue(smtp_server.c_str()));
    json.insert(fsmtpport, QJsonValue(smtp_port));
    json.insert(fsmtptype, QJsonValue(smtp_type));
    json.insert(fsmtpusername, QJsonValue(smtp_username.c_str()));
    json.insert(fsmtppassword, QJsonValue(smtp_password.c_str()));

    json.insert(freportviaemail, QJsonValue(report_via_email));
    json.insert(freporttofile, QJsonValue(report_save_to_file));
    json.insert(freportdirectory, QJsonValue(report_directory.c_str()));

    return json;
}

QString CAdminRec::jsonRecordAsString()
{
    QJsonObject jsonObj;
    jsonObj = this->jsonRecord(jsonObj);
    QJsonDocument doc(jsonObj);
    QString str(doc.toJson(QJsonDocument::Compact));
    return str;
}

bool CAdminRec::setFromJsonObject(QJsonObject jsonObj)
{
    try {
    //    ids = jsonObj.value(fid).toInt();
        if(!jsonObj.value(fname).isUndefined())
            admin_name = jsonObj.value(fname).toString().toStdString();
        if(!jsonObj.value(femail).isUndefined())
            admin_email = jsonObj.value(femail).toString().toStdString();
        if(!jsonObj.value(fphone).isUndefined())
            admin_phone = jsonObj.value(fphone).toString().toStdString();
        if(!jsonObj.value(freportActive).isUndefined())
            email_report_active = jsonObj.value(freportActive).toBool();
        if(!jsonObj.value(ffreq).isUndefined())
            default_report_freq.fromString(jsonObj.value(ffreq).toString(), "yyyy-MM-dd HH:mm:ss");
        if(!jsonObj.value(fstart).isUndefined())
            default_report_start.fromString(jsonObj.value(fstart).toString(), "yyyy-MM-dd HH:mm:ss");
        if(!jsonObj.value(fpassword).isUndefined())
            password = jsonObj.value(fpassword).toString().toStdString();
        if(!jsonObj.value(faccesscd).isUndefined())
            access_code = jsonObj.value(faccesscd).toString().toStdString();

        if(!jsonObj.value(fassistpassword).isUndefined())
            assist_password = jsonObj.value(fassistpassword).toString().toStdString();
        if(!jsonObj.value(fassistcode).isUndefined())
            assist_code = jsonObj.value(fassistcode).toString().toStdString();
        if(!jsonObj.value(fshowFingerprint).isUndefined())
            show_fingerprint = jsonObj.value(fshowFingerprint).toBool();

        if(!jsonObj.value(fpredictivecode).isUndefined())
            use_predictive_access_code = jsonObj.value(fpredictivecode).toBool();
        if(!jsonObj.value(fpredkey).isUndefined())
            pred_key = jsonObj.value(fpredkey).toString().toStdString();
        if(!jsonObj.value(fpredres).isUndefined())
            pred_resolution = jsonObj.value(fpredres).toInt();
        if(!jsonObj.value(fmaxlocks).isUndefined())
            max_locks = jsonObj.value(fmaxlocks).toInt();
        if(!jsonObj.value(fsmtpserver).isUndefined())
            smtp_server = jsonObj.value(fsmtpserver).toString().toStdString();
        if(!jsonObj.value(fsmtpport).isUndefined())
            smtp_port = jsonObj.value(fsmtpport).toInt();
        if(!jsonObj.value(fsmtpusername).isUndefined())
            smtp_username = jsonObj.value(fsmtpusername).toString().toStdString();
        if(!jsonObj.value(fsmtppassword).isUndefined())
            smtp_password = jsonObj.value(fsmtppassword).toString().toStdString();
	if(!jsonObj.value(fvncport).isUndefined())
	    vnc_port = jsonObj.value(fvncport).toInt();
	if(!jsonObj.value(fvncpassword).isUndefined())
	    vnc_password = jsonObj.value(fvncpassword).toString().toStdString();
        if(!jsonObj.value(freportviaemail).isUndefined())
            report_via_email = jsonObj.value(freportviaemail).toBool();
        if(!jsonObj.value(freporttofile).isUndefined())
            report_save_to_file = jsonObj.value(freporttofile).toBool();
        if(!jsonObj.value(freportdirectory).isUndefined())
            report_directory = jsonObj.value(freportdirectory).toString().toStdString();
    } catch(std::exception &e)
    {
        qDebug() << "CAdminRec::setFromJsonObject()" << e.what();
    }
}

/**
 * @brief CAdminRec::setFromJsonString
 * @param strJson - must be an object "{}"
 * @return
 */
bool CAdminRec::setFromJsonString(std::string strJson)
{
    QString     strIn = strJson.c_str();
    QJsonDocument doc = QJsonDocument::fromJson(strIn.toUtf8());
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
        qDebug() << "Invalid JSON...\n" << strIn << endl;
        return false;
    }

    return true;
}

bool CTblAdmin::isAccessCode(QString code)
{
    qDebug() << "CTblAdmin::isAccessCode():" << code.toStdString().c_str() << " =? " << _currentAdmin.getAccessCode().c_str();

    std::string access_code = _currentAdmin.getAccessCode();  // already unencrypted
    std::string access_code_old = _currentAdmin.getAccessCodeOld();
    std::string in_access_code = code.toStdString();  // CEncryption::encryptDecrypt(code.size(), code.toStdString());


    //quit code
    std::string quit_code = "123456789987654321";
    if(boost::iequals(in_access_code, quit_code))
    {
        qDebug() << "CTblAdmin::isAccessCode(): shutting down!";
        QCoreApplication::quit();
    }
    
    if(boost::iequals(in_access_code, access_code) || boost::iequals(in_access_code, access_code_old))
    {
        return true;
    } else {
        return false;
    }
}

bool CTblAdmin::isPassword(QString password)
{
    // unencrypt
    std::string PW = _currentAdmin.getPassword();   // Already unencrypted in currentAdmin
    std::string in_PW = CEncryption::decryptString(password).toStdString();

    if(boost::iequals(PW, in_PW) )
    {
        return true;
    } else {
        return false;
    }
}

bool CTblAdmin::createTable()
{
    if( _pDB && _pDB->isOpen() ) {
        QSqlQuery qry(*_pDB);
        QString sql = "CREATE TABLE IF NOT EXISTS " + QString(TABLENAME.c_str()) +
                "(ids integer primary key unique, admin_name text,"
                " admin_email text, admin_phone text, email_report_active bool,"
                " default_report_freq DATETIME,"
                " default_report_start DATETIME, password text, "
                " access_code text,"
                " assist_password, assist_code, show_fingerprint, "
                " use_predictive_access_code bool, "
                " predictive_key text, predictive_resolution integer,"
                " max_locks,"
                " smtp_server, smtp_port, smtp_type, smtp_username, smtp_password,"
                " vnc_port, vnc_password,"
                " report_via_email, report_to_file, report_directory)";

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
void CTblAdmin::currentTimeFormat(std::string format, std::string strBuffer, int nExpectedLength)
{
    time_t rawtime;
    struct tm *currentTime;
    time ( &rawtime );
    currentTime = gmtime( &rawtime );
    char buffer [nExpectedLength+1];

    strftime(buffer, nExpectedLength, format.c_str(), currentTime);

    strBuffer = buffer;
}


bool CTblAdmin::createAdminDefault()
{
    qDebug() << "CTblAdmin::createAdminDefault()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("INSERT OR IGNORE INTO ") + QString(TABLENAME.c_str()) +
                    QString("(admin_name,"\
                            "admin_email, admin_phone, email_report_active, default_report_freq, "\
                            "default_report_start, password, access_code, "
                            "assist_password, assist_code, show_fingerprint, "
                            "use_predictive_access_code, "
                            "predictive_key, predictive_resolution, max_locks, "
                            "smtp_server, smtp_port, smtp_type, smtp_username, smtp_password, "
                            "report_via_email, report_to_file, report_directory, "
                            "vnc_port, vnc_password)"    
                  " VALUES ('admin', 'admin@email.com', '555.555.5555', 1, :freq, "\
                            ":start, :pw, :code, :assistpw, :assistCode, :showFingerprint, "
                            ":use_pred, :pred_key, :pred_res, 32, "
                            "'', 0, 0, '', '', "
                            "1, 0, '', "
                            "5900, 'vnc_password')");
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

    std::string encPW = CEncryption::encryptString("").toStdString();
    std::string encCode = CEncryption::encryptString("112233").toStdString();
    std::string encAssistPw = CEncryption::encryptString("").toStdString();
    std::string encAssistCode = CEncryption::encryptString("332211").toStdString();

    qDebug() << "pw:" << encPW.c_str() << " code:" << encCode.c_str();

    qry.bindValue(":pw", encPW.c_str());
    qry.bindValue(":code", encCode.c_str());
    qry.bindValue(":assistpw", encAssistPw.c_str());
    qry.bindValue(":assistCode", encAssistCode.c_str());
    qry.bindValue(":showFingerprint", QVariant(false));

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
    QString sql = "SELECT admin_name, admin_email, admin_phone, default_report_freq," \
                  "default_report_start, password, access_code, "
                  "assist_password, assist_code, show_fingerprint, "
                  "use_predictive_access_code, "
                  "predictive_key, predictive_resolution, max_locks, "
                  "smtp_server, smtp_port, smtp_type, smtp_username, smtp_password, "
                  "vnc_port,vnc_password, "
                  "report_via_email, report_to_file, report_directory"
                  " FROM ";
    sql += QString(TABLENAME.c_str());

    if( query.exec(sql)) {
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

        qDebug() << "fdls: Predictive:" << QString::number(fldPredictive) << " PredKey:" << QString::number(fldPredKey) << " PredRes:" << QString::number(fldPredRes);

        if (query.next())
        {
            // it exists
            _currentAdmin.setAdminName(query.value(fldAdmin_name).toString().toStdString());
            _currentAdmin.setAdminEmail(query.value(fldAdmin_email).toString().toStdString());
            _currentAdmin.setAdminPhone(query.value(fldAdmin_phone).toString().toStdString());
            _currentAdmin.setEmailReportActive(query.value(fldEmail_report_active).toBool());
            _currentAdmin.setDefaultReportFreq( QDateTime::fromString(query.value(fldReport_freq).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss") ); // t is now your desired time_t
            _currentAdmin.setDefaultReportStart( QDateTime::fromString(query.value(fldReport_start).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss") );

            std::string accessCode = query.value(fldCode).toString().toStdString();
            std::string accessCodeOld = accessCode;
            qDebug() << "CTblAdmin::readAdmin():code before encryptDecrypt():" << accessCode.c_str();

            accessCodeOld = CEncryption::encryptDecryptOld(accessCodeOld.size(), accessCodeOld);  // Old version

            accessCode = CEncryption::decryptString(accessCode.c_str()).toStdString();

            std::string password = query.value(fldPassword).toString().toStdString();
            password = CEncryption::decryptString(password.c_str()).toStdString();

            std::string assistCode = query.value(fldAssistCode).toString().toStdString();
            assistCode = CEncryption::decryptString(assistCode.c_str()).toStdString();

            std::string assistPassword = query.value(fldAssistPassword).toString().toStdString();
            assistPassword = CEncryption::decryptString(assistPassword.c_str()).toStdString();

            // Unencrypted in currentAdmin
            _currentAdmin.setPassword(password);
            _currentAdmin.setAccessCode(accessCode);
            _currentAdmin.setAssistPassword(assistPassword);
            _currentAdmin.setAssistCode(assistCode);
            _currentAdmin.setAccessCodeOld(accessCodeOld);

            _currentAdmin.setShowFingerprint(query.value(fldShowFingerprint).toBool());
            _currentAdmin.setUsePredictiveAccessCode(query.value(fldPredictive).toBool());
            _currentAdmin.setPredictiveKey(query.value(fldPredKey).toString().toStdString());
            _currentAdmin.setPredictiveResolution(query.value(fldPredRes).toInt());
            _currentAdmin.setMaxLocks(query.value(fldMaxLocks).toInt());

            _currentAdmin.setSMTPServer(query.value(fldSMTPServer).toString().toStdString());
            _currentAdmin.setSMTPPort(query.value(fldSMTPPort).toInt());
            _currentAdmin.setSMTPType(query.value(fldSMTPType).toInt());
            _currentAdmin.setSMTPUsername(query.value(fldSMTPUsername).toString().toStdString());

            std::string smtppw = query.value(fldSMTPPassword).toString().toStdString();
            smtppw = CEncryption::decryptString(smtppw.c_str()).toStdString();
            _currentAdmin.setSMTPPassword(smtppw);

            _currentAdmin.setVNCPort(query.value(fldVNCPort).toInt());
            std::string vncpw = query.value(fldVNCPassword).toString().toStdString();
            vncpw = CEncryption::decryptString(vncpw.c_str()).toStdString();
            _currentAdmin.setVNCPassword(vncpw);
	    
            _currentAdmin.setReportViaEmail(query.value(fldReportToEmail).toBool());
            _currentAdmin.setReportToFile(query.value(fldReportToFile).toBool());
            _currentAdmin.setReportDirectory(query.value(fldReportDirectory).toString().toStdString());

            qDebug() << "CTblAdmin::readAdmin(): vCode:" << query.value(fldCode).toString() << " len:" << query.value(fldCode).toString().size();
            qDebug() << "CTblAdmin::readAdmin(): :access_code:" <<_currentAdmin.getAccessCode().c_str() << " len:" << _currentAdmin.getAccessCode().size();
            qDebug() << "CTblAdmin::readAdmin(): Use Pred:" << QVariant(_currentAdmin.getUsePredictiveAccessCode()).toString();
            qDebug() << "    Pred Key:" << _currentAdmin.getPredictiveKey().c_str();
            qDebug() << "    Pred Res:" << QString::number(_currentAdmin.getPredictiveResolution());
            qDebug() << "SMTP:" << _currentAdmin.getSMTPServer().c_str() << ":" << QVariant(_currentAdmin.getSMTPPort()).toString();

            return true;
        }
    }
    return false;
}

void CTblAdmin::addAdmin()
{

}

bool CTblAdmin::updateAdminClear(std::string name, std::string email, std::string phone,
                bool emailReportActive, QDateTime repFreq, QDateTime startReport,
                std::string passwordClear, std::string accessCdClear,
                std::string assistPasswordClear, std::string assistCodeClear,
                bool showFingerprint,
                bool usePredictive, std::string predKey, int predRes,
                uint32_t nMaxLocks,
                std::string smtpserver, int smtpport, int smtptype,
                std::string smtpusername, std::string smtppassword,
                int vncport, std::string vncpassword,
                bool bReportToEmail, bool bReportToFile, std::string reportDirectory)
{
    std::string encPW = CEncryption::encryptString(passwordClear.c_str()).toStdString();
    std::string encCode = CEncryption::encryptString(accessCdClear.c_str()).toStdString();
    std::string encAssistPw = CEncryption::encryptString(assistPasswordClear.c_str()).toStdString();
    std::string encAssistCode = CEncryption::encryptString(assistCodeClear.c_str()).toStdString();
    std::string encSMTPPW = CEncryption::encryptString(smtppassword.c_str()).toStdString();
    qDebug() << "pw:" << encPW.c_str() << " code:" << encCode.c_str();

    std::string encVNCPW = CEncryption::encryptString(vncpassword.c_str()).toStdString();
    
    return updateAdmin(name, email, phone, emailReportActive, repFreq, startReport, encPW, encCode,
                       encAssistPw, encAssistCode, showFingerprint, usePredictive, predKey, predRes,
                       nMaxLocks, smtpserver, smtpport, smtptype, smtpusername, encSMTPPW, vncport,
                       encVNCPW, bReportToEmail, bReportToFile, reportDirectory);
}

bool CTblAdmin::updateAdmin(std::string name, std::string email, std::string phone,
                bool emailReportActive, QDateTime repFreq, QDateTime startReport,
                std::string passwordEnc, std::string accessCdEnc,
                std::string assistPasswordEnc, std::string assistCodeEnc,
                bool showFingerprint,
                bool usePredictive, std::string predKey, int predRes,
                uint32_t nMaxLocks,
                std::string smtpserver, int smtpport, int smtptype,
                std::string smtpusername, std::string smtppassword,
                int vncport, std::string vncpassword,
                bool bReportToEmail, bool bReportToFile, std::string reportDirectory)
{
    qDebug() << "CTblAdmin::updateAdminDefault()";

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + QString(TABLENAME.c_str()) +
                  " SET " + QString("admin_name=:name,"\
                  "admin_email=:email, admin_phone=:phone, email_report_active=:active, "
                  "default_report_freq=:freq, "
                  "default_report_start=:start, password=:pw, access_code=:code, "
                  "assist_passwrod=:assistpw, assist_code=:assistcode, show_fingerprint=:showFingerprint, "
                  "use_predictive_access_code=:usePred, predictive_key=:pKey, predictive_resolution=:pRes,"
                  "max_locks=:maxLocks, "
                  "smtp_server=:smtpserver, smtp_port=:smtpport, smtp_type=:smtptype, "
                  "smtp_username=:smtpusername, smtp_password=:smtppassword, "
                  "vnc_port=:vncport, vnc_password=:vncpassword, "
                  "report_via_email=:reportViaEmail, report_to_file=:reportToFile, report_directory=:reportDir"
                  " WHERE 1;");
    qry.prepare(sql);

    qry.bindValue(":name", name.c_str());
    qry.bindValue(":email", email.c_str());
    qry.bindValue(":phone", phone.c_str());
    qry.bindValue(":active", emailReportActive);
    QString sTime = repFreq.toString("yyyy-MM-dd HH:mm:ss");
    qry.bindValue(":freq", sTime);

    QString sStart  = startReport.toString("yyyy-MM-dd HH:mm:ss");
    qry.bindValue(":start", sStart);

    qry.bindValue(":pw", passwordEnc.c_str());
    qry.bindValue(":code", accessCdEnc.c_str());
    qry.bindValue(":assistpw", assistPasswordEnc.c_str());
    qry.bindValue(":assistcode", assistCodeEnc.c_str());
    qry.bindValue(":showFingerprint", showFingerprint);
    qry.bindValue(":usePred", usePredictive);
    qry.bindValue(":pKey", predKey.c_str());
    qry.bindValue(":pRes", predRes);
    qry.bindValue(":maxLocks", nMaxLocks);

    qry.bindValue(":smtpserver", smtpserver.c_str());
    qry.bindValue(":smtpport", smtpport);
    qry.bindValue(":smtptype", smtptype);
    qry.bindValue(":smtpusername", smtpusername.c_str());
    qry.bindValue(":smtppassword", smtppassword.c_str());

    qry.bindValue(":vncport", vncport);
    qry.bindValue(":vncpassword", vncpassword.c_str());
    
    qry.bindValue(":reportViaEmail", bReportToEmail);
    qry.bindValue(":reportToFile", bReportToFile);
    qry.bindValue(":reportDir", reportDirectory.c_str());

    QMap<QString, QVariant> mapVals = qry.boundValues();
    qDebug() << "Mapped count:" << mapVals.count();

    if( !qry.exec() ) {
        qDebug() << "CTblAdmin::updateAdmin():" << qry.lastError();
        return false;
    }
    else {
        _pDB->commit();
        qDebug() << "Updated Admin! SMTP Server:" << smtpserver.c_str() << ":" << QVariant(smtpport).toString();
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
                       rec.getAssistPassword(), rec.getAssistCode(), rec.getShowFingerprint(),
                       rec.getUsePredictiveAccessCode(),
                       rec.getPredictiveKey(), rec.getPredictiveResolution(),
                       rec.getMaxLocks(),
                       rec.getSMTPServer(), rec.getSMTPPort(), rec.getSMTPType(),
                       rec.getSMTPUsername(), rec.getSMTPPassword(),
                       rec.getVNCPort(), rec.getVNCPassword(),
                       rec.getReportViaEmail(), rec.getReportToFile(), rec.getReportDirectory());
}

/**
 * @brief CTblAdmin::updateAdmin - update the admin record with the new values from JSON Object
 * @param adminObj - json object with given values
 * @return true on success, false otherwise
 */
bool CTblAdmin::updateAdmin(QJsonObject adminObj)
{
    std::string name, email, phone;
    bool active;
    QDateTime freq;
    QDateTime start;
    std::string pw, accessCd, assistpw, assistCode;
    bool showFingerprint, usePredictive;
    std::string predKey;
    int predResolution;    
    uint32_t unMaxLocks;

    std::string smtpserver, smtpusername, smtppassword, vncpassword;
    int smtpport, smtptype, vncport;
    bool bReportToEmail, bReportToFile;
    std::string reportDir;

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
            adminObj.value(freportdirectory).isUndefined()
            )
    {
        return false;
    }

    name = adminObj.value(fname).toString().toStdString();
    email = adminObj.value(femail).toString().toStdString();
    phone = adminObj.value(fphone).toString().toStdString();
    active = adminObj.value(freportActive).toBool();
    freq.fromString(adminObj.value(ffreq).toString(), "yyyy-MM-dd HH:mm:ss");
    start.fromString(adminObj.value(fstart).toString(), "yyyy-MM-dd HH:mm:ss");
    pw = adminObj.value(fpassword).toString().toStdString();
    accessCd = adminObj.value(faccesscd).toString().toStdString();
    assistpw = adminObj.value(fassistpassword).toString().toStdString();
    assistCode = adminObj.value(fassistcode).toString().toStdString();
    showFingerprint = adminObj.value(fshowFingerprint).toBool();
    usePredictive = adminObj.value(fpredictivecode).toBool();
    predKey = adminObj.value(fpredkey).toString().toStdString();
    predResolution = adminObj.value(fpredres).toInt();
    unMaxLocks = adminObj.value(fmaxlocks).toInt();

    if( adminObj.value(fsmtpserver).isUndefined()) {
        smtpserver = "";
    } else {
        smtpserver = adminObj.value(fsmtpserver).toString().toStdString();
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
        smtpusername = adminObj.value(fsmtpusername).toString().toStdString();
    }
    const char *fsmtppassword = "smtp_password";
    if( adminObj.value(fsmtppassword).isUndefined()) {
        smtppassword = "";
    } else {
        smtppassword = adminObj.value(fsmtppassword).toString().toStdString();
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
        vncpassword = adminObj.value(fvncpassword).toString().toStdString();
    }
    
    bReportToEmail = adminObj.value(freportviaemail).toBool();
    bReportToFile= adminObj.value(freporttofile).toBool();
    reportDir = adminObj.value(freportdirectory).toString().toStdString();

    return updateAdminClear(name, email, phone, active, freq, start, pw, accessCd, assistpw, assistCode, showFingerprint,
                            usePredictive, predKey, predResolution, unMaxLocks, smtpserver, smtpport, smtptype,
                            smtpusername, smtppassword, vncport, vncpassword, bReportToEmail, bReportToFile, reportDir);
}


bool CTblAdmin::tableExists()
{
    QStringList lstTables = _pDB->tables();
    QStringList::iterator  itor;

    for(itor = lstTables.begin(); itor != lstTables.end(); itor++)
    {
        if((*itor)==TABLENAME.c_str()) {
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


