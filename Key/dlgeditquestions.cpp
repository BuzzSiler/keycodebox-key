#include "dlgeditquestions.h"
#include "ui_dlgeditquestions.h"
#include <QDebug>
#include "kcbutils.h"
#include "kcbkeyboarddialog.h"

CDlgEditQuestions::CDlgEditQuestions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgEditQuestions)
{
    ui->setupUi(this);

    CDlgEditQuestions::showFullScreen();

    ui->buttonBoxQuestions->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->buttonBoxQuestions->button(QDialogButtonBox::Cancel)->setEnabled(true);
}

CDlgEditQuestions::~CDlgEditQuestions()
{
    Kcb::Utils::DestructorMsg(this);
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
    if (questions[0].isEmpty())
    {
        ui->edtQuestion1->setEnabled(true);
    }
    else
    {
        ui->edtQuestion1->setEnabled(true);
        ui->edtQuestion1->setText(questions[0]);
        ui->clrQuestion1->setEnabled(true);

        if (questions[1].isEmpty())
        {
            ui->edtQuestion2->setEnabled(true);
        }
        else
        {
            ui->edtQuestion2->setEnabled(true);
            ui->edtQuestion2->setText(questions[1]);
            ui->clrQuestion2->setEnabled(true);

            if (questions[2].isEmpty())
            {
                ui->edtQuestion3->setEnabled(true);
            }
            else
            {
                ui->edtQuestion3->setEnabled(true);
                ui->edtQuestion3->setText(questions[2]);
                ui->clrQuestion3->setEnabled(true);
            }
        }
    }
}

void CDlgEditQuestions::showKeyboard(CClickableLineEdit *p_lineEdit)
{
    KcbKeyboardDialog kkd;

    kkd.setValue(p_lineEdit->text());
    if (kkd.exec())
    {
        auto text = kkd.getValue();
        p_lineEdit->setText(text);
        ui->buttonBoxQuestions->button(QDialogButtonBox::Save)->setEnabled(!text.isEmpty());
    }
}

void CDlgEditQuestions::on_edtQuestion1_clicked()
{
    showKeyboard(ui->edtQuestion1);
    bool is_empty = ui->edtQuestion1->text().isEmpty();
    ui->clrQuestion1->setDisabled(is_empty);
    ui->edtQuestion2->setDisabled(is_empty);
}

void CDlgEditQuestions::on_edtQuestion2_clicked()
{
    if (ui->edtQuestion2->isEnabled())
    {
        showKeyboard(ui->edtQuestion2);
        bool is_empty = ui->edtQuestion2->text().isEmpty();
        ui->clrQuestion1->setDisabled(true);
        ui->clrQuestion2->setDisabled(is_empty);
        ui->edtQuestion3->setDisabled(is_empty);
    }
}

void CDlgEditQuestions::on_edtQuestion3_clicked()
{
    if (ui->edtQuestion3->isEnabled())
    {
        showKeyboard(ui->edtQuestion3);
        bool is_empty = ui->edtQuestion3->text().isEmpty();
        ui->clrQuestion1->setDisabled(true);
        ui->clrQuestion2->setDisabled(true);
        ui->clrQuestion3->setDisabled(is_empty);
    }
}

void CDlgEditQuestions::enableOk()
{
    bool q1_is_empty = ui->edtQuestion1->text().isEmpty();
    bool q2_is_empty = ui->edtQuestion2->text().isEmpty();
    bool q3_is_empty = ui->edtQuestion3->text().isEmpty();

    ui->buttonBoxQuestions->button(QDialogButtonBox::Save)->setEnabled(!q1_is_empty || !q2_is_empty || !q3_is_empty);
}

void CDlgEditQuestions::on_clrQuestion1_clicked()
{
    ui->edtQuestion1->setText("");
    ui->clrQuestion1->setDisabled(true);
    ui->edtQuestion2->setDisabled(true);
    enableOk();
}

void CDlgEditQuestions::on_clrQuestion2_clicked()
{
  ui->edtQuestion2->setText("");
  ui->clrQuestion2->setDisabled(true);
  ui->edtQuestion3->setDisabled(true);
  ui->clrQuestion1->setEnabled(true);
  enableOk();
}

void CDlgEditQuestions::on_clrQuestion3_clicked()
{
  ui->edtQuestion3->setText("");
  ui->clrQuestion3->setDisabled(true);
  ui->clrQuestion2->setEnabled(true);
  enableOk();
}

void CDlgEditQuestions::on_buttonBoxQuestions_accepted()
{
    done(QDialog::Accepted);
}

void CDlgEditQuestions::on_buttonBoxQuestions_rejected()
{
    done(QDialog::Rejected);
}
