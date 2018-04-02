#include "frmcodeeditmulti.h"
#include "ui_frmcodeeditmulti.h"
#include <QSignalMapper>
#include <QDebug>
#include <QTouchEvent>
#include <QMessageBox>
#include <QDir>
#include "lockcabinetwidget.h"
#include "lockstate.h"
#include "dlgnumberpad.h"
#include "dlgfullkeyboard.h"
#include "currentedit.h"
#include "kcbcommon.h"
#include "kcbkeyboarddialog.h"
#include "dlgeditquestions.h"

FrmCodeEditMulti::FrmCodeEditMulti(QWidget *parent) :
    QDialog(parent),
    m_lock_cab(* new LockCabinetWidget(this, 2)),
    ui(new Ui::FrmCodeEditMulti)
{
    ui->setupUi(this);

    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setDisabled(true);

    FrmCodeEditMulti::showFullScreen();

    connect(&m_lock_cab, SIGNAL(NotifyLockSelected(QString, bool)), this, SLOT(OnNotifyLockSelected(QString, bool)));

    ui->hloLockCabinet->addWidget(&m_lock_cab);

    updateAccessType(ACCESS_TYPE_ALWAYS);
    ui->dtStartAccess->setDateTime(QDateTime::currentDateTime());
    ui->dtStartAccess->setVisible(false);
    ui->dtEndAccess->setDateTime(QDateTime::currentDateTime());
    ui->dtEndAccess->setVisible(false);

    ui->lblStartAccessTextOverlay->setText(tr("ALWAYS"));
    ui->lblStartAccessTextOverlay->setVisible(true);
    ui->lblEndAccessTextOverlay->setText(tr("ACTIVE"));
    ui->lblEndAccessTextOverlay->setVisible(true);

    ui->edCode1->setText("");
    ui->edCode2->setText("");
    ui->cbEnableCode2->setChecked(false);
    ui->edUsername->setText("");

    ui->cbEnableQuestions->setEnabled(true);
    ui->cbEnableQuestions->setChecked(false);
    ui->pbEditQuestions->setDisabled(true);
    resetQuestions();

    updateUi();
}

FrmCodeEditMulti::~FrmCodeEditMulti()
{
    delete ui;
}

void FrmCodeEditMulti::resetQuestions()
{
    m_questions.clear();
    m_questions.append("");
    m_questions.append("");
    m_questions.append("");
}

void FrmCodeEditMulti::displayWarning(QWidget* p_widget, bool is_valid)
{
    QPalette palette;
    palette.setColor(QPalette::Base, is_valid ? Qt::white : Qt::red);
    p_widget->setPalette(palette);
}

void FrmCodeEditMulti::setValues(CLockState * const state)
{
    qDebug() << "FrmCodeEditMulti::setValues";
    state->show();

    resetQuestions();

    // Set the code state to the current values Lock State for tracking changes
    m_code_state.code1 = state->getCode1();
    m_code_state.code2_enabled = state->getCode2() != "";
    m_code_state.code2 = state->getCode2();
    m_code_state.username = state->getDescription();
    m_code_state.start_datetime = state->getStartTime();
    m_code_state.end_datetime = state->getEndTime();
    m_code_state.questions_enabled = state->getAskQuestions();
    m_code_state.question1 = "";
    m_code_state.question2 = "";
    m_code_state.question3 = "";
    m_code_state.fp_enabled = state->getFingerprint1();
    m_code_state.access_type = state->getAccessType();
    m_code_state.locks = state->getLockNums();
    m_code_state.question1 = state->getQuestion1();
    m_code_state.question2 = state->getQuestion2();
    m_code_state.question3 = state->getQuestion3();

    // Set up the UI based on current Lock State
    ui->edCode1->setText(m_code_state.code1);
    ui->edCode2->setText(m_code_state.code2);
    ui->cbEnableCode2->setEnabled(m_code_state.code2_enabled);
    ui->cbEnableCode2->setChecked(m_code_state.code2_enabled);
    ui->edCode2->setEnabled(m_code_state.code2_enabled);
    ui->edUsername->setText(m_code_state.username);

    // Note: updateAccessType controls date/time and text overlays including
    // setting the start/end date/time to the current date/time.  We want to
    // leverage that functionality.  But, since we want to set the start/end
    // date/time to the provided values, we set the start/end date/time after
    // calling updateAccessType [just in case you were wondering :-)].
    updateAccessType(m_code_state.access_type);
    ui->dtStartAccess->setDateTime(m_code_state.start_datetime);
    ui->dtEndAccess->setDateTime(m_code_state.end_datetime);

    ui->cbFingerprint->setChecked(m_code_state.fp_enabled);
    ui->cbEnableQuestions->setChecked(m_code_state.questions_enabled);
    ui->pbEditQuestions->setEnabled(m_code_state.questions_enabled);
    m_questions[0] = m_code_state.question1;
    m_questions[1] = m_code_state.question2;
    m_questions[2] = m_code_state.question3;

    m_lock_cab.enableAllLocks();
    m_lock_cab.clrAllLocks();
    m_lock_cab.setSelectedLocks(m_code_state.locks);

    updateUi();
}

