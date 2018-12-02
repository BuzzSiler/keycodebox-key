#include "adminrec.h"

#include "kcbcommon.h"


CAdminRec::CAdminRec() : 
    QObject(nullptr),
    default_report_freq(*new QDateTime(NEVER)),
    default_report_start(*new QDateTime(DEFAULT_DATE_TIME)),
    default_report_delete_freq(*new QDateTime(MONTHLY))
{
    ids = -1;
    admin_name = "";
    admin_email = "";
    admin_phone = "";

    password = "";
    assist_password = "";
    max_locks = 32;

    show_fingerprint = false;
    show_password = false;
    show_takereturn = false;
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
    default_report_freq = newRec.default_report_freq;
    default_report_start = newRec.default_report_start;
    password = newRec.password;
    assist_password = newRec.assist_password;
    max_locks = newRec.max_locks;

    show_fingerprint = newRec.show_fingerprint;
    show_password = newRec.show_password;
    show_takereturn = newRec.show_takereturn;
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

    default_report_delete_freq = newRec.default_report_delete_freq;

    return *this;
}