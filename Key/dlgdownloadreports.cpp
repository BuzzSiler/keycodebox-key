#include "dlgdownloadreports.h"
#include "ui_dlgdownloadreports.h"

#include <QDir>
#include <QFile>

#include "kcbcommon.h"

DlgDownloadReports::DlgDownloadReports(QWidget *parent) :
    QDialog(parent),
    m_dir(""),
    m_selected_files(QStringList()),
    ui(new Ui::DlgDownloadReports)
{
    ui->setupUi(this);
    ui->pbDownload->setRange(0, 100);
}

DlgDownloadReports::~DlgDownloadReports()
{
    delete ui;
}

void DlgDownloadReports::setValues(const QString dir, const QStringList list, const QStringList drives)
{
    m_dir = dir;
    m_selected_files = list;
    foreach (auto entry, drives)
    {
        KCB_DEBUG_TRACE(entry);
        ui->lwUsbDrives->addItem(entry);
    }
    ui->pbStartDownload->setEnabled(false);
}

void DlgDownloadReports::on_pbStartDownload_clicked()
{
    ui->pbStartDownload->setDisabled(true);
    ui->bbCloseDownloadDialog->button(QDialogButtonBox::Close)->setDisabled(true);
    int step = 100 / m_selected_files.count();
    int value = 0;

    QDir srcdir(m_dir);
    QDir tgtdir(ui->lwUsbDrives->currentItem()->text());
    KCB_DEBUG_TRACE(m_selected_files.count());
    foreach (auto entry, m_selected_files)
    {
        // Copy the file to the USB drive
        KCB_DEBUG_TRACE("Copying" << entry);

        QFile::copy(srcdir.filePath(entry), tgtdir.filePath(entry));

        if (ui->cbDeleteAfterDownload->isChecked())
        {
            KCB_DEBUG_TRACE("Deleting" << entry);
            QFile file(srcdir.filePath(entry));
            file.remove();
        }

        ui->pbDownload->setValue(value);
        value += step;
    }
    ui->pbDownload->setValue(value < 100 ? 100 : value);
    ui->pbStartDownload->setEnabled(true);
    ui->bbCloseDownloadDialog->button(QDialogButtonBox::Close)->setEnabled(true);
}

void DlgDownloadReports::on_lwUsbDrives_itemSelectionChanged()
{
    ui->pbStartDownload->setEnabled(true);
}
