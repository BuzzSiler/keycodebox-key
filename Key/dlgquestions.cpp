#include "dlgquestions.h"
#include "ui_dlgquestions.h"
#include <QDebug>
#include "kcbkeyboarddialog.h"
#include "kcbutils.h"
#include "kcbcommon.h"
#include "keycodeboxsettings.h"

static const QString css_warn = "color: black; background-color: red";
static const QString css_none = "";

static bool fleetwave_enabled;

CDlgQuestions::CDlgQuestions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgQuestions),
    _lockNum(""),
    _answer1("0"),
    _answer2("0"),
    _answer3("0")

{
    ui->setupUi(this);

    // For some reason, the admin form does not show full screen without the following
    // flags being set.  Maybe this should be don't at in the main so it gets
    // inherited?  Not sure.  Until this is resolved, just set these flags.
    CDlgQuestions::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    CDlgQuestions::showFullScreen();

    fleetwave_enabled = KeyCodeBoxSettings::isFleetwaveEnabled();

    ui->rbAnswer1No->setVisible(fleetwave_enabled);
    ui->rbAnswer1Yes->setVisible(fleetwave_enabled);
    ui->rbAnswer2No->setVisible(fleetwave_enabled);
    ui->rbAnswer2Yes->setVisible(fleetwave_enabled);
    ui->rbAnswer3No->setVisible(fleetwave_enabled);
    ui->rbAnswer3Yes->setVisible(fleetwave_enabled);

    ui->edtAnswer1->setVisible(!fleetwave_enabled);
    ui->edtAnswer2->setVisible(!fleetwave_enabled);
    ui->edtAnswer3->setVisible(!fleetwave_enabled);
    ui->clrAnswer1->setVisible(!fleetwave_enabled);
    ui->clrAnswer2->setVisible(!fleetwave_enabled);
    ui->clrAnswer3->setVisible(!fleetwave_enabled);

    ui->rbAnswer1No->setChecked(fleetwave_enabled);
    ui->rbAnswer2No->setChecked(fleetwave_enabled);
    ui->rbAnswer3No->setChecked(fleetwave_enabled);

    on_rbAnswer1No_clicked();
    on_rbAnswer2No_clicked();
    on_rbAnswer3No_clicked();

    ui->bbOkCancel->button(QDialogButtonBox::Ok)->setDisabled(!fleetwave_enabled);
    ui->bbOkCancel->button(QDialogButtonBox::Cancel)->setEnabled(!fleetwave_enabled);
}

CDlgQuestions::~CDlgQuestions()
{
    kcb::Utils::DestructorMsg(this);
    delete ui;
}

void CDlgQuestions::getValues(QString& question1, QString& question2, QString& question3)
{
    if (fleetwave_enabled)
    {
        question1 = _answer1;
        question2 = _answer2;
        question3 = _answer3;
    }
    else
    {
        question1 = ui->edtAnswer1->text();
        question2 = ui->edtAnswer2->text();
        question3 = ui->edtAnswer3->text();
    }
}

void CDlgQuestions::setValues(QString lockNum, QString question1, QString question2, QString question3)
{
    _lockNum = lockNum;

    if (!question1.isEmpty())
    {
        ui->label_question1->setText(question1);
        if (fleetwave_enabled)
        {
            ui->rbAnswer1No->setChecked(fleetwave_enabled);
            on_rbAnswer1No_clicked();
        }
        else
        {
            ui->edtAnswer1->setEnabled(true);
            ui->edtAnswer1->setText("");
        }
    }
    if (!question2.isEmpty())
    {
        ui->label_question2->setText(question2);
        if (fleetwave_enabled)
        {
            ui->rbAnswer2No->setChecked(fleetwave_enabled);
            on_rbAnswer2No_clicked();
        }
        else
        {
          ui->edtAnswer2->setEnabled(true);
          ui->edtAnswer2->setText("");
        }
    }
    if (!question3.isEmpty())
    {
        ui->label_question3->setText(question3);
        if (fleetwave_enabled)
        {
            ui->rbAnswer3No->setChecked(fleetwave_enabled);
            on_rbAnswer3No_clicked();
        }
        else
        {
            ui->edtAnswer3->setEnabled(true);
            ui->edtAnswer3->setText("");
        }
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
    QString answer1;
    QString answer2;
    QString answer3;

    if (fleetwave_enabled)
    {
        answer1 = _answer1;
        answer2 = _answer2;
        answer3 = _answer3;
    }
    else
    {
        answer1 = ui->edtAnswer1->text();
        answer2 = ui->edtAnswer2->text();
        answer3 = ui->edtAnswer3->text();
    }
    emit __OnQuestionsSave(_lockNum, answer1, answer2, answer3);
    emit __OnQuestionsClose();

    ui->rbAnswer1No->setChecked(fleetwave_enabled);
    ui->rbAnswer2No->setChecked(fleetwave_enabled);
    ui->rbAnswer3No->setChecked(fleetwave_enabled);

    on_rbAnswer1No_clicked();
    on_rbAnswer2No_clicked();
    on_rbAnswer3No_clicked();
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

    if (!fleetwave_enabled)
    {
        ui->edtAnswer1->setStyleSheet(q1_valid ? css_none : css_warn);
        ui->edtAnswer2->setStyleSheet(q2_valid ? css_none : css_warn);
        ui->edtAnswer3->setStyleSheet(q3_valid ? css_none : css_warn);

        ui->clrAnswer1->setEnabled(q1_specified);
        ui->clrAnswer2->setEnabled(q2_specified);
        ui->clrAnswer3->setEnabled(q3_specified);

        ui->bbOkCancel->button(QDialogButtonBox::Ok)->setEnabled(q1_valid && q2_valid && q3_valid);
    }
}

void CDlgQuestions::on_rbAnswer1Yes_clicked()
{
    _answer1 = "1";
}

void CDlgQuestions::on_rbAnswer1No_clicked()
{
    _answer1 = "0";
}

void CDlgQuestions::on_rbAnswer2Yes_clicked()
{
    _answer2 = "1";
}

void CDlgQuestions::on_rbAnswer2No_clicked()
{
    _answer2 = "0";
}

void CDlgQuestions::on_rbAnswer3Yes_clicked()
{
    _answer3 = "1";
}

void CDlgQuestions::on_rbAnswer3No_clicked()
{
    _answer3 = "0";
}