void FrmCodeEditMulti::getValues(CLockState * const state)
{
    state->setCode1(ui->edCode1->text());
    state->setCode2(ui->edCode2->text());
    state->setDescription(ui->edUsername->text());
    state->setStartTime(ui->dtStartAccess->dateTime());
    state->setEndTime(ui->dtEndAccess->dateTime());
    ui->cbFingerprint->isChecked() ? state->setFingerprint1() : state->clearFingerprint1();
    state->setAskQuestions(ui->cbEnableQuestions->isChecked());
    state->setQuestion1(m_questions[0]);
    state->setQuestion2(m_questions[1]);
    state->setQuestion3(m_questions[2]);

    state->setStartTime(ui->dtStartAccess->dateTime());
    state->setEndTime(ui->dtEndAccess->dateTime());
    state->setAccessCount(0);
    state->setMaxAccess(ui->cbAccessType->currentIndex() == ACCESS_TYPE_LIMITED_USE ? 2 : -1);
    state->setAccessType(ui->cbAccessType->currentIndex());
    state->setLockNums(m_lock_cab.getSelectedLocks());
    state->show();
}

void FrmCodeEditMulti::updateAccessType(int index)
{

    Q_ASSERT_X(index >= ACCESS_TYPE_ALWAYS && index <= ACCESS_TYPE_LIMITED_USE,
               Q_FUNC_INFO,
               "access type index out of range");

    bool is_timed = index == ACCESS_TYPE_TIMED;

    ui->cbAccessType->setCurrentIndex(index);

    ui->dtStartAccess->setEnabled(is_timed);
    ui->dtEndAccess->setEnabled(is_timed);
    ui->dtStartAccess->setVisible(is_timed);
    ui->dtEndAccess->setVisible(is_timed);

    ui->lblStartAccessTextOverlay->setVisible(!is_timed);
    ui->lblStartAccessTextOverlay->setEnabled(!is_timed);
    ui->lblEndAccessTextOverlay->setVisible(!is_timed);
    ui->lblEndAccessTextOverlay->setEnabled(!is_timed);

    switch (index)
    {
        case ACCESS_TYPE_TIMED:
            {
            QDateTime curr = QDateTime::currentDateTime();
            // Note: There are checks for start/end access so we set the same date/time to both
            ui->dtStartAccess->setDateTime(curr);
            ui->dtEndAccess->setDateTime(curr);
            }
            break;

        case ACCESS_TYPE_LIMITED_USE:
            ui->lblStartAccessTextOverlay->setText(tr("LIMITED USE (Take/Return only)"));
            ui->lblEndAccessTextOverlay->setText(tr("ACTIVE"));
            ui->dtStartAccess->setDateTime(_DATENONE);
            ui->dtEndAccess->setDateTime(_DATENONE);
            break;

        case ACCESS_TYPE_ALWAYS:
        default:
            ui->lblStartAccessTextOverlay->setText(tr("ALWAYS"));
            ui->lblEndAccessTextOverlay->setText(tr("ACTIVE"));
            ui->dtStartAccess->setDateTime(_DATENONE);
            ui->dtEndAccess->setDateTime(_DATENONE);
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

void FrmCodeEditMulti::on_pbEditQuestions_clicked()
{
    CDlgEditQuestions *eq = new CDlgEditQuestions();
    eq->setValues(m_questions);
    if (eq->exec())
    {
        eq->getValues(m_questions);
    }

    updateUi();
}

void FrmCodeEditMulti::on_cbEnableQuestions_stateChanged(int arg1)
{
    bool result = arg1 == Qt::Checked ? true : false;

    ui->pbEditQuestions->setEnabled(result);
    if (!result)
    {
        resetQuestions();
    }

    updateUi();
}

void FrmCodeEditMulti::on_cbAccessType_currentIndexChanged(int index)
{
    updateAccessType(index);
    updateUi();
}

void FrmCodeEditMulti::on_dtStartAccess_dateTimeChanged(const QDateTime &dateTime)
{
    Q_UNUSED(dateTime);
    if (ui->dtStartAccess->isEnabled())
    {
        updateUi();
    }
}

void FrmCodeEditMulti::on_dtEndAccess_dateTimeChanged(const QDateTime &dateTime)
{
    Q_UNUSED(dateTime);
    if (ui->dtEndAccess->isEnabled())
    {
        updateUi();
    }
}

void FrmCodeEditMulti::on_edCode1_clicked()
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(true);
    kkd.setValue(ui->edCode1->text());
    if (kkd.exec())
    {
        ui->edCode1->setText(kkd.getValue());
    }

    updateUi();
}

void FrmCodeEditMulti::on_edCode2_clicked()
{
    if (ui->edCode2->isEnabled())
    {
        KcbKeyboardDialog kkd;

        kkd.numbersOnly(true);
        kkd.setValue(ui->edCode2->text());
        if (kkd.exec())
        {
            ui->edUsername->setText(kkd.getValue());
        }
    }

    updateUi();
}

void FrmCodeEditMulti::on_edUsername_clicked()
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(false);
    kkd.setValue(ui->edUsername->text());
    if (kkd.exec())
    {
        ui->edUsername->setText(kkd.getValue());
    }

    updateUi();
}

