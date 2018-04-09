#include "dlgquestions.h"
#include "ui_dlgquestions.h"
#include <QDebug>
#include "kcbkeyboarddialog.h"

CDlgQuestions::CDlgQuestions(QWidget *parent) :
    QDialog(parent),
    _lockNum(""),
    ui(new Ui::CDlgQuestions)
{
    ui->setupUi(this);
}

CDlgQuestions::~CDlgQuestions()
{
    delete ui;
}

void CDlgQuestions::getValues(QString *question1, QString *question2, QString *question3)
{
  *question1 = ui->edtAnswer1->text();
  *question2 = ui->edtAnswer2->text();
  *question3 = ui->edtAnswer3->text();
}

void CDlgQuestions::setValues(QString lockNum, QString question1, QString question2, QString question3)
{
  qDebug() << "Editing Questions, " << " question1: " << question1 << " question2: " << question2 << " question3: " << question3;
  _lockNum = lockNum;
  ui->label_question1->setText(question1);
  ui->label_question2->setText(question2);
  ui->label_question3->setText(question3);

  ui->edtAnswer1->setText("");
  ui->edtAnswer2->setText("");
  ui->edtAnswer3->setText("");
}

void CDlgQuestions::on_buttonBoxQuestions_accepted()
{
    emit __OnQuestionsSave(_lockNum, ui->edtAnswer1->text(), ui->edtAnswer2->text(), ui->edtAnswer3->text());
    emit __OnQuestionsClose();
}

void CDlgQuestions::on_buttonBoxQuestions_rejected()
{
    emit __OnQuestionsClose();
    emit __OnQuestionsCancel();
}

void CDlgQuestions::RunKeyboard(QString& text)
{
    KcbKeyboardDialog kkd;

    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
}

void CDlgQuestions::on_edtAnswer1_clicked()
{
    QString text = ui->edtAnswer1->text();
    RunKeyboard(text);
    ui->edtAnswer1->setText(text);
}

void CDlgQuestions::on_edtAnswer2_clicked()
{
    QString text = ui->edtAnswer2->text();
    RunKeyboard(text);
    ui->edtAnswer2->setText(text);
}

void CDlgQuestions::on_edtAnswer3_clicked()
{
    QString text = ui->edtAnswer3->text();
    RunKeyboard(text);
    ui->edtAnswer3->setText(text);
}

void CDlgQuestions::on_clrAnswer1_clicked()
{
  ui->edtAnswer1->setText("");
}

void CDlgQuestions::on_clrAnswer2_clicked()
{
  ui->edtAnswer2->setText("");
}

void CDlgQuestions::on_clrAnswer3_clicked()
{
  ui->edtAnswer3->setText("");
}

