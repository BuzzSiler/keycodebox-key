#include "kcbkeyboarddialog.h"
#include "ui_kcbkeyboarddialog.h"

#include <QDebug>

#include "kcbkeyboardwidget.h"

KcbKeyboardDialog::KcbKeyboardDialog(QWidget *parent) :
    QDialog(parent),
    m_keyboard(* new KcbKeyboardWidget(this)),
    ui(new Ui::KcbKeyboardDialog)
{
    ui->setupUi(this);
    ui->vloKeyboard->addWidget(&m_keyboard);

    setWindowState(Qt::WindowFullScreen);

    connect(&m_keyboard, &KcbKeyboardWidget::NotifyClose, this, &KcbKeyboardDialog::reject);
    connect(&m_keyboard, &KcbKeyboardWidget::NotifyEnter, this, &KcbKeyboardDialog::accept);

    m_keyboard.setValue("");
}

KcbKeyboardDialog::~KcbKeyboardDialog()
{
    delete ui;
}

void KcbKeyboardDialog::setValue(const QString value)
{
    qDebug() << Q_FUNC_INFO << value;
    m_keyboard.setValue(value);
}

QString KcbKeyboardDialog::getValue()
{
    QString value = m_keyboard.getValue();
    qDebug() << Q_FUNC_INFO << value;
    return value;
}

void KcbKeyboardDialog::numbersOnly(bool state)
{
    m_keyboard.numbersOnly(state);
}
