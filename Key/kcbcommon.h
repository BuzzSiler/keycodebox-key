#ifndef KCBCOMMON_H
#define KCBCOMMON_H

#include <QtGlobal>
#include <QDateTime>


// Debug Macros
#define KCB_DEBUG_ENTRY qDebug() << Q_FUNC_INFO << "ENTRY"
#define KCB_DEBUG_EXIT qDebug() << Q_FUNC_INFO << "EXIT"
#define KCB_DEBUG_TRACE(text)   qDebug() << Q_FUNC_INFO << text

const QDateTime _DATENONE = QDateTime(QDate(1990,1,1), QTime(0,0,0));
const QString _DATENONE_STR = _DATENONE.toString("yyyy-MM-dd HH:mm:ss");

typedef enum {ACCESS_TYPE_ALWAYS=0, ACCESS_TYPE_TIMED=1, ACCESS_TYPE_LIMITED_USE=2} ACCESS_TYPE;

#endif
