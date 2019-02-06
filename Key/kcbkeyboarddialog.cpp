#include "kcbkeyboarddialog.h"
#include "ui_kcbkeyboarddialog.h"

#include <QDebug>

#include "kcbkeyboardwidget.h"
#include "kcbutils.h"
#include "kcbcommon.h"
#include "kcbsystem.h"

KcbKeyboardDialog::KcbKeyboardDialog(QWidget *parent, bool for_password) :
    QDialog(parent),
    m_keyboard(* new KcbKeyboardWidget(this, for_password)),
    ui(new Ui::KcbKeyboardDialog)
{
    ui->setupUi(this);

    kcb::SetWindowParams(this);

    ui->hloKeyboard->addWidget(&m_keyboard);


    connect(&m_keyboard, SIGNAL(NotifyClose()), this, SLOT(reject()));
    if (for_password)
    {
        connect(&m_keyboard, SIGNAL(NotifyEnter()), this, SLOT(Accept()));
    }
    else
    {
        connect(&m_keyboard, SIGNAL(NotifyEnter()), this, SLOT(accept()));
    }
    connect(&m_keyboard, SIGNAL(NotifyClose()), this, SIGNAL(NotifyCancelled()));

    m_empty_list.clear();
    m_keyboard.clear();
}

KcbKeyboardDialog::~KcbKeyboardDialog()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void KcbKeyboardDialog::setValue(const QString value,
                                 const QStringList codes_in_use)
{
    KCB_DEBUG_ENTRY;
    m_keyboard.setValue(value, codes_in_use);
    KCB_DEBUG_EXIT;
}

void KcbKeyboardDialog::setValue(const QString value)
{
    m_keyboard.setValue(value, m_empty_list);
}

QString KcbKeyboardDialog::getValue()
{
    return m_keyboard.getValue();
}

void KcbKeyboardDialog::numbersOnly(bool state)
{
    m_keyboard.numbersOnly(state);
}

void KcbKeyboardDialog::ipAddress(bool state)
{
    m_keyboard.ipAddress(state);
}

void KcbKeyboardDialog::ClearText()
{
    KCB_DEBUG_ENTRY;
    m_keyboard.clear();
    KCB_DEBUG_EXIT;
}

void KcbKeyboardDialog::Accept()
{
    KCB_DEBUG_ENTRY;
    QString value = m_keyboard.getValue();
    emit NotifyEntered(value);
    KCB_DEBUG_EXIT;
}

void KcbKeyboardDialog::resetPassword()
{
    m_keyboard.resetPassword();
}

void KcbKeyboardDialog::invalidPassword()
{
    m_keyboard.invalidPassword();
}