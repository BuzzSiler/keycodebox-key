#include "autocodegenwidget.h"
#include "ui_autocodegenwidget.h"

#include <unistd.h>

#include <QDebug>
#include <QMessageBox>
#include <QVector>
#include <QBitmap>
#include <QPixmap>
#include <QCryptographicHash>
#include <QTimer>
#include <QProgressDialog>

#include "kcbutils.h"
#include "kcbcommon.h"
#include "lockcabinetwidget.h"
#include "kcbkeyboarddialog.h"
#include "kcbsystem.h"
#include "encryption.h"
#include "autocodegenerator.h"
#include "autocodegenstatic.h"

#define VALUE_TO_INDEX(v) ((v) - 1)
#define INDEX_TO_VALUE(i) ((i) + 1)

struct AutoCodePeriodSetting
{
    QString text;
    QStringList values;
    int index;
};

enum class Periods { SHIFT, DAILY, WEEKLY, MONTHLY, YEARLY };


static const QString css_warn = "color: black; background-color: red";
static const QString css_none = "";


static const QString CODE1_MODE_MSG = QObject::tr("AutoCode Generation is in Code 1 Mode."
                     "\n"
                     "The following changes will be made:"
                     "\n"
                     "\t1. All existing codes will be deleted."
                     "\n"
                     "\t2. New codes will be generated for all locks."
                     "\n"
                     "\t3. Multi-lock selection will be disabled."
                     );

static const QString CODE2_MODE_MSG = QObject::tr("AutoCode Generation is in Code 2 Mode."
                     "\n"
                     "All existing codes will be preserved and modified as follows:"
                     "\n"
                     "\t1. All associated locks for all codes will be cleared."
                     "\n"
                     "\t2. All 'Code1' codes will remain unchanged."
                     "\n"
                     "\t3. All 'Code2' codes will have the 'code2' field cleared."
                     "\n"
                     "\t4. Multi-lock selection will be disabled.");

static const AutoCodePeriodSetting AUTOCODE_PERIOD_SETTINGS[] = {
    {
        "Hours:", QStringList() << "6" << "8" << "12", 1
    },
    {
        "Days:", QStringList() << "1" << "2" << "3", 0
    },
    {
        "Weeks:", QStringList() << "1" << "2" << "3" << "5" << "6", 0
    },
    {
        "Months:", QStringList() << "1" << "2" << "3" << "4" << "6", 0
    },
    {
        "Years:", QStringList() << "1", 0
    }
};

AutoCodeGenWidget::AutoCodeGenWidget(QWidget *parent) : 
    QWidget(parent),
    m_lock_cabinet(* new LockCabinetWidget(this)),
    m_init(true),
    m_params({}),
    ui(new Ui::AutoCodeGenWidget)
{
    KCB_DEBUG_ENTRY;
    ui->setupUi(this);

    ui->pgbAutoCodeCommit->setVisible(false);

    InitLockCabinet();

    if (AutoCodeGeneratorStatic::IsEnabled())
    {
        EnableAutoCode();

        AutoCodeGenerator::CodeMap map;
        map = ProcessCommittedAutoCodeSettings(map);
        map = ProcessEnabledAutoCodeSettings(map);
        
        m_lock_cabinet.setLockDisplay(map);
    }
    else
    {
        DisableAutoCode();
    }

    InitControls();

    m_init = false;
    KCB_DEBUG_EXIT;
}

AutoCodeGenWidget::~AutoCodeGenWidget()
{
    kcb::ClassAllocation::DestructorMsg(this);
    delete ui;
}

bool AutoCodeGenWidget::IsCode1Mode()
{
    return m_params.code_mode == static_cast<int>(CodeMode::CODE_MODE_1);
}

bool AutoCodeGenWidget::IsCode2Mode()
{
    return m_params.code_mode == static_cast<int>(CodeMode::CODE_MODE_2);
}

AutoCodeGenerator::CodeMap& AutoCodeGenWidget::RequestCodesFromDatabase(AutoCodeGenerator::CodeMap& map)
{
    if (IsCode1Mode())
    {
        QStringList codes;
        emit RequestCodes1(codes);

        map.clear();
        for (int ii = 0; ii < codes.count(); ++ii)
        {
            map[QString::number(ii + 1)] = codes[ii];
        }
    }
    return map;
}

