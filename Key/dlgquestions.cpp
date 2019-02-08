#include "dlgquestions.h"
#include "ui_dlgquestions.h"
#include <QDebug>
#include "kcbkeyboarddialog.h"
#include "kcbutils.h"
#include "kcbcommon.h"
#include "kcbsystem.h"
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
    KCB_DEBUG_ENTRY;
    ui->setupUi(this);

    kcb::SetWindowParams(this);

    raise();

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
    KCB_DEBUG_EXIT;
}

CDlgQuestions::~CDlgQuestions()
{
    Kcb::Utils::DestructorMsg(this);
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
    KCB_DEBUG_ENTRY;
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
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::RunKeyboard(QString& text)
{
    KCB_DEBUG_ENTRY;
    KcbKeyboardDialog kkd;

    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_edtAnswer1_clicked()
{
    KCB_DEBUG_ENTRY;
    QString text = ui->edtAnswer1->text();
    RunKeyboard(text);
    ui->clrAnswer1->setEnabled(!text.isEmpty());
    ui->edtAnswer1->setText(text);
    enableOk();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_edtAnswer2_clicked()
{
    KCB_DEBUG_ENTRY;
    if (ui->edtAnswer1->isEnabled())
    {
        QString text = ui->edtAnswer2->text();
        RunKeyboard(text);
        ui->clrAnswer2->setEnabled(!text.isEmpty());
        ui->edtAnswer2->setText(text);
    }
    enableOk();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_edtAnswer3_clicked()
{
    KCB_DEBUG_ENTRY;
    if (ui->edtAnswer3->isEnabled())
    {
        QString text = ui->edtAnswer3->text();
        RunKeyboard(text);
        ui->clrAnswer3->setEnabled(!text.isEmpty());
        ui->edtAnswer3->setText(text);
    }
    enableOk();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_clrAnswer1_clicked()
{
    KCB_DEBUG_ENTRY;
    ui->edtAnswer1->setText("");
    ui->edtAnswer1->setFocus();
    enableOk();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_clrAnswer2_clicked()
{
    KCB_DEBUG_ENTRY;
    ui->edtAnswer2->setText("");
    ui->edtAnswer2->setFocus();
    enableOk();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_clrAnswer3_clicked()
{
    KCB_DEBUG_ENTRY;
    ui->edtAnswer3->setText("");
    ui->edtAnswer3->setFocus();
    enableOk();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_bbOkCancel_accepted()
{
    KCB_DEBUG_ENTRY;
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
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::on_bbOkCancel_rejected()
{
    KCB_DEBUG_ENTRY;
    emit __OnQuestionsClose();
    emit __OnQuestionsCancel();
    KCB_DEBUG_EXIT;
}

void CDlgQuestions::enableOk()
{
    KCB_DEBUG_ENTRY;
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
    KCB_DEBUG_EXIT;
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
