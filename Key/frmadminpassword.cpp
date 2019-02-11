#include <QTimer>

#include "frmadminpassword.h"
#include "ui_frmadminpassword.h"

CFrmAdminPassword::CFrmAdminPassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFrmAdminPassword)
{
    ui->setupUi(this);
}

CFrmAdminPassword::~CFrmAdminPassword()
{
    delete ui;
}

void CFrmAdminPassword::on_btn_Return_clicked()
{
    // Make sure both edit boxes match
    onButtonClick(0x0D);
}

void CFrmAdminPassword::on_btn_Close_clicked()
{
    // Close - NO changes
}

void CFrmAdminPassword::enableKeypad(bool bEnable)
{
    ui->widgetKeyboard->setEnabled(bEnable);
    ui->edPassword1->setFocus();
}

void CFrmAdminPassword::OnEnableKeyboard(bool bEnable)
{
    enableKeypad(bEnable);
}

void CFrmAdminPassword::onButtonClick(char key) {
    QString qkey(key);
    QString sCurrKey;

    // Central button handling.
    // Process local
    switch(key) {
        case 0x00:  // This is clear
            OnClearCodeDisplay();
            break;
        case 0x0D:  // Enter pressed
            onPasswordEntered();
            break;
        case 0x08:  // Del (destructive backspace) pressed
            onBackSpace();
            break;
        case 0x7F:  // Del Forward
            //onDelete();
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
            sCurrKey = ui->edPassword1->text() + qkey;
            ui->edPassword1->setText(sCurrKey);
        break;
    default:    // Any
        sCurrKey = ui->edPassword1->text() + qkey;
        ui->edPassword1->setText(sCurrKey);

        break;
    }

    // Process through controller
    this->__KeyPressed(key);
}


void CFrmAdminPassword::on_btn_Clear_clicked()
{
    // Clear
    onButtonClick(0x00);
}

void CFrmAdminPassword::on_btn_Back_clicked()
{
    onButtonClick(0x08);
}


void CFrmAdminPassword::onPasswordEntered()
{
    QString sCode = ui->edPassword1->text();
//    if(sCode.length() > 0 ) {
        this->enableKeypad(false);      // disable the keypad (momentarily)
        emit __PasswordEntered(sCode);     // Signal that the code was entered.
//    }
}

void CFrmAdminPassword::onBackSpace()
{
    // Destructive Backspace
    QString sCode = ui->edPassword1->text();
    int nLen = sCode.length();
    if( nLen > 0 ) {
        sCode = sCode.left(nLen - 1);
        this->ui->edPassword1->setText(sCode);
    }
}

void CFrmAdminPassword::OnClearCodeDisplay()
{
    ui->edPassword1->clear();
}

void CFrmAdminPassword::OnNewMessage(QString sMsg, int nDurationMS)
{
    ui->edPassword1->setPlaceholderText(sMsg);
    OnClearCodeDisplay();

    // Start a timer for 5 seconds default
    QTimer::singleShot(nDurationMS, this, SLOT(ResetPlaceholderText()));
}


void CFrmAdminPassword::ResetPlaceholderText()
{
    ui->edPassword1->setPlaceholderText(tr("<Enter Password>"));
    this->OnEnableKeyboard(true);
}


void CFrmAdminPassword::on_btnA_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('A');
    }
    else {
        onButtonClick('a');
    }
}

void CFrmAdminPassword::on_btnB_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('B');
    }
    else {
        onButtonClick('b');
    }
}

void CFrmAdminPassword::on_btnC_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('C');
    }
    else {
        onButtonClick('c');
    }
}
void CFrmAdminPassword::on_btnD_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('D');
    }
    else {
        onButtonClick('d');
    }
}
void CFrmAdminPassword::on_btnE_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('E');
    }
    else {
        onButtonClick('e');
    }
}
void CFrmAdminPassword::on_btnF_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('F');
    }
    else {
        onButtonClick('f');
    }
}
void CFrmAdminPassword::on_btnG_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('G');
    }
    else {
        onButtonClick('g');
    }
}
void CFrmAdminPassword::on_btnH_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('H');
    }
    else {
        onButtonClick('h');
    }
}
void CFrmAdminPassword::on_btnI_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('I');
    }
    else {
        onButtonClick('i');
    }
}

void CFrmAdminPassword::on_btnJ_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('J');
    }
    else {
        onButtonClick('j');
    }
}

