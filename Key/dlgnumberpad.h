#ifndef DLGNUMBERPAD_H
#define DLGNUMBERPAD_H

#include <QDialog>

namespace Ui {
    class DlgNumberPad;
}

class QPushButton;
class QSignalMapper;
class QString;

class DlgNumberPad : public QDialog
{
        Q_OBJECT

    public:
        explicit DlgNumberPad(QWidget *parent = 0);
        ~DlgNumberPad();

        void setValue(const QString value);
        void getValue(QString& value);

    signals:
        void NotifyAccepted();
        void NotifyRejected();

    private slots:
        void on_pbBack_clicked();
        void on_pbClear_clicked();
        void on_pbCancel_clicked();
        void on_pbEnter_clicked();
        void buttonClicked(int index);

    private:
        QList<QPushButton *> m_buttons;
        QSignalMapper& m_mapper;
        QString m_value;
        bool m_only_once;
        Ui::DlgNumberPad *ui;

};

#endif // DLGNUMBERPAD_H
