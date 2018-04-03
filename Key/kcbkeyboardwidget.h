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
        explicit KcbKeyboardWidget(QWidget *parent = 0);
        ~KcbKeyboardWidget();

        void setValue(const QString value);
        QString getValue();
        void numbersOnly(bool state);

    signals:
        void NotifyClose();
        void NotifyEnter();

    private slots:
        void digitClicked(QString value);
        void alphaClicked(QString value);
        void controlClicked(QString value);

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

        Ui::KcbKeyboardWidget *ui;

        void updateValue(QString value);
        void updateUi();

};

#endif // KCBKEYBOARDWIDGET_H