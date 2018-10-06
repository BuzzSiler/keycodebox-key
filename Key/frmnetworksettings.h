#ifndef FRMNETWORKSETTINGS_H
#define FRMNETWORKSETTINGS_H

#include <QDialog>
#include <QString>

namespace Ui {
    class FrmNetworkSettings;
}

class FrmNetworkSettings : public QDialog
{
        Q_OBJECT

    public:
        explicit FrmNetworkSettings(QWidget *parent = 0);
        ~FrmNetworkSettings();

        void setValues(int vncPort, QString vncPassword,
                       QString smtpServer, int smtpPort, int smtpType, QString smtpUsername, QString smtpPassword);
        void getValues(int &vncPort, QString &vncPassword,
                       QString &smtpServer, int &smtpPort, int &smtpType, QString &smtpUsername, QString &smtpPassword);

    private slots:
        void on_lblVncPortValue_clicked();
        void on_lblVncPasswordValue_clicked();
        void on_lblSmtpServerValue_clicked();
        void on_lblSmtpPortValue_clicked();
        void on_lblSmtpUsernameValue_clicked();
        void on_lblSmtpPasswordValue_clicked();

    private:
        Ui::FrmNetworkSettings *ui;
        void RunKeyboard(QString& text, bool numbersOnly);
};

#endif // FRMNETWORKSETTINGS_H