void CFrmAdminPassword::on_btnK_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('K');
    }
    else {
        onButtonClick('k');
    }
}
void CFrmAdminPassword::on_btnL_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('L');
    }
    else {
        onButtonClick('l');
    }
}
void CFrmAdminPassword::on_btnM_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('M');
    }
    else {
        onButtonClick('m');
    }
}
void CFrmAdminPassword::on_btnN_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('N');
    }
    else {
        onButtonClick('n');
    }
}
void CFrmAdminPassword::on_btnO_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('O');
    }
    else {
        onButtonClick('o');
    }

}
void CFrmAdminPassword::on_btnP_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('P');
    }
    else {
        onButtonClick('p');
    }
}
void CFrmAdminPassword::on_btnQ_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('Q');
    }
    else {
        onButtonClick('q');
    }
}
void CFrmAdminPassword::on_btnR_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('R');
    }
    else {
        onButtonClick('r');
    }
}
void CFrmAdminPassword::on_btnS_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('S');
    }
    else {
        onButtonClick('s');
    }
}
void CFrmAdminPassword::on_btnT_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('T');
    }
    else {
        onButtonClick('t');
    }
}
void CFrmAdminPassword::on_btnU_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('U');
    }
    else {
        onButtonClick('u');
    }
}
void CFrmAdminPassword::on_btnV_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('V');
    }
    else {
        onButtonClick('v');
    }
}
void CFrmAdminPassword::on_btnW_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('W');
    }
    else {
        onButtonClick('w');
    }
}
void CFrmAdminPassword::on_btnX_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('X');
    }
    else {
        onButtonClick('x');
    }
}
void CFrmAdminPassword::on_btnY_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('Y');
    }
    else {
        onButtonClick('y');
    }
}
void CFrmAdminPassword::on_btnZ_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('Z');
    }
    else {
        onButtonClick('z');
    }
}

void CFrmAdminPassword::on_btn_At_clicked()
{
    onButtonClick('@');
}

void CFrmAdminPassword::on_btn_Del_clicked()
{
    onButtonClick(0x7F);
}

void CFrmAdminPassword::on_btn_Space_clicked()
{
    onButtonClick(0x20);
}

void CFrmAdminPassword::on_btnPeriod_clicked()
{
    onButtonClick('.');
}

void CFrmAdminPassword::on_btn_Underscore_clicked()
{
    onButtonClick('_');
}

void CFrmAdminPassword::on_btn_Dash_clicked()
{
    onButtonClick('-');
}

void CFrmAdminPassword::on_btn_Exclamation_clicked()
{
    onButtonClick('!');
}

void CFrmAdminPassword::on_btn_Pound_clicked()
{
    onButtonClick('#');
}

void CFrmAdminPassword::on_btn_Caret_clicked()
{
    onButtonClick('^');
}

void CFrmAdminPassword::on_btn_Plus_clicked()
{
    onButtonClick('+');
}

void CFrmAdminPassword::on_btn_QuestionMark_clicked()
{
    onButtonClick('?');
}

void CFrmAdminPassword::on_btn_Slash_clicked()
{
    onButtonClick('/');
}

void CFrmAdminPassword::on_btn_Colon_clicked()
{
    onButtonClick(':');
}

void CFrmAdminPassword::on_btn_Semicolon_clicked()
{
    onButtonClick(';');
}

void CFrmAdminPassword::on_btn_Apostrophe_clicked()
{
    onButtonClick('\'');
}

void CFrmAdminPassword::on_btn_Quote_clicked()
{
    onButtonClick('"');
}

void CFrmAdminPassword::on_btn_Splat_clicked()
{
    onButtonClick('*');
}

void CFrmAdminPassword::on_btn_Ampersand_clicked()
{
    onButtonClick('&');
}

void CFrmAdminPassword::on_btn_Dollar_clicked()
{
    onButtonClick('$');
}

void CFrmAdminPassword::on_btn_Percent_clicked()
{
    onButtonClick('%');
}

void CFrmAdminPassword::on_btn_LeftParen_clicked()
{
    onButtonClick('(');
}

void CFrmAdminPassword::on_btn_RightParen_clicked()
{
    onButtonClick(')');
}

void CFrmAdminPassword::on_btn_0_clicked()
{
    onButtonClick('0');
}

void CFrmAdminPassword::on_btn_1_clicked()
{
    onButtonClick('1');
}

void CFrmAdminPassword::on_btn_2_clicked()
{
    onButtonClick('2');
}

void CFrmAdminPassword::on_btn_3_clicked()
{
    onButtonClick('3');
}

void CFrmAdminPassword::on_btn_4_clicked()
{
    onButtonClick('4');
}

void CFrmAdminPassword::on_btn_5_clicked()
{
    onButtonClick('5');
}

void CFrmAdminPassword::on_btn_6_clicked()
{
    onButtonClick('6');
}

void CFrmAdminPassword::on_btn_7_clicked()
{
    onButtonClick('7');
}

void CFrmAdminPassword::on_btn_8_clicked()
{
    onButtonClick('8');
}

void CFrmAdminPassword::on_btn_9_clicked()
{
    onButtonClick('9');
}

/**
 * @brief CFrmAdminPassword::on_btnShowPassword_clicked
 * toggle the password show/hide
 * @param checked
 */
