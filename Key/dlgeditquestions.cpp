#include "dlgeditquestions.h"
#include "ui_dlgeditquestions.h"
#include <QDebug>
#include "kcbutils.h"
#include "kcbkeyboarddialog.h"

static const QString css_warn = "color: black; background-color: red";
static const QString css_none = "";

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
    kcb::Utils::DestructorMsg(this);
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

    updateUi();    
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
    updateUi();
}

void CDlgEditQuestions::on_edtQuestion2_clicked()
{
    if (ui->edtQuestion2->isEnabled())
    {
        showKeyboard(ui->edtQuestion2);
        updateUi();
    }
}

void CDlgEditQuestions::on_edtQuestion3_clicked()
{
    if (ui->edtQuestion3->isEnabled())
    {
        showKeyboard(ui->edtQuestion3);
        updateUi();
    }
}

void CDlgEditQuestions::on_clrQuestion1_clicked()
{
    if (ui->clrQuestion1->isEnabled())
    {
        ui->edtQuestion1->setText("");
        updateUi();
    }
}

void CDlgEditQuestions::on_clrQuestion2_clicked()
{
    if (ui->clrQuestion2->isEnabled())
    {
        ui->edtQuestion2->setText("");
        updateUi();
    }
}

void CDlgEditQuestions::on_clrQuestion3_clicked()
{
    if (ui->clrQuestion3->isEnabled())
    {
        ui->edtQuestion3->setText("");
        updateUi();
    }
}

void CDlgEditQuestions::updateUi()
{
    bool questions_valid = false;
    bool q1_specified = !ui->edtQuestion1->text().isEmpty();
    bool q2_specified = !ui->edtQuestion2->text().isEmpty();
    bool q3_specified = !ui->edtQuestion3->text().isEmpty();

    ui->clrQuestion1->setEnabled(q1_specified);
    ui->edtQuestion1->setStyleSheet(q1_specified ? css_none : css_warn);

    ui->edtQuestion2->setStyleSheet(css_none);
    ui->edtQuestion2->setEnabled(q1_specified);
    ui->clrQuestion2->setEnabled(ui->edtQuestion2->isEnabled() && q2_specified);
    ui->edtQuestion2->setStyleSheet(q1_specified && !q2_specified && q3_specified ? css_warn : css_none);

    ui->edtQuestion3->setEnabled(q1_specified && q2_specified);
    ui->clrQuestion3->setEnabled(ui->edtQuestion3->isEnabled() && q3_specified);

    bool only_q1_specified = q1_specified && !(q2_specified || q3_specified);
    bool q1_and_q2_specified = (q1_specified && q2_specified) && !q3_specified;
    bool all_specified = q1_specified && q2_specified && q3_specified;

    questions_valid = only_q1_specified || q1_and_q2_specified || all_specified;

    ui->buttonBoxQuestions->button(QDialogButtonBox::Save)->setEnabled(questions_valid);
}

void CDlgEditQuestions::on_buttonBoxQuestions_accepted()
{
    done(QDialog::Accepted);
}

void CDlgEditQuestions::on_buttonBoxQuestions_rejected()
{
    done(QDialog::Rejected);
}
