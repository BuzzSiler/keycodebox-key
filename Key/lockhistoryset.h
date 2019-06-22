#ifndef CLOCKHISTORYSET_H
#define CLOCKHISTORYSET_H

#include <QObject>
#include <QVector>
#include <QVectorIterator>

#include "lockhistoryrec.h"


class CLockHistorySet : public QObject
{
    Q_OBJECT
    
    public:
        const char *flockhistoryset = "lock history set";

        explicit CLockHistorySet(QObject *parent = 0);

        typedef QVectorIterator<CLockHistoryRec*> Iterator;

        void clearSet();

        const QVector<CLockHistoryRec*>*getLockHistoryMap() { return &_storage; }

        
        Iterator getIterator() { return Iterator(_storage); }

        void addToSet(CLockHistoryRec &lockHistoryRec);
    private:
        QVector<CLockHistoryRec*>  _storage;

};


#endif // CLOCKHISTORYSET_H
