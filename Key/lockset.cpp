#include "lockset.h"
#include <exception>
#include <string>
#include <QDebug>
#include <QJsonDocument>
#include <QString>



CLockSet::CLockSet(QObject *parent) : QObject(parent)
{

}

void CLockSet::clearSet()
{
    CLockState  *pLockState;
    Iterator itor;
    for(itor=begin();itor!=end();itor++)
    {
        pLockState = itor.value();
        delete pLockState;
    }
    _mmapLocks.clear();
}

void CLockSet::addToSet(CLockState *plockState)
{
    _mmapLocks.insertMulti(plockState->getLockNum(), plockState);
}

CLockSet::~CLockSet()
{
    clearSet();
}

bool CLockSet::setFromJsonObject(QJsonObject &jsonObj)
{
    QJsonArray  ary;
    QJsonObject obj;
    QJsonValue  val;
    QJsonArray::Iterator    itor;

    // Clear
    clearSet();
    //
    try {
        if((jsonObj[flockset].isUndefined()) || (!jsonObj[flockset].isArray()))
            return false;
        val = jsonObj[flockset]; // array of
        if(val.isArray()) {
            ary = val.toArray();
            for(itor = ary.begin(); itor != ary.end(); itor++)
            {
                val = *itor;
                if(val.isObject()) {
                    obj = val.toObject();
                    CLockState  *plockState = new CLockState();
                    plockState->setFromJsonObject(obj);
                    addToSet(plockState);
                } else
                {
                    qDebug() << "CLockSet::setFromJsonObject(): bad form";
                }
            }
        } else {
            qDebug() << "CLockSet::setFromJsonObject(): Not a JSON Array";
        }
    } catch(std::exception &e) {
        qDebug() << "ClockSet::setFromJsonObject():" << e.what();
        return false;
    }

    return true;
}


bool CLockSet::setFromJsonString(QString strJson)
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


