#ifndef KCBKEYBOARDDIALOG_H
#define KCBKEYBOARDDIALOG_H

#include <QDialog>

namespace Ui {
    class KcbKeyboardDialog;
}

class KcbKeyboardWidget;

class KcbKeyboardDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit KcbKeyboardDialog(QWidget *parent = 0);
        ~KcbKeyboardDialog();

        void setValue(const QString value);
        QString getValue();
        void numbersOnly(bool state);

    private:
        KcbKeyboardWidget& m_keyboard;
        Ui::KcbKeyboardDialog *ui;


};

#endif // KCBKEYBOARDDIALOG_H
