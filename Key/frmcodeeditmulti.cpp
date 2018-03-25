#include "frmcodeeditmulti.h"
#include "ui_frmcodeeditmulti.h"
#include <QSignalMapper>
#include <QDebug>
#include <QTouchEvent>
#include <QMessageBox>
#include "lockcabinetwidget.h"
#include "lockstate.h"
#include "dlgnumberpad.h"

FrmCodeEditMulti::FrmCodeEditMulti(QWidget *parent) :
    QDialog(parent),
    m_access_type(0),
    m_lock_cab(* new LockCabinetWidget(this, 2)),
    m_num_pad(* new DlgNumberPad(this)),
    ui(new Ui::FrmCodeEditMulti)
{
    ui->setupUi(this);

    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setDisabled(true);

    FrmCodeEditMulti::showFullScreen();

    connect(ui->edCode1, SIGNAL(clicked()), this, SLOT(on_edCode1_clicked()));
    connect(ui->edCode2, SIGNAL(clicked()), this, SLOT(on_edCode2_clicked()));
    connect(ui->edUsername, SIGNAL(clicked()), this, SLOT(on_edUsername_clicked()));

    connect(&m_num_pad, SIGNAL(NotifyAccepted()), this, SLOT(OnAccepted()));
    connect(&m_num_pad, SIGNAL(NotifyRejected()), this, SLOT(OnRejected()));

    connect(&m_lock_cab, SIGNAL(NotifyLockSelected(QString, bool)), this, SLOT(OnNotifyLockSelected(QString, bool)));

    ui->hloLockCabinet->addWidget(&m_lock_cab);

    ui->dtStartAccess->setDateTime(QDateTime::currentDateTime());
    ui->dtEndAccess->setDateTime(QDateTime::currentDateTime());
    updateAccessType(0);
    ui->edCode1->setText("");
    ui->edCode2->setText("");
    ui->edUsername->setText("");
    m_questions.clear();

    updateUi();
}

FrmCodeEditMulti::~FrmCodeEditMulti()
{
    delete ui;
}

void FrmCodeEditMulti::setValues(CLockState * const state)
{
    qDebug() << "FrmCodeEditMulti::setValues";
    state->show();

    m_lock_cab.clrAllLocks();

    ui->edCode1->setText(state->getCode1());
    if (state->getCode2() != "")
    {
        ui->edCode2->setText(state->getCode2());
        ui->cbEnableCode2->setEnabled(true);
    }
    ui->edUsername->setText(state->getDescription());
    ui->dtStartAccess->setDateTime(state->getStartTime());
    ui->dtEndAccess->setDateTime(state->getEndTime());
    //ui->cbFingerprint->setEnabled(state->getFingerprint1());

    m_questions.clear();
    ui->pbQuestions->setEnabled(state->getAskQuestions());
    if (state->getAskQuestions())
    {
        m_questions.append(state->getQuestion1());
        m_questions.append(state->getQuestion2());
        m_questions.append(state->getQuestion3());
    }

    updateAccessType(state->getAccessType());

    m_lock_cab.enableAllLocks();
    m_lock_cab.clrAllLocks();
    m_lock_cab.setSelectedLocks(state->getLockNums());

    updateUi();
    updateExitState();
}

void FrmCodeEditMulti::getValues(CLockState * const state)
{
    state->setCode1(ui->edCode1->text());
    state->setCode2(ui->edCode2->text());
    state->setDescription(ui->edUsername->text());
    state->setStartTime(ui->dtStartAccess->dateTime());
    state->setEndTime(ui->dtEndAccess->dateTime());
    ui->cbFingerprint->isChecked() ? state->setFingerprint1() : state->clearFingerprint1();
    state->setAskQuestions(ui->pbQuestions->isChecked());
    if (ui->pbQuestions->isChecked())
    {
        state->setQuestion1(m_questions[0]);
        state->setQuestion2(m_questions[1]);
        state->setQuestion3(m_questions[2]);
    }

    state->setAccessType(ui->cbAccessType->currentIndex());
    state->setLockNums(m_lock_cab.getSelectedLocks());
    //state->show();
}

