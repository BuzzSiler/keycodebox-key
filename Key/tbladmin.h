#ifndef CTBLADMIN_H
#define CTBLADMIN_H

#include <ctime>
#include <string>
#include <QObject>
#include <QDateTime>

class QSqlDatabase;
class QJsonObject;
class QString;


class CAdminRec : public QObject 
{
    Q_OBJECT

public:
    CAdminRec();

    CAdminRec &operator=(CAdminRec &newRec);

    int getID() { return ids; }    //  integer primary key unique,
    void setID(int id) { ids = id; }

    QString getAdminName() { return admin_name; }// text,
    void setAdminName(QString adminName) { admin_name = adminName; }

    QString getAdminEmail() { return admin_email; } // text,
    void setAdminEmail(QString adminEmail) { admin_email = adminEmail; }

    QString getAdminPhone() { return admin_phone; } // text,
    void setAdminPhone(QString adminPhone) { admin_phone = adminPhone; }

    bool getEmailReportActive() { return email_report_active; }
    void setEmailReportActive(bool bActive) { email_report_active = bActive; }

    QDateTime getDefaultReportFreq() { return default_report_freq; } // DATETIME
    void  setDefaultReportFreq(QDateTime freq) { default_report_freq = freq; }

    QDateTime getDefaultReportStart() { return default_report_start; }    // DATETIME
    void setDefaultReportStart(QDateTime start) { default_report_start = start; }    // DATETIME

    QString getPassword() { return password; }   // text,
    void setPassword(QString pw) { password = pw; }

    QString getAccessCodeOld() { return access_code_old; } // text
    void setAccessCodeOld(QString accessCode) { access_code_old = accessCode; }

    QString getAccessCode() { return access_code; } // text
    void setAccessCode(QString accessCode) { access_code = accessCode; }

    QString getAssistCode() { return assist_code; } //text
    void setAssistCode(QString assistCode) { assist_code = assistCode; }

    QString getAssistPassword() { return assist_password; } //text
    void setAssistPassword(QString assistPassword) { assist_password = assistPassword; }

    bool getUsePredictiveAccessCode() { return use_predictive_access_code; }
    void setUsePredictiveAccessCode(bool bSet) { use_predictive_access_code = bSet; }

    QString getPredictiveKey() { return pred_key; }
    void setPredictiveKey(QString key) { pred_key = key; }

    int getPredictiveResolution() { return pred_resolution; }
    void setPredictiveResolution(int nValue) { pred_resolution = nValue; }

    uint32_t getMaxLocks() { return max_locks; }
    void setMaxLocks(uint32_t nLocks) { max_locks = nLocks; }

    QString getSMTPServer() { return smtp_server; }
    void setSMTPServer(QString smtpserver) { smtp_server = smtpserver; }

    bool getDisplayFingerprintButton() { return show_fingerprint; }
    void setDisplayFingerprintButton(bool showFingerprint) { show_fingerprint = showFingerprint; }

    bool getDisplayShowHideButton() { return show_password; }
    void setDisplayShowHideButton(bool showPassword) { show_password = showPassword; }

    int getSMTPPort() { return smtp_port; }
    void setSMTPPort(int port) { smtp_port = port; }

    int getSMTPType() { return smtp_type; }
    void setSMTPType(int type) { smtp_type = type; }

    QString getSMTPUsername() { return smtp_username; }
    void setSMTPUsername(QString smtpusername) { smtp_username = smtpusername; }

    QString getSMTPPassword() { return smtp_password; }
    void setSMTPPassword(QString password) { smtp_password = password; }

    // remote desktop definitions
    QString getVNCServer() { return vnc_server; }
    void setVNCServer(QString vncserver) { vnc_server = vncserver; }
    int getVNCPort() { return vnc_port; }
    void setVNCPort(int port) { vnc_port = port; }
    int getVNCType() { return vnc_type; }
    void setVNCType(int type) { vnc_type = type; }
    QString getVNCUsername() { return vnc_username; }
    void setVNCUsername(QString vncusername) { vnc_username = vncusername; }
    QString getVNCPassword() { return vnc_password; }
    void setVNCPassword(QString password) { vnc_password = password; }
    
