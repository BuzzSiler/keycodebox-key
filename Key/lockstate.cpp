#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QString>
#include <QVector>
#include <exception>
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
    _starttime(_DATENONE),
    _endtime(_DATENONE),
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

QJsonObject &CLockState::jsonRecord(QJsonObject &json)
{
    json.insert(JSON_VALUE(IDS, _ids));
    json.insert(SEQUENCE, QJsonValue(_sequence));
    json.insert(SEQUENCE_ORDER, QJsonValue(_sequence_order));
    json.insert(LOCKNUMS, QJsonValue(_lock_nums)); 
    json.insert(DESCRIPTION, QJsonValue(_description));
    json.insert(CODE1, QJsonValue(_code1));
    json.insert(CODE2, QJsonValue(_code2));    
    json.insert(STARTTIME, QJsonValue(_starttime.toString(DATETIME_FORMAT)));    
    json.insert(ENDTIME, QJsonValue(_endtime.toString(DATETIME_FORMAT)));    
    json.insert(STATUS, QJsonValue(_status));
    json.insert(ACCESS_COUNT, QJsonValue(_access_count));
    json.insert(RETRY_COUNT, QJsonValue(_retry_count));
    json.insert(MAX_ACCESS, QJsonValue(_max_access));
    json.insert(MAX_RETRY, QJsonValue(_max_retry));

    json.insert(MODIFIED, QJsonValue(_bModified));
    json.insert(DELETE, QJsonValue(_bMarkForDeletion));

    json.insert(FINGERPRINT1, QJsonValue(_bFingerprint1));
    json.insert(FINGERPRINT2, QJsonValue(_bFingerprint2));

    json.insert(ASK_QUESTIONS, QJsonValue(_bAskQuestions));
    json.insert(QUESTION1, QJsonValue(_question1));
    json.insert(QUESTION2, QJsonValue(_question2));
    json.insert(QUESTION3, QJsonValue(_question3));
    json.insert(ACCESS_TYPE, QJsonValue(_access_type));
    
    return json;
}

QString CLockState::jsonRecordAsString()
{
    QJsonObject jsonObj;
    jsonObj = this->jsonRecord(jsonObj);
    QJsonDocument doc(jsonObj);
    QString str(doc.toJson(QJsonDocument::Compact));
    return str;
}
        
bool CLockState::setFromJsonObject(QJsonObject jsonObj)
{
    try 
    {
        if(!jsonObj.value(IDS).isUndefined())
		{
            _ids = jsonObj.value(IDS).toInt();
		}
        if(!jsonObj.value(SEQUENCE).isUndefined())
		{
            _sequence = jsonObj.value(SEQUENCE).toString();
		}
        if(!jsonObj.value(SEQUENCE_ORDER).isUndefined())
		{
            _sequence_order = jsonObj.value(SEQUENCE_ORDER).toInt();
		}
        if(!jsonObj.value(LOCKNUMS).isUndefined())
		{
            _lock_nums = jsonObj.value(LOCKNUMS).toString();
		}
        if(!jsonObj.value(DESCRIPTION).isUndefined())
		{
            _description = jsonObj.value(DESCRIPTION).toString();
		}
        if(!jsonObj.value(CODE1).isUndefined())
		{
            _code1 = jsonObj.value(CODE1).toString();
		}
        if(!jsonObj.value(CODE2).isUndefined())
		{
            _code2 = jsonObj.value(CODE2).toString();
		}
        if(!jsonObj.value(STARTTIME).isUndefined())
		{
            _starttime.fromString(jsonObj.value(STARTTIME).toString(), DATETIME_FORMAT);
		}
        if(!jsonObj.value(ENDTIME).isUndefined())
		{
            _endtime.fromString(jsonObj.value(ENDTIME).toString(), DATETIME_FORMAT);
		}
        if(!jsonObj.value(STATUS).isUndefined())
		{
            _status = jsonObj.value(STATUS).toString();
		}
        if(!jsonObj.value(ACCESS_COUNT).isUndefined())
		{
            _access_count = jsonObj.value(ACCESS_COUNT).toInt();
		}
        if(!jsonObj.value(RETRY_COUNT).isUndefined())
		{
            _retry_count = jsonObj.value(RETRY_COUNT).toInt();
		}
        if(!jsonObj.value(MAX_ACCESS).isUndefined())
		{
            _max_access = jsonObj.value(MAX_ACCESS).toInt();
		}
        if(!jsonObj.value(MAX_RETRY).isUndefined())
		{
            _max_retry = jsonObj.value(MAX_RETRY).toInt();
		}
        if(!jsonObj.value(MODIFIED).isUndefined())
		{
            _bModified = jsonObj.value(MODIFIED).toBool();
		}
        if(!jsonObj.value(DELETE).isUndefined())
		{
            _bMarkForDeletion = jsonObj.value(DELETE).toBool();
		}
        if(!jsonObj.value(FINGERPRINT1).isUndefined())
		{
            _bFingerprint1 = jsonObj.value(FINGERPRINT1).toBool();
		}
        if(!jsonObj.value(FINGERPRINT2).isUndefined())
		{
            _bFingerprint2 = jsonObj.value(FINGERPRINT2).toBool();
		}
        if(!jsonObj.value(ASK_QUESTIONS).isUndefined())
		{
            _bAskQuestions = jsonObj.value(ASK_QUESTIONS).toBool();
		}
        if(!jsonObj.value(QUESTION1).isUndefined())
		{
            _question1 = jsonObj.value(QUESTION1).toString();
		}
        if(!jsonObj.value(QUESTION2).isUndefined())
		{
            _question2 = jsonObj.value(QUESTION2).toString();
		}
        if(!jsonObj.value(QUESTION3).isUndefined())
		{
            _question3 = jsonObj.value(QUESTION3).toString();
		}
        if(!jsonObj.value(ACCESS_TYPE).isUndefined())
		{
            _access_type = jsonObj.value(ACCESS_TYPE).toInt();
		}
    } 
    catch(std::exception &e) 
    {
        qDebug() << "CLockState::setFromJsonObject():" << e.what();
        return false;
    }

    return true;
}

bool CLockState::setFromJsonString(QString strJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8());
    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            setFromJsonObject(doc.object());
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
            return false;
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << strJson << endl;
        return false;
    }
    return true;
}

void CLockState::show()
{
    qDebug() << "       Id: " << _ids;
    qDebug() << "      Seq: " << _sequence;
    qDebug() << "Seq Order: " << _sequence_order;
    qDebug() << " LockNums: " << _lock_nums;
    qDebug() << "     Desc: " << _description;
    qDebug() << "    Code1: " << _code1;
    qDebug() << "    Code2: " << _code2;
    qDebug() << "    Start: " << _starttime.toString();
    qDebug() << "      End: " << _endtime.toString();
    qDebug() << "     Stat: " << _status;
    qDebug() << "      FP1: " << _bFingerprint1;
    qDebug() << "      FP2: " << _bFingerprint2;
    qDebug() << "      Ask: " << _bAskQuestions;
    qDebug() << "       Q1: " << _question1;
    qDebug() << "       Q2: " << _question2;
    qDebug() << "       Q3: " << _question3;
    qDebug() << "       AT: " << _access_type;
    qDebug() << "       AC: " << _access_count;
    qDebug() << "       MA: " << _max_access;
    qDebug() << "       MR: " << _max_retry;
    qDebug() << "       RC: " << _retry_count;
    qDebug() << "    IsNew: " << _bIsNew;
    qDebug() << "      MOD: " << _bModified;
    qDebug() << "      MFD: " << _bMarkForDeletion;
}