AutoCodeGenerator::CodeMap& AutoCodeGenWidget::RequestActiveCodes(AutoCodeGenerator::CodeMap& map)
{
    QPair<AutoCodeGenerator::CodeMap, QDateTime> result = AutoCodeGenerator::CreateCodeMap(m_params);
    map = result.first;
    return map;
}

AutoCodeGenerator::CodeMap& AutoCodeGenWidget::ProcessCommittedAutoCodeSettings(AutoCodeGenerator::CodeMap& map)
{
    if (m_settings.enabled && m_settings.committed)
    {
        map = RequestCodesFromDatabase(map);
        map = RequestActiveCodes(map);
    }

    return map;
}

AutoCodeGenerator::CodeMap& AutoCodeGenWidget::ProcessEnabledAutoCodeSettings(AutoCodeGenerator::CodeMap& map)
{
    if (m_settings.enabled && !m_settings.committed)
    {
        map = AutoCodeGeneratorStatic::CreateCodeMapAndStoreNextGenDateTime(m_params);
    }
    return map;
}

void AutoCodeGenWidget::ShowQrCode(const QString& data)
{
    if (data.isEmpty())
    {
        ui->lblQrCode->setPixmap(QPixmap());
    }
    else
    {
        QByteArray ba;
        ba.append(data);
        QPixmap pm = kcb::CreateQrCode(ba);
        ui->lblQrCode->setPixmap(pm.scaled(120, 120, Qt::KeepAspectRatio));
        ui->lblQrCode->setMask(pm.mask());
        ui->lblQrCode->show();
    }
}

void AutoCodeGenWidget::InitLockCabinet()
{
    m_lock_cabinet.OnNotifyDisableLockSelection();
}

void AutoCodeGenWidget::InitControls()
{
    ui->vloAutoCodeActiveCodes->addWidget(&m_lock_cabinet);
    ui->pbAutoCodeGenerate->setDisabled(true);
    ui->pbAutoCodeCommit->setDisabled(true);
}

void AutoCodeGenWidget::on_cbAutoCodeMode_currentIndexChanged(int index)
{
    QString message("");

    if (index == 0)
    {
        message = CODE1_MODE_MSG;
    }
    else if (index == 1)
    {
        message = CODE2_MODE_MSG;
    }
    else
    {
        KCB_DEBUG_TRACE("Invalid index");
        return;
    }

    if (!m_init)
    {
        QMessageBox::warning(this,
                            tr("AutoCode Generation Mode"),
                            message);
    }

    updateUi();
}


void AutoCodeGenWidget::EnableAutoCode()
{
    KeyCodeBoxSettings::EnableAutoCode();
    LoadAutoCodeSettings();
    ui->gbAutoCodeEnableDisable->setTitle("Enabled");
    ui->gbAutoCodeEnableDisable->setChecked(true);
    ui->gbAutoCodeSettings->setEnabled(true);
    ui->gbAutoCodeActiveCodes->setEnabled(true);
    ui->cbAutoCodeMode->setEnabled(true);
    emit NotifyDisableLockSelection();
    emit NotifyAutoCodeEnabled();
}

void AutoCodeGenWidget::DisableAutoCode()
{
    KeyCodeBoxSettings::DisableAutoCode();
    LoadAutoCodeSettings();
    m_lock_cabinet.clearLockDisplay();
    ui->gbAutoCodeEnableDisable->setChecked(false);
    ui->gbAutoCodeEnableDisable->setTitle("Disabled");
    ui->gbAutoCodeSettings->setEnabled(false);
    ui->gbAutoCodeActiveCodes->setEnabled(false);
    ShowQrCode("");
    emit NotifyEnableLockSelection();
    emit NotifyAutoCodeDisabled();
}

int AutoCodeGenWidget::GetUnitIndex()
{
    int period_index = ui->cbAutoCodePeriod->currentIndex();
    bool valid_unit = AUTOCODE_PERIOD_SETTINGS[period_index].values.contains(QString::number(m_params.units));
    int index = AUTOCODE_PERIOD_SETTINGS[period_index].index;
    if (valid_unit)
    {
        index = ui->cbAutoCodePeriodUnits->findText(QString::number(m_params.units));
    }

    return index;
}