void CFrmAdminPassword::on_btnShowPassword_clicked(bool checked)
{
    if(checked) {
        ui->edPassword1->setEchoMode(QLineEdit::Normal);
        ui->btnShowPassword->setText(tr("Hide"));
    } else {
        ui->edPassword1->setEchoMode(QLineEdit::Password);
        ui->btnShowPassword->setText(tr("Show"));
    }
}

void CFrmAdminPassword::on_btn_Cancel_clicked()
{
    // Close the Admin Password - this will go back to the UserCodeOne
    emit __OnAdminPasswordCancel();
}



void CFrmAdminPassword::on_btn_Back_Slash_clicked()
{
    onButtonClick('\\');
}

void CFrmAdminPassword::on_btn_OpenApost_clicked()
{
    onButtonClick('`');
}

void CFrmAdminPassword::on_btn_Tilde_clicked()
{
    onButtonClick('~');
}

void CFrmAdminPassword::on_btn_LeftAngle_clicked()
{
    onButtonClick('<');
}

void CFrmAdminPassword::on_btn_RightAngle_clicked()
{
    onButtonClick('>');
}

void CFrmAdminPassword::on_btn_LeftCurly_clicked()
{
    onButtonClick('{');
}

void CFrmAdminPassword::on_btn_RightCurly_clicked()
{
    onButtonClick('}');
}

void CFrmAdminPassword::on_btn_LeftSquare_clicked()
{
    onButtonClick('[');
}

void CFrmAdminPassword::on_btn_RightSquare_clicked()
{
    onButtonClick(']');
}

void CFrmAdminPassword::on_btn_Upper_Lower_clicked(bool checked)
{
    if(checked)
    {
        ui->btnA->setText(ui->btnA->text().toLower());
        ui->btnB->setText(ui->btnB->text().toLower());
        ui->btnC->setText(ui->btnC->text().toLower());
        ui->btnD->setText(ui->btnD->text().toLower());
        ui->btnE->setText(ui->btnE->text().toLower());
        ui->btnF->setText(ui->btnF->text().toLower());
        ui->btnG->setText(ui->btnG->text().toLower());
        ui->btnH->setText(ui->btnH->text().toLower());
        ui->btnI->setText(ui->btnI->text().toLower());
        ui->btnJ->setText(ui->btnJ->text().toLower());
        ui->btnK->setText(ui->btnK->text().toLower());
        ui->btnL->setText(ui->btnL->text().toLower());
        ui->btnM->setText(ui->btnM->text().toLower());
        ui->btnN->setText(ui->btnN->text().toLower());
        ui->btnO->setText(ui->btnO->text().toLower());
        ui->btnP->setText(ui->btnP->text().toLower());
        ui->btnQ->setText(ui->btnQ->text().toLower());
        ui->btnR->setText(ui->btnR->text().toLower());
        ui->btnS->setText(ui->btnS->text().toLower());
        ui->btnT->setText(ui->btnT->text().toLower());
        ui->btnU->setText(ui->btnU->text().toLower());
        ui->btnV->setText(ui->btnV->text().toLower());
        ui->btnW->setText(ui->btnW->text().toLower());
        ui->btnX->setText(ui->btnX->text().toLower());
        ui->btnY->setText(ui->btnY->text().toLower());
        ui->btnZ->setText(ui->btnZ->text().toLower());
    }
    else {
        ui->btnA->setText(ui->btnA->text().toUpper());
        ui->btnB->setText(ui->btnB->text().toUpper());
        ui->btnC->setText(ui->btnC->text().toUpper());
        ui->btnD->setText(ui->btnD->text().toUpper());
        ui->btnE->setText(ui->btnE->text().toUpper());
        ui->btnF->setText(ui->btnF->text().toUpper());
        ui->btnG->setText(ui->btnG->text().toUpper());
        ui->btnH->setText(ui->btnH->text().toUpper());
        ui->btnI->setText(ui->btnI->text().toUpper());
        ui->btnJ->setText(ui->btnJ->text().toUpper());
        ui->btnK->setText(ui->btnK->text().toUpper());
        ui->btnL->setText(ui->btnL->text().toUpper());
        ui->btnM->setText(ui->btnM->text().toUpper());
        ui->btnN->setText(ui->btnN->text().toUpper());
        ui->btnO->setText(ui->btnO->text().toUpper());
        ui->btnP->setText(ui->btnP->text().toUpper());
        ui->btnQ->setText(ui->btnQ->text().toUpper());
        ui->btnR->setText(ui->btnR->text().toUpper());
        ui->btnS->setText(ui->btnS->text().toUpper());
        ui->btnT->setText(ui->btnT->text().toUpper());
        ui->btnU->setText(ui->btnU->text().toUpper());
        ui->btnV->setText(ui->btnV->text().toUpper());
        ui->btnW->setText(ui->btnW->text().toUpper());
        ui->btnX->setText(ui->btnX->text().toUpper());
        ui->btnY->setText(ui->btnY->text().toUpper());
        ui->btnZ->setText(ui->btnZ->text().toUpper());
    }

}
