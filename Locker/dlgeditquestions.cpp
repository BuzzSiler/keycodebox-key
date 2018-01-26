#include "dlgeditquestions.h"
#include "ui_dlgeditquestions.h"
#include "tblcodes.h"
#include <QDebug>
#include <sqlite3.h>
#include <sqlite3ext.h>
#include <QtSql>
#include <string>
#include <QMessageBox>

CDlgEditQuestions::CDlgEditQuestions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgEditQuestions),
    _pcurrentLineEdit(0)
{
    ui->setupUi(this);
}

void CDlgEditQuestions::setMaxLocks(int nLocks)
{
    Q_UNUSED(nLocks);
  //ui->spinLockNum->setMaximum(nLocks);
}

CDlgEditQuestions::~CDlgEditQuestions()
{
    delete ui;
    if(_pcurrentLineEdit) {
        delete _pcurrentLineEdit;
    }
}

void CDlgEditQuestions::getValues(QString *question1, QString *question2, QString *question3)
{
  *question1 = ui->edtQuestion1->text();
  *question2 = ui->edtQuestion2->text();
  *question3 = ui->edtQuestion3->text();
}

void CDlgEditQuestions::setValues(QString question1, QString question2, QString question3)
{
  qDebug() << "Editing Questions, " << " question1: " << question1 << " question2: " << question2 << " question3: " << question3;
  ui->edtQuestion1->setText(question1);
  ui->edtQuestion2->setText(question2);
  ui->edtQuestion3->setText(question3);
}

void CDlgEditQuestions::on_buttonBoxQuestions_accepted()
{
    qDebug() << "on_buttonBoxQuestions_accepted()";
    emit __OnQuestionEditSave();
    emit __OnQuestionEditClose();
}

void CDlgEditQuestions::on_buttonBoxQuestions_rejected()
{
    qDebug() << "on_buttonBoxQuestions_rejected()";
    emit __OnQuestionEditClose();
}

void CDlgEditQuestions::on_edtQuestion1_clicked()
{
    checkAndCreateCurrentLineEdit();
    onStartEditLine(ui->edtQuestion1, "Question #1");
}

void CDlgEditQuestions::on_edtQuestion2_clicked()
{
    checkAndCreateCurrentLineEdit();
    onStartEditLine(ui->edtQuestion2, "Question #2");
}

void CDlgEditQuestions::on_edtQuestion3_clicked()
{
    checkAndCreateCurrentLineEdit();
    onStartEditLine(ui->edtQuestion3, "Question #3");
}

void CDlgEditQuestions::checkAndCreateCurrentLineEdit()
{
    if(!_pcurrentLineEdit) {
        _pcurrentLineEdit = new CCurrentEdit();
        connect(this, SIGNAL(__OnCodes(QString)), _pcurrentLineEdit, SLOT(OnStringEntry(QString)));
    }
}

void CDlgEditQuestions::onStartEditLine(QLineEdit* pLine, QString sLabelText)
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

void CDlgEditQuestions::OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *currEdit)
{
    currEdit->getLineBeingEdited()->setText(currEdit->getNewText());
    keyboard->hide();
}


void CDlgEditQuestions::on_buttonBoxQuestions_clicked(QAbstractButton *button)
{
    Q_UNUSED(button);
}

void CDlgEditQuestions::OnAdminInfoCodes(QString code1, QString code2)
{
    Q_UNUSED(code2);
    emit __OnCodes(code1);
}

void CDlgEditQuestions::on_clrQuestion1_clicked()
{
  qDebug() << "on_clrQuestion1_clicked()";  
  ui->edtQuestion1->setText("");
}

void CDlgEditQuestions::on_clrQuestion2_clicked()
{
  qDebug() << "on_clrQuestion2_clicked()";  
  ui->edtQuestion2->setText("");
}

void CDlgEditQuestions::on_clrQuestion3_clicked()
{
  qDebug() << "on_clrQuestion3_clicked()";  
  ui->edtQuestion3->setText("");
}

