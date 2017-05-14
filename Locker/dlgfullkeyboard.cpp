#include "dlgfullkeyboard.h"
#include "ui_dlgfullkeyboard.h"

#include <QDebug>
#include <QChar>
#include <QKeyEvent>
#include <QPushButton>


CDlgFullKeyboard::CDlgFullKeyboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgFullKeyboard)
{
    ui->setupUi(this);
    CDlgFullKeyboard::showFullScreen();
    qDebug() << "CDlgFullKeyboard::CDlgFullKeyboard(QWidget *parent)";

//    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::Tool |
//                       Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    ui->edText->setFocus();

    _pmapper = new QSignalMapper(this);
    connect(_pmapper, SIGNAL(mapped(QWidget*)), SLOT(buttonClicked(QWidget*)));

    QPushButton*        pb;
    QList<QPushButton*> lstWidgets;
    QList<QPushButton*>::Iterator   itor;
    lstWidgets = ui->widgetKeyboard->findChildren<QPushButton *>();
    for(itor = lstWidgets.begin(); itor != lstWidgets.end(); itor++)
    {
        pb = (QPushButton*)(*itor);
        qDebug() << "Mapping key:" << pb->text();
        if(pb->text().length()==1) {
            _pmapper->setMapping(pb, pb);
        }
    }
}

CDlgFullKeyboard::~CDlgFullKeyboard()
{
    delete ui;
    delete _pmapper;
}

void CDlgFullKeyboard::setCurrentEdit(CCurrentEdit *pEdit)
{
    _pcurrentEdit = pEdit;
    _pcurrentEdit->setKeyboardEditLine(this->ui->edText);
    _pcurrentEdit->setKeyboardLabel(this->ui->lblCurrentEditFieldName);
}


void CDlgFullKeyboard::onDelete()
{
    // Destructive - Not yet implemented
}

const QLineEdit &CDlgFullKeyboard::getLineEdit()
{
    return *(ui->edText);
}

const QLabel &CDlgFullKeyboard::getLabel()
{
    return *(ui->lblCurrentEditFieldName);
}

void CDlgFullKeyboard::setActive()
{
    this->activateWindow();
    this->raise();
    this->ui->edText->setFocus();
    this->show();
}

void CDlgFullKeyboard::OnClearCodeDisplay()
{
    ui->edText->clear();
}

void CDlgFullKeyboard::highlightNumbers(bool bSet)
{
    ui->btn_0->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_1->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_2->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_3->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_4->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_5->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_6->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_7->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_8->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
    ui->btn_9->setStyleSheet("{ background-color: rgb(60,179,113) }: white");
}

void CDlgFullKeyboard::buttonClicked(QWidget *btn)
{

    QPushButton *pb = (QPushButton*)btn;
    qDebug() << "buttonClicked: " << pb->text();

    if(pb->text() == "Enter") {
        onButtonClick(0x0D);
    } else if(pb->text() == "Space") {
        onButtonClick(0x20);
    } else if(pb->text() == "Del") {
        onButtonClick(0x7F);
    } else if(pb->text() == "Clear") {
        onButtonClick(0x00);
    } else if(pb->text() == "Close") {
        on_btn_Close_clicked();
    } else if(pb->text() == "Back") {
        onBackSpace();
    } else {
        onButtonClick(pb->text().at(0).toLatin1());
    }

//    if ((key == Qt::Key_Enter) || (key == Qt::Key_Backspace))
//        emit specialKeyClicked(key);
//    else
//        emit keyClicked(keyToCharacter(key));
}

static QString keyToCharacter(int key)
{
//    for (int i = 0; i < layoutSize; ++i) {
//        if (keyboardLayout[i].key == key)
//            return QString::fromLatin1(keyboardLayout[i].label);
//    }

//    return QString();
}

void CDlgFullKeyboard::onButtonClick(char key) {
    QString qkey(key);
    QString sCurrKey;
    bool    bKey = true;
    // Central button handling.
    // Process local
    switch(key) {
        case 0x00:  // This is clear
            OnClearCodeDisplay();
            break;
        case 0x0D:  // Enter pressed
            onTextEntered();
            break;
        case 0x08:  // Del (destructive backspace) pressed
            onBackSpace();
            break;
        case 0x7F:  // Del Forware
            onDelete();
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
            sCurrKey = ui->edText->text() + qkey;
            ui->edText->setText(sCurrKey);
        break;
    default:    // Any
        sCurrKey = ui->edText->text() + qkey;
        int nPos;
        if(_pcurrentEdit->isNumbersOnly()) {
            qDebug() << "  >> Curr Edit is NumbersOnly";
            if(QValidator::Acceptable == _pcurrentEdit->pregExpNums->validate(sCurrKey, nPos))
                ui->edText->setText(sCurrKey);
            else
                bKey = false;
        } else {
            qDebug() << "  >> Curr Edit is any character";
            ui->edText->setText(sCurrKey);
        }
        break;
    }

    if(bKey) {
        // Process through controller
        this->__KeyPressed(key);
    }
}