void FrmCodeEditMulti::OnNotifyLockSelected(QString lock, bool is_selected)
{
    qDebug() << "Lock:" << lock << "Is Selected:" << is_selected << "Locks:" << m_lock_cab.getSelectedLocks();
    updateUi();
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
}

bool FrmCodeEditMulti::isModified()
{
    bool code1_changed = m_code_state.code1 != ui->edCode1->text();
    bool fp_changed = m_code_state.fp_enabled != ui->cbFingerprint->isChecked();
    bool code2_changed = m_code_state.code2_enabled != ui->cbEnableCode2->isChecked() || m_code_state.code2 != ui->edCode2->text();
    bool username_changed = m_code_state.username != ui->edUsername->text();
    bool locks_changed = m_code_state.locks != m_lock_cab.getSelectedLocks();
    bool accesstype_changed = m_code_state.access_type != ui->cbAccessType->currentIndex();
    bool datetime_changed = (ui->dtStartAccess->isEnabled() && (m_code_state.start_datetime != ui->dtStartAccess->dateTime())) ||
                            (ui->dtEndAccess->isEnabled() && (m_code_state.end_datetime != ui->dtEndAccess->dateTime()));
    bool questions_changed = m_code_state.questions_enabled != ui->cbEnableQuestions->isChecked() ||
                             m_code_state.question1 != m_questions[0] ||
                             m_code_state.question2 != m_questions[1] ||
                             m_code_state.question3 != m_questions[2];

//    qDebug() << "Code1 Changed" << code1_changed;
//    qDebug() << "FP Changed" << fp_changed;
//    qDebug() << "Code2 Changed" << code2_changed;
//    qDebug() << "Username Changed" << username_changed;
//    qDebug() << "Locks Changed" << locks_changed;
//    qDebug() << "Access Type Changed" << accesstype_changed;
//    qDebug() << "DateTime Changed" << datetime_changed;
//    qDebug() << "Questions Changed" << questions_changed;

    return code1_changed || fp_changed || code2_changed || username_changed ||
           accesstype_changed || locks_changed || questions_changed ||
           datetime_changed;
}