void FrmCodeEditMulti::updateAccessType(int index)
{
    m_access_type = index;

    switch (m_access_type)
    {
        case 0: // ALWAYS
            ui->cbAccessType->setCurrentIndex(0);
            ui->dtStartAccess->setDisabled(true);
            ui->dtEndAccess->setDisabled(true);
            break;
        case 1: // TIMED
            ui->cbAccessType->setCurrentIndex(1);
            ui->dtStartAccess->setEnabled(true);
            ui->dtEndAccess->setEnabled(true);
            break;
        case 2: // LIMITED_USE
            ui->cbAccessType->setCurrentIndex(2);
            ui->dtStartAccess->setDisabled(true);
            ui->dtEndAccess->setDisabled(true);
            break;
        default:
            ui->cbAccessType->setCurrentIndex(0);
            break;
    }
}

void FrmCodeEditMulti::on_pbClearCode1_clicked()
{
    ui->edCode1->setText("");
    ui->edCode2->setText("");
    updateUi();
}

void FrmCodeEditMulti::on_pbClearCode2_clicked()
{
    ui->edCode2->setText("");
    updateUi();
}

void FrmCodeEditMulti::on_pbClearUsername_clicked()
{
    ui->edUsername->setText("");
    updateUi();
}

void FrmCodeEditMulti::on_pbQuestions_clicked()
{
    // Show Questions dialog
    // CDlgQuestions dlg;
    // dlg.setQuestions(m_questions);
    // if (dlg.exec())
    // {
    //     m_questions = dlg.getQuestions();
    // }
}

void FrmCodeEditMulti::on_cbAccessType_currentIndexChanged(int index)
{
    updateAccessType(index);
}

void FrmCodeEditMulti::on_dtStartAccess_dateTimeChanged(const QDateTime &dateTime)
{
    // update start access date/time
}

void FrmCodeEditMulti::on_dtEndAccess_dateTimeChanged(const QDateTime &dateTime)
{
    // update end access date/time
}

void FrmCodeEditMulti::on_edCode1_clicked()
{
    // In the short term, we are using the number pad to enter a manual code.
    // Thinking forward, we can add a 'code source' drop down box in which the
    // user will select the 'code source', e.g., manual, hid, barcode, fingerprint,
    // magstipe, etc., and then we can prompt with an appropriate dialog which
    // can then be wired to the appropriate device input.
    m_num_pad.setValue(ui->edCode1->text());
    m_num_pad.show();
    m_p_line_edit = ui->edCode1;        
}

void FrmCodeEditMulti::on_edCode2_clicked()
{
    if (ui->edCode2->isEnabled())
    {
        m_num_pad.setValue(ui->edCode2->text());
        m_num_pad.show();
        m_p_line_edit = ui->edCode2;
    }
}

void FrmCodeEditMulti::on_edUsername_clicked()
{
    m_num_pad.setValue(ui->edUsername->text());
    m_num_pad.show();
    m_p_line_edit = ui->edUsername;
}

void FrmCodeEditMulti::OnAccepted()
{
    qDebug() << "Received NotifyAccepted";

    QString value;
    m_num_pad.getValue(value);

    if (value != m_p_line_edit->text())
    {
        m_p_line_edit->setText(value);
    }
    m_num_pad.hide();

    updateUi();
    updateExitState();
}

void FrmCodeEditMulti::OnRejected()
{
    qDebug() << "Receved NotifyRejected";
    m_num_pad.hide();
}

void FrmCodeEditMulti::OnNotifyLockSelected(QString lock, bool is_selected)
{
    qDebug() << "Lock:" << lock << "Is Selected:" << is_selected;
    updateExitState();
}

void FrmCodeEditMulti::on_cbEnableCode2_stateChanged(int arg1)
{
    bool result = arg1 == Qt::Checked ? true : false;
    ui->edCode2->setEnabled(result);
    if (!result)
    {
        ui->edCode2->setText("");
    }

    updateUi();
    updateExitState();
}

void FrmCodeEditMulti::updateExitState()
{
    bool code1_valid = ui->edCode1->text() != "";
    bool code2_valid = !ui->cbEnableCode2->isChecked() || (ui->cbEnableCode2->isChecked() && ui->edCode2->text() != "");

    bool result = code1_valid && code2_valid && m_lock_cab.getSelectedLocks() != "";

    qDebug() << "updateExitState" << result;
    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setEnabled(result);
}

void FrmCodeEditMulti::updateUi()
{
    ui->cbEnableCode2->setDisabled(ui->edCode1->text() == "");
    ui->edCode2->setEnabled(ui->cbEnableCode2->isChecked() && ui->edCode1->text() != "");

    ui->pbClearCode1->setDisabled(ui->edCode1->text() == "");
    ui->pbClearCode2->setDisabled(ui->edCode2->text() == "");
    ui->pbClearUsername->setDisabled(ui->edUsername->text() == "");
}

