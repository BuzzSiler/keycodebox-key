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
        typedef QMultiMap<QString, CLockState*>::Iterator   Iterator;

        explicit CLockSet(QObject *parent = 0);
        ~CLockSet();
        void clearSet();

        QJsonArray &jsonArraySet(QJsonObject &json);
        QString jsonArrayAsStringObject();

        bool setFromJsonObject(QJsonObject &jsonObj);
        bool setFromJsonString(QString strJson);

        const QMultiMap<QString, CLockState*>*  getLockMap() { return &_mmapLocks; }

        Iterator    begin() { return _mmapLocks.begin(); }
        Iterator    end() { return _mmapLocks.end(); }
        int count() { return _mmapLocks.count(); }

        void addToSet(CLockState *plockState);
    private:
        QMultiMap<QString, CLockState*> _mmapLocks;

};

#endif // CLOCKSET_H
