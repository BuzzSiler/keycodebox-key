#ifndef ADMINREC_H
#define ADMINREC_H

#include <QString>
#include <QObject>
#include <QJsonObject>
#include <QDateTime>
#include "kcbcommon.h"


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

    QDateTime getDefaultReportFreq() { return default_report_freq; } // DATETIME
    void  setDefaultReportFreq(QDateTime freq) { default_report_freq = freq; }

    QDateTime getDefaultReportStart() { return default_report_start; }    // DATETIME
    void setDefaultReportStart(QDateTime start) { default_report_start = start; }    // DATETIME

    QDateTime getDefaultReportDeleteFreq() { return default_report_delete_freq; }
    void setDefaultReportDeleteFreq(QDateTime deletion) { default_report_delete_freq = deletion; }

    QString getPassword() { return password; }   // text,
    void setPassword(QString pw) { password = pw; }

    QString getAssistPassword() { return assist_password; } //text
    void setAssistPassword(QString assistPassword) { assist_password = assistPassword; }

    uint32_t getMaxLocks() { return max_locks; }
    void setMaxLocks(uint32_t nLocks) { max_locks = nLocks; }

    QString getSMTPServer() { return smtp_server; }
    void setSMTPServer(QString smtpserver) { smtp_server = smtpserver; }

    bool getDisplayFingerprintButton() { 
        KCB_DEBUG_ENTRY; 
        bool result = show_fingerprint;
        KCB_DEBUG_TRACE("fp" << result);
        KCB_DEBUG_EXIT; 
        return result;
    }
    void setDisplayFingerprintButton(bool showFingerprint) { show_fingerprint = showFingerprint; }

    bool getDisplayShowHideButton() { 
        KCB_DEBUG_ENTRY; 
        bool result = show_password;
        KCB_DEBUG_TRACE("sh" << result);
        KCB_DEBUG_EXIT; 
        return result;
    }
    void setDisplayShowHideButton(bool showPassword) { show_password = showPassword; }

    bool getDisplayTakeReturnButtons() { 
        KCB_DEBUG_ENTRY; 
        bool result = show_takereturn;
        KCB_DEBUG_TRACE("tr" << result);
        KCB_DEBUG_EXIT; 
        return result;
    }
    void setDisplayTakeReturnButtons(bool showTakeReturn) { show_takereturn = showTakeReturn; }

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

    int getDisplayPowerDownTimeout() { return display_power_down_timeout; }
    void setDisplayPowerDownTimeout(int index) { display_power_down_timeout = index; }

private:
    int ids;    //  integer primary key unique,
    QString admin_name; // text,
    QString admin_email; // text,
    QString admin_phone; // text,
    QDateTime&   default_report_freq; // DATETIME
    QDateTime&   default_report_start;    // DATETIME,
    QDateTime&   default_report_delete_freq;
    QString password;    // text,
    QString access_code_old; // text
    QString access_code; // text
    QString assist_code; //text
    QString assist_password; //text
    uint32_t    max_locks;

    bool        show_fingerprint; // bool: true = display button
    bool        show_password; // bool: true = display button
    bool        show_takereturn;
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


#endif