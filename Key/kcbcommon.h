#ifndef KCBCOMMON_H
#define KCBCOMMON_H

#include <QtGlobal>
#include <QDateTime>
#include <QDebug>
#include <QObject>
#include "logger.h"

#define MAX_NUM_LOCKS_PER_CABINET 64
#define MAX_NUM_LOCKS_PER_BANK    32
#define MIN_NUM_LOCKS_PER_CABINET 8


#define KCB_FAILED (-1)
#define KCB_SUCCESS (0)

#define DATETIME_FORMAT QStringLiteral("yyyy-MM-dd HH:mm:ss")

const QDateTime DEFAULT_DATETIME = QDateTime(QDate(1990,1,1), QTime(0,0,0));
const QString DEFAULT_DATETIME_STR = DEFAULT_DATETIME.toString(DATETIME_FORMAT);

typedef enum {ACCESS_TYPE_ALWAYS=0, ACCESS_TYPE_TIMED=1, ACCESS_TYPE_LIMITED_USE=2} ACCESS_TYPE;

#define FIELD_INDEX(query, fld_name) (query.record().indexOf(fld_name))
#define QUERY_VALUE(q, fld_name)  q.value(FIELD_INDEX(q, fld_name))

#define SECS_IN_MINUTE (60)
#define SECS_IN_HOUR (60*SECS_IN_MINUTE)
#define SECS_IN_12_HOURS (12*SECS_IN_HOUR)
#define HOURS_IN_DAY (24)
#define SECS_IN_DAY (HOURS_IN_DAY*SECS_IN_HOUR)
#define DAYS_IN_WEEK (7)
#define SECS_IN_WEEK (DAYS_IN_WEEK*SECS_IN_DAY)
#define SECS_IN_MONTH(days_in_month) (days_in_month*SECS_IN_DAY)
#define SECS_IN_YEAR(days_in_year) (days_in_year*SECS_IN_DAY)


static const QDateTime NEVER = QDateTime(QDate(), QTime(0,0));
static const QDateTime EACH_ACTIVITY = QDateTime(QDate(1,1,1), QTime(0,0));
static const QDateTime HOURLY = QDateTime(QDate(1,1,1), QTime(1,0));
static const QDateTime EVERY_12_HOURS = QDateTime(QDate(1,1,1), QTime(12,0));
static const QDateTime DAILY = QDateTime(QDate(1,1,1), QTime(23,59));
static const QDateTime WEEKLY = QDateTime(QDate(1,1,7), QTime(0,0));
static const QDateTime BIWEEKLY = QDateTime(QDate(1,1,14), QTime(0,0));
static const QDateTime MONTHLY = QDateTime(QDate(1,12,1), QTime(0,0));
static const QDateTime DEFAULT_DATE_TIME = QDateTime(QDate(2016,6,1), QTime(12,00,00));

#define IS_EVERY_ACTIVITY(dt) (dt == EACH_ACTIVITY)
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



const QString USER_CODE_TAKE_RETURN_PROMPT = QString("<%1>").arg(QObject::tr("Select Take or Return"));
const QString USER_CODE_FLEETWAVE_PROMPT = QString("<%1>").arg(QObject::tr("Please Present Your Card"));
const QString USER_CODE_PROMPT = QString("<%1 #1>").arg(QObject::tr("Please Enter Code"));
const QString USER_CODE_CODE2_PROMPT = QString("<%1>").arg(QObject::tr("Please Enter Second Code"));


#define REPORT_FILE_FORMAT "yyyy-MM-dd-HH_mm_ss"


typedef enum { EMAIL_ADMIN_RECV, EMAIL_ADMIN_SEND, EMAIL_INVALID } EMAIL_ADMIN_SELECT;

QString const KCB_SYSTEM_PATH("/home/pi");
QString const KCB_SYSTEM_CONFIG_ROOT(KCB_SYSTEM_PATH + "/kcb-config");

QString const KCB_SCRIPTS_PATH(KCB_SYSTEM_CONFIG_ROOT + "/scripts");
QString const KCB_BIN_PATH(KCB_SYSTEM_CONFIG_ROOT + "/bin");
QString const KCB_RUN_PATH(KCB_SYSTEM_CONFIG_ROOT + "/run");
QString const KCB_SYSTEM_IMAGE_PATH(KCB_SYSTEM_CONFIG_ROOT + "/images");
QString const KCB_SYSTEM_CONFIG_PATH(KCB_SYSTEM_CONFIG_ROOT + "/config");
QString const KCB_SYSTEM_SETTINGS_PATH(KCB_SYSTEM_CONFIG_ROOT + "/settings");
QString const KCB_LOGS_PATH(KCB_SYSTEM_CONFIG_ROOT + "/logs");

QString const KCB_STORAGE_PATH("/mnt/storage");
QString const KCB_USER_CONFIG_ROOT(KCB_STORAGE_PATH + "/config");
QString const KCB_IMAGE_PATH(KCB_USER_CONFIG_ROOT + "/images");
QString const KCB_CONFIG_PATH(KCB_USER_CONFIG_ROOT + "/config");
QString const KCB_FPRINT_PATH(KCB_USER_CONFIG_ROOT + "/prints");
QString const KCB_SETTINGS_PATH(KCB_USER_CONFIG_ROOT + "/settings");
QString const KCB_DATA_PATH(KCB_USER_CONFIG_ROOT + "/data");
QString const KCB_REPORTS_PATH(KCB_USER_CONFIG_ROOT + "/reports");
QString const KCB_DATABASE_PATH(KCB_USER_CONFIG_ROOT + "/database");


// QString const KCB_BRANDING_IMAGE(KCB_IMAGE_PATH + "/kcbbranding.jpg");
QString const KCB_DEFAULT_IMAGE(KCB_SYSTEM_IMAGE_PATH + "/kcbdefault.jpg");
QString const KCB_INIT_IMAGE(KCB_SYSTEM_IMAGE_PATH + "/kcbinit.jpg");



#endif
