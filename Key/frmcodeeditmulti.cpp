#include "frmcodeeditmulti.h"
#include "ui_frmcodeeditmulti.h"

#include <QSignalMapper>
#include <QDebug>
#include <QTouchEvent>
#include <QMessageBox>
#include <QDir>
#include <QLabel>
#include <QFont>

#include "lockcabinetwidget.h"
#include "lockstate.h"
#include "kcbcommon.h"
#include "kcbutils.h"
#include "kcbsystem.h"
#include "kcbkeyboarddialog.h"
#include "dlgeditquestions.h"
#include "keycodeboxsettings.h"
#include "autocodegenstatic.h"

static const QString FP_HOME_DIR = "/home/pi/run/prints/";
static const QString css_warn = "color: black; background-color: red";
static const QString css_none = "";

FrmCodeEditMulti::FrmCodeEditMulti(QWidget *parent) :
    QDialog(parent),
    m_lock_cab(* new LockCabinetWidget(this)),
    m_initialized(false),
    ui(new Ui::FrmCodeEditMulti),
    m_access_type_label(*new QLabel(this)),
    m_access_state_label(*new QLabel(this)),
    m_warn_no_locks_selected(true)
{
    // KCB_DEBUG_ENTRY;
    ui->setupUi(this);

    kcb::SetWindowParams(this);

    connect(&m_lock_cab, SIGNAL(NotifyLockSelected(QString, bool)), this, SLOT(OnNotifyLockSelected(QString, bool)));
    connect(&m_lock_cab, SIGNAL(NotifyNoLocksSelected()), this, SLOT(OnNotifyNoLocksSelected()));

    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->bbSaveCancel->button(QDialogButtonBox::Cancel)->setEnabled(true);

    ui->vloLockCabinet->addWidget(&m_lock_cab);

    ui->edCode1->setText("");
    ui->pbCode1Random->setEnabled(true);
    ui->spCode1RandomCodeLength->setEnabled(true);
    ui->cbFingerprint->setDisabled(true);
    ui->edCode2->setText("");
    ui->edCode2->setDisabled(true);
    ui->cbEnableCode2->setChecked(false);
    ui->edUsername->setText("");

    ui->cbEnableQuestions->setDisabled(true);
    ui->cbEnableQuestions->setChecked(false);
    ui->pbEditQuestions->setDisabled(true);

    updateAccessType(ACCESS_TYPE_ALWAYS);
    ui->dtStartAccess->setDateTime(QDateTime::currentDateTime());
    ui->dtStartAccess->setVisible(false);
    ui->dtEndAccess->setDateTime(QDateTime::currentDateTime());
    ui->dtEndAccess->setVisible(false);

    m_access_type_label.setText("");
    m_access_state_label.setText("");
    m_access_type_label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_access_state_label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_access_type_label.setEnabled(false);
    m_access_type_label.hide();
    m_access_state_label.setEnabled(false);
    m_access_state_label.hide();

    resetQuestions();
    updateCabinetConfig();
    OnLockSelectionChanged();

    ui->cbAccessTypeAllCodes->setChecked(KeyCodeBoxSettings::IsApplyAccessTypeToAllCodes());

    updateUi();
    // KCB_DEBUG_EXIT;
}

FrmCodeEditMulti::~FrmCodeEditMulti()
{
    kcb::ClassAllocation::DestructorMsg(this);
    delete ui;
}

void FrmCodeEditMulti::updateCabinetConfig()
{
    // KCB_DEBUG_ENTRY;
    m_lock_cab.setLockDisplay();
    m_lock_cab.updateCabinetConfig();
    // KCB_DEBUG_EXIT;
}

void FrmCodeEditMulti::resetQuestions()
{
    m_questions.clear();
    m_questions.append("");
    m_questions.append("");
    m_questions.append("");
}

