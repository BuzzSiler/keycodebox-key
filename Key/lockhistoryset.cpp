#include <QDebug>
#include <QJsonDocument>

#include "lockstate.h"
#include "lockhistoryset.h"

CLockHistorySet::CLockHistorySet(QObject *parent) : QObject(parent)
{

}

void CLockHistorySet::clearSet()
{
    foreach (auto s, _storage)
    {
        delete s;
    }
    _storage.clear();
}

void CLockHistorySet::addToSet(CLockHistoryRec &lockHistoryRec)
{
    _storage.append(&lockHistoryRec);
}

