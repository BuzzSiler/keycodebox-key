#ifndef KCBKEYBOARDWIDGET_H
#define KCBKEYBOARDWIDGET_H

#include <QWidget>

namespace Ui {
    class KcbKeyboardWidget;
}

class QSignalMapper;
class QPushButton;
class QString;

class KcbKeyboardWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit KcbKeyboardWidget(QWidget *parent = 0, bool for_password = false);
        ~KcbKeyboardWidget();

        void setValue(const QString value,
                      const QStringList codes_in_use);
        QString getValue();
        void clear();
        void numbersOnly(bool state);
        void ipAddress(bool state);
        void resetPassword();
        void invalidPassword();

    signals:
        void NotifyClose();
        void NotifyEnter();

    public slots:
        void OnTextChanged(QString text);
        
    private slots:
        void digitClicked(QString value);
        void alphaClicked(QString value);
        void controlClicked(QString value);
        void OnReturnClicked();
        void on_pbCtrlShowHide_clicked();

private:
        typedef void (*SLOT_PTR_TYPE)(QWidget *);

        QList<QPushButton *> m_digit;
        QList<QPushButton *> m_alpha;
        QList<QPushButton *> m_control;
        QList<QPushButton *> m_letters;
        QSignalMapper& m_digit_mapper;
        QSignalMapper& m_alpha_mapper;
        QSignalMapper& m_ctrl_mapper;

        QString m_value;
        QStringList m_codes_in_use;
        bool m_for_password;

        Ui::KcbKeyboardWidget *ui;

        void updateValue(QString value);
        void updateUi();
        void hidePlaceholders();
        void retainAndHide(QWidget* widget);


};

#endif // KCBKEYBOARDWIDGET_H
