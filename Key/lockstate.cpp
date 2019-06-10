#include <exception>

#include <QString>
#include <QVector>

#include "lockstate.h"
#include "tblcodes.h"
#include "kcbcommon.h"

#define IDS             QStringLiteral("ids")
#define SEQUENCE        QStringLiteral("sequence")
#define SEQUENCE_ORDER  QStringLiteral("sequence_order")
#define LOCKNUMS        QStringLiteral("locknums")
#define DESCRIPTION     QStringLiteral("description")
#define CODE1           QStringLiteral("code1")
#define CODE2           QStringLiteral("code2")
#define STARTTIME       QStringLiteral("starttime")
#define ENDTIME         QStringLiteral("endtime")
#define STATUS          QStringLiteral("status")
#define ACCESS_COUNT    QStringLiteral("access_count")
#define RETRY_COUNT     QStringLiteral("retry_count")
#define MAX_ACCESS      QStringLiteral("max_access")
#define MAX_RETRY       QStringLiteral("max_retry")
#define MODIFIED        QStringLiteral("modified")
#define DELETE          QStringLiteral("delete")
#define FINGERPRINT1    QStringLiteral("fingerprint1")
#define FINGERPRINT2    QStringLiteral("fingerprint2")
#define ASK_QUESTIONS   QStringLiteral("ask_questions")
#define QUESTION1       QStringLiteral("question1")
#define QUESTION2       QStringLiteral("question2")
#define QUESTION3       QStringLiteral("question3")
#define ACCESS_TYPE     QStringLiteral("access_type")

#define JSON_VALUE(k, v) k, QJsonValue(v)

CLockState::CLockState(QObject *parent) : 
    QObject(parent),
    _ids(-1),
    _sequence(""),
    _sequence_order(-1),
    _lock_nums(""),
    _description(""),
    _code1(""),
    _code2(""),
    _starttime(DEFAULT_DATETIME),
    _endtime(DEFAULT_DATETIME),
    _access_count(-1),
    _retry_count(-1),
    _max_access(-1),
    _max_retry(-1),
    _bIsNew(false),
    _bModified(false),
    _bMarkForDeletion(false),
    _bMarkForReset(false),
    _bFingerprint1(false),
    _bFingerprint2(false),
    _bAskQuestions(false),
    _question1(""),
    _question2(""),
    _question3(""),
    _access_type(ACCESS_TYPE_ALWAYS)
{
}




void CLockState::show()
{
    KCB_DEBUG_TRACE("       Id: " << _ids);
    KCB_DEBUG_TRACE("      Seq: " << _sequence);
    KCB_DEBUG_TRACE("Seq Order: " << _sequence_order);
    KCB_DEBUG_TRACE(" LockNums: " << _lock_nums);
    KCB_DEBUG_TRACE("     Desc: " << _description);
    KCB_DEBUG_TRACE("    Code1: " << _code1);
    KCB_DEBUG_TRACE("    Code2: " << _code2);
    KCB_DEBUG_TRACE("    Start: " << _starttime.toString());
    KCB_DEBUG_TRACE("      End: " << _endtime.toString());
    KCB_DEBUG_TRACE("     Stat: " << _status);
    KCB_DEBUG_TRACE("      FP1: " << _bFingerprint1);
    KCB_DEBUG_TRACE("      FP2: " << _bFingerprint2);
    KCB_DEBUG_TRACE("      Ask: " << _bAskQuestions);
    KCB_DEBUG_TRACE("       Q1: " << _question1);
    KCB_DEBUG_TRACE("       Q2: " << _question2);
    KCB_DEBUG_TRACE("       Q3: " << _question3);
    KCB_DEBUG_TRACE("       AT: " << _access_type);
    KCB_DEBUG_TRACE("       AC: " << _access_count);
    KCB_DEBUG_TRACE("       MA: " << _max_access);
    KCB_DEBUG_TRACE("       MR: " << _max_retry);
    KCB_DEBUG_TRACE("       RC: " << _retry_count);
    KCB_DEBUG_TRACE("    IsNew: " << _bIsNew);
    KCB_DEBUG_TRACE("      MOD: " << _bModified);
    KCB_DEBUG_TRACE("      MFD: " << _bMarkForDeletion);
}
