#include "dlgeditquestions.h"
#include "ui_dlgeditquestions.h"
#include <QDebug>
#include "kcbkeyboarddialog.h"

CDlgEditQuestions::CDlgEditQuestions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgEditQuestions)
{
    ui->setupUi(this);

    CDlgEditQuestions::showFullScreen();

}

CDlgEditQuestions::~CDlgEditQuestions()
{
    delete ui;
}

void CDlgEditQuestions::getValues(QVector<QString>& questions)
{
    questions[0] = ui->edtQuestion1->text();
    questions[1] = ui->edtQuestion2->text();
    questions[2] = ui->edtQuestion3->text();
}

void CDlgEditQuestions::setValues(QVector<QString>& questions)
{
    ui->edtQuestion1->setText(questions[0]);
    ui->edtQuestion2->setText(questions[1]);
    ui->edtQuestion3->setText(questions[2]);
}

void CDlgEditQuestions::on_buttonBoxQuestions_accepted()
{
    done(QDialog::Accepted);    
}

void CDlgEditQuestions::on_buttonBoxQuestions_rejected()
{
    done(QDialog::Rejected);    
}

void CDlgEditQuestions::showKeyboard(CClickableLineEdit *p_lineEdit)
{
    KcbKeyboardDialog kkd;

    kkd.setValue(p_lineEdit->text());
    if (kkd.exec())
    {
        p_lineEdit->setText(kkd.getValue());
    }
}

void CDlgEditQuestions::on_edtQuestion1_clicked()
{
    showKeyboard(ui->edtQuestion1);
}

void CDlgEditQuestions::on_edtQuestion2_clicked()
{
    showKeyboard(ui->edtQuestion2);
}

void CDlgEditQuestions::on_edtQuestion3_clicked()
{
    showKeyboard(ui->edtQuestion3);
}


void CDlgEditQuestions::on_clrQuestion1_clicked()
{
  ui->edtQuestion1->setText("");
}

void CDlgEditQuestions::on_clrQuestion2_clicked()
{
  ui->edtQuestion2->setText("");
}

void CDlgEditQuestions::on_clrQuestion3_clicked()
{
  ui->edtQuestion3->setText("");
}

