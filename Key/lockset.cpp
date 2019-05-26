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
    _mmapLocks.insertMulti(QString("%1").arg(plockState->getLockNums(), 2, QChar('0')), plockState);
}

CLockSet::~CLockSet()
{
    clearSet();
}
