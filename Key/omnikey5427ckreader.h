#ifndef OMNIKEY5427CKREADER_H
#define OMNIKEY5427CKREADER_H

#include <QObject>
#include <QString>

class QThread;
class CardReader;


class Omnikey5427CKReader : public QObject
{
    Q_OBJECT
    public:
        explicit Omnikey5427CKReader(QThread* const thread, CardReader* const reader, QObject *parent = nullptr);
        void Start();
    signals:
        void __onHIDSwipeCodes(QString,QString);
    public slots:
        void OnDigitsReceived(QString);
    private:
        QThread& m_thread;
        CardReader& m_cardReader;
};

#endif // OMNIKEY5427CKREADER_H
