#include "frmcodeeditmulti.h"
#include "ui_frmcodeeditmulti.h"
#include <QSignalMapper>
#include <QDebug>
#include <QTouchEvent>
#include <QMessageBox>
#include "lockcabinetwidget.h"
#include "lockstate.h"

FrmCodeEditMulti::FrmCodeEditMulti(QWidget *parent) :
    QDialog(parent),
    m_access_type(0),
    m_lock_cab(* new LockCabinetWidget(this, 2)),
    ui(new Ui::FrmCodeEditMulti)
{
    ui->setupUi(this);
    FrmCodeEditMulti::showFullScreen();

    setAttribute(Qt::WA_AcceptTouchEvents, true);

    ui->edCode1->setAttribute(Qt::WA_AcceptTouchEvents, true);
    ui->edCode1->installEventFilter(this);
    ui->edCode2->setAttribute(Qt::WA_AcceptTouchEvents, true);
    ui->edCode2->installEventFilter(this);
    ui->edUsername->setAttribute(Qt::WA_AcceptTouchEvents, true);
    ui->edUsername->installEventFilter(this);


    ui->hloLockCabinet->addWidget(&m_lock_cab);

    ui->dtStartAccess->setDateTime(QDateTime::currentDateTime());
    ui->dtEndAccess->setDateTime(QDateTime::currentDateTime());
    updateAccessType(0);
    ui->edCode1->setText("");
    ui->edCode2->setText("");
    ui->edUsername->setText("");
    m_questions.clear();
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
    ui->edCode2->setText(state->getCode2());
    ui->edUsername->setText(state->getDescription());
    ui->dtStartAccess->setDateTime(state->getStartTime());
    ui->dtEndAccess->setDateTime(state->getEndTime());
    ui->cbFingerprint->setEnabled(state->getFingerprint1());

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
    m_lock_cab.setSelectedLocks(state->getLockNums());
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

    state->show();
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
}

void FrmCodeEditMulti::on_pbClearCode2_clicked()
{
    ui->edCode2->setText("");
}

void FrmCodeEditMulti::on_pbClearUsername_clicked()
{
    ui->edUsername->setText("");
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

void FrmCodeEditMulti::on_bbSaveCancel_accepted()
{
    emit accepted();
}

void FrmCodeEditMulti::on_bbSaveCancel_rejected()
{
    emit rejected();
}

void FrmCodeEditMulti::on_edCode1_cursorPositionChanged(int arg1, int arg2)
{
    qDebug() << "Code 1 text edited";
}

void FrmCodeEditMulti::touchEvent(QTouchEvent *ev)
{
    qDebug() << "Received touch event";
}

bool FrmCodeEditMulti::eventFilter(QObject *target, QEvent *event)
{
    QMessageBox msgBox;
    //qDebug() << "eventFilter. Event type:" << QVariant(event->type()).toString();
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (ui->edCode1 == target)
        {
            qDebug() << "Code 1 Selected";
            return false;
        }
        else if (ui->edCode2 == target)
        {
            qDebug() << "Code 2 Selected";
            return false;
        }
        else if (ui->edUsername == target)
        {
            qDebug() << "Username Selected";
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QDialog::eventFilter(target, event);
    }
}
