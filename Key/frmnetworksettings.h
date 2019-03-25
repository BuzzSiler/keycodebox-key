#ifndef FRMNETWORKSETTINGS_H
#define FRMNETWORKSETTINGS_H

#include <QDialog>
#include <QString>

class CClickableLabel;

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

        void on_cbAddressingType_currentIndexChanged(int index);

        void on_lblIpAddressValue_clicked();
        void on_lblIpMaskValue_clicked();
        void on_lblIpBroadcastValue_clicked();
        void on_lblIpGatewayValue_clicked();
        void on_lblIpDnsValue_clicked();

        void on_bbNetworkingOkCancel_accepted();
        void on_bbNetworkingOkCancel_rejected();

        void on_cbxSmtpSecurityValue_currentIndexChanged(int index);

    private:
        typedef struct {
            bool is_static;
            QString host;
            QString broadcast;
            QString mask;
            QString gateway;
            QString dns;
        } IP_STATE;

        typedef struct {
            QString server;
            QString port;
            int type;
            QString username;
            QString password;
        } SMTP_STATE;

        typedef struct {
            QString port;
            QString password;
        } VNC_STATE;

        IP_STATE ip_state;
        SMTP_STATE smtp_state;
        VNC_STATE vnc_state;

        Ui::FrmNetworkSettings *ui;
        void RunKeyboard(QString& text, bool numbersOnly, bool ipAddress = false);
        void HandleIpSettingClick(CClickableLabel& label);
        void updateUi();
        void SetLabelsForStaticAddressing();
        void SetLabelsForDhcpAddressing();


};

#endif // FRMNETWORKSETTINGS_H