void FrmCodeEditMulti::updateUi()
{
    // Update UI
    ui->cbEnableCode2->setDisabled(ui->edCode1->text() == "");
    ui->edCode2->setEnabled(ui->cbEnableCode2->isChecked() && ui->edCode1->text() != "");

    ui->pbClearCode1->setDisabled(ui->edCode1->text() == "");
    ui->pbClearCode2->setDisabled(ui->edCode2->text() == "");
    ui->pbClearUsername->setDisabled(ui->edUsername->text() == "");

    // Note:
    // If fingerprint is enabled, code 2 is not allowed and will be cleared
    if (ui->cbFingerprint->isChecked())
    {
        ui->edCode2->setText("");
        ui->edCode2->setDisabled(true);
        ui->cbEnableCode2->setDisabled(true);
    }

    // Update exit condition
    bool code1_valid_text = ui->edCode1->text() != "";
    bool code2_valid_text = ui->edCode2->text() != "";
    bool code2_valid = !ui->cbEnableCode2->isChecked() || (ui->cbEnableCode2->isChecked() && code2_valid_text);
    bool fp_valid = !ui->cbFingerprint->isChecked() || (ui->cbFingerprint->isChecked() && code1_valid_text);
    bool locks_valid = m_lock_cab.getSelectedLocks() != "";    
    bool start_end_valid = ui->cbAccessType->currentIndex() == ACCESS_TYPE_ALWAYS ||
                           ui->cbAccessType->currentIndex() == ACCESS_TYPE_LIMITED_USE ||
                           (
                               (ui->cbAccessType->currentIndex() == ACCESS_TYPE_TIMED) &&
                               (ui->dtEndAccess->dateTime() > ui->dtStartAccess->dateTime())
                           );
    bool questions_valid = !ui->cbEnableQuestions->isChecked() ||
                           (ui->cbEnableQuestions->isChecked() &&
                            (m_questions[0] != "" ||
                             m_questions[1] != "" ||
                             m_questions[2] != ""));

    displayWarning(qobject_cast<QWidget *>(ui->edCode1), code1_valid_text);
    displayWarning(qobject_cast<QWidget *>(ui->cbFingerprint), fp_valid);
    displayWarning(qobject_cast<QWidget *>(ui->edCode2), code2_valid);
    displayWarning(qobject_cast<QWidget *>(ui->cbEnableCode2), code2_valid);
    displayWarning(qobject_cast<QWidget *>(ui->dtStartAccess), start_end_valid);
    displayWarning(qobject_cast<QWidget *>(ui->dtEndAccess), start_end_valid);
    displayWarning(qobject_cast<QWidget *>(ui->cbEnableQuestions), questions_valid);
    displayWarning(qobject_cast<QWidget *>(ui->pbEditQuestions), questions_valid);
    if (locks_valid)
    {
        m_lock_cab.clrWarning();
    }
    else
    {
        m_lock_cab.setWarning();
    }

    qDebug() << "Exit Condition:";
    qDebug() << "\tModified:" << isModified();
    qDebug() << "\tCode1 Text:" << code1_valid_text;
    qDebug() << "\tCode2 Text:" << code2_valid_text;
    qDebug() << "\tCode2 Valid:" << code2_valid;
    qDebug() << "\tFP Valid:" << fp_valid;
    qDebug() << "\tSelected Locks:" << locks_valid;
    qDebug() << "\tStart/End:" << start_end_valid;
    qDebug() << "\tQuestions:" << questions_valid;

    bool valid_exit = isModified() &&
                      code1_valid_text &&
                      code2_valid &&
                      fp_valid &&
                      locks_valid &&
                      start_end_valid &&
                      questions_valid;

    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setEnabled(valid_exit);
}

void FrmCodeEditMulti::clrCodeState()
{
    m_code_state.code1 = "";
    m_code_state.code2 = "";
    m_code_state.code2_enabled = false;
    m_code_state.fp_enabled = false;
    m_code_state.access_type = 0;
    m_code_state.locks = "";
    m_code_state.question1 = "";
    m_code_state.question2 = "";
    m_code_state.question3 = "";
    m_code_state.questions_enabled = false;
}

void FrmCodeEditMulti::on_bbSaveCancel_accepted()
{
    clrCodeState();
}

void FrmCodeEditMulti::on_bbSaveCancel_rejected()
{
    clrCodeState();
}

void FrmCodeEditMulti::on_cbFingerprint_clicked()
{
    if (ui->cbFingerprint->isChecked())
    {
        QString title = tr("Warning! Adding Fingerprint Authentication");
        QString message = tr("You are enabling fingerprint authentication.\n"
                             "Fingerprint authentication is allowed only for Code #1.\n"
                             "If Code #2 is set, it will be cleared and disabled.\n"
                             "Do you want to continue?");

        int nRC = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::Cancel);

        if( nRC == QMessageBox::Cancel )
        {
            ui->cbFingerprint->setChecked(false);
        }
    }
    else
    {
        QString printDirectory = "/home/pi/run/prints/" + ui->edCode1->text();
        if( QDir(printDirectory).exists() )
        {
            QString title = tr("Verify Fingerprint Scan Removal");
            QString message = tr("Saved fingerprint scan exists for this code.\n"
                                 "Do you want to remove it?");
            int nRC = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::Cancel);
            if (nRC == QMessageBox::Yes)
            {
                std::system( ("sudo rm -rf " + printDirectory.toStdString()).c_str());
            }
            else
            {
                ui->cbFingerprint->setChecked(true);
            }

        }
    }

    updateUi();
}
