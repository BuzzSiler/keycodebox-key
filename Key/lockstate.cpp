#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QString>
#include <exception>
#include "lockstate.h"
#include "tblcodes.h"

CLockState::CLockState(QObject *parent) : QObject(parent)
{
    _bModified = false;
    _bMarkForDeletion = false;
    _access_type = 0;
}

QJsonObject &CLockState::jsonRecord(QJsonObject &json)
{
    json.insert(fids, QJsonValue(_ids));
    json.insert(fsequence, QJsonValue(_sequence.c_str()));
    json.insert(fsequence_order, QJsonValue(_sequence_order)); // int
    json.insert(flocknum, QJsonValue(_lock_num)); // text,
    json.insert(fdescription, QJsonValue(_description.c_str()));
    json.insert(fcode1, QJsonValue(_code1.c_str()));    // DATETIME,
    json.insert(fcode2, QJsonValue(_code2.c_str()));    // text,
    json.insert(fstarttime, QJsonValue(_starttime.toString("yyyy-MM-dd HH:mm:ss")));    // Datetime as text
    json.insert(fendtime, QJsonValue(_endtime.toString("yyyy-MM-dd HH:mm:ss")));    // Datetime as text
    json.insert(fstatus, QJsonValue(_status.c_str()));
    json.insert(faccess_count, QJsonValue(_access_count));
    json.insert(fretry_count, QJsonValue(_retry_count));
    json.insert(fmax_access, QJsonValue(_max_access));
    json.insert(fmax_retry, QJsonValue(_max_retry));

    json.insert(fmodified, QJsonValue(_bModified));
    json.insert(fmarktodelete, QJsonValue(_bMarkForDeletion));

    json.insert(ffingerprint1, QJsonValue(_bFingerprint1));
    json.insert(ffingerprint2, QJsonValue(_bFingerprint2));

    json.insert(fask_questions, QJsonValue(_bAskQuestions));
    json.insert(fquestion1, QJsonValue(_question1.c_str()));
    json.insert(fquestion2, QJsonValue(_question2.c_str()));
    json.insert(fquestion3, QJsonValue(_question3.c_str()));
    json.insert(faccess_type, QJsonValue(_access_type));
    
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
    try {
        if(!jsonObj.value(fids).isUndefined())
            _ids = jsonObj.value(fids).toInt();
        if(!jsonObj.value(fsequence).isUndefined())
            _sequence = jsonObj.value(fsequence).toString().toStdString();
        if(!jsonObj.value(fsequence_order).isUndefined())
            _sequence_order = jsonObj.value(fsequence_order).toInt();
        if(!jsonObj.value(flocknum).isUndefined())
            _lock_num = jsonObj.value(flocknum).toInt();
        if(!jsonObj.value(fdescription).isUndefined())
            _description = jsonObj.value(fdescription).toString().toStdString();
        if(!jsonObj.value(fcode1).isUndefined())
            _code1 = jsonObj.value(fcode1).toString().toStdString();
        if(!jsonObj.value(fcode2).isUndefined())
            _code2 = jsonObj.value(fcode2).toString().toStdString();
        if(!jsonObj.value(fstarttime).isUndefined())
            _starttime.fromString(jsonObj.value(fstarttime).toString(), "yyyy-MM-dd HH:mm:ss");
        if(!jsonObj.value(fendtime).isUndefined())
            _endtime.fromString(jsonObj.value(fendtime).toString(), "yyyy-MM-dd HH:mm:ss");
        if(!jsonObj.value(fstatus).isUndefined())
            _status = jsonObj.value(fstatus).toString().toStdString();
        if(!jsonObj.value(faccess_count).isUndefined())
            _access_count = jsonObj.value(faccess_count).toInt();
        if(!jsonObj.value(fretry_count).isUndefined())
            _retry_count = jsonObj.value(fretry_count).toInt();
        if(!jsonObj.value(fmax_access).isUndefined())
            _max_access = jsonObj.value(fmax_access).toInt();
        if(!jsonObj.value(fmax_retry).isUndefined())
            _max_retry = jsonObj.value(fmax_retry).toInt();
        if(!jsonObj.value(fmodified).isUndefined())
            _bModified = jsonObj.value(fmodified).toBool();
        if(!jsonObj.value(fmarktodelete).isUndefined())
            _bMarkForDeletion = jsonObj.value(fmarktodelete).toBool();
        if(!jsonObj.value(ffingerprint1).isUndefined())
            _bFingerprint1 = jsonObj.value(ffingerprint1).toBool();
        if(!jsonObj.value(ffingerprint2).isUndefined())
            _bFingerprint2 = jsonObj.value(ffingerprint2).toBool();
        if(!jsonObj.value(fask_questions).isUndefined())
            _bAskQuestions = jsonObj.value(fask_questions).toBool();
        if(!jsonObj.value(fquestion1).isUndefined())
            _question1 = jsonObj.value(fquestion1).toString().toStdString();
        if(!jsonObj.value(fquestion2).isUndefined())
            _question2 = jsonObj.value(fquestion2).toString().toStdString();
        if(!jsonObj.value(fquestion3).isUndefined())
            _question3 = jsonObj.value(fquestion3).toString().toStdString();
        if(!jsonObj.value(faccess_type).isUndefined())
            _access_type = jsonObj.value(faccess_type).toInt();
    } catch(std::exception &e) {
        qDebug() << "CLockState::setFromJsonObject():" << e.what();
        return false;
    }

    return true;
}

bool CLockState::setFromJsonString(std::string strJson)
{
    QString     strIn = strJson.c_str();
    QJsonDocument doc = QJsonDocument::fromJson(strIn.toUtf8());
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
        qDebug() << "Invalid JSON...\n" << strIn << endl;
        return false;
    }
    return true;
}