void CDlgFullKeyboard::keyPressEvent( QKeyEvent* event )
{
    unsigned int n = 0;

    switch ( event->key() )
    {
    case Qt::Key_Enter:
        onButtonClick(0x0D);
        break;
    case Qt::Key_Delete:
        onButtonClick(0x08);
        break;
    case Qt::Key_Shift:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_NumLock:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
        on_btn_Upper_Lower_clicked(false);
        break;
    case Qt::Key_CapsLock:
        on_btn_Upper_Lower_clicked(true);

        break;
    default:
        onButtonClick(event->key());
    }
}

void CDlgFullKeyboard::hideKeyboard(bool bHide) {
    ui->widgetEdit->setVisible(bHide);
}

void CDlgFullKeyboard::onTextEntered()
{
    _pcurrentEdit->retrieveEditedText();
    _pcurrentEdit->stopEdit();
    qDebug() << "onTextEntered(): " << _pcurrentEdit->newText;
    emit __TextEntered(this, _pcurrentEdit);     // Signal that the code was entered.
    hide();
}

void CDlgFullKeyboard::onBackSpace()
{
    // Destructive Backspace
    QString sCode = ui->edText->text();
    int nLen = sCode.length();
    if( nLen > 0 ) {
        sCode = sCode.left(nLen - 1);
        this->ui->edText->setText(sCode);
    }
}

void CDlgFullKeyboard::on_btnA_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('A');
    }
    else {
        onButtonClick('a');
    }
}

void CDlgFullKeyboard::on_btnB_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('B');
    }
    else {
        onButtonClick('b');
    }
}

void CDlgFullKeyboard::on_btnC_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('C');
    }
    else {
        onButtonClick('c');
    }
}
void CDlgFullKeyboard::on_btnD_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('D');
    }
    else {
        onButtonClick('d');
    }
}
void CDlgFullKeyboard::on_btnE_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('E');
    }
    else {
        onButtonClick('e');
    }
}
void CDlgFullKeyboard::on_btnF_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('F');
    }
    else {
        onButtonClick('f');
    }
}
void CDlgFullKeyboard::on_btnG_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('G');
    }
    else {
        onButtonClick('g');
    }
}
void CDlgFullKeyboard::on_btnH_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('H');
    }
    else {
        onButtonClick('h');
    }
}
void CDlgFullKeyboard::on_btnI_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('I');
    }
    else {
        onButtonClick('i');
    }
}

void CDlgFullKeyboard::on_btnJ_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('J');
    }
    else {
        onButtonClick('j');
    }
}

void CDlgFullKeyboard::on_btnK_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('K');
    }
    else {
        onButtonClick('k');
    }
}
void CDlgFullKeyboard::on_btnL_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('L');
    }
    else {
        onButtonClick('l');
    }
}
void CDlgFullKeyboard::on_btnM_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('M');
    }
    else {
        onButtonClick('m');
    }
}
void CDlgFullKeyboard::on_btnN_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('N');
    }
    else {
        onButtonClick('n');
    }
}
void CDlgFullKeyboard::on_btnO_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('O');
    }
    else {
        onButtonClick('o');
    }

}
void CDlgFullKeyboard::on_btnP_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('P');
    }
    else {
        onButtonClick('p');
    }
}
void CDlgFullKeyboard::on_btnQ_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('Q');
    }
    else {
        onButtonClick('q');
    }
}
void CDlgFullKeyboard::on_btnR_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('R');
    }
    else {
        onButtonClick('r');
    }
}
void CDlgFullKeyboard::on_btnS_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('S');
    }
    else {
        onButtonClick('s');
    }
}
void CDlgFullKeyboard::on_btnT_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('T');
    }
    else {
        onButtonClick('t');
    }
}
void CDlgFullKeyboard::on_btnU_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('U');
    }
    else {
        onButtonClick('u');
    }
}
void CDlgFullKeyboard::on_btnV_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('V');
    }
    else {
        onButtonClick('v');
    }
}
void CDlgFullKeyboard::on_btnW_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('W');
    }
    else {
        onButtonClick('w');
    }
}
void CDlgFullKeyboard::on_btnX_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('X');
    }
    else {
        onButtonClick('x');
    }
}
void CDlgFullKeyboard::on_btnY_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('Y');
    }
    else {
        onButtonClick('y');
    }
}
void CDlgFullKeyboard::on_btnZ_clicked()
{
    if(!ui->btn_Upper_Lower->isChecked())
    {
        onButtonClick('Z');
    }
    else {
        onButtonClick('z');
    }
}

void CDlgFullKeyboard::on_btn_At_clicked()
{
    onButtonClick('@');
}

void CDlgFullKeyboard::on_btn_Back_clicked()
{
    onButtonClick(0x08);
}

void CDlgFullKeyboard::on_btn_Del_clicked()
{
    onButtonClick(0x7F);
}

