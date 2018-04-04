#include "kcbkeyboarddialog.h"
#include "ui_kcbkeyboarddialog.h"

#include <QDebug>

#include "kcbkeyboardwidget.h"
#include "kcbutils.h"

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

    m_empty_list.clear();
    m_keyboard.setValue("", m_empty_list);
}

KcbKeyboardDialog::~KcbKeyboardDialog()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void KcbKeyboardDialog::setValue(const QString value, const QStringList codes_in_use)
{
//    qDebug() << Q_FUNC_INFO << value;
    m_keyboard.setValue(value, codes_in_use);
}

void KcbKeyboardDialog::setValue(const QString value)
{
    m_keyboard.setValue(value, m_empty_list);
}

QString KcbKeyboardDialog::getValue()
{
    QString value = m_keyboard.getValue();
//    qDebug() << Q_FUNC_INFO << value;
    return value;
}

void KcbKeyboardDialog::numbersOnly(bool state)
{
    m_keyboard.numbersOnly(state);
}
