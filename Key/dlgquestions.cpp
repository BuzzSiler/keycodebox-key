#include "dlgquestions.h"
#include "ui_dlgquestions.h"
#include "tblcodes.h"
#include <QDebug>
#include <sqlite3.h>
#include <sqlite3ext.h>
#include <QtSql>
#include <string>
#include <QMessageBox>

CDlgQuestions::CDlgQuestions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgQuestions),
    _pcurrentLineEdit(0)
{
    ui->setupUi(this);
}

void CDlgQuestions::setMaxLocks(int nLocks)
{
  nLocks = nLocks;
  //ui->spinLockNum->setMaximum(nLocks);
}

CDlgQuestions::~CDlgQuestions()
{
    delete ui;
    if(_pcurrentLineEdit) {
        delete _pcurrentLineEdit;
    }
}

void CDlgQuestions::getValues(QString *question1, QString *question2, QString *question3)
{
  *question1 = ui->edtAnswer1->text();
  *question2 = ui->edtAnswer2->text();
  *question3 = ui->edtAnswer3->text();
}

void CDlgQuestions::setValues(int doorNum, QString question1, QString question2, QString question3)
{
  qDebug() << "Editing Questions, " << " question1: " << question1 << " question2: " << question2 << " question3: " << question3;
  _doorNum = doorNum;
  ui->label_question1->setText(question1);
  ui->label_question2->setText(question2);
  ui->label_question3->setText(question3);

  ui->edtAnswer1->setText("");
  ui->edtAnswer2->setText("");
  ui->edtAnswer3->setText("");
}

void CDlgQuestions::on_buttonBoxQuestions_accepted()
{
    qDebug() << "on_buttonBoxQuestions_accepted()";
    emit __OnQuestionsSave(_doorNum, ui->edtAnswer1->text(), ui->edtAnswer2->text(), ui->edtAnswer3->text());
    emit __OnQuestionsClose();
}

void CDlgQuestions::on_buttonBoxQuestions_rejected()
{
    qDebug() << "on_buttonBoxQuestions_rejected()";
    emit __OnQuestionsClose();
    emit __OnQuestionsCancel();
}

void CDlgQuestions::on_edtAnswer1_clicked()
{
    checkAndCreateCurrentLineEdit();
    onStartEditLine(ui->edtAnswer1, tr("Answer #1"));
}

void CDlgQuestions::on_edtAnswer2_clicked()
{
    checkAndCreateCurrentLineEdit();
    onStartEditLine(ui->edtAnswer2, tr("Answer #2"));
}

void CDlgQuestions::on_edtAnswer3_clicked()
{
    checkAndCreateCurrentLineEdit();
    onStartEditLine(ui->edtAnswer3, tr("Answer #3"));
}

void CDlgQuestions::checkAndCreateCurrentLineEdit()
{
    if(!_pcurrentLineEdit) {
        _pcurrentLineEdit = new CCurrentEdit();
        connect(this, SIGNAL(__OnCodes(QString)), _pcurrentLineEdit, SLOT(OnStringEntry(QString)));
    }
}

void CDlgQuestions::onStartEditLine(QLineEdit* pLine, QString sLabelText)
{
    if(!_pKeyboard) {
        _pKeyboard = new CDlgFullKeyboard();
        connect(_pKeyboard, SIGNAL(__TextEntered(CDlgFullKeyboard*,CCurrentEdit*)),
                this, SLOT(OnKeyboardTextEntered(CDlgFullKeyboard*, CCurrentEdit*)));
    }
    if(!_pcurrentLineEdit) {
        _pcurrentLineEdit = new CCurrentEdit();
    }
    if(_pKeyboard && _pcurrentLineEdit) {
        _pKeyboard->setCurrentEdit(_pcurrentLineEdit);
        _pcurrentLineEdit->setLineToBeEdited(pLine);
        _pcurrentLineEdit->setOriginalTextToEdit(pLine->text());
        _pcurrentLineEdit->setLabelText(sLabelText);

        _pKeyboard->setActive();
    }
}

void CDlgQuestions::OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *currEdit)
{
    currEdit->getLineBeingEdited()->setText(currEdit->getNewText());
    keyboard->hide();
}


void CDlgQuestions::on_buttonBoxQuestions_clicked(QAbstractButton *button)
{
    button = button;
}

void CDlgQuestions::OnAdminInfoCodes(QString code1, QString code2)
{
    code2 = code2;
    emit __OnCodes(code1);
}

void CDlgQuestions::on_clrAnswer1_clicked()
{
  qDebug() << "on_clrAnswer1_clicked()";  
  ui->edtAnswer1->setText("");
}

void CDlgQuestions::on_clrAnswer2_clicked()
{
  qDebug() << "on_clrAnswer2_clicked()";  
  ui->edtAnswer2->setText("");
}

void CDlgQuestions::on_clrAnswer3_clicked()
{
  qDebug() << "on_clrAnswer3_clicked()";  
  ui->edtAnswer3->setText("");
}

