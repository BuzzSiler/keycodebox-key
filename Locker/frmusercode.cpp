#include <QDebug>
#include <QLineEdit>
#include <QDateTime>
#include "frmusercode.h"
#include "ui_frmusercode.h"
#include "hidreader.h"
#include <libfprint/fprint.h>
#include "dlgfingerprint.h"

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
    _dtTimer.setInterval(500);
    _dtTimer.connect(&_dtTimer, SIGNAL(timeout()), this, SLOT(OnDateTimeTimerTimeout()));
    _dtTimer.start();
    
    QString placeholderMessage = "<b><u>PLEASE READ THIS FIRST:</u></b><br /><br /><b>TO STORE AN ITEM:</b><br />Swipe any card with a magnetic stripe<br />and an empty locker will open.<br /><br /><b>TO REMOVE AN ITEM:</b><br />Swipe the same card again and the same<br />locker will open.";
    ui->lMessage->setText(placeholderMessage);
   
    if( !ui->grpKeypad->isVisible() )
      ui->lVersion->setVisible(true);
    //hideKeypad();
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
    QApplication::processEvents();
    qDebug() << "Code Entered:" << sCode;
    if(sCode.length() > 0 ) {
      //this->enableKeypad(false);      // disable the keypad (momentarily)
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
  //ui->grpKeypad->setEnabled(bEnable);
    ui->edCode->setFocus();

    //might seem unintuitive, but this function
    // enables the widget itself, not its operation.
    // doesn't ever need to be disabled.
    if(bEnable)
      hideKeypad();
}

void CFrmUserCode::OnEnableKeyboard(bool bEnable)
{
    enableKeypad(bEnable);
}

void CFrmUserCode::OnNewMessage(QString sMsg)
{
    //TODO: Disabled for now. The lblMessage has been removed but this SLOT is still called.
//   ui->lblMessage->setText(sMsg);
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
    ui->edCode->setText(sCode);
    QApplication::processEvents();
    qDebug() << "Code Entered:" << sCode;
    if(sCode.size() > 0 ) {
      //this->enableKeypad(false);      // disable the keypad (momentarily)
        emit __CodeEntered(sCode);     // Signal that the code was entered.
    }
}


void CFrmUserCode::OnNewCodeMessage(QString sCodeMsg, bool lockboxState)
{
  ui->lMessage->setText("");
  //ui->edCode->setPlaceholderText(sCodeMsg);
    OnClearCodeDisplay();

    // Start a timer for 7 seconds default
    //QTimer::singleShot(100000, this, SLOT(ResetPlaceholderText()));

    QString noLockboxesLeftMessage = "Incorrect Code";
    QString enterCodeOneMessage = "<Please Enter Code #1>";
    QString enterCodeTwoMessage = "<Please Enter Second Code>";
    
    QString inMessage = "Please place your item\ninto the locker and close door firmly.\nThank you.";
    QString outMessage = "<b><u>PLEASE REMOVE EVERYTHING NOW!</u></b><br /><br />You will NOT be able to open this box again<br />after the door is closed and locked.<br /><br />Please remove your\nitem from the locker and<br />close door firmly.\nThank you.";
    QString outOfLockboxesMessage = "Sorry, all lockers are now in use.\nPlease try again later.\nThank you.";

    qDebug() << "CFrmUserCode::OnNewCodeMessage(), lockboxState: " << lockboxState;
    
    if(lockboxState)
      {
	ui->lMessage->setText(inMessage);
      }
    else
      {
	ui->lMessage->setText(outMessage);
      }
    if( QString::compare(noLockboxesLeftMessage, sCodeMsg, Qt::CaseSensitive) == 0 )
      {
	ui->lMessage->setText(outOfLockboxesMessage);
      }
    
    QString placeholderMessage = "<b><u>PLEASE READ THIS FIRST:</u></b><br /><br /><b>TO STORE AN ITEM:</b><br />Swipe any card with a magnetic stripe<br />and an empty locker will open.<br /><br /><b>TO REMOVE AN ITEM:</b><br />Swipe the same card again and the same<br />locker will open.";
    if( QString::compare(enterCodeOneMessage, sCodeMsg, Qt::CaseSensitive) == 0  ||
	QString::compare(enterCodeTwoMessage, sCodeMsg, Qt::CaseSensitive) == 0 )
      {
	if( !ui->grpKeypad->isVisible() )
	  {
	    ui->lMessage->setText(placeholderMessage);
	    //emit __OnLockerScreen();
	    ui->lVersion->setVisible(true);
	  }
	else
	  ui->lVersion->setVisible(false);
      }

    if( QString::compare(placeholderMessage, ui->lMessage->text(), Qt::CaseSensitive) != 0)
      ui->lVersion->setVisible(false);
}

