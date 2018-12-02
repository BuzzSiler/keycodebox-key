#include "reportcontrolwidget.h"
#include "ui_reportcontrolwidget.h"

#include <QDir>
#include <QMessageBox>
#include <QTimer>

#include "tbladmin.h"
#include "kcbcommon.h"
#include "checkablestringlistmodel.h"
#include "dlgdownloadreports.h"

static const QString DEFAULT_REPORT_DIRECTORY = QString("/home/pi/kcb-config/reports");

ReportControlWidget::ReportControlWidget(QWidget *parent) :
    QWidget(parent),
    m_report_freq(NEVER),
    m_report_frequencies(QVector<QDateTime>() << EACH_ACTIVITY << HOURLY << EVERY_12_HOURS << DAILY << WEEKLY << MONTHLY),
    m_delete_frequencies(QVector<QDateTime>() << DAILY << WEEKLY << BIWEEKLY << MONTHLY ),
    m_start(DEFAULT_DATE_TIME),
    m_send_via_email(false),
    m_report_directory("None"),
    m_curr_report_directory(""),
    m_lv_model(* new CheckableStringListModel(this)),
    m_usb_drives(QStringList()),
    m_auto_report_enabled(false),
    m_report_count(0),
    ui(new Ui::ReportControlWidget)
{
    ui->setupUi(this);

    // Set UI Defaults
    ui->gbAutoReport->setChecked(false);
    ui->gbAutoReport->setDisabled(true);
    ui->pbGenerateReport->setDisabled(true);
    ui->cbSaveToFile->setEnabled(true);

    ui->cbFrequency->insertItem(0, "None");
    ui->cbFrequency->setCurrentIndex(0);
    ui->dtStartDateTime->setDisplayFormat(DATETIME_FORMAT);
    ui->dtStartDateTime->setDateTime(DEFAULT_DATE_TIME);

    ui->dtReportStart->setDisplayFormat(DATETIME_FORMAT);
    ui->dtReportEnd->setDisplayFormat(DATETIME_FORMAT);

    ui->lvAvailableReports->setModel(&m_lv_model);
    m_lv_model.setStringList(QStringList());

    ui->cbUsbDrives->clear();
    ui->cbUsbDrives->addItem("DEFAULT");
    ui->cbUsbDrives->setCurrentIndex(0);

    updateUi();
}

ReportControlWidget::~ReportControlWidget()
{
    delete ui;
}

