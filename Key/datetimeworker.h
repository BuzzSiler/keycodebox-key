#ifndef DATETIMEWORKER_H
#define DATETIMEWORKER_H

#include <QObject>

class DateTimeWorker : public QObject
{
    Q_OBJECT
    public:
        explicit DateTimeWorker(QObject *parent = nullptr);

    private slots:
        void onTimeout();
};

#endif // DATETIMEWORKER_H
