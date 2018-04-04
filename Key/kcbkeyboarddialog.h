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

        void setValue(const QString value,
                      const QObject *sender = nullptr,
                      const char *signal = nullptr);
        void setValue(const QString value,
                      const QStringList codes_in_use,
                      const QObject *sender = nullptr,
                      const char *signal = nullptr);
        QString getValue();
        void numbersOnly(bool state);

    private:
        KcbKeyboardWidget& m_keyboard;
        QStringList m_empty_list;
        Ui::KcbKeyboardDialog *ui;


};

#endif // KCBKEYBOARDDIALOG_H
