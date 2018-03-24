#ifndef CLOCKSET_H
#define CLOCKSET_H

#include <QObject>
#include <QMap>
#include "lockstate.h"


class CLockSet : public QObject
{
    Q_OBJECT

public:
    const char *flockset = "lock set";

    explicit CLockSet(QObject *parent = 0);
    ~CLockSet();
    void clearSet();

    QJsonArray &jsonArraySet(QJsonObject &json);
    QString jsonArrayAsStringObject();  // Object containing the array of CLockState objects { "lock_set":[],[],.. }

    bool setFromJsonObject(QJsonObject &jsonObj);    // Object containing array
    bool setFromJsonString(QString strJson); // Stringified object containing array of CLockState objects "{ }"

    const QMultiMap<int, CLockState*>*  getLockMap() { return &_mmapLocks; }

    typedef QMultiMap<int, CLockState*>::Iterator   Iterator;

    Iterator    begin() { return _mmapLocks.begin(); }
    Iterator    end() { return _mmapLocks.end(); }

    void addToSet(CLockState *plockState);
private:
    QMultiMap<int, CLockState*>       _mmapLocks;     // Container keyed off lock number

signals:

public slots:
};

#endif // CLOCKSET_H
