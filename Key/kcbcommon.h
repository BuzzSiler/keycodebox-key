#ifndef KCBCOMMON_H
#define KCBCOMMON_H

#include <QDateTime>

const QDateTime _DATENONE = QDateTime(QDate(1990,1,1), QTime(0,0,0));
const QString _DATENONE_STR = _DATENONE.toString("yyyy-MM-dd HH:mm:ss");

#endif