void FrmCodeEditMulti::setValues(CLockState * const state, const QStringList& codes1_in_use, const QStringList& codes2_in_use)
{
    // KCB_DEBUG_ENTRY;

    resetQuestions();
    m_codes1_in_use = codes1_in_use;
    m_codes2_in_use = codes2_in_use;

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
    m_code_state.fp_deleted = false;
    m_code_state.access_type = state->getAccessType();
    m_code_state.locks = state->getLockNums();
    m_code_state.question1 = state->getQuestion1();
    m_code_state.question2 = state->getQuestion2();
    m_code_state.question3 = state->getQuestion3();
    m_code_state.autocode = state->getAutoCode();
    m_code_state.apply_to_all = KeyCodeBoxSettings::IsApplyAccessTypeToAllCodes();

    // Set up the UI based on current Lock State
    ui->edCode1->setText(m_code_state.code1);

    ui->cbFingerprint->setEnabled(m_code_state.fp_enabled);
    ui->cbFingerprint->setChecked(m_code_state.fp_enabled);

    ui->edCode2->setText(m_code_state.code2);
    ui->cbEnableCode2->setEnabled(m_code_state.code2_enabled);
    ui->cbEnableCode2->setChecked(m_code_state.code2_enabled);
    ui->edCode2->setEnabled(m_code_state.code2_enabled);
    ui->edUsername->setText(m_code_state.username);

    ui->cbEnableQuestions->setEnabled(m_code_state.code2_enabled);
    ui->cbEnableQuestions->setChecked(m_code_state.questions_enabled);
    ui->pbEditQuestions->setEnabled(m_code_state.questions_enabled);
    m_questions[0] = m_code_state.question1;
    m_questions[1] = m_code_state.question2;
    m_questions[2] = m_code_state.question3;

    // Note: updateAccessType controls date/time and text overlays including
    // setting the start/end date/time to the current date/time.  We want to
    // leverage that functionality.  But, since we want to set the start/end
    // date/time to the provided values, we set the start/end date/time after
    // calling updateAccessType [just in case you were wondering :-)].
    updateAccessType(m_code_state.access_type);
    ui->dtStartAccess->setDateTime(m_code_state.start_datetime);
    ui->dtEndAccess->setDateTime(m_code_state.end_datetime);

    m_lock_cab.enableAllLocks();
    m_lock_cab.setSelectedLocks(m_code_state.locks);

    updateUi();

    m_initialized = true;
    // KCB_DEBUG_EXIT;
}

void FrmCodeEditMulti::getValues(CLockState * const state, bool& apply_access_type_to_all)
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

    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->bbSaveCancel->button(QDialogButtonBox::Cancel)->setEnabled(true);

    m_initialized = false;
    m_codes1_in_use.clear();
    m_codes2_in_use.clear();

    apply_access_type_to_all = ui->cbAccessTypeAllCodes->isChecked();
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

    m_access_type_label.setText("");
    m_access_state_label.setText("");
    m_access_type_label.setEnabled(!is_timed);
    m_access_state_label.setEnabled(!is_timed);
    m_access_type_label.setVisible(!is_timed);
    m_access_state_label.setVisible(!is_timed);
    ui->hloStartAccess->removeWidget(&m_access_type_label);
    ui->hloEndAccess->removeWidget(&m_access_state_label);
    ui->dtStartAccess->setVisible(is_timed);
    ui->dtEndAccess->setVisible(is_timed);

    switch (index)
    {
        case ACCESS_TYPE_TIMED:
            {
            QDateTime curr = QDateTime::currentDateTime();
            // Note: There are checks for start/end access so we set the same date/time to both
            ui->dtStartAccess->setVisible(true);
            ui->dtEndAccess->setVisible(true);
            ui->dtStartAccess->setDateTime(curr);
            ui->dtEndAccess->setDateTime(curr);
            }
            break;

        case ACCESS_TYPE_LIMITED_USE:
            {
            ui->hloStartAccess->addWidget(&m_access_type_label);
            ui->hloEndAccess->addWidget(&m_access_state_label);
            m_access_type_label.setText(tr("LIMITED USE (Take/Return only)"));
            m_access_state_label.setText(tr("ACTIVE"));
            m_access_type_label.show();
            m_access_state_label.show();
            ui->dtStartAccess->setDateTime(DEFAULT_DATETIME);
            ui->dtEndAccess->setDateTime(DEFAULT_DATETIME);
            }
            break;

        case ACCESS_TYPE_ALWAYS:
        default:
            {
            ui->hloStartAccess->addWidget(&m_access_type_label);
            ui->hloEndAccess->addWidget(&m_access_state_label);
            m_access_type_label.setText(tr("ALWAYS"));
            m_access_state_label.setText(tr("ACTIVE"));
            m_access_type_label.show();
            m_access_state_label.show();
            ui->dtStartAccess->setDateTime(DEFAULT_DATETIME);
            ui->dtEndAccess->setDateTime(DEFAULT_DATETIME);
            }
            break;
    }


}

void FrmCodeEditMulti::on_pbClearCode1_clicked()
{
    ui->edCode1->setText("");
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
    CDlgEditQuestions *eq = new CDlgEditQuestions(this);
    eq->setValues(m_questions);
    if (eq->exec())
    {
        eq->getValues(m_questions);
    }

    updateUi();
}

