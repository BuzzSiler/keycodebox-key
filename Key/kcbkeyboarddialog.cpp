#include "kcbkeyboarddialog.h"
#include "ui_kcbkeyboarddialog.h"

#include <QDebug>

#include "kcbkeyboardwidget.h"
#include "kcbutils.h"
#include "kcbcommon.h"

KcbKeyboardDialog::KcbKeyboardDialog(QWidget *parent) :
    QDialog(parent),
    m_keyboard(* new KcbKeyboardWidget(this)),
    ui(new Ui::KcbKeyboardDialog)
{
    ui->setupUi(this);
    ui->vloKeyboard->addWidget(&m_keyboard);

    setWindowState(Qt::WindowFullScreen);

    connect(&m_keyboard, SIGNAL(NotifyClose()), this, SLOT(reject()));
    connect(&m_keyboard, SIGNAL(NotifyEnter()), this, SLOT(accept()));

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
    m_keyboard.setValue(value, codes_in_use);
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