    bool getReportViaEmail() { return report_via_email; }
    void setReportViaEmail(bool bReportViaEmail) { report_via_email = bReportViaEmail; }
    bool getReportToFile() { return report_save_to_file; }
    void setReportToFile(bool bReportToFile) { report_save_to_file = bReportToFile; }
    QString getReportDirectory() { return report_directory; }
    void setReportDirectory(QString directory) { report_directory = directory; }

    QJsonObject& jsonRecord(QJsonObject &json);
    //QString jsonRecordAsString();

    bool setFromJsonObject(QJsonObject jsonObj);
    bool setFromJsonString(QString strJson);
    
    int getDisplayPowerDownTimeout() { return display_power_down_timeout; }
    void setDisplayPowerDownTimeout(int index) { display_power_down_timeout = index; }

private:
    int ids;    //  integer primary key unique,
    QString admin_name; // text,
    QString admin_email; // text,
    QString admin_phone; // text,
    bool        email_report_active;  // bool - true = email report, false = no email report
    QDateTime&   default_report_freq; // DATETIME
    QDateTime&   default_report_start;    // DATETIME,
    QString password;    // text,
    QString access_code_old; // text
    QString access_code; // text
    QString assist_code; //text
    QString assist_password; //text
    bool        use_predictive_access_code;
    QString pred_key;
    int         pred_resolution;
    uint32_t    max_locks;

    bool        show_fingerprint; // bool: true = display button
    bool        show_password; // bool: true = display button
    QString smtp_server;
    int smtp_port;
    int smtp_type;
    QString smtp_username;
    QString smtp_password;

    QString vnc_server;
    int vnc_port;
    int vnc_type;
    QString vnc_username;
    QString vnc_password;
    
    bool        report_via_email;
    bool        report_save_to_file;
    QString     report_directory;
    int        display_power_down_timeout;
};

class CTblAdmin
{
    const QString TABLENAME = "admin";

private:
    QSqlDatabase *_pDB;

    bool createTable();
    bool tableExists();

    CAdminRec   _currentAdmin;

    void currentTimeFull(QString strBuffer);
    void currentTimeFormat(QString format, QString strBuffer, int nExpectedLength);
    bool createAdminDefault();
    void initialize();
    bool readAdmin();

public:
    CTblAdmin(QSqlDatabase *db);

    void addAdmin();
    bool updateAdmin(QString name, QString email, QString phone,
                     bool activeReport, QDateTime repFreq, QDateTime startReport,
                     QString passwordEnc, QString accessCdEnc,
                     QString assistPasswordEnc, QString assistCodeEnc,
                     bool showFingerprint, bool showPassword,
                     bool usePredictive, QString predKey, int predRes,
                     uint32_t nMaxLocks,
                     QString smtpserver, int smptport, int smtptype,
                     QString smtpusername, QString smtppassword,
                     int vncport, QString vncpasword,
                     bool bEmailReport, bool bSaveReport, QString reportDirectory,
                     int displayPowerDownTimeout);

    bool updateAdminClear(QString name, QString email, QString phone,
                     bool activeReport, QDateTime repFreq, QDateTime startReport,
                     QString passwordClear, QString accessCdClear,
                     QString assistPasswordClear, QString assistCodeClear,
                     bool showFingerprint, bool showPassword,
                     bool usePredictive, QString predKey, int predRes,
                     uint32_t nMaxLocks,
                     QString smtpserver, int smptport, int smtptype,
                     QString smtpusername, QString smtppassword,
                     int vncport, QString vncpassword,
                     bool bEmailReport, bool bSaveReport, QString reportDirectory,
                     int displayPowerDownTimeout);
    bool updateAdmin(CAdminRec &rec);
    bool updateAdmin(QJsonObject adminObj);

    time_t  getReportStartTime();
    time_t  getReportFrequency();

    QString isAccessCode(QString code);     // Numeric
    bool isPassword(QString password, QString type);

    CAdminRec &getCurrentAdmin() { return _currentAdmin; }
};

#endif // CTBLADMIN_H
