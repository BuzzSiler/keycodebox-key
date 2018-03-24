#include <QDebug>
#include <QJsonDocument>

#include "lockstate.h"
#include "lockhistoryset.h"

CLockHistorySet::CLockHistorySet(QObject *parent) : QObject(parent)
{

}

void CLockHistorySet::clearSet()
{
    CLockHistoryRec  *pLockHistoryRec;
    Iterator itor;
    for(itor=begin();itor!=end();itor++)
    {
        pLockHistoryRec = itor.value();
        delete pLockHistoryRec;
    }
    _mmapLocks.clear();
}

void CLockHistorySet::addToSet(CLockHistoryRec &lockHistoryRec)
{
    _mmapLocks.insert(lockHistoryRec.getLockNums(), &lockHistoryRec);
}

bool CLockHistorySet::setFromJsonObject(QJsonObject &jsonObj)
{
    QJsonArray  ary;
    QJsonObject obj;
    QJsonValue  val;
    QJsonArray::Iterator    itor;

    // Clear
    clearSet();
    //
    try {
        if((jsonObj[flockhistoryset].isUndefined()) || (!jsonObj[flockhistoryset].isArray()))
            return false;
        val = jsonObj[flockhistoryset]; // array of
        if(val.isArray()) {
            ary = val.toArray();
            for(itor = ary.begin(); itor != ary.end(); itor++)
            {
                val = *itor;
                if(val.isObject()) {
                    obj = val.toObject();
                    CLockHistoryRec  *plockHistoryRec = new CLockHistoryRec();
                    plockHistoryRec->setFromJsonObject(obj);
                    addToSet(*plockHistoryRec);
                } else
                {
                    qDebug() << "CLockHistorySet::setFromJsonObject(): bad form";
                }
            }
        } else {
            qDebug() << "CLockHistorySet::setFromJsonObject(): Not a JSON Array";
        }
    } catch(std::exception &e) {
        qDebug() << "ClockHistorySet::setFromJsonObject():" << e.what();
        return false;
    }

    return true;
}


bool CLockHistorySet::setFromJsonString(QString strJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8());
    QJsonObject     obj;
    
    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
            setFromJsonObject(obj);
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


