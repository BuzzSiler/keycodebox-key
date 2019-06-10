#include "tbladmin.h"

#include <sqlite3.h>
#include <exception>

#include <QtSql>
#include <QtDebug>
#include <QMap>

#include "encryption.h"
#include "kcbcommon.h"

const char *fid = "ids";
const char *fname = "admin_name";
const char *femail = "admin_email";
const char *fphone = "admin_phone";
const char *ffreq = "default_report_freq";
const char *fstart = "default_report_start";
const char *fpassword = "password";
const char *fassistpassword = "assist_password";
const char *fshowFingerprint = "show_fingerprint";
const char *fshowPassword = "show_password";
const char *fshowTakeReturn = "show_takereturn";
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
const char *freportdeletion = "report_deletion";

static const QString VERSION = "1.0";

static const QStringList DATA_FIELDS = {"admin_name", "admin_email", "admin_phone", "default_report_freq", "default_report_start", "password",
                                        "assist_password", "show_fingerprint", "show_password", "show_takereturn", "max_locks", "smtp_server", 
                                        "smtp_port", "smtp_type", "smtp_username", "smtp_password", "vnc_port", "vnc_password", 
                                        "report_via_email", "report_to_file", "report_directory", "display_power_down_timeout", "report_deletion DATETIME", 
                                        "version"};

CTblAdmin::CTblAdmin(QSqlDatabase *db)
{
    // KCB_DEBUG_ENTRY;
    _pDB = db;
    initialize();
    // KCB_DEBUG_EXIT;
}

QString CTblAdmin::isAccessCode(QString code)
{
    if (code == "Admin")
    {
        return "Admin";
    }

    if (code == "Assist")
    {
        return "Assist";
    }

    return "User";
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
    // KCB_DEBUG_TRACE("Creating Table");
    if( _pDB && _pDB->isOpen() ) 
    {
        QSqlQuery qry(*_pDB);
        QString sql = "CREATE TABLE IF NOT EXISTS " + TABLENAME +
                "(ids integer primary key unique, admin_name text,"
                " admin_email text, admin_phone text, "
                " default_report_freq DATETIME,"
                " default_report_start DATETIME, password text, "
                " assist_password text, show_fingerprint bool, show_password bool, show_takereturn bool, "
                " max_locks text,"
                " smtp_server text, smtp_port text, smtp_type text, smtp_username text, smtp_password text,"
                " vnc_port text, vnc_password text,"
                " report_via_email text, report_to_file text, report_directory text, display_power_down_timeout integer,"
                " report_deletion DATETIME,"
                " version)";

        qry.prepare( sql );

        if( !qry.exec() ) 
        {
            KCB_DEBUG_TRACE(qry.lastError());
            return false;
        }
        else 
        {
            return true;
        }
    }
    return false;
}

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
    // KCB_DEBUG_ENTRY;

    QSqlQuery qry(*_pDB);
    QString sql = QString("INSERT OR IGNORE INTO ") + TABLENAME +
                    QString("(admin_name, "
                            "admin_email, admin_phone, default_report_freq, "
                            "default_report_start, password, "
                            "assist_password, show_fingerprint, show_password, show_takereturn, "
                            "max_locks, "
                            "smtp_server, smtp_port, smtp_type, smtp_username, smtp_password, "
                            "report_via_email, report_to_file, report_directory, "
                            "vnc_port, vnc_password, display_power_down_timeout, "
                            "report_deletion, version)"
                  " VALUES ('admin', "
                            "'admin@email.com', '555.555.5555', :freq, "
                            ":start, :pw, "
                            ":assistpw, :showFingerprint, :showPassword, :showTakeReturn, "
                            "32, "
                            ":smtp_server, :smtp_port, :smtp_type, :smtp_username, :smtp_password, "
                            "1, 0, '', "
                            "5901, ':vnc_password', 0, :deletion,"
                            ":version)");
    qry.prepare(sql);

    // KCB_DEBUG_TRACE("SQL:" << sql);

    QString sTime;
    sTime = EVERY_12_HOURS.toString(DATETIME_FORMAT);
    qry.bindValue(":freq", sTime);

    QString sStart;
    QDateTime   dtTime = QDateTime::currentDateTime();
    sStart = dtTime.toString(DATETIME_FORMAT);
    qry.bindValue(":start", sStart);

    QString sDeletion = MONTHLY.toString(DATETIME_FORMAT);
    qry.bindValue(":deletion", sDeletion);

    QString encPW = CEncryption::encryptString("");
    QString encAssistPw = CEncryption::encryptString("");
    QString encSMTPPW = CEncryption::encryptString("keycodebox");
    QString encVNCPW = CEncryption::encryptString("keycodebox");

    qry.bindValue(":pw", encPW);
    qry.bindValue(":assistpw", encAssistPw);
    qry.bindValue(":showFingerprint", QVariant(false));
    qry.bindValue(":showPassword", QVariant(false));
    qry.bindValue(":showTakeReturn", QVariant(false));
    qry.bindValue(":smtp_server", "smtpout.secureserver.net");
    qry.bindValue(":smtp_port", 465);
    qry.bindValue(":smtp_type", 1);
    qry.bindValue(":smtp_username", "kcb@keycodebox.com");
    qry.bindValue(":smtp_password", encSMTPPW);
    qry.bindValue(":vnc_password", encVNCPW);
    qry.bindValue(":version", VERSION);

    if( !qry.exec() ) 
    {
        // KCB_DEBUG_TRACE(qry.lastError());
        return false;
    }
    else 
    {
        // KCB_DEBUG_TRACE( "Inserted!" );
        return true;
    }
}