void CDlgFullKeyboard::on_btn_Space_clicked()
{
    onButtonClick(0x20);
}

void CDlgFullKeyboard::on_btn_Return_clicked()
{
    onButtonClick(0x0D);
}

void CDlgFullKeyboard::on_btnPeriod_clicked()
{
    onButtonClick('.');
}

void CDlgFullKeyboard::on_btn_Underscore_clicked()
{
    onButtonClick('_');
}

void CDlgFullKeyboard::on_btn_Dash_clicked()
{
    onButtonClick('-');
}

void CDlgFullKeyboard::on_btn_Exclamation_clicked()
{
    onButtonClick('!');
}

void CDlgFullKeyboard::on_btn_Pound_clicked()
{
    onButtonClick('#');
}

void CDlgFullKeyboard::on_btn_Caret_clicked()
{
    onButtonClick('^');
}

void CDlgFullKeyboard::on_btn_Plus_clicked()
{
    onButtonClick('+');
}

void CDlgFullKeyboard::on_btn_QuestionMark_clicked()
{
    onButtonClick('?');
}

void CDlgFullKeyboard::on_btn_Slash_clicked()
{
    onButtonClick('/');
}

void CDlgFullKeyboard::on_btn_Colon_clicked()
{
    onButtonClick(':');
}

void CDlgFullKeyboard::on_btn_Semicolon_clicked()
{
    onButtonClick(';');
}

void CDlgFullKeyboard::on_btn_Apostrophe_clicked()
{
    onButtonClick('\'');
}

void CDlgFullKeyboard::on_btn_Quote_clicked()
{
    onButtonClick('"');
}

void CDlgFullKeyboard::on_btn_Splat_clicked()
{
    onButtonClick('*');
}

void CDlgFullKeyboard::on_btn_Ampersand_clicked()
{
    onButtonClick('&');
}

void CDlgFullKeyboard::on_btn_Dollar_clicked()
{
    onButtonClick('$');
}

void CDlgFullKeyboard::on_btn_Percent_clicked()
{
    onButtonClick('%');
}

void CDlgFullKeyboard::on_btn_LeftParen_clicked()
{
    onButtonClick('(');
}

void CDlgFullKeyboard::on_btn_RightParen_clicked()
{
    onButtonClick(')');
}

void CDlgFullKeyboard::on_btn_0_clicked()
{
    onButtonClick('0');
}

void CDlgFullKeyboard::on_btn_1_clicked()
{
    onButtonClick('1');
}

void CDlgFullKeyboard::on_btn_2_clicked()
{
    onButtonClick('2');
}

void CDlgFullKeyboard::on_btn_3_clicked()
{
    onButtonClick('3');
}

void CDlgFullKeyboard::on_btn_4_clicked()
{
    onButtonClick('4');
}

void CDlgFullKeyboard::on_btn_5_clicked()
{
    onButtonClick('5');
}

void CDlgFullKeyboard::on_btn_6_clicked()
{
    onButtonClick('6');
}

void CDlgFullKeyboard::on_btn_7_clicked()
{
    onButtonClick('7');
}

void CDlgFullKeyboard::on_btn_8_clicked()
{
    onButtonClick('8');
}

void CDlgFullKeyboard::on_btn_9_clicked()
{
    onButtonClick('9');
}

void CDlgFullKeyboard::on_btn_Close_clicked()
{
    // Special
    _pcurrentEdit->stopEdit();
    emit __OnCancelKeyboard(this, _pcurrentEdit);
    hide();
}

void CDlgFullKeyboard::on_btn_Clear_clicked()
{
    //
    onButtonClick(0x00);
}

void CDlgFullKeyboard::on_btn_Back_Slash_clicked()
{
    onButtonClick('\\');
}

void CDlgFullKeyboard::on_btn_OpenApost_clicked()
{
    onButtonClick('`');
}

void CDlgFullKeyboard::on_btn_Tilde_clicked()
{
    onButtonClick('~');
}

void CDlgFullKeyboard::on_btn_LeftAngle_clicked()
{
    onButtonClick('<');
}

void CDlgFullKeyboard::on_btn_RightAngle_clicked()
{
    onButtonClick('>');
}

void CDlgFullKeyboard::on_btn_LeftCurly_clicked()
{
    onButtonClick('{');
}

void CDlgFullKeyboard::on_btn_RightCurly_clicked()
{
    onButtonClick('}');
}

void CDlgFullKeyboard::on_btn_LeftSquare_clicked()
{
    onButtonClick('[');
}

void CDlgFullKeyboard::on_btn_RightSquare_clicked()
{
    onButtonClick(']');
}

void CDlgFullKeyboard::makeLower()
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

void CDlgFullKeyboard::makeUpper()
{
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

void CDlgFullKeyboard::on_btn_Upper_Lower_clicked(bool checked)
{
    if(checked)
    {
        makeLower();
    }
    else {
        makeUpper();
    }

}
