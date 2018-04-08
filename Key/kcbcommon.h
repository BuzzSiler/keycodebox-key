#ifndef KCBCOMMON_H
#define KCBCOMMON_H

#include <QtGlobal>
#include <QDateTime>


#define KCB_FAILED (-1)
#define KCB_SUCCESS (0)

// Debug Macros
#define KCB_DEBUG_ENTRY qDebug() << Q_FUNC_INFO << "ENTRY"
#define KCB_DEBUG_EXIT qDebug() << Q_FUNC_INFO << "EXIT"
#define KCB_DEBUG_TRACE(text)   qDebug() << Q_FUNC_INFO << text
#define KCB_WARNING_TRACE(text)   qWarning() << Q_FUNC_INFO << text

#define DATETIME_FORMAT QStringLiteral("yyyy-MM-dd HH:mm:ss")

const QDateTime _DATENONE = QDateTime(QDate(1990,1,1), QTime(0,0,0));
const QString _DATENONE_STR = _DATENONE.toString(DATETIME_FORMAT);

typedef enum {ACCESS_TYPE_ALWAYS=0, ACCESS_TYPE_TIMED=1, ACCESS_TYPE_LIMITED_USE=2} ACCESS_TYPE;

#define FIELD_INDEX(query, fld_name) (query.record().indexOf(fld_name))
#define QUERY_VALUE(q, fld_name)  q.value(FIELD_INDEX(q, fld_name))




#endif