void ReportControlWidget::setValues(CAdminRec& adminInfo)
{
    // KCB_DEBUG_ENTRY;
    m_report_freq = adminInfo.getDefaultReportFreq();
    m_auto_report_enabled = m_report_freq != NEVER;
    ui->gbAutoReport->setChecked(m_auto_report_enabled);
    m_start = adminInfo.getDefaultReportStart();
    ui->dtStartDateTime->setDateTime(m_start);    

    m_send_via_email = adminInfo.getReportViaEmail();
    ui->cbSendViaEmail->setChecked(m_send_via_email);

    m_report_directory = m_curr_report_directory = adminInfo.getReportDirectory();

    // If the report directory is empty, then we are not storing report to files
    // If the report directory is the default directory, then we are storing reports to the default directory
    // If the report directory is not empty and not the default directory then we are storing reports to a USB drive
    bool dir_empty = m_curr_report_directory.isEmpty();
    bool dir_default = m_curr_report_directory == DEFAULT_REPORT_DIRECTORY;
    bool on_usb_drive = !dir_empty && !dir_default && m_usb_drives.count() > 0;

    ui->cbSaveToFile->setChecked(!dir_empty);
    ui->cbStoreToUsbDrive->setChecked(on_usb_drive);

    ui->lblDeleteOlderThan->setEnabled(!dir_empty);
    ui->cbDeleteOlderThan->setEnabled(!dir_empty);
    ui->cbStoreToUsbDrive->setEnabled(on_usb_drive);
    ui->cbUsbDrives->setEnabled(on_usb_drive);
    if (on_usb_drive)
    {   ui->cbUsbDrives->clear();
        ui->cbUsbDrives->addItems(m_usb_drives);
    }

    m_delete_freq = adminInfo.getDefaultReportDeleteFreq();
    KCB_DEBUG_TRACE(m_delete_freq.toString());
    if (dir_default || on_usb_drive)
    {
        int index = m_delete_frequencies.indexOf(m_delete_freq);
        Q_ASSERT_X(index >= 0 && index <= m_delete_frequencies.count(), Q_FUNC_INFO, "delete frequency index out of range");
        ui->cbDeleteOlderThan->setCurrentIndex(index);
    }

    ui->dtReportEnd->setDateTime(QDateTime().currentDateTime());

    // KCB_DEBUG_TRACE("m_report_freq" << m_report_freq);
    // KCB_DEBUG_TRACE("m_start" << m_start);
    // KCB_DEBUG_TRACE("m_auto_report_enabled" << m_auto_report_enabled);
    // KCB_DEBUG_TRACE("m_send_via_email" << m_send_via_email);
    // KCB_DEBUG_TRACE("m_report_directory" << m_report_directory);
    // KCB_DEBUG_TRACE("m_delete_freq" << m_delete_freq);
    

    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::getValues(CAdminRec& adminInfo)
{
    // KCB_DEBUG_ENTRY;

    if (!isModified())
    {
        // KCB_DEBUG_EXIT;
        return;
    }

    QDateTime reportFreq = NEVER;
    if (ui->gbAutoReport->isChecked())
    {
        int index = ui->cbFrequency->currentIndex();
        Q_ASSERT_X(index >= 0 && index <= m_report_frequencies.count(), Q_FUNC_INFO, "frequency index out of range");
        reportFreq = m_report_frequencies[index];
    }
    adminInfo.setDefaultReportFreq(reportFreq);
    adminInfo.setDefaultReportStart(ui->dtStartDateTime->dateTime());
    adminInfo.setReportDirectory(m_curr_report_directory);
    adminInfo.setReportToFile(ui->cbSaveToFile->isChecked());
    adminInfo.setReportViaEmail(ui->cbSendViaEmail->isChecked());

    QDateTime deleteFreq = MONTHLY;
    if (ui->cbSaveToFile->isChecked())
    {
        int index = ui->cbDeleteOlderThan->currentIndex();
        Q_ASSERT_X(index >= 0 && index <= m_delete_freq.count(), Q_FUNC_INFO, "delete frequency index out of range");
        deleteFreq = m_delete_frequencies[index];
    }
    KCB_DEBUG_TRACE(m_delete_freq.toString());
    adminInfo.setDefaultReportDeleteFreq(deleteFreq);

    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::getValues(CAdminRec& adminInfo, QDateTime& start, QDateTime& end)
{
    // KCB_DEBUG_ENTRY;
    getValues(adminInfo);
    start = ui->dtReportStart->dateTime();
    end = ui->dtReportEnd->dateTime();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::OnNotifyUsbDrive(QStringList drives)
{
    // KCB_DEBUG_ENTRY;
    m_usb_drives = drives;
    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_cbFrequency_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    // KCB_DEBUG_ENTRY;
    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_dtStartDateTime_dateTimeChanged(const QDateTime &dateTime)
{
    Q_UNUSED(dateTime);
    // KCB_DEBUG_ENTRY;
    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_gbAutoReport_toggled(bool state)
{
    // KCB_DEBUG_ENTRY;

    // Restore is true any time we are returning to the 'setValues' state, e.g. for both enable and disable of report frequency
    bool restored = m_auto_report_enabled == state;
    QDateTime dt;

    // If we are transitioning from no frequency (None/NEVER) to setting a frequency
    // we check to see if "None" as added to the list.  If so, it will be the first
    // element.
    // If "None" is the first element then remove it from the items
    int index = ui->cbFrequency->findText("None");
    if (index > -1)
    {
        KCB_DEBUG_TRACE("Removing None" << index);
        ui->cbFrequency->removeItem(index);
    }

    if (state)
    {
        if (restored)
        {
            index = m_report_frequencies.indexOf(m_report_freq);
            Q_ASSERT_X(index >= 0 && index <= m_report_frequencies.count(), Q_FUNC_INFO, "frequency index out of range");
            ui->gbAutoReport->setChecked(m_auto_report_enabled);
            dt = m_start;
        }
        else
        {
            index = 0;
            dt = QDateTime::currentDateTime();
        }
    }
    else
    {
        KCB_DEBUG_TRACE("Adding None");
        ui->cbFrequency->insertItem(0, "None");
        index = 0;
        dt = DEFAULT_DATETIME;
    }

    ui->cbFrequency->setCurrentIndex(index);
    ui->dtStartDateTime->setDateTime(dt);

    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::checkInvalidReportStorage()
{
    if (!ui->cbSendViaEmail->isChecked() && !ui->cbSaveToFile->isChecked())
    {
        if (ui->gbAutoReport->isChecked())
        {
            ui->gbAutoReport->setChecked(false);
        }
    }
}

bool ReportControlWidget::ConfirmDisableSaveToFile()
{
    bool confirmed = true;

    QDir dir(m_curr_report_directory);
    QStringList strList = dir.entryList(QStringList() << "KeyCodeBox*.txt", QDir::Files, QDir::Time);
    if (!strList.isEmpty())
    {
        QString title = tr("Disable Store Report to File");
        QString message = tr("You are disabling 'Store Report to File'\n"
                             "Continung will delete all report files in the default location.\n"
                             "To save the report files, select 'No', insert a USB drive and download the reports.\n"
                             "Do you want to continue?");
        auto btn = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::No);
        if (btn == QMessageBox::No)
        {
            confirmed = false;
        }
        else
        {
            foreach (auto entry, strList)
            {
                QFile file(dir.filePath(entry));
                file.remove();
            }

        }
    }

    return confirmed;
}

void ReportControlWidget::on_cbSendViaEmail_clicked()
{
    checkInvalidReportStorage();
    updateUi();
}

void ReportControlWidget::on_cbSaveToFile_clicked()
{
    if (ui->cbSaveToFile->checkState() == Qt::Checked)
    {
        m_curr_report_directory = DEFAULT_REPORT_DIRECTORY;
    }
    else
    {
        bool confirmed = ConfirmDisableSaveToFile();
        if (confirmed)
        {
            m_curr_report_directory = "";
            checkInvalidReportStorage();
        }
        else
        {
            ui->cbSaveToFile->setChecked(true);
        }

    }
    updateUi();
}

void ReportControlWidget::on_cbStoreToUsbDrive_clicked()
{
    // KCB_DEBUG_ENTRY;
    if (ui->cbStoreToUsbDrive->checkState() == Qt::Checked)
    {
        ui->cbUsbDrives->clear();
        ui->cbUsbDrives->addItems(m_usb_drives);
        ui->cbUsbDrives->setCurrentIndex(0);
    }
    else
    {
        bool set_to_default = true;
        QDir dir(m_curr_report_directory);
        QStringList strList = dir.entryList(QStringList() << "KeyCodeBox*.txt", QDir::Files, QDir::Time);
        if (!strList.isEmpty())
        {
            QString title = tr("Disable Store to USB Drive");
            QString message = tr("You are disabling 'Store to USB Drive'\n"
                                 "Continung will set report storage to the default location.\n"
                                 "All reports on the USB drive will remain.\n"
                                 "Do you want to continue?");
            auto btn = QMessageBox::warning(this, title, message, QMessageBox::Yes, QMessageBox::No);
            if (btn == QMessageBox::No)
            {
                ui->cbStoreToUsbDrive->setChecked(true);
                set_to_default = false;
            }
        }

        if (set_to_default)
        {
            ui->cbUsbDrives->clear();
            ui->cbUsbDrives->addItem(tr("DEFAULT"));
            ui->cbUsbDrives->setCurrentIndex(0);
        }
    }
    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_lvAvailableReports_clicked(const QModelIndex &index)
{
    // KCB_DEBUG_ENTRY;
    bool enable = false;
    auto state = Qt::CheckState(m_lv_model.data(index, Qt::CheckStateRole).toInt());
    KCB_DEBUG_TRACE(state);
    if (state == Qt::Checked)
    {
        enable = true;
    }
    else
    {
        QStringList list;
        GetSelectedReports(list);
        if (list.count() > 0)
        {
            enable = true;
        }
    }

    // Note: It would preferrable to have these set in updateUi, but when updateUi
    // is called, you can no longer check an item.  Something is conflicting.
    // In the interim, the logic is duplicated.  Maybe put common logic in a
    // common function

    ui->pbClearAllReports->setEnabled(enable);
    ui->pbDeleteSelectedReports->setEnabled(enable);
    ui->pbDownloadSelectedReports->setEnabled(!ui->cbStoreToUsbDrive->isChecked() && enable);

    ui->lvAvailableReports->setFocus();

    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_pbSelectAllReports_clicked()
{
    // KCB_DEBUG_ENTRY;

    ClearSelectAllReports(Qt::Checked);
    ui->pbSelectAllReports->setDisabled(true);
    ui->pbClearAllReports->setEnabled(true);
    ui->pbDeleteSelectedReports->setEnabled(true);
    ui->pbDownloadSelectedReports->setEnabled(!ui->cbStoreToUsbDrive->isChecked());

    ui->lvAvailableReports->setFocus();

    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_pbClearAllReports_clicked()
{
    // KCB_DEBUG_ENTRY;

    ClearSelectAllReports(Qt::Unchecked);
    ui->pbSelectAllReports->setEnabled(true);
    ui->pbClearAllReports->setEnabled(false);
    ui->pbDeleteSelectedReports->setEnabled(false);
    ui->pbDownloadSelectedReports->setEnabled(false);

    ui->lvAvailableReports->setFocus();

    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_pbDeleteSelectedReports_clicked()
{
    // KCB_DEBUG_ENTRY;

    QStringList list;
    GetSelectedReports(list);

    QDir dir(m_curr_report_directory);

    foreach (auto entry, list)
    {
        QFile file(dir.filePath(entry));
        file.remove();
    }

    updateUi();
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::on_pbDownloadSelectedReports_clicked()
{

    if (m_usb_drives.count() == 0)
    {
        QMessageBox::warning(this, tr("Downloading Reports"),
                             tr("You have selected to download reports but there is no USB drive inserted.\n"
                                "You must insert a USB drive before downloading reports."),
                             QMessageBox::Close);
        ClearSelectAllReports(Qt::Unchecked);
    }
    else
    {
        auto dlg = new DlgDownloadReports();
        QStringList list;
        GetSelectedReports(list);
        dlg->setValues(m_curr_report_directory, list, m_usb_drives);
        dlg->exec();
    }
    updateUi();
}

void ReportControlWidget::on_cbUsbDrives_currentTextChanged(const QString &text)
{
    if (text == "DEFAULT")
    {
        m_curr_report_directory = DEFAULT_REPORT_DIRECTORY;
    }
    else
    {
        m_curr_report_directory = text;
    }
    updateUi();
}

void ReportControlWidget::on_pbGenerateReport_clicked()
{
    // Get the current number of report files and save it
    QDir dir(m_curr_report_directory);
    QStringList strList = dir.entryList(QStringList() << "KeyCodeBox*.txt", QDir::Files, QDir::Time);
    m_report_count = strList.count();
    emit NotifyGenerateReport();
    updateUi();
    // Start a timer that compares the number of report files with the above
    QTimer::singleShot(100, this, SLOT(updateAvailableReports()));
}

void ReportControlWidget::updateAvailableReports()
{
    QDir dir(m_curr_report_directory);
    QStringList strList = dir.entryList(QStringList() << "KeyCodeBox*.txt", QDir::Files, QDir::Time);
    if (strList.count() > m_report_count)
    {
        updateUi();
        m_report_count = 0;
    }
    else
    {
        QTimer::singleShot(100, this, SLOT(updateAvailableReports()));
    }
}

void ReportControlWidget::ClearSelectAllReports(Qt::CheckState state)
{
    // KCB_DEBUG_ENTRY;
    int count = m_lv_model.rowCount();
    for(int row = 0; row < count; row++)
    {
        QModelIndex index = m_lv_model.index(row,0);
        m_lv_model.setData(index, state, Qt::CheckStateRole);
    }
    // KCB_DEBUG_EXIT;
}

void ReportControlWidget::GetSelectedReports(QStringList& list)
{
    // KCB_DEBUG_ENTRY;
    int count = m_lv_model.rowCount();
    for(int row = 0; row < count; row++)
    {
        QModelIndex index = m_lv_model.index(row,0);
        auto state = Qt::CheckState(m_lv_model.data(index, Qt::CheckStateRole).toInt());
        if (state == Qt::Checked)
        {
            auto entry = m_lv_model.data(index, Qt::DisplayRole).toString();
            list.append(entry);
        }
    }
    // KCB_DEBUG_EXIT;
}

bool ReportControlWidget::OneOrMoreReportsSelected()
{
    // KCB_DEBUG_ENTRY;
    int count = m_lv_model.rowCount();
    for(int row = 0; row < count; row++)
    {
        QModelIndex index = m_lv_model.index(row,0);
        auto state = Qt::CheckState(m_lv_model.data(index, Qt::CheckStateRole).toInt());
        if (state == Qt::Checked)
        {
            // KCB_DEBUG_EXIT;
            return true;
        }
    }

    // KCB_DEBUG_EXIT;
    return false;
}

bool ReportControlWidget::isModified()
{
    // KCB_DEBUG_ENTRY;
    int index = ui->cbFrequency->currentIndex();
    //KCB_DEBUG_TRACE(index);
    Q_ASSERT_X(index >= 0 && index <= m_report_frequencies.count(), Q_FUNC_INFO, "report frequency index out of range");

    // Changed from disabled to enabled OR
    // Remained enabled and changed frequency OR
    // Remained enabled and changed date/time OR
    // Changed from enabled to disabled
    bool changed_enablement = m_auto_report_enabled != ui->gbAutoReport->isChecked();
    bool changed_freq = ui->gbAutoReport->isChecked() && (m_report_freq != m_report_frequencies[index]);
    bool changed_start_datetime = ui->gbAutoReport->isChecked() && (m_start != ui->dtStartDateTime->dateTime());
    bool auto_gen_changed = changed_enablement || changed_freq || changed_start_datetime;

    bool send_via_email_changed = ui->cbSendViaEmail->isChecked() != m_send_via_email;
    bool save_to_file_changed = m_report_directory != m_curr_report_directory;

    index = ui->cbDeleteOlderThan->currentIndex();
    Q_ASSERT_X(index >= 0 && index <= m_delete_frequencies.count(), Q_FUNC_INFO, "delete frequency index out of range");
    bool delete_freq_changed = m_delete_frequencies[index] != m_delete_freq;

    // KCB_DEBUG_TRACE("auto_gen_changed" << auto_gen_changed);
    // KCB_DEBUG_TRACE("\tchanged_enablement" << changed_enablement);
    // KCB_DEBUG_TRACE("\tchanged_freq" << changed_freq);
    // KCB_DEBUG_TRACE("\tchanged_start_datetime" << changed_start_datetime);
    // KCB_DEBUG_TRACE("send_via_email_changed" << send_via_email_changed);
    // KCB_DEBUG_TRACE("save_to_file_changed" << save_to_file_changed);
    // KCB_DEBUG_TRACE("delete_freq_changed" << delete_freq_changed);
    bool is_modified = auto_gen_changed || send_via_email_changed || save_to_file_changed || delete_freq_changed;
    // KCB_DEBUG_EXIT;

    return is_modified;
}

void ReportControlWidget::updateUi()
{
    // KCB_DEBUG_ENTRY;
    bool send_email_enabled = ui->cbSendViaEmail->isChecked();
    bool store_file_enabled = ui->cbSaveToFile->isChecked();
    bool store_usb_enabled = store_file_enabled && m_usb_drives.count() > 0;
    // valid report storage must have email or file or both
    bool valid_report_storage = send_email_enabled || store_file_enabled;

    ui->pbGenerateReport->setEnabled(valid_report_storage);
    ui->gbAutoReport->setEnabled(valid_report_storage);
    ui->lblDeleteOlderThan->setEnabled(store_file_enabled);
    ui->cbDeleteOlderThan->setEnabled(store_file_enabled);
    ui->cbStoreToUsbDrive->setEnabled(store_usb_enabled);
    ui->cbUsbDrives->setEnabled(store_usb_enabled && ui->cbStoreToUsbDrive->isChecked());

    // KCB_DEBUG_TRACE(m_curr_report_directory);
    bool dir_empty = m_curr_report_directory.isEmpty();
    bool dir_default = m_curr_report_directory == DEFAULT_REPORT_DIRECTORY;
    bool on_usb_drive = !dir_empty && !dir_default && m_usb_drives.count() > 0;

    if (dir_default || on_usb_drive)
    {
        auto dir = QDir(m_curr_report_directory);
        QStringList strList = dir.entryList(QStringList() << "KeyCodeBox*.txt", QDir::Files, QDir::Time);
        m_lv_model.setStringList(strList);

        bool reports_remaining = strList.count() > 0;
        ui->pbSelectAllReports->setEnabled(reports_remaining);               
    }
    else
    {
        m_lv_model.setStringList(QStringList());
        ui->pbSelectAllReports->setDisabled(true);
    }

    bool one_or_more_selected = OneOrMoreReportsSelected();
    ui->pbClearAllReports->setEnabled(one_or_more_selected);
    ui->pbDeleteSelectedReports->setEnabled(one_or_more_selected);
    ui->pbDownloadSelectedReports->setEnabled(!ui->cbStoreToUsbDrive->isChecked() && one_or_more_selected);

    // KCB_DEBUG_EXIT;
}
