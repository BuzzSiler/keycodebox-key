#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QJsonValue>
#include <QDebug>
#include "lockhistoryrec.h"

CLockHistoryRec::CLockHistoryRec(CLockState *parent) : CLockState(parent)
{

}

QJsonObject &CLockHistoryRec::jsonRecord(QJsonObject &json)
{
    CLockState::jsonRecord(json);
    json.insert(faccess_time, QJsonValue(_access_time.toString("yyyy-MM-dd HH:mm:ss")));    // Datetime as text
    json.insert(fadmin_notification_sent, QJsonValue(_adminNotificationSent));
    json.insert(fuser_notification_email, QJsonValue(_userNotificationEmail));
    json.insert(fuser_notification_sent, QJsonValue(_userNotificationSent));

    return json;

}

QString CLockHistoryRec::jsonRecordAsString()
{
    QJsonObject jsonObj;
    jsonObj = this->jsonRecord(jsonObj);
    QJsonDocument doc(jsonObj);
    QString str(doc.toJson(QJsonDocument::Compact));
    return str;
}

void CLockHistoryRec::setFromLockState(CLockState &newLockState)
{
    _ids = newLockState.getID();            // Record id // integer primary key unique, (if -1 then this is a new record)
    _sequence = newLockState.getSequence();    // Sequence // text,
    _sequence_order = newLockState.getSequenceOrder();
    _lock_nums = newLockState.getLockNums();
    _description = newLockState.getDescription();
    _code1 = newLockState.getCode1();
    _code2 = newLockState.getCode2();
    _starttime = newLockState.getStartTime();
    _endtime = newLockState.getEndTime();
    _status = newLockState.getStatus();
    _access_count = newLockState.getAccessCount();
    _retry_count = newLockState.getRetryCount();
    _max_access = newLockState.getMaxAccess();
    _max_retry = newLockState.getMaxRetry();
    _bModified = newLockState.isModified();
    _bMarkForDeletion = newLockState.isMarkedForDeletion();

    _access_time = QDateTime::currentDateTime();
    _adminNotificationSent = false;
    _userNotificationEmail = "";
    _userNotificationSent = false;

    _question1 = newLockState.getQuestion1();
    _question2 = newLockState.getQuestion2();
    _question3 = newLockState.getQuestion3();
}

bool CLockHistoryRec::setFromJsonObject(QJsonObject jsonObj)
{
    //
    try {
        if(!jsonObj.value(faccess_time).isUndefined())
        {            
            _access_time.fromString(jsonObj.value(faccess_time).toString(), "yyyy-MM-dd HH:mm:ss");
        }        
        if(!jsonObj.value(fadmin_notification_sent).isUndefined())
        {            
            _adminNotificationSent = jsonObj.value(fadmin_notification_sent).toBool();
        }        
        if(!jsonObj.value(fuser_notification_email).isUndefined())
        {            
            _userNotificationEmail = jsonObj.value(fuser_notification_email).toString();
        }        
        if(!jsonObj.value(fuser_notification_sent).isUndefined())
        {            
            _userNotificationSent = jsonObj.value(fuser_notification_sent).toBool();
        }
        return CLockState::setFromJsonObject(jsonObj);
    } catch(std::exception &e) {
        qDebug() << "CLockHistoryRec::setFromJsonObject():" << e.what();
        return false;
    }

    return true;
}

bool CLockHistoryRec::setFromJsonString(QString strJson)
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
