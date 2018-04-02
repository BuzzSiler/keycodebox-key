#include <QDebug>
#include <QLineEdit>
#include <QDateTime>
#include "frmusercode.h"
#include "ui_frmusercode.h"
#include "hidreader.h"
#include <libfprint/fprint.h>
#include "dlgfingerprint.h"
#include "version.h"

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
}

void CFrmUserCode::OnDateTimeTimerTimeout()
{
    QDateTime   dt = QDateTime::currentDateTime();
    ui->lblDateTime->setText(dt.toString("MM/dd/yyyy HH:mm:ss"));
}

/**
 * @brief CFrmUserCode::onButtonClick
 * @param key
 */
void CFrmUserCode::onButtonClick(char key) {
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
    
    QString sCode = ui->edCode->text();
    qDebug() << "CFrmUserCode::onCodeEntered" << sCode;
    QApplication::processEvents();
    qDebug() << "Code Entered:" << sCode;
    if(sCode.length() > 0 )
    {
        this->enableKeypad(false);     // disable the keypad (momentarily)
        emit __CodeEntered(sCode);     // Signal that the code was entered.
    }
}

void CFrmUserCode::onBackSpace()
{
    // Destructive Backspace
    QString sCode = ui->edCode->text();
    int nLen = sCode.length();
    if( nLen > 0 ) {
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
    qDebug() << "Pre CFrmUserCode::OnClearCodeDisplay()";
    ui->edCode->clear();
    ui->edCode->setText("");
    qDebug() << "Post CFrmUserCode::OnClearCodeDisplay()";
}

/**
 * @brief CFrmUserCode::OnSwipeCode
 * @param sCode
 * This will circumvent the numbers-only entry requirement.
 */
void CFrmUserCode::OnSwipeCode(QString sCode)
{
    qDebug() << "CFrmUserCode::OnSwipeCode" << sCode;
    ui->edCode->setText(sCode);
    QApplication::processEvents();
    qDebug() << "Code Entered:" << sCode;
    if(sCode.size() > 0 ) {
        emit __CodeEntered(sCode);     // Signal that the code was entered.
    }
}

void CFrmUserCode::OnNewCodeMessage(QString sCodeMsg)
{
    ui->edCode->setPlaceholderText(sCodeMsg);
    OnClearCodeDisplay();
    // Start a timer for 4 seconds default
    QTimer::singleShot(4000, this, SLOT(ResetPlaceholderText()));
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
    if(checked) {
        ui->edCode->setEchoMode(QLineEdit::Normal);
        ui->btnShowHideCode->setText(tr("Hide"));
    } else {
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

    ui->btnShowHideCode->setVisible(state);
}

void CFrmUserCode::OnDisplayFingerprintButton(bool state)
{
    SetDisplayFingerprintButton(state);
}

void CFrmUserCode::OnDisplayShowHideButton(bool state)
{
    SetDisplayShowHideButton(state);
}



