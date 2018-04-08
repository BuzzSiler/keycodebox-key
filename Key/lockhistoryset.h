#ifndef CLOCKHISTORYSET_H
#define CLOCKHISTORYSET_H

#include <QObject>
//#include <QMap>
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

        QJsonArray &jsonArraySet(QJsonObject &json);
        QString jsonArrayAsStringObject();  // Object containing the array of CLockHistoryRec objects { "lock_set":[],[],.. }

        bool setFromJsonObject(QJsonObject &jsonObj);    // Object containing array
        bool setFromJsonString(QString strJson); // Stringified object containing array of CLockHistoryRec objects "{ }"

        const QVector<CLockHistoryRec*>*getLockHistoryMap() { return &_storage; }

        
        Iterator getIterator() { return VIterator(_storage); }

        void addToSet(CLockHistoryRec &lockHistoryRec);
    private:
        QVector<CLockHistoryRec*>  _storage;

};


#endif // CLOCKHISTORYSET_H