bool CTblAdmin::readAdmin()
{
    // KCB_DEBUG_ENTRY;

    QSqlQuery query(*_pDB);
    QString sql = "SELECT admin_name, admin_email, admin_phone, "
                  "default_report_freq,"
                  "default_report_start, password, "
                  "assist_password, show_fingerprint, show_password, show_takereturn, "
                  "max_locks, "
                  "smtp_server, smtp_port, smtp_type, smtp_username, smtp_password, "
                  "vnc_port,vnc_password, "
                  "report_via_email, report_to_file, report_directory, display_power_down_timeout, "
                  "report_deletion"
                  " FROM ";
    sql += TABLENAME;

    if( query.exec(sql)) 
    {
        int fldAdmin_name= query.record().indexOf(fname);
        int fldAdmin_email = query.record().indexOf(femail);
        int fldAdmin_phone = query.record().indexOf(fphone);
        int fldReport_freq = query.record().indexOf(ffreq);
        int fldReport_start = query.record().indexOf(fstart);
        int fldPassword = query.record().indexOf(fpassword);
        int fldAssistPassword = query.record().indexOf(fassistpassword);
        int fldShowFingerprint = query.record().indexOf(fshowFingerprint);
        int fldShowPassword = query.record().indexOf(fshowPassword);
        int fldShowTakeReturn = query.record().indexOf(fshowTakeReturn);
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
        int fldReportDeletion = query.record().indexOf(freportdeletion);


        if (query.next())
        {
            // it exists
            _currentAdmin.setAdminName(query.value(fldAdmin_name).toString());
            _currentAdmin.setAdminEmail(query.value(fldAdmin_email).toString());
            _currentAdmin.setAdminPhone(query.value(fldAdmin_phone).toString());
            _currentAdmin.setDefaultReportFreq( QDateTime::fromString(query.value(fldReport_freq).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss") ); // t is now your desired time_t
            _currentAdmin.setDefaultReportStart( QDateTime::fromString(query.value(fldReport_start).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss") );

            QString password = query.value(fldPassword).toString();
            password = CEncryption::decryptString(password);

            QString assistPassword = query.value(fldAssistPassword).toString();
            assistPassword = CEncryption::decryptString(assistPassword);

            // Unencrypted in currentAdmin
            _currentAdmin.setPassword(password);
            _currentAdmin.setAssistPassword(assistPassword);

            _currentAdmin.setDisplayFingerprintButton(query.value(fldShowFingerprint).toBool());
            _currentAdmin.setDisplayShowHideButton(query.value(fldShowPassword).toBool());
            _currentAdmin.setDisplayTakeReturnButtons(query.value(fldShowTakeReturn).toBool());
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

            _currentAdmin.setDefaultReportDeleteFreq( QDateTime::fromString(query.value(fldReportDeletion).toDateTime().toString("yyyy-MM-dd HH:mm:ss"), "yyyy-MM-dd HH:mm:ss"));
            


            return true;
        }
    }
    return false;
}

void CTblAdmin::addAdmin()
{

}

bool CTblAdmin::updateAdminClear(QString name, QString email, QString phone,
                QDateTime repFreq, QDateTime startReport,
                QString passwordClear,
                QString assistPasswordClear,
                bool showFingerprint, bool showPassword,
                uint32_t nMaxLocks,
                QString smtpserver, int smtpport, int smtptype,
                QString smtpusername, QString smtppassword,
                int vncport, QString vncpassword,
                bool bReportToEmail, bool bReportToFile, QString reportDirectory,
                int displayPowerDownTimeout, QDateTime reportDeletion, bool showTakeReturn)
{
    QString encPW = CEncryption::encryptString(passwordClear);
    QString encAssistPw = CEncryption::encryptString(assistPasswordClear);
    QString encSMTPPW = CEncryption::encryptString(smtppassword);

    QString encVNCPW = CEncryption::encryptString(vncpassword);

    return updateAdmin(name, email, phone, repFreq, startReport, encPW,
                       encAssistPw, showFingerprint, showPassword,
                       nMaxLocks, smtpserver, smtpport, smtptype, smtpusername, encSMTPPW, vncport,
                       encVNCPW, bReportToEmail, bReportToFile, reportDirectory, displayPowerDownTimeout,
                       reportDeletion, showTakeReturn);
}

bool CTblAdmin::updateAdmin(QString name, QString email, QString phone,
                QDateTime repFreq, QDateTime startReport,
                QString passwordEnc,
                QString assistPasswordEnc,
                bool showFingerprint, bool showPassword,
                uint32_t nMaxLocks,
                QString smtpserver, int smtpport, int smtptype,
                QString smtpusername, QString smtppassword,
                int vncport, QString vncpassword,
                bool bReportToEmail, bool bReportToFile, QString reportDirectory,
                int displayPowerDownTimeout,
                QDateTime reportDeletion, bool showTakeReturn)
{
    // KCB_DEBUG_ENTRY;

    QSqlQuery qry(*_pDB);
    QString sql = QString("UPDATE ") + TABLENAME +
                  " SET " + QString("admin_name=:name,"\
                  "admin_email=:email, admin_phone=:phone, "
                  "default_report_freq=:freq, "
                  "default_report_start=:start, password=:pw, "
                  "assist_password=:assistpw, show_fingerprint=:showFingerprint, show_password=:showPassword, show_takereturn=:showTakeReturn, "
                  "max_locks=:maxLocks, "
                  "smtp_server=:smtpserver, smtp_port=:smtpport, smtp_type=:smtptype, "
                  "smtp_username=:smtpusername, smtp_password=:smtppassword, "
                  "vnc_port=:vncport, vnc_password=:vncpassword, "
                  "report_via_email=:reportViaEmail, report_to_file=:reportToFile, report_directory=:reportDir, "
                  "display_power_down_timeout=:displayPowerDownTimeout, "
                  "report_deletion=:reportDeletion "
                  " WHERE 1;");
    qry.prepare(sql);

    qry.bindValue(":name", name);
    qry.bindValue(":email", email);
    qry.bindValue(":phone", phone);
    QString sTime = repFreq.toString(DATETIME_FORMAT);
    qry.bindValue(":freq", sTime);

    QString sStart  = startReport.toString(DATETIME_FORMAT);
    qry.bindValue(":start", sStart);

    qry.bindValue(":pw", passwordEnc);
    qry.bindValue(":assistpw", assistPasswordEnc);
    qry.bindValue(":showFingerprint", showFingerprint);
    qry.bindValue(":showPassword", showPassword);
    qry.bindValue(":showTakeReturn", showTakeReturn);
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

    qry.bindValue(":reportDeletion", reportDeletion);

    if( !qry.exec() ) 
    {
        qDebug() << "CTblAdmin::updateAdmin():" << qry.lastError();
        return false;
    }
    else 
    {
        _pDB->commit();
        qDebug() << "Updated Admin! SMTP Server:" << smtpserver << ":" << QVariant(smtpport).toString();
        readAdmin();
        return true;
    }

}

bool CTblAdmin::updateAdmin(CAdminRec &rec)
{
    return updateAdmin(rec.getAdminName(), rec.getAdminEmail(), rec.getAdminPhone(),
                       rec.getDefaultReportFreq(), rec.getDefaultReportStart(),
                       rec.getPassword(),
                       rec.getAssistPassword(),
                       rec.getDisplayFingerprintButton(), rec.getDisplayShowHideButton(),
                       rec.getMaxLocks(),
                       rec.getSMTPServer(), rec.getSMTPPort(), rec.getSMTPType(),
                       rec.getSMTPUsername(), rec.getSMTPPassword(),
                       rec.getVNCPort(), rec.getVNCPassword(),
                       rec.getReportViaEmail(), rec.getReportToFile(), rec.getReportDirectory(),
                       rec.getDisplayPowerDownTimeout(),
                       rec.getDefaultReportDeleteFreq(),
                       rec.getDisplayTakeReturnButtons());
}


bool CTblAdmin::tableExists()
{
    // KCB_DEBUG_ENTRY;
    QStringList lstTables = _pDB->tables();
    QStringList::iterator  itor;

    for(itor = lstTables.begin(); itor != lstTables.end(); itor++)
    {
        if((*itor)==TABLENAME.toStdString().c_str()) 
        {
            // KCB_DEBUG_TRACE(true);
            return true;
        }
    }
    // KCB_DEBUG_TRACE(false);
    return false;
}

bool CTblAdmin::columnExists(QString column)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    
    QStringList::iterator  itor;
    QSqlQuery qry(*_pDB);
    bool foundColumn = false;

    // KCB_DEBUG_TRACE("Opening Database" << _pDB << _pDB->isOpen());

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");

    if( _pDB && _pDB->isOpen() )
    {
        QString sql("PRAGMA TABLE_INFO(admin);");

        qry.prepare( sql );

        if( qry.exec() )
        {
            while( qry.next() )
            {
                if( qry.value(1).toString().compare(column) == 0 )
                {
                    // KCB_DEBUG_TRACE("found column: " << column);

                    foundColumn = true;
                    break;
                }
            }
        }
        else
        {
            qDebug() << qry.lastError();
        }

    } 
    else 
    {
        KCB_DEBUG_TRACE("Either _pDB is NULL or _pDB is not open");
    }
    return foundColumn;
}

void CTblAdmin::createColumn(QString column, QString fieldType, QString value)
{
    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE("Opening Database" << _pDB << _pDB->isOpen());

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database pointer is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");
    
    if( _pDB && _pDB->isOpen() ) 
    {
        // KCB_DEBUG_TRACE("Creating table");
        QSqlQuery qry(*_pDB);

        QString sql("ALTER TABLE  ");
        sql += TABLENAME;
        sql += " ADD ";
        sql += column;
        sql += " ";
        sql += fieldType;
        sql += " ";
        sql += QString("DEFAULT '%1' NOT NULL").arg(value);

        qry.prepare( sql );

        if( !qry.exec() )
        {
            qDebug() << qry.lastError();
        }
        else
        {
            qDebug() << "Table altered!";
        }
    } 
    else 
    {
        KCB_DEBUG_TRACE("Either _pDB is NULL or _pDB is not open");
    }
}



QSqlQuery CTblAdmin::createQuery(const QStringList& column_list,
                                 const QString& table)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(_pDB != nullptr, Q_FUNC_INFO, "database is null");
    Q_ASSERT_X(_pDB->isOpen(), Q_FUNC_INFO, "database is not open");

    QSqlQuery query(*_pDB);
    QString sql;
    
    query.setForwardOnly(true);

    auto select = QString("SELECT %1").arg(column_list.join(","));
    sql += QString("%1").arg(select);
    auto from = QString("FROM %1").arg(table);
    sql += QString(" %1").arg(from);

    // KCB_DEBUG_TRACE("SQL:" << sql);

    if ( !query.prepare(sql) )
    {
        KCB_WARNING_TRACE("prepare failed" << query.lastError());
    }

    // KCB_DEBUG_EXIT;

    return query;
}


int CTblAdmin::queryCommonFields(const QStringList& common_fields, QMap<QString, QString>& dict)
{
    // KCB_DEBUG_ENTRY;

    auto query = createQuery(common_fields, QString("ADMIN"));

    if (!query.exec())
    {
        qDebug() << query.lastError().text() << query.lastQuery();
        return KCB_FAILED;
    }

    KCB_DEBUG_TRACE("Active" << query.isActive() << "Select" << query.isSelect());

    while (query.next())
    {
        foreach (auto field, common_fields)
        {
            QString value = QUERY_VALUE(query, field).toString();
            // KCB_DEBUG_TRACE("Adding" << field << "value" << value << "to dict");
            dict[field] = value;
        }
    }

    // KCB_DEBUG_EXIT;

    return KCB_SUCCESS;
}

int CTblAdmin::dropTable()
{
    QSqlQuery query(*_pDB);
    QString sql("DROP TABLE ADMIN");
    
    query.setForwardOnly(true);

    qDebug() << "SQL:" << sql;

    if ( !query.prepare(sql) )
    {
        KCB_WARNING_TRACE("prepare failed" << query.lastError());
        return KCB_FAILED;
    }

    if ( !query.exec() )
    {
        qDebug() << query.lastError().text() << query.lastQuery();
        return KCB_FAILED;
    }

    return KCB_SUCCESS;
}

bool CTblAdmin::getVersion()
{
    _version = "";

    auto query = createQuery(QStringList() << "version", "ADMIN");

    if (!query.exec())
    {
        KCB_DEBUG_TRACE(query.lastError().text() << query.lastQuery());
        return false;
    }

    // KCB_DEBUG_TRACE("Active" << query.isActive() << "Select" << query.isSelect());

    while (query.next())
    {
        _version = QUERY_VALUE(query, "version").toString();
    }

    return !_version.isEmpty();
}

void CTblAdmin::populateAdminWithDefaults()
{
    if (!readAdmin()) 
    {
        KCB_DEBUG_TRACE("Failed to read admin table, creating defaults");
        if( createAdminDefault() ) 
        {
            KCB_DEBUG_TRACE("Defaults created, reading admin");
            bool result = readAdmin();
            Q_UNUSED(result);
            // KCB_DEBUG_TRACE("readAdmin returned" << result);
        }
    }
}

void CTblAdmin::updateAdminFromCommonFields(const QMap<QString, QString>& fieldValues)
{          
    if (fieldValues.count() > 0)
    {
        foreach (auto key, fieldValues.keys())
        {
            if (key == "admin")
            {
                _currentAdmin.setAdminName(fieldValues[key]);
            }
            else if (key == "admin_email")
            {
                _currentAdmin.setAdminEmail(fieldValues[key]);
            }
            else if (key == "admin_phone")
            {
                _currentAdmin.setAdminPhone(fieldValues[key]);
            }
            else if (key == "default_report_freq")
            {
                _currentAdmin.setDefaultReportFreq(QDateTime::fromString(fieldValues[key], DATETIME_FORMAT));
            }
            else if (key == "default_report_start")
            {
                _currentAdmin.setDefaultReportStart(QDateTime::fromString(fieldValues[key], DATETIME_FORMAT));
            }
            else if (key == "password")
            {
                _currentAdmin.setPassword(fieldValues[key]);
            }
            else if (key == "assist_password")
            {
                _currentAdmin.setAssistPassword(fieldValues[key]);
            }
            else if (key == "show_fingerprint")
            {
                _currentAdmin.setDisplayFingerprintButton(static_cast<bool>(fieldValues[key].toInt()));
            }
            else if (key == "show_password")
            {
                _currentAdmin.setDisplayShowHideButton(static_cast<bool>(fieldValues[key].toInt()));
            }
            else if (key == "max_locks")
            {
                _currentAdmin.setMaxLocks(fieldValues[key].toInt());
            }
            else if (key == "smtp_server")
            {
                _currentAdmin.setSMTPServer(fieldValues[key]);
            }
            else if (key == "smtp_port")
            {
                _currentAdmin.setSMTPPort(fieldValues[key].toInt());
            }
            else if (key == "smtp_type")
            {
                _currentAdmin.setSMTPType(fieldValues[key].toInt());
            }
            else if (key == "smtp_username")
            {
                _currentAdmin.setSMTPUsername(fieldValues[key]);
            }
            else if (key == "smtp_password")
            {
                _currentAdmin.setSMTPPassword(fieldValues[key]);
            }
            else if (key == "vnc_port")
            {
                _currentAdmin.setVNCPort(fieldValues[key].toInt());
            }
            else if (key == "vnc_password")
            {
                _currentAdmin.setVNCPassword(fieldValues[key]);
            }
            else if (key == "report_via_email")
            {
                _currentAdmin.setReportViaEmail(static_cast<bool>(fieldValues[key].toInt()));
            }
            else if (key == "report_to_file")
            {
                _currentAdmin.setReportToFile(static_cast<bool>(fieldValues[key].toInt()));
            }
            else if (key == "report_directory")
            {
                _currentAdmin.setReportDirectory(fieldValues[key]);
            }
            else if (key == "display_power_down_timeout")
            {
                _currentAdmin.setDisplayPowerDownTimeout(fieldValues[key].toInt());
            }
        }

        updateAdmin(_currentAdmin.getAdminName(), 
                    _currentAdmin.getAdminEmail(),
                    _currentAdmin.getAdminPhone(),
                    _currentAdmin.getDefaultReportFreq(),
                    _currentAdmin.getDefaultReportStart(),
                    _currentAdmin.getPassword(),
                    _currentAdmin.getAssistPassword(),
                    _currentAdmin.getDisplayFingerprintButton(),
                    _currentAdmin.getDisplayShowHideButton(),
                    _currentAdmin.getMaxLocks(),
                    _currentAdmin.getSMTPServer(),
                    _currentAdmin.getSMTPPort(),
                    _currentAdmin.getSMTPType(),
                    _currentAdmin.getSMTPUsername(),
                    _currentAdmin.getSMTPPassword(),
                    _currentAdmin.getVNCPort(),
                    _currentAdmin.getVNCPassword(),
                    _currentAdmin.getReportViaEmail(),
                    _currentAdmin.getReportToFile(),
                    _currentAdmin.getReportDirectory(),
                    _currentAdmin.getDisplayPowerDownTimeout(),
                    _currentAdmin.getDefaultReportDeleteFreq(),
                    _currentAdmin.getDisplayTakeReturnButtons());
    }
}

QStringList CTblAdmin::getCommonFields()
{
    QStringList commonFields;

    foreach (auto field, DATA_FIELDS)
    {
        if (columnExists(field))
        {
            commonFields.append(field);
        }
    }

    return commonFields;
}

QMap<QString, QString> CTblAdmin::mergeCommonFields(const QStringList& fields)
{
    QMap<QString, QString> commonFieldValues;

    // KCB_DEBUG_TRACE("Found" << fields.count() << "common fields");
    if (fields.count() > 0)
    {
        int result = queryCommonFields(fields, commonFieldValues);
        Q_UNUSED(result);
        result = dropTable();
        // KCB_DEBUG_TRACE("dropTable returned" << result);

        // Create a new table
        createTable();
    }

    return commonFieldValues;
}

void CTblAdmin::initialize()
{
    // KCB_DEBUG_ENTRY;
    if (tableExists())
    {
        // KCB_DEBUG_TRACE("Table found, checking version info ...");

        // See if the table has a version field
        if (getVersion())
        {
            if (_version == VERSION)
            {
                KCB_DEBUG_TRACE("Table is at current version " << _version << ".  No migration needed");
                bool result = readAdmin();
                Q_UNUSED(result);
                // KCB_DEBUG_TRACE("readAdmin returned" << result);
            }
            else
            {
                KCB_DEBUG_TRACE("Migrating from" << _version << "to" << VERSION);
            }
        }
        else
        {
            // KCB_DEBUG_TRACE("Handling non-version table migration");

            QStringList commonFields = getCommonFields(); 
            QMap<QString, QString> commonFieldValues = mergeCommonFields(commonFields);           
            populateAdminWithDefaults();
            updateAdminFromCommonFields(commonFieldValues);
        }
    }
    else
    {
        KCB_DEBUG_TRACE("No table found, creating new with defaults");
        createTable();
        populateAdminWithDefaults();
    }
}
