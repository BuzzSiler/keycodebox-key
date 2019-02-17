#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>

class QThread;
class QProcess;

class Scanner : public QObject
{
    Q_OBJECT
    public:
        explicit Scanner(QObject *parent = nullptr);
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

#endif // SCANNER_H
