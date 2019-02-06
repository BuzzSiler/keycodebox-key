#ifndef KCBKEYBOARDDIALOG_H
#define KCBKEYBOARDDIALOG_H

#include <QDialog>

namespace Ui {
    class KcbKeyboardDialog;
}


class KcbKeyboardWidget;
class QStringList;

class KcbKeyboardDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit KcbKeyboardDialog(QWidget *parent = 0, bool for_password = false);
        ~KcbKeyboardDialog();

        void setValue(const QString value);
        void setValue(const QString value,
                      const QStringList codes_in_use);
        QString getValue();
        void numbersOnly(bool state);
        void ipAddress(bool state);
        void invalidPassword();
        void resetPassword();

    signals:
        void NotifyCancelled();
        void NotifyEntered(QString text);

    private slots:
        void Accept();

    public slots:
        void ClearText();

    private:
        KcbKeyboardWidget& m_keyboard;
        QStringList m_empty_list;
        Ui::KcbKeyboardDialog *ui;


};

#endif // KCBKEYBOARDDIALOG_H
