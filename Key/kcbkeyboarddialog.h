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
        explicit KcbKeyboardDialog(QWidget *parent = 0);
        ~KcbKeyboardDialog();

        void setValue(const QString value);
        void setValue(const QString value,
                      const QStringList codes_in_use);
        QString getValue();
        void numbersOnly(bool state);
        void ipAddress(bool state);

    private:
        KcbKeyboardWidget& m_keyboard;
        QStringList m_empty_list;
        Ui::KcbKeyboardDialog *ui;


};

#endif // KCBKEYBOARDDIALOG_H
