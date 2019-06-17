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

        const QMultiMap<QString, CLockState*>*  getLockMap() { return &_mmapLocks; }


        Iterator    begin() { return _mmapLocks.begin(); }
        Iterator    end() { return _mmapLocks.end(); }

        void addToSet(CLockState *plockState);
        bool isValid() {return _mmapLocks.count() > 0;}
        
    private:
        QMultiMap<QString, CLockState*> _mmapLocks;

};

#endif // CLOCKSET_H
