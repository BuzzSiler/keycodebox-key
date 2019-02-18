#ifndef CARDREADER_H
#define CARDREADER_H

#include <QObject>

class QThread;
class QProcess;
class QString;

class CardReader : public QObject
{
    Q_OBJECT
    public:
        explicit CardReader(QObject *parent = nullptr);
        virtual ~CardReader();
        QString Convert(QString value);
        virtual void Connect(QThread& thread);

    signals:
        void DigitsReceived(QString);
    public slots:
        void ParseStdOut();
        void Scan();

    protected:
        virtual void DoScan() = 0;
        virtual void DoParse() = 0;
        virtual QString DoConvert(QString value) = 0;
        QProcess& m_proc;

};

#endif // CARDREADER_H
