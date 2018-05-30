#include <QDebug>
#include <QLineEdit>
#include <QDateTime>
#include "frmusercode.h"
#include "ui_frmusercode.h"
#include "dlgfingerprint.h"
#include "version.h"
#include "kcbcommon.h"
#include "kcbapplication.h"



CFrmUserCode::CFrmUserCode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFrmUserCode)
{
    ui->setupUi(this);
    CFrmUserCode::showFullScreen();
    initialize();
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
    DisableControls();

    kcb::Application::clearAccessSelection();
}

void CFrmUserCode::DisableControls()
{
    ui->grpKeypad->setDisabled(true);
    ui->btnShowHideCode->setDisabled(true);
    ui->btnIdentifyFingerPrint->setDisabled(true);
    ui->edCode->setDisabled(true);
}

void CFrmUserCode::EnableControls()
{
    ui->grpKeypad->setEnabled(true);
    ui->btnShowHideCode->setEnabled(true);
    ui->btnIdentifyFingerPrint->setEnabled(true);
    ui->edCode->setEnabled(true);
}

void CFrmUserCode::OnDateTimeTimerTimeout()
{
    QDateTime   dt = QDateTime::currentDateTime();
    ui->lblDateTime->setText(dt.toString("MM/dd/yyyy HH:mm:ss"));
}

void CFrmUserCode::onButtonClick(char key)
{
    //KCB_DEBUG_ENTRY;

    QString qkey(key);
    QString sCurrKey;
    // Central button handling.
    // Process local
    switch(key) {
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
    KCB_DEBUG_TRACE("Code Entered:" << sCode);

    QString quit_code = QStringLiteral("123456789987654321");
    if (sCode == quit_code)
    {
        KCB_DEBUG_TRACE("shutting down!");
        std::exit(1);
    }
    QApplication::processEvents();    
    if(sCode.length() > 0 )
    {
        this->enableKeypad(false);     // disable the keypad (momentarily)
        emit __CodeEntered(sCode);     // Signal that the code was entered.
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
        this->ui->edCode->setText(sCode);
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

void CFrmUserCode::OnNewMessage(QString sMsg)
{
    Q_UNUSED(sMsg);
    // TODO: Disabled for now. The lblMessage has been removed but this SLOT is still called.
}

void CFrmUserCode::OnClearCodeDisplay()
{
    ui->edCode->clear();
    ui->edCode->setText("");
}

void CFrmUserCode::OnSwipeCode(QString sCode)
{
    ui->edCode->setText(sCode);
    QApplication::processEvents();
    qDebug() << "Code Entered:" << sCode;
    if(sCode.size() > 0 ) 
    {
        emit __CodeEntered(sCode);
    }
}

void CFrmUserCode::OnNewCodeMessage(QString sCodeMsg)
{
    ui->edCode->setPlaceholderText(sCodeMsg);
    OnClearCodeDisplay();
    // Start a timer for 4 seconds default
    //QTimer::singleShot(4000, this, SLOT(ResetPlaceholderText()));
    ui->edCode->setFocus();
}

void CFrmUserCode::ResetPlaceholderText()
{
    this->OnEnableKeyboard(true);
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
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE(state);
    ui->btnIdentifyFingerPrint->setVisible(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::SetDisplayShowHideButton(bool state)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE(state);
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

    ui->btnShowHideCode->setVisible(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::SetDisplayTakeReturnButtons(bool state)
{
    KCB_DEBUG_ENTRY;
    KCB_DEBUG_TRACE(state);
    ui->pbTake->setVisible(state);
    ui->pbReturn->setVisible(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::show(bool enabled)
{
    KCB_DEBUG_ENTRY;

    if (ui->pbTake->isVisible() && ui->pbReturn->isVisible())
    {
        enabled == true ? EnableControls() : DisableControls();
    }
    else
    {
        EnableControls();
    }

    QDialog::show();
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnDisplayButtonsUpdate(bool dispFp, bool dispShowHide, bool dispTakeReturn)
{
    KCB_DEBUG_ENTRY;
    ui->btnIdentifyFingerPrint->setVisible(dispFp);
    ui->btnShowHideCode->setVisible(dispShowHide);
    ui->pbTake->setVisible(dispTakeReturn);
    ui->pbReturn->setVisible(dispTakeReturn);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::hide()
{
    DisableControls();
    QDialog::hide();
}

void CFrmUserCode::OnDisplayFingerprintButton(bool state)
{
    KCB_DEBUG_ENTRY;
    SetDisplayFingerprintButton(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnDisplayShowHideButton(bool state)
{
    KCB_DEBUG_ENTRY;
    SetDisplayShowHideButton(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::OnDisplayTakeReturnButtons(bool state)
{
    KCB_DEBUG_ENTRY;
    SetDisplayTakeReturnButtons(state);
    KCB_DEBUG_EXIT;
}

void CFrmUserCode::on_pbTake_clicked()
{
    kcb::Application::setAccessSelection(kcb::ACCESS_TAKE);
    EnableControls();    
}

void CFrmUserCode::on_pbReturn_clicked()
{
    kcb::Application::setAccessSelection(kcb::ACCESS_RETURN);
    EnableControls();
}
