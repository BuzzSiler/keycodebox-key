#ifndef FRMNETWORKSETTINGS_H
#define FRMNETWORKSETTINGS_H

#include <QDialog>

namespace Ui {
    class FrmNetworkSettings;
}

class FrmNetworkSettings : public QDialog
{
        Q_OBJECT

    public:
        explicit FrmNetworkSettings(QWidget *parent = 0);
        ~FrmNetworkSettings();

        void setValues(bool enableDhcp,
                       QString ipAddress, QString ipMask, QString ipGateway, QString ipDns,
                       QString vncPort, QString vncPassword,
                       QString emailServer, QString emailPort,
                       QString emailUsername, QString emailPassword, int emailSecurity);
        void getValues(bool& enableDhcp,
                       QString& ipAddress, QString& ipMask, QString& ipGateway, QString& ipDns,
                       QString& vncPort, QString& vncPassword,
                       QString& emailServer, QString& emailPort,
                       QString& emailUsername, QString& emailPassword, int& emailSecurity);

    private slots:
        void on_cbEnableDhcp_clicked();
        void on_edIpAddress_clicked();
        void on_edIpMask_clicked();
        void on_edIpGateway_clicked();
        void on_edIpDns_clicked();

        void on_edVncPort_clicked();
        void on_edVncPassword_clicked();

        void on_edEmailServer_clicked();
        void on_edEmailPort_clicked();
        void on_edEmailAddress_clicked();
        void on_edEmailPassword_clicked();
        void on_cbSecurity_currentIndexChanged(int index);
        void on_pbEmailSend_clicked();


    private:
        Ui::FrmNetworkSettings *ui;
        bool m_enable_dhcp;
        QString m_ip_address;
        QString m_ip_mask;
        QString m_ip_gateway;
        QString m_ip_dns;

        QString m_vnc_port;
        QString m_vnc_password;

        QString m_email_server;
        QString m_email_port;
        QString m_email_user;
        QString m_email_pw;
        int m_email_security;

        void RunKeyboard(QString& text, bool numbersOnly = false);
        bool isModified();
        void updateUi();


};

#endif // FRMNETWORKSETTINGS_H