void AutoCodeGenWidget::LoadAutoCodeSettings()
{
    m_settings = KeyCodeBoxSettings::GetAutoCodeSettings();
    ui->leAutoCodePassword->setText(m_settings.password);

    m_params = AutoCodeGenerator::ParamsFromSecureKey(m_settings.key.toUtf8(), m_settings.password);

    ui->cbAutoCodeMode->setCurrentIndex(VALUE_TO_INDEX(m_params.code_mode));
    ui->teAutoCodeStartOfDay->setTime(QTime(VALUE_TO_INDEX(m_params.startofday), 0, 0));
    ui->spAutoCodeCodeLength->setValue(m_params.codelength);
    ui->cbAutoCodePeriod->setCurrentIndex(VALUE_TO_INDEX(m_params.period));
    int index = GetUnitIndex();
    ui->cbAutoCodePeriodUnits->setCurrentIndex(index);
    ui->cbAutoCodeEmailKey->setChecked(m_settings.email);

    ShowQrCode(m_settings.key);

    m_params.numcodes = KeyCodeBoxSettings::getTotalLocks();
    m_params.id = kcb::GetSystemId();
}

void AutoCodeGenWidget::StoreAutoCodeSettings()
{
    m_settings.password = ui->leAutoCodePassword->text();
    m_settings.email = ui->cbAutoCodeEmailKey->isChecked();

    KeyCodeBoxSettings::setAutoCodeSettings(m_settings);
}

void AutoCodeGenWidget::UpdateAutoCodeSettings()
{
    StoreAutoCodeSettings();
    LoadAutoCodeSettings();
}

void AutoCodeGenWidget::UpdateAutoCodeParams()
{
    m_params.code_mode = INDEX_TO_VALUE(ui->cbAutoCodeMode->currentIndex());
    m_params.codelength = ui->spAutoCodeCodeLength->value();
    m_params.startofday = INDEX_TO_VALUE(ui->teAutoCodeStartOfDay->time().hour());
    m_params.period = INDEX_TO_VALUE(ui->cbAutoCodePeriod->currentIndex());
    m_params.units = ui->cbAutoCodePeriodUnits->currentText().toInt();
    m_params.numcodes = KeyCodeBoxSettings::getTotalLocks();
    m_params.id = kcb::GetSystemId();
}

void AutoCodeGenWidget::on_pbAutoCodeGenerate_clicked()
{
    DisplayParams(m_params);
    if (ui->leAutoCodePassword->text().isEmpty())
    {
        (void) QMessageBox::warning(this,
                            tr("AutoCode Generate"),
                            tr("The password field is required."
                                "\n"
                                "Please enter a password (1 to 20 characters)."),
                            QMessageBox::StandardButton::Ok);
        return;
    }
    ui->pbAutoCodeGenerate->setDisabled(true);

    QApplication::processEvents();
    QProgressDialog progress("Generating codes ...", "", 1, 100, this);
    progress.setMinimumWidth(100);
    progress.setCancelButton(nullptr);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("AutoCode Generate");
    progress.show();

    QApplication::processEvents();
    progress.setValue(10);

    m_settings.nextgendatetime = "";
    m_settings.key = "";
    StoreAutoCodeSettings();
    UpdateAutoCodeParams();
    
    QApplication::processEvents();
    progress.setValue(30);

    AutoCodeGeneratorStatic::CreateAndStoreSecureKey(m_params, m_settings.password);

    QApplication::processEvents();
    progress.setValue(60);

    AutoCodeGenerator::CodeMap map = AutoCodeGeneratorStatic::CreateCodeMapAndStoreNextGenDateTime(m_params);

    QApplication::processEvents();
    progress.setValue(90);

    m_lock_cabinet.setLockDisplay(map);

    QApplication::processEvents();
    progress.setValue(100);

    LoadAutoCodeSettings();
    ui->pbAutoCodeCommit->setEnabled(true);
}

