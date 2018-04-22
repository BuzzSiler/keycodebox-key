#include "dlgquestions.h"
#include "ui_dlgquestions.h"
#include <QDebug>
#include "kcbkeyboarddialog.h"
#include "kcbutils.h"

static const QString css_warn = "color: black; background-color: red";
static const QString css_none = "";

CDlgQuestions::CDlgQuestions(QWidget *parent) :
    QDialog(parent),
    _lockNum(""),
    ui(new Ui::CDlgQuestions)
{
    ui->setupUi(this);

    CDlgQuestions::showFullScreen();

    ui->bbOkCancel->button(QDialogButtonBox::Ok)->setDisabled(true);
    ui->bbOkCancel->button(QDialogButtonBox::Cancel)->setEnabled(true);
}

CDlgQuestions::~CDlgQuestions()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void CDlgQuestions::getValues(QString& question1, QString& question2, QString& question3)
{
  question1 = ui->edtAnswer1->text();
  question2 = ui->edtAnswer2->text();
  question3 = ui->edtAnswer3->text();
}

void CDlgQuestions::setValues(QString lockNum, QString question1, QString question2, QString question3)
{
    _lockNum = lockNum;

    if (!question1.isEmpty())
    {
      ui->label_question1->setText(question1);
      ui->edtAnswer1->setEnabled(true);
      ui->edtAnswer1->setText("");
    }
    if (!question2.isEmpty())
    {
      ui->label_question2->setText(question2);
      ui->edtAnswer2->setEnabled(true);
      ui->edtAnswer2->setText("");
    }
    if (!question3.isEmpty())
    {
      ui->label_question3->setText(question3);
      ui->edtAnswer3->setEnabled(true);
      ui->edtAnswer3->setText("");
    }

    enableOk();
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
    ui->clrAnswer1->setEnabled(!text.isEmpty());
    ui->edtAnswer1->setText(text);
    enableOk();
}

void CDlgQuestions::on_edtAnswer2_clicked()
{
    if (ui->edtAnswer1->isEnabled())
    {
        QString text = ui->edtAnswer2->text();
        RunKeyboard(text);
        ui->clrAnswer2->setEnabled(!text.isEmpty());
        ui->edtAnswer2->setText(text);
    }
    enableOk();
}

void CDlgQuestions::on_edtAnswer3_clicked()
{
    if (ui->edtAnswer3->isEnabled())
    {
        QString text = ui->edtAnswer3->text();
        RunKeyboard(text);
        ui->clrAnswer3->setEnabled(!text.isEmpty());
        ui->edtAnswer3->setText(text);
    }
    enableOk();
}

void CDlgQuestions::on_clrAnswer1_clicked()
{
    ui->edtAnswer1->setText("");
    ui->edtAnswer1->setFocus();
    enableOk();
}

void CDlgQuestions::on_clrAnswer2_clicked()
{
    ui->edtAnswer2->setText("");
    ui->edtAnswer2->setFocus();
    enableOk();
}

void CDlgQuestions::on_clrAnswer3_clicked()
{
    ui->edtAnswer3->setText("");
    ui->edtAnswer3->setFocus();
    enableOk();
}

void CDlgQuestions::on_bbOkCancel_accepted()
{
    emit __OnQuestionsSave(_lockNum, ui->edtAnswer1->text(), ui->edtAnswer2->text(), ui->edtAnswer3->text());
    emit __OnQuestionsClose();
}

void CDlgQuestions::on_bbOkCancel_rejected()
{
    emit __OnQuestionsClose();
    emit __OnQuestionsCancel();
}

void CDlgQuestions::enableOk()
{
    bool q1_required = !ui->label_question1->text().isEmpty();
    bool q2_required = !ui->label_question2->text().isEmpty();
    bool q3_required = !ui->label_question3->text().isEmpty();

    bool q1_specified = !ui->edtAnswer1->text().isEmpty();
    bool q2_specified = !ui->edtAnswer2->text().isEmpty();
    bool q3_specified = !ui->edtAnswer3->text().isEmpty();

    bool q1_valid = !q1_required || (q1_required && q1_specified);
    bool q2_valid = !q2_required || (q2_required && q2_specified);
    bool q3_valid = !q3_required || (q3_required && q3_specified);

    ui->edtAnswer1->setStyleSheet(q1_valid ? css_none : css_warn);
    ui->edtAnswer2->setStyleSheet(q2_valid ? css_none : css_warn);
    ui->edtAnswer3->setStyleSheet(q3_valid ? css_none : css_warn);

    ui->clrAnswer1->setEnabled(q1_specified);
    ui->clrAnswer2->setEnabled(q2_specified);
    ui->clrAnswer3->setEnabled(q3_specified);

    ui->bbOkCancel->button(QDialogButtonBox::Ok)->setEnabled(q1_valid && q2_valid && q3_valid);

}
