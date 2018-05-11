#ifndef DLGDOWNLOADREPORTS_H
#define DLGDOWNLOADREPORTS_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QListWidgetItem>

namespace Ui {
    class DlgDownloadReports;
}

class DlgDownloadReports : public QDialog
{
    Q_OBJECT

    public:
        explicit DlgDownloadReports(QWidget *parent = 0);
        ~DlgDownloadReports();

        void setValues(const QString dir, const QStringList list, const QStringList drives);

    private slots:
        void on_pbStartDownload_clicked();

        void on_lwUsbDrives_itemSelectionChanged();

    private:
        QString m_dir;
        QStringList m_selected_files;
        Ui::DlgDownloadReports *ui;
};

#endif // DLGDOWNLOADREPORTS_H
