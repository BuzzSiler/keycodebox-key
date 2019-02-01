#include "kcbkeyboardwidget.h"
#include "ui_kcbkeyboardwidget.h"

#include <QDebug>
#include <QSignalMapper>
#include <QMessageBox>
#include "kcbutils.h"
#include "kcbcommon.h"

KcbKeyboardWidget::KcbKeyboardWidget(QWidget *parent) :
    QWidget(parent),
    m_digit_mapper(* new QSignalMapper(this)),
    m_alpha_mapper(* new QSignalMapper(this)),
    m_ctrl_mapper(* new QSignalMapper(this)),
    m_value(""),
    ui(new Ui::KcbKeyboardWidget)
{
    ui->setupUi(this);

    setWindowState(Qt::WindowFullScreen);

    ui->pbCtrlReturn->setDisabled(true);

    // Find buttons for capturing click event and mapping to slots to edit the value
    m_digit = this->findChildren<QPushButton *>(QRegularExpression("pbDigit.*"), Qt::FindChildrenRecursively);
    m_alpha = this->findChildren<QPushButton *>(QRegularExpression("pb(A|L).*"), Qt::FindChildrenRecursively);
    m_control = this->findChildren<QPushButton *>(QRegularExpression("pbCtrl.*"), Qt::FindChildrenRecursively);

    // Find buttons that are letters only for upper/lower case button
    m_letters = this->findChildren<QPushButton *>(QRegularExpression("pbLetter.*"), Qt::FindChildrenRecursively);

    // Map the click signal from the digit buttons to the slot via a mapper
    for (int ii = 0; ii < m_digit.length(); ++ii)
    {
        QPushButton *btn = m_digit[ii];
        connect(btn, SIGNAL(clicked()), &m_digit_mapper, SLOT(map()));
        m_digit_mapper.setMapping(btn, m_digit[ii]->text());
    }
    connect(&m_digit_mapper, SIGNAL(mapped(QString)), this, SLOT(digitClicked(QString)));

    // Map the click signal from the alpha buttons to the slot via a mapper
    for (int ii = 0; ii < m_alpha.length(); ++ii)
    {
        QPushButton *btn = m_alpha[ii];
        connect(btn, SIGNAL(clicked()), &m_alpha_mapper, SLOT(map()));
        m_alpha_mapper.setMapping(btn, m_alpha[ii]->text());
    }
    connect(&m_alpha_mapper, SIGNAL(mapped(QString)), this, SLOT(alphaClicked(QString)));

    // Map the click signal from the control buttons to the slot via a mapper
    for (int ii = 0; ii < m_control.length(); ++ii)
    {
        if (m_control[ii]->text() == tr("Enter") || m_control[ii]->text() == tr("Close"))
        {
            continue;
        }

        QPushButton *btn = m_control[ii];
        connect(btn, SIGNAL(clicked()), &m_ctrl_mapper, SLOT(map()));
        m_ctrl_mapper.setMapping(btn, m_control[ii]->text());
    }
    connect(&m_ctrl_mapper, SIGNAL(mapped(QString)), this, SLOT(controlClicked(QString)));

    connect(ui->pbCtrlClose, SIGNAL(clicked()), this, SIGNAL(NotifyClose()));
    connect(ui->pbCtrlReturn, SIGNAL(clicked()), this, SLOT(OnReturnClicked()));
    connect(ui->edText, SIGNAL(textChanged(QString)), this, SLOT(OnTextChanged(QString)));
}

KcbKeyboardWidget::~KcbKeyboardWidget()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void KcbKeyboardWidget::clear()
{
    m_codes_in_use.clear();
    m_value = "";
}

void KcbKeyboardWidget::OnTextChanged(QString text)
{
    Q_UNUSED(text);
    //KCB_DEBUG_TRACE("Text" << text);
    updateUi();
}

void KcbKeyboardWidget::setValue(const QString value,
                                 const QStringList codes_in_use)
{
    KCB_DEBUG_ENTRY;

    m_codes_in_use.clear();
    if (codes_in_use.count())
    {
        m_codes_in_use = codes_in_use;
    }
    m_value = value;
    ui->edText->setText(value);
    updateUi();    
}

QString KcbKeyboardWidget::getValue()
{
    return ui->edText->text();
}

void KcbKeyboardWidget::numbersOnly(bool state)
{
    ui->wgAlpha->setDisabled(state);
}

void KcbKeyboardWidget::ipAddress(bool state)
{
    Q_UNUSED(state);
    KCB_DEBUG_TRACE("Not implemented");
}

void KcbKeyboardWidget::updateValue(QString value)
{
    int curr_pos = ui->edText->cursorPosition();
    QString curr_text = ui->edText->text();

    curr_text = curr_text.mid(0, curr_pos) + value + curr_text.mid(curr_pos);
    ui->edText->setText(curr_text);
    ui->edText->setCursorPosition(curr_pos + 1);
    updateUi();
}

void KcbKeyboardWidget::digitClicked(QString value)
{
    updateValue(value);
}

void KcbKeyboardWidget::alphaClicked(QString value)
{
    if ( (value >= "a" && value <= "z")  || (value >= "A" && value <= "Z") )
    {
        if (ui->pbCtrlUpperLower->isChecked())
        {
            updateValue(value.toUpper());
        }
        else
        {
            updateValue(value.toLower());
        }
    }
    else
    {
        updateValue(value);
    }
}

void KcbKeyboardWidget::controlClicked(QString value)
{
    if (value == tr("Back"))
    {
        QString value = ui->edText->text();
        int curr_pos = ui->edText->cursorPosition();
        value = value.mid(0, curr_pos - 1) + value.mid(curr_pos);
        ui->edText->setText(value);
        ui->edText->setCursorPosition(curr_pos - 1);
    }
    else if (value == tr("Clear"))
    {
        ui->edText->setText("");
    }
    else if (value == tr("Lower") || value == tr("Upper"))
    {
        if ( ui->pbCtrlUpperLower->isChecked() )
        {
            foreach (auto l, m_letters)
            {
                l->setText(l->text().toUpper());
            }
            ui->pbCtrlUpperLower->setText(tr("Upper"));
        }
        else
        {
            foreach (auto l, m_letters)
            {
                l->setText(l->text().toLower());
            }
            ui->pbCtrlUpperLower->setText(tr("Lower"));
        }
    }
    else if (value == tr("Space"))
    {
        updateValue(" ");
    }

    updateUi();
}

void KcbKeyboardWidget::updateUi()
{
    ui->pbCtrlReturn->setDisabled(ui->edText->text() == m_value);
}

void KcbKeyboardWidget::OnReturnClicked()
{
    KCB_DEBUG_ENTRY;
    // Disallow duplicate codes
    if (m_codes_in_use.contains(ui->edText->text()))
    {
        (void) QMessageBox::warning(this,
                                    tr( "Duplicate Code Detected" ),
                                    tr( "You have entered a code that is already in use.\n"
                                        "Duplicate codes for 'Code 1' are not allowed.\n"
                                        "Please enter a different code."),
                                    QMessageBox::Ok);
    }
    else
    {
        KCB_DEBUG_TRACE("Emitting NotifyEnter");
        emit NotifyEnter();
    }
}