void FrmCodeEditMulti::on_cbEnableQuestions_stateChanged(int state)
{
    if (ui->cbFingerprint->isChecked() ||
        !ui->cbEnableCode2->isChecked())
    {
        // Note: If fingerprint is enabled, we handle the disabling of
        // code2 and questions in updateUi.
        // Note: If code 2 enable is unchecked, we already handled the disabling of
        // questions.
        return;
    }

    if (state == Qt::Unchecked)
    {
        if (m_questions[0] != "" || m_questions[1] != "" || m_questions[2] != "")
        {
            QString title = tr("Warning! Disabling Questions");
            QString message = tr("You have selected to disable questions.\n"
                                 "This will clear ALL existing questions.\n"
                                 "Do you want to continue?");
            int result = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::No);
            if (result == QMessageBox::Yes)
            {
                resetQuestions();
            }
            else if( result == QMessageBox::No )
            {
                ui->cbEnableQuestions->setCheckState(Qt::Checked);
            }
        }
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
    // KCB_DEBUG_ENTRY;
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(true);
    kkd.setValue(ui->edCode1->text(), m_codes1_in_use);
    if (kkd.exec())
    {
        ui->edCode1->setText(kkd.getValue());
    }

    updateUi();
    // KCB_DEBUG_EXIT;
}

