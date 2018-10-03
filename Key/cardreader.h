#ifndef CARDREADER_H
#define CARDREADER_H

#include <QObject>

class QThread;
class QProcess;

class CardReader : public QObject
{
    Q_OBJECT
    public:
        explicit CardReader(QObject *parent = nullptr);
        void Connect(QThread& thread);
    signals:
        void DigitsReceived(QString);
    private slots:
        void ParseStdOut();
    public slots:
        void Scan();

    private:
        QProcess& m_proc;

};

#endif // CARDREADER_H
