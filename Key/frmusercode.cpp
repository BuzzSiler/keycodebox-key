#include <QDebug>
#include <QLineEdit>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>
#include "frmusercode.h"
#include "ui_frmusercode.h"
#include "hidreader.h"
#include <libfprint/fprint.h>
#include "dlgfingerprint.h"
#include "version.h"
#include "kcbcommon.h"
#include "kcbapplication.h"
#include "keycodeboxsettings.h"
#include "fleetwave.h"

static bool fleetwave_enabled;
static QString prompt;
static bool fleetwave_hidcard;

CFrmUserCode::CFrmUserCode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFrmUserCode),
    m_fp_state(false),
    m_showhide_state(false),
    m_takereturn_state(false)
{
    ui->setupUi(this);
    CFrmUserCode::showFullScreen();
    initialize();

    fleetwave_enabled = KeyCodeBoxSettings::isFleetwaveEnabled();
    prompt = fleetwave_enabled ? fleetwave::FleetwaveSettings::getPrompt() : USER_CODE_PROMPT;
    fleetwave_hidcard = fleetwave_enabled ? fleetwave::FleetwaveSettings::getInput() == fleetwave::FLEETWAVE_INPUT::HIDCARD : false;

}

CFrmUserCode::~CFrmUserCode()
{
    delete ui;
}

void CFrmUserCode::initialize()
{
    _dtTimer.setInterval(1000);
    _dtTimer.connect(&_dtTimer, SIGNAL(timeout()), this, SLOT(OnDateTimeTimerTimeout()));
    _dtTimer.start();
    ui->lVersion->setText(VERSION);
    QDateTime   dt = QDateTime::currentDateTime();
    ui->lblDateTime->setText(dt.toString("MM/dd/yyyy HH:mm:ss"));
    SetDisplayCodeEntryControls(false);
}

void CFrmUserCode::mousePressEvent(QMouseEvent* event)
{
    KCB_DEBUG_TRACE(event->x() << event->y());

    if (event->x() >= 830 && event->y() <= 20)
    {
        EnterAdminControl();
    }
}

void CFrmUserCode::OnDateTimeTimerTimeout()
{
    QDateTime   dt = QDateTime::currentDateTime();
    ui->lblDateTime->setText(dt.toString("MM/dd/yyyy HH:mm:ss"));
}

void CFrmUserCode::onButtonClick(char key)
{
    QString qkey(key);
    QString sCurrKey;

    switch(key)
    {
        case 0x00:  // This is clear
            OnClearCodeDisplay();
            break;
        case 0x0D:  // Enter pressed
            onCodeEntered();
            break;
        case 0x08:  // Del (destructive backspace) pressed
            onBackSpace();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            sCurrKey = ui->edCode->text() + qkey;
            ui->edCode->setText(sCurrKey);
            ui->btn_Return->setEnabled(sCurrKey.length() > 0);
            break;
        default:    // Any
            break;
    }

    // Process through controller
    this->__KeyPressed(key);
}