bool AutoCodeGenWidget::WarnAutoCodeCommit()
{
    int result = QMessageBox::warning(this,
                         tr("AutoCode Apply"),
                         tr("IMPORTANT: The AutoCode settings will now be applied."
                            "\n"
                            "These changes cannot be undone."
                            "\n"
                            "To accept and apply these settings, press Ok"
                            "\n"
                            "To cancel and abort applying these settings, press Cancel"),
                         QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);

    // Note: This loop is contrary to "best practices", but for some reason the message box
    // remains visible for some time after a button is pressed.  The goal is to force the 
    // message box to be removed before leaving this method.
    if (result == QMessageBox::StandardButton::Ok)
    {
        for (int ii = 0; ii < 100; ++ii)
        {
            QCoreApplication::processEvents();
        }
    }
    return result == QMessageBox::StandardButton::Ok;
}

void AutoCodeGenWidget::SendAutoCodeEmail()
{
    if (ui->cbAutoCodeEmailKey->isChecked())
    {
        // if email is checked, then send an email with the encrypted key
    }
}

void AutoCodeGenWidget::HandleAutoCodeCommit()
{
    if (IsCode1Mode())
    {
        emit CommitCodes1(m_lock_cabinet.getLockDisplay());
    }
    else if (IsCode2Mode())
    {
        emit CommitCodes2(m_lock_cabinet.getLockDisplay());
    }
    else
    {
        KCB_DEBUG_TRACE("Invalid code mode settings");
        return;
    }

    m_settings.committed = true;
    StoreAutoCodeSettings();
    ui->pbAutoCodeCommit->setEnabled(false);
}

void AutoCodeGenWidget::on_pbAutoCodeCommit_clicked()
{
    bool commit = WarnAutoCodeCommit();
    if (commit)
    {
        HandleAutoCodeCommit();
        SendAutoCodeEmail();
        emit NotifyAutoCodeEnabled();
    }
}

void AutoCodeGenWidget::OnLockSelectionChanged()
{
    if (AutoCodeGeneratorStatic::IsEnabled())
    {
        m_lock_cabinet.OnNotifyDisableLockSelection();
    }
}

bool AutoCodeGenWidget::WarnAutoCodeEnable()
{
    int result = QMessageBox::warning(this,
                            tr("AutoCode Enable"),
                            tr("You have selected to enable AutoCode Generation."
                            "\n"
                            "Continuing will enable the KeyCodeBox system to generate lock codes per the AutoCode Generation parameters."
                            "\n"
                            "Existing codes may be modified or deleted and cannot be undone."
                            "\n"
                            "No changes will be made until the 'Apply' button is pressed."
                            "\n\n"
                            "Do you want to continue?"),
                            QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);

    return result == QMessageBox::StandardButton::Ok;
}

bool AutoCodeGenWidget::WarnAutoCodeDisable()
{
    int result = QMessageBox::warning(this,
                            tr("AutoCode Disable"),
                            tr("You have selected to disable AutoCode Generation."
                            "\n"
                            "The KeyCodeBox system will no longer generate lock codes. "
                            "All existing codes will remain unchanged."
                            "\n"
                            "Do you want to continue?"),
                            QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);

    return result == QMessageBox::StandardButton::Ok;
}

void AutoCodeGenWidget::HandleAutoCodeEnable()
{
    if (!m_init)
    {
        bool enable = WarnAutoCodeEnable();
        if (enable)
        {
            EnableAutoCode();
        }
        else
        {
            DisableAutoCode();
        }
    }
    else
    {
        EnableAutoCode();
    }

    updateUi();
}

void AutoCodeGenWidget::HandleAutoCodeDisable()
{
    bool disable = WarnAutoCodeDisable();
    if (disable)
    {
        DisableAutoCode();
    }
    else
    {
        EnableAutoCode();
        updateUi();
    }
}

void AutoCodeGenWidget::on_gbAutoCodeEnableDisable_clicked(bool checked)
{
    if (checked)
    {
        HandleAutoCodeEnable();
    }
    else
    {
        HandleAutoCodeDisable();
    }
}

void AutoCodeGenWidget::on_cbAutoCodePeriod_currentIndexChanged(int index)
{
    auto setting = AUTOCODE_PERIOD_SETTINGS[index];
    ui->lblAutoCodePeriodUnits->setText(setting.text);
    ui->cbAutoCodePeriodUnits->clear();
    ui->cbAutoCodePeriodUnits->addItems(setting.values);
    ui->cbAutoCodePeriodUnits->setCurrentIndex(setting.index);

    updateUi();
}