void CFrmUserCode::ResetPlaceholderText()
{
    this->OnEnableKeyboard(false);
    
    QString placeholderMessage = "<b><u>PLEASE READ THIS FIRST:</u></b><br /><br /><b>TO STORE AN ITEM:</b><br />Swipe any card with a magnetic stripe<br />and an empty locker will open.<br /><br /><b>TO REMOVE AN ITEM:</b><br />Swipe the same card again and the same<br />locker will open.";
    ui->lMessage->setText(placeholderMessage);

    //hideKeypad*();
    
    if( !ui->grpKeypad->isVisible() )
      ui->lVersion->setVisible(true);
    
}

void CFrmUserCode::showKeypad()
{
  ui->lVersion->setVisible(false);
  ui->lMessage->setVisible(false);
  ui->btn_1->setVisible(true);
  ui->btn_2->setVisible(true);
  ui->btn_3->setVisible(true);
  ui->btn_4->setVisible(true);
  ui->btn_5->setVisible(true);
  ui->btn_6->setVisible(true);
  ui->btn_7->setVisible(true);
  ui->btn_8->setVisible(true);
  ui->btn_9->setVisible(true);
  ui->btn_0->setVisible(true);
  ui->btn_Clear->setVisible(true);
  ui->btn_Return->setVisible(true);
  ui->btn_Del->setVisible(true);
  ui->btn_Cancel->setVisible(true);
  ui->btnShowPassword->setVisible(true);
  ui->edCode->setVisible(true);
  ui->lblDateTime->setVisible(true);
  ui->frame->setVisible(true);
  ui->grpKeypad->setVisible(true);
}

void CFrmUserCode::hideKeypad()
{
  ui->lMessage->setVisible(true);
  ui->btn_1->setVisible(false);
  ui->btn_2->setVisible(false);
  ui->btn_3->setVisible(false);
  ui->btn_4->setVisible(false);
  ui->btn_5->setVisible(false);
  ui->btn_6->setVisible(false);
  ui->btn_7->setVisible(false);
  ui->btn_8->setVisible(false);
  ui->btn_9->setVisible(false);
  ui->btn_0->setVisible(false);
  ui->btn_Clear->setVisible(false);
  ui->btn_Return->setVisible(false);
  ui->btn_Del->setVisible(false);
  ui->btn_Cancel->setVisible(false);
  ui->btnShowPassword->setVisible(false);
  ui->edCode->setVisible(false);
  ui->lblDateTime->setVisible(false);
  ui->frame->setVisible(false);
  ui->grpKeypad->setVisible(false);
  emit __OnLockerScreen();
}

void CFrmUserCode::on_lVersion_clicked()
{
  showKeypad();
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

void CFrmUserCode::on_btnShowPassword_clicked(bool checked)
{
    //
    if(checked) {
        ui->edCode->setEchoMode(QLineEdit::Normal);
        ui->btnShowPassword->setText("Hide");
    } else {
        ui->edCode->setEchoMode(QLineEdit::Password);
        ui->btnShowPassword->setText("Show");
    }
}

void CFrmUserCode::on_btn_Cancel_clicked()
{
    //
    emit __OnUserCodeCancel();
    hideKeypad();
    ui->lVersion->setVisible(true);
}

void CFrmUserCode::on_btnIdentifyFingerPrint_clicked()
{
  qDebug() << "CFrmUserCode::on_IdentifyFingerPrint_clicked()";

  emit __onVerifyFingerprintDialog();
  emit __onVerifyFingerprint();
  
}
