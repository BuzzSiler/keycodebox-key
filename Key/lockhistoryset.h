#ifndef CLOCKHISTORYSET_H
#define CLOCKHISTORYSET_H

#include <QObject>
#include <QMap>
#include "lockhistoryrec.h"


class CLockHistorySet : public QObject
{
    Q_OBJECT
public:
    const char *flockhistoryset = "lock history set";

    explicit CLockHistorySet(QObject *parent = 0);
    void clearSet();

    QJsonArray &jsonArraySet(QJsonObject &json);
    QString jsonArrayAsStringObject();  // Object containing the array of CLockHistoryRec objects { "lock_set":[],[],.. }

    bool setFromJsonObject(QJsonObject &jsonObj);    // Object containing array
    bool setFromJsonString(std::string strJson); // Stringified object containing array of CLockHistoryRec objects "{ }"

    const QMultiMap<int, CLockHistoryRec*>        *getLockHistoryMap() { return &_mmapLocks; }

    typedef QMultiMap<int, CLockHistoryRec*>::Iterator   Iterator;

    Iterator    begin() { return _mmapLocks.begin(); }
    Iterator    end() { return _mmapLocks.end(); }

    void addToSet(CLockHistoryRec &lockHistoryRec);
private:
    QMultiMap<int, CLockHistoryRec*>       _mmapLocks;     // Container keyed off lock number

signals:

public slots:
};


#endif // CLOCKHISTORYSET_H