void AutoCodeGenWidget::on_teAutoCodeStartOfDay_timeChanged(const QTime &time)
{
    Q_UNUSED(time);
    updateUi();
}

void AutoCodeGenWidget::on_spAutoCodeCodeLength_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    updateUi();
}

void AutoCodeGenWidget::on_cbAutoCodePeriodUnits_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateUi();
}

void AutoCodeGenWidget::updateUi()
{
    bool autocode_enabled = ui->gbAutoCodeEnableDisable->isChecked();
    bool units_changed = m_params.units != ui->cbAutoCodePeriodUnits->currentText().toInt();
    bool code_len_changed = m_params.codelength != ui->spAutoCodeCodeLength->value();
    bool startofday_changed = m_params.startofday != INDEX_TO_VALUE(ui->teAutoCodeStartOfDay->time().hour());
    bool password_changed = m_settings.password != ui->leAutoCodePassword->text();
    bool password_empty = ui->leAutoCodePassword->text().isEmpty();
    bool period_changed = m_params.period != INDEX_TO_VALUE(ui->cbAutoCodePeriod->currentIndex());
    bool codemode_changed = m_params.code_mode != INDEX_TO_VALUE(ui->cbAutoCodeMode->currentIndex());
    bool email_changed = m_settings.email != ui->cbAutoCodeEmailKey->isChecked();

    KCB_DEBUG_TRACE(units_changed << code_len_changed << startofday_changed << password_changed << period_changed << codemode_changed << email_changed);
    bool modified = units_changed || code_len_changed || startofday_changed || password_changed || period_changed || codemode_changed || email_changed;

    ui->pbAutoCodeGenerate->setEnabled(modified);
    ui->leAutoCodePassword->setStyleSheet((autocode_enabled && password_empty) ? css_warn : css_none);
}

void AutoCodeGenWidget::on_cbAutoCodeEmailKey_clicked()
{
    updateUi();
}

QString AutoCodeGenWidget::RunKeyboard(const QString& text)
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(false);
    kkd.setValue(text);
    if (kkd.exec())
    {
        return kkd.getValue();
    }

    return text;
}

QString AutoCodeGenWidget::GetHashedPassword(const QString& text)
{
    QString value = RunKeyboard(text);
    if (value.isEmpty())
    {
        return value;
    }
    QByteArray hash = QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash);
}

void AutoCodeGenWidget::on_leAutoCodePassword_clicked()
{
    QString hashed_password = GetHashedPassword("");
    ui->leAutoCodePassword->setText(hashed_password);
    updateUi();
}

void AutoCodeGenWidget::on_pbAutoCodeRandomPassword_clicked()
{
    int count = ui->spAutoCodeNumPasswordChars->value();

    QString password;
    for (int ii = 0; ii < count; ++ii)
    {
        password += QString(kcb::GetRandomChar());
    }

    QString hashed_password = GetHashedPassword(password);

    ui->leAutoCodePassword->setText(hashed_password);
    updateUi();
}

void AutoCodeGenWidget::OnNotifyCodesUpdate(const QStringList& codes)
{
    if (codes.isEmpty())
    {
        QPair<AutoCodeGenerator::CodeMap, QDateTime> result = AutoCodeGenerator::CreateCodeMap(m_params);
        m_lock_cabinet.setLockDisplay(result.first);
    }
    else
    {
        QMap<QString, QString> map;
        for (int ii=0; ii < codes.count(); ++ii)
        {
            map[QString::number(ii + 1)] = codes[ii];
        }
        m_lock_cabinet.setLockDisplay(map);
    }
}

void AutoCodeGenWidget::UpdateCodes()
{
    QStringList codes;
    if (IsCode1Mode())
    {
        emit RequestCodes1(codes);
        QMap<QString,QString> map;
        for (int ii = 0; ii < codes.count(); ++ii)
        {
            map[QString::number(ii + 1)] = codes[ii];
        }

        m_lock_cabinet.setLockDisplay(map);
    }
    else if (IsCode2Mode())
    {
        QPair<AutoCodeGenerator::CodeMap, QDateTime> result = AutoCodeGenerator::CreateCodeMap(m_params);
        m_lock_cabinet.setLockDisplay(result.first);
    }
    else
    {
        KCB_DEBUG_TRACE("Invalid code mode settings");
    }

}