#ifndef KCBCOMMON_H
#define KCBCOMMON_H

#include <QtGlobal>
#include <QDateTime>
#include <QDebug>


#define KCB_FAILED (-1)
#define KCB_SUCCESS (0)

// Debug Macros
#define KCB_DEBUG_ENTRY qDebug() << Q_FUNC_INFO << "ENTRY"
#define KCB_DEBUG_EXIT qDebug() << Q_FUNC_INFO << "EXIT"
#define KCB_DEBUG_TRACE(text)   qDebug() << Q_FUNC_INFO << text
#define KCB_WARNING_TRACE(text)   qWarning() << Q_FUNC_INFO << text

#define DATETIME_FORMAT QStringLiteral("yyyy-MM-dd HH:mm:ss")

const QDateTime DEFAULT_DATETIME = QDateTime(QDate(1990,1,1), QTime(0,0,0));
const QString DEFAULT_DATETIME_STR = DEFAULT_DATETIME.toString(DATETIME_FORMAT);

typedef enum {ACCESS_TYPE_ALWAYS=0, ACCESS_TYPE_TIMED=1, ACCESS_TYPE_LIMITED_USE=2} ACCESS_TYPE;

#define FIELD_INDEX(query, fld_name) (query.record().indexOf(fld_name))
#define QUERY_VALUE(q, fld_name)  q.value(FIELD_INDEX(q, fld_name))

static const QDateTime NEVER = QDateTime(QDate(), QTime(0,0));
static const QDateTime EACH_ACTIVITY = QDateTime(QDate(1,1,1), QTime(0,0));
static const QDateTime HOURLY = QDateTime(QDate(1,1,1), QTime(1,0));
static const QDateTime EVERY_12_HOURS = QDateTime(QDate(1,1,1), QTime(12,0));
static const QDateTime DAILY = QDateTime(QDate(1,1,1), QTime(23,59));
static const QDateTime WEEKLY = QDateTime(QDate(1,1,7), QTime(0,0));
static const QDateTime BIWEEKLY = QDateTime(QDate(1,1,14), QTime(0,0));
static const QDateTime MONTHLY = QDateTime(QDate(1,12,1), QTime(0,0));
static const QDateTime DEFAULT_DATE_TIME = QDateTime(QDate(2016,6,1), QTime(12,00,00));

#endif