void CFrmUserCode::onCodeEntered()
{
    KCB_DEBUG_ENTRY;

    QString sCode = ui->edCode->text();

    QString quit_code = QStringLiteral("123456789987654321");
    if (sCode == quit_code)
    {
        KCB_DEBUG_TRACE("shutting down!");
        std::exit(1);
    }

    if (fleetwave_enabled)
    {
        if (!kcb::Application::isTakeSelection() && !kcb::Application::isReturnSelection())
        {
            KCB_DEBUG_TRACE("Neither Take nor Return have been selected, returning");
            return;
        }
    }

    QApplication::processEvents();    
    KCB_DEBUG_TRACE("Code Entered:" << sCode);
    if(sCode.length() > 0 )
    {
        SetDisplayCodeEntryControls(false);
        enableKeypad(false);

        emit __CodeEntered(sCode);
    }
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::onBackSpace()
{
    // Destructive Backspace
    QString sCode = ui->edCode->text();
    int nLen = sCode.length();
    if( nLen > 0 )
    {
        sCode = sCode.left(nLen - 1);
        ui->edCode->setText(sCode);        
        ui->btn_Return->setEnabled(sCode.length() > 0);
    }
}

void CFrmUserCode::enableKeypad(bool bEnable)
{
    Q_UNUSED(bEnable);
    ui->edCode->setFocus();
}

void CFrmUserCode::OnEnableKeyboard(bool bEnable)
{
    enableKeypad(bEnable);
}

void CFrmUserCode::OnClearCodeDisplay()
{
    KCB_DEBUG_ENTRY;
    ui->btn_Return->setDisabled(true);
    ui->edCode->clear();
    ui->edCode->setText("");
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnSwipeCode(QString sCode)
{
    KCB_DEBUG_TRACE(sCode);
    if (fleetwave_enabled)
    {
        if (!kcb::Application::isTakeSelection() && !kcb::Application::isReturnSelection())
        {
            KCB_DEBUG_TRACE("Neither Take nor Return have been selected, returning");
            return;
        }
    }

    ui->edCode->setText(sCode);
    QApplication::processEvents();
    KCB_DEBUG_TRACE("Code Entered:" << sCode);
    if(sCode.size() > 0 ) 
    {
        emit __CodeEntered(sCode);     // Signal that the code was entered.
    }
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnNewCodeMessage(QString sCodeMsg)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE(sCodeMsg);

    if (sCodeMsg == "Code 1")
    {
        if (m_takereturn_state)
        {            
            SetDisplayTakeReturnButtons(true);
            SetDisplayCodeEntryControls(false);  
            ui->edCode->setPlaceholderText(USER_CODE_TAKE_RETURN_PROMPT);
        }
        else
        {
            SetDisplayCodeEntryControls(true);
            ui->edCode->setPlaceholderText(prompt);
        }
    }
    else
    {
        bool is_thankyou = sCodeMsg.startsWith(tr("Thank You!"));
        bool is_openinglocks = sCodeMsg.startsWith(tr("Opening Locks"));
        bool is_cleared = sCodeMsg == "";
        bool is_cancelling = sCodeMsg.startsWith(tr("Cancelling"));
        bool is_incorrect = sCodeMsg.startsWith(tr("Incorrect"));

        bool enable_controls = (is_thankyou || is_openinglocks || is_cleared || is_cancelling || is_incorrect) ? false : true;
        SetDisplayCodeEntryControls(enable_controls); 
        SetDisplayTakeReturnButtons(false);
        OnClearCodeDisplay();
        ui->edCode->setPlaceholderText(sCodeMsg);
    }
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::on_btn_1_clicked()
{
    onButtonClick('1');
}

void CFrmUserCode::on_btn_2_clicked()
{
    onButtonClick('2');
}

void CFrmUserCode::on_btn_3_clicked()
{
    onButtonClick('3');
}

void CFrmUserCode::on_btn_4_clicked()
{
    onButtonClick('4');
}

void CFrmUserCode::on_btn_5_clicked()
{
    onButtonClick('5');
}

void CFrmUserCode::on_btn_6_clicked()
{
    onButtonClick('6');
}

void CFrmUserCode::on_btn_7_clicked()
{
    onButtonClick('7');
}

void CFrmUserCode::on_btn_8_clicked()
{
    onButtonClick('8');
}

void CFrmUserCode::on_btn_9_clicked()
{
    onButtonClick('9');
}

void CFrmUserCode::on_btn_0_clicked()
{
    onButtonClick('0');
}

void CFrmUserCode::on_btn_Return_clicked()
{
    onButtonClick(0x0D);
}

void CFrmUserCode::on_btn_Del_clicked()
{
    onButtonClick(0x08);
}

void CFrmUserCode::on_btn_Clear_clicked()
{
    onButtonClick(0x00);
}

void CFrmUserCode::on_btnShowHideCode_clicked(bool checked)
{
    if(checked)
    {
        ui->edCode->setEchoMode(QLineEdit::Normal);
        ui->btnShowHideCode->setText(tr("Hide"));
    }
    else
    {
        ui->edCode->setEchoMode(QLineEdit::Password);
        ui->btnShowHideCode->setText(tr("Show"));
    }
}

void CFrmUserCode::on_btn_Cancel_clicked()
{
    emit __OnUserCodeCancel();
}

void CFrmUserCode::on_btnIdentifyFingerPrint_clicked()
{
    emit __onVerifyFingerprintDialog();
    emit __onVerifyFingerprint();
}

void CFrmUserCode::SetDisplayFingerprintButton(bool state)
{
    m_fp_state = state;
    ui->btnIdentifyFingerPrint->setVisible(state);
}

void CFrmUserCode::SetDisplayShowHideButton(bool state)
{
    /* Set the Show Password button to unchecked state whenever it is checked
       so that we don't inadvertently enable showing the password.  This applies 
       to the transition of showing/hiding the Show Password button.

       If the Show Password button is checked when we make the button invisible then we want
       to not showing the password.  So, if checked uncheck and emit signal to set text and
       password display appropriately.

       If we are making the Show Password button visiable, then the button should not be checked,
       but, just in case, go ahead and check anyway.  It can't hurt.
    */
    if (ui->btnShowHideCode->isChecked())
    {
        emit ui->btnShowHideCode->click();
    }

    m_showhide_state = state;
    ui->btnShowHideCode->setVisible(state);
}

void CFrmUserCode::show()
{
    KCB_DEBUG_ENTRY;
    QDialog::show();

    SetDisplayTakeReturnButtons(m_takereturn_state);
    SetDisplayCodeEntryControls(!m_takereturn_state);
    SetDisplayFingerprintButton(m_fp_state);
    SetDisplayShowHideButton(m_showhide_state);

    if (m_takereturn_state)
    {
        ui->edCode->setPlaceholderText(tr("Select Take or Return"));
    }
    else
    {
        OnEnableKeyboard(true);
    }
    
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::SetDisplayTakeReturnButtons(bool state)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE("TakeReturn state" << state);
    m_takereturn_state = state;
    ui->pbTake->setEnabled(state);
    ui->pbReturn->setEnabled(state);
    ui->pbTake->setVisible(state);
    ui->pbReturn->setVisible(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::SetDisplayCodeEntryControls(bool state)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE("CodeEntryControls state" << state);

    bool keypad_state = fleetwave_hidcard ? false : state;
    ui->grpKeypad->setEnabled(keypad_state);
    
    ui->btnShowHideCode->setEnabled(state);
    ui->btnIdentifyFingerPrint->setEnabled(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnDisplayFingerprintButton(bool state)
{
    KCB_DEBUG_ENTRY;
    m_fp_state = state;
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnDisplayShowHideButton(bool state)
{
    KCB_DEBUG_ENTRY;
    m_showhide_state = state;
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnDisplayTakeReturnButtons(bool state)
{
    KCB_DEBUG_ENTRY;
    m_takereturn_state = state;
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::on_pbTake_clicked()
{
    KCB_DEBUG_ENTRY;
    SetDisplayCodeEntryControls(true);
    ui->pbTake->setDisabled(fleetwave_enabled);
    ui->pbReturn->setDisabled(fleetwave_enabled);
    kcb::Application::setTakeAccessSelection();
    ui->edCode->setPlaceholderText(prompt);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::on_pbReturn_clicked()
{
    KCB_DEBUG_ENTRY;
    SetDisplayCodeEntryControls(true);
    ui->pbTake->setDisabled(fleetwave_enabled);
    ui->pbReturn->setDisabled(fleetwave_enabled);    
    kcb::Application::setReturnAccessSelection();
    ui->edCode->setPlaceholderText(prompt);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::EnterAdminControl()
{
    KCB_DEBUG_TRACE("Admin Button Pressed");

    QMessageBox msgbox;

    msgbox.setWindowTitle(tr("Administrator"));
    msgbox.setText(tr("You are accessing administrative features."));
    msgbox.addButton(QMessageBox::Ok);
    msgbox.addButton(QMessageBox::Cancel);
    msgbox.setDefaultButton(QMessageBox::Cancel);

    QCheckBox* cb = new QCheckBox(tr("Check for Assistant Login"));
    cb->setChecked(false);
    msgbox.setCheckBox(cb);

    int result = msgbox.exec();
    if (result == QMessageBox::Ok)
    {
        QString mode = "Admin";
        if (cb->isChecked())
        {
            mode = "Assist";
        }
        KCB_DEBUG_TRACE("Setting mode to " << mode);
        emit __CodeEntered(mode);
    }
}