void FrmCodeEditMulti::on_edCode2_clicked()
{
    if (ui->edCode2->isEnabled())
    {
        KcbKeyboardDialog kkd;

        kkd.numbersOnly(true);
        kkd.setValue(ui->edCode2->text(), m_codes2_in_use);
        if (kkd.exec())
        {
            ui->edCode2->setText(kkd.getValue());
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

void FrmCodeEditMulti::OnNotifyNoLocksSelected()
{
    KCB_DEBUG_ENTRY;
    warn_no_locks_selected();
    KCB_DEBUG_EXIT;
}

void FrmCodeEditMulti::OnNotifyLockSelected(QString lock, bool is_selected)
{
    // KCB_DEBUG_ENTRY;
    Q_UNUSED(lock);

    if (AutoCodeGeneratorStatic::IsEnabled())
    {
        auto code2map = AutoCodeGeneratorStatic::GetCurrentCode2Codes();

        if (!code2map.empty())
        {
            if (is_selected)
            {
                ui->edCode2->setText(code2map[lock]);
            }
            else
            {
                ui->edCode2->setText("");
            }
        }
    }
    else
    {
        // Nothing to do here
    }

    updateUi();
    // KCB_DEBUG_EXIT;
}

void FrmCodeEditMulti::on_cbEnableCode2_stateChanged(int state)
{
    // KCB_DEBUG_ENTRY;
    if (ui->cbFingerprint->isChecked() || !m_initialized)
    {
        return;
    }

    if (state == Qt::Unchecked)
    {
        QString title = tr("Warning! Disabling Code 2");
        QString message = tr("You have selected to disable Code 2.\n"
                             "Continuing will clear Code 2 and ALL existing questions.\n"
                             "Do you want to continue?");
        int result = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            disableCode2();
            disableQuestions();
        }
        else if( result == QMessageBox::No )
        {
            ui->cbEnableCode2->setCheckState(Qt::Checked);
        }
    }

    updateUi();
    // KCB_DEBUG_EXIT;
}

bool FrmCodeEditMulti::isModified()
{
    bool code1_changed = m_code_state.code1 != ui->edCode1->text();
    bool fp_changed = m_code_state.fp_enabled != ui->cbFingerprint->isChecked() || m_code_state.fp_deleted;
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
    bool apply_to_all_changed = m_code_state.apply_to_all != ui->cbAccessTypeAllCodes->isChecked();

    // KCB_DEBUG_TRACE("Code1 Changed" << code1_changed);
    // KCB_DEBUG_TRACE("FP Changed" << fp_changed);
    // KCB_DEBUG_TRACE("Code2 Changed" << code2_changed);
    // KCB_DEBUG_TRACE("Username Changed" << username_changed);
    // KCB_DEBUG_TRACE("Locks Changed" << locks_changed);
    // KCB_DEBUG_TRACE("Access Type Changed" << accesstype_changed);
    // KCB_DEBUG_TRACE("DateTime Changed" << datetime_changed);
    // KCB_DEBUG_TRACE("Questions Changed" << questions_changed);

    return code1_changed || fp_changed || code2_changed || username_changed ||
           accesstype_changed || locks_changed || questions_changed ||
           datetime_changed || apply_to_all_changed;
}

void FrmCodeEditMulti::disableCode2()
{
    ui->cbEnableCode2->setCheckState(Qt::Unchecked);
    ui->cbEnableCode2->setDisabled(true);
    ui->edCode2->setText("");
    ui->edCode2->setDisabled(true);
    ui->pbClearCode2->setDisabled(true);
}

void FrmCodeEditMulti::disableQuestions()
{
    ui->cbEnableQuestions->setCheckState(Qt::Unchecked);
    ui->cbEnableQuestions->setDisabled(true);
    ui->pbEditQuestions->setDisabled(true);
    resetQuestions();
}

void FrmCodeEditMulti::updateUi()
{
    // Update exit condition
    // Rules:
    //    At minimum, Code 1 must specified and 1 or more locks must be selected
    //    If Code 1 is specified, then Fingerprint can be enabled
    //    If Code 1 is specified, Code 2 can be enabled
    //    If Fingerprint is enabled, Code 2 is disallowed (handled in fingerprint slot handler)
    //    If Code 2 is enabled, Code 2 must be specified
    //    If Code 2 is specified, then Questions can be enabled
    //    If Questions are enabled, Questions must be specified
    bool autocode_enabled = AutoCodeGeneratorStatic::IsEnabled();
    bool autocode_code1mode = AutoCodeGeneratorStatic::IsCode1Mode();
    bool autocode_code2mode = AutoCodeGeneratorStatic::IsCode2Mode();

    bool code1_is_specified = ui->edCode1->text() != "";
    bool min_locks_selected = !m_lock_cab.getSelectedLocks().isEmpty();
    bool fp_is_required = ui->cbFingerprint->isChecked();
    bool code2_is_required = ui->cbEnableCode2->isChecked();
    bool code2_is_specified = ui->edCode2->text() != "";
    bool code2_is_valid = (autocode_code2mode || !code2_is_required || (code2_is_required && code2_is_specified));
    bool questions_required = ui->cbEnableQuestions->isChecked();
    bool questions_specified = ( (m_questions[0] != "") ||
                                 (m_questions[1] != "") ||
                                 (m_questions[2] != "") );
    bool questions_valid = (!questions_required || (questions_required && questions_specified));

    bool valid_access_type = ui->cbAccessType->currentIndex() == ACCESS_TYPE_ALWAYS ||
                             ui->cbAccessType->currentIndex() == ACCESS_TYPE_LIMITED_USE ||
                             (
                                 (ui->cbAccessType->currentIndex() == ACCESS_TYPE_TIMED) &&
                                 (ui->dtEndAccess->dateTime() > ui->dtStartAccess->dateTime())
                             );

    ui->edCode1->setEnabled(!autocode_enabled);
    ui->pbCode1Random->setEnabled(!autocode_enabled);
    ui->spCode1RandomCodeLength->setEnabled(!autocode_enabled);
    ui->pbClearCode1->setEnabled(code1_is_specified);
    ui->cbFingerprint->setEnabled(code1_is_specified);
    ui->cbEnableCode2->setEnabled(!fp_is_required && code1_is_specified);
    ui->edCode2->setEnabled(!autocode_code2mode && !fp_is_required && code1_is_specified && code2_is_required);
    ui->pbCode2Random->setEnabled(!autocode_enabled && !fp_is_required && code1_is_specified && code2_is_required);
    ui->spCode2RandomCodeLength->setEnabled(!autocode_enabled && !fp_is_required && code1_is_specified && code2_is_required);
    ui->pbClearCode2->setEnabled(!fp_is_required && code1_is_specified && code2_is_specified && code2_is_required);
    ui->cbEnableQuestions->setEnabled(!fp_is_required && code1_is_specified && code2_is_specified && code2_is_required);
    ui->pbEditQuestions->setEnabled(!fp_is_required && code1_is_specified && questions_required);

    ui->edCode1->setStyleSheet(!code1_is_specified ? css_warn : css_none);
    ui->edCode2->setStyleSheet(!code2_is_valid ? css_warn : css_none);
    ui->pbEditQuestions->setStyleSheet(!questions_valid ? css_warn : css_none);
    ui->dtStartAccess->setStyleSheet(!valid_access_type ? css_warn : css_none);
    ui->dtEndAccess->setStyleSheet(!valid_access_type ? css_warn : css_none);

    bool valid_codes_entered = code1_is_specified &&
                               code2_is_valid &&
                               questions_valid &&
                               valid_access_type;

    if (min_locks_selected)
    {
        m_lock_cab.clrWarning();
    }
    else
    {
        m_lock_cab.setWarning();
    }

    bool valid_exit = isModified() && valid_codes_entered;

    ui->bbSaveCancel->button(QDialogButtonBox::Save)->setEnabled(valid_exit);

    if (autocode_code1mode)
    {
        ui->edCode1->setDisabled(autocode_code1mode);
        ui->pbClearCode1->setDisabled(autocode_code1mode);
        ui->cbFingerprint->setDisabled(autocode_code1mode);
        ui->edCode2->setDisabled(autocode_code1mode);
        ui->pbClearCode2->setDisabled(autocode_code1mode);
        ui->cbEnableCode2->setDisabled(autocode_code1mode);
        m_lock_cab.disableAllLocks();
        m_lock_cab.hideSelectClearAll();
    }
    if (autocode_code2mode)
    {
        ui->edCode1->setEnabled(autocode_code2mode);
        ui->pbClearCode1->setEnabled(autocode_code2mode);
        ui->cbFingerprint->setDisabled(autocode_code2mode);
        ui->edCode2->setDisabled(autocode_code2mode);
        ui->pbClearCode2->setDisabled(autocode_code2mode);
        ui->cbEnableCode2->setDisabled(autocode_code2mode);
        m_lock_cab.hideSelectClearAll();
    }
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
    m_code_state.fp_deleted = false;
    m_initialized = false;
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
                             "Continuing will clear and disable Code 1.\n"
                             "Continuing will clear and disable ALL existing questions.\n"
                             "Do you want to continue?");

        int result = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::No);
        if( result == QMessageBox::Yes )
        {
            disableCode2();
            disableQuestions();
        }
        else if( result == QMessageBox::No )
        {
            ui->cbFingerprint->setCheckState(Qt::Unchecked);
        }
    }
    else
    {
        if (!ui->edCode1->text().isEmpty())
        {
            QString printDirectory = FP_HOME_DIR + ui->edCode1->text();
            if( QDir(printDirectory).exists() )
            {
                QString title = tr("Verify Fingerprint Scan Removal");
                QString message = tr("Saved fingerprint scan exists for this code.\n"
                                     "Do you want to remove it?");
                int nRC = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::Cancel);
                if (nRC == QMessageBox::Yes)
                {
                    std::system( ("sudo rm -rf " + printDirectory.toStdString()).c_str());
                    m_code_state.fp_deleted = true;
                    ui->bbSaveCancel->button(QDialogButtonBox::Cancel)->setDisabled(true);
                }
                else
                {
                    ui->cbFingerprint->setCheckState(Qt::Checked);
                }
            }
        }
    }

    updateUi();
}

