#ifndef REPORTCONTROLWIDGET_H
#define REPORTCONTROLWIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QVector>

namespace Ui {
    class ReportControlWidget;
}

class CAdminRec;
class CheckableStringListModel;

class ReportControlWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ReportControlWidget(QWidget *parent = 0);
        ~ReportControlWidget();

        void setValues(CAdminRec& adminInfo);
        void getValues(CAdminRec& adminInfo);
        void getValues(CAdminRec& adminInfo, QDateTime& start, QDateTime& end);

    signals:
        void NotifyUsbDrive(const QStringList drives);
        void NotifyGenerateReport();

    public slots:
        void OnNotifyUsbDrive(const QStringList drives);        

    private slots:
        void on_cbFrequency_currentIndexChanged(int index);
        void on_gbAutoReport_toggled(bool state);
        void on_cbSendViaEmail_clicked();
        void on_cbSaveToFile_clicked();
        void on_cbStoreToUsbDrive_clicked();
        void on_lvAvailableReports_clicked(const QModelIndex &index);
        void on_pbSelectAllReports_clicked();
        void on_pbClearAllReports_clicked();
        void on_pbDeleteSelectedReports_clicked();
        void on_pbDownloadSelectedReports_clicked();
        void on_cbUsbDrives_currentTextChanged(const QString &text);
        void on_pbGenerateReport_clicked();
        void on_dtStartDateTime_dateTimeChanged(const QDateTime &dateTime);
        void updateAvailableReports();

    private:
        QDateTime m_report_freq;
        QDateTime m_delete_freq;
        QVector<QDateTime> m_report_frequencies;
        QVector<QDateTime> m_delete_frequencies;
        QDateTime m_start;
        bool m_send_via_email;
        QString m_report_directory;
        QString m_curr_report_directory;
        CheckableStringListModel& m_lv_model;
        QStringList m_usb_drives;
        bool m_auto_report_enabled;
        int m_report_count;
        Ui::ReportControlWidget *ui;
        void updateUi();
        bool isModified();
        void GetSelectedReports(QStringList& list);
        bool OneOrMoreReportsSelected();
        void ClearSelectAllReports(Qt::CheckState state);
        void checkInvalidReportStorage();
        bool ConfirmDisableSaveToFile();
};

#endif // REPORTCONTROLWIDGET_H
