#ifndef SECUREREADER_H
#define SECUREREADER_H

#include "cardreader.h"

class QObject;
class QThread;
class QProcess;

class SecureReader : public CardReader
{
    public:
        explicit SecureReader(QObject *parent = nullptr);
        ~SecureReader();
        void Connect(QThread& thread);
        void DoParserStdOut();
    protected:
        void DoScan();
        void DoParse();
        QString DoConvert(QString value);
};

#endif // SECUREREADER_H