void FrmCodeEditMulti::on_pbCode1Random_clicked()
{
    QStringList values = kcb::CreateRandomValues(1, ui->spCode1RandomCodeLength->value());
    ui->edCode1->setText(values[0]);
    updateUi();
}

void FrmCodeEditMulti::on_pbCode2Random_clicked()
{
    QStringList values = kcb::CreateRandomValues(1, ui->spCode2RandomCodeLength->value());
    ui->edCode2->setText(values[0]);
    updateUi();
}

void FrmCodeEditMulti::OnLockSelectionChanged()
{
    // KCB_DEBUG_ENTRY;
    if (KeyCodeBoxSettings::IsLockSelectionSingle())
    {
        m_lock_cab.OnNotifySingleLockSelection();
    }
    else if (KeyCodeBoxSettings::IsLockSelectionMulti())
    {
        m_lock_cab.OnNotifyMultiLockSelection();
    }
    else
    {
        m_lock_cab.OnNotifyDisableLockSelection();
    }
    // KCB_DEBUG_EXIT;
}

void FrmCodeEditMulti::warn_no_locks_selected()
{
    if (m_warn_no_locks_selected)
    {
        QMessageBox msgbox;

        msgbox.setWindowTitle(tr("Lock Selection"));
        msgbox.setText(tr("Warning! No lock has been selected."
                            "\n\n"
                            "A lock must be associated with a code in order to open a door."));
        msgbox.addButton(QMessageBox::Ok);
        msgbox.setWindowModality(Qt::ApplicationModal);

        QCheckBox* cb = new QCheckBox(tr("Don't ask again."));
        cb->setChecked(false);
        msgbox.setCheckBox(cb);

        int result = msgbox.exec();
        if (result == QMessageBox::Ok)
        {
            m_warn_no_locks_selected = !cb->isChecked();
        }
    }
}
void FrmCodeEditMulti::on_cbAccessTypeAllCodes_clicked()
{
    if (ui->cbAccessTypeAllCodes->isChecked())
    {
        KeyCodeBoxSettings::EnableApplyAccessTypeToAllCodes();
    }
    else
    {
        KeyCodeBoxSettings::DisableApplyAccessTypeToAllCodes();
    }
    updateUi();
}
