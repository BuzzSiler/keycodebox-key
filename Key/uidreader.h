#ifndef UIDREADER_H
#define UIDREADER_H

#include "cardreader.h"

class QObject;
class QThread;
class QProcess;

class UidReader : public CardReader
{
    public:
        explicit UidReader(QObject *parent = nullptr);
        ~UidReader();
        void Connect(QThread& thread);
        void DoParseStdOut();
    protected:
        void DoScan();
        void DoParse();
        QString DoConvert(QString value);
};

#endif // UIDREADER_H
