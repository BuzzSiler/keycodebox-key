#ifndef FRMNETWORKSETTINGS_H
#define FRMNETWORKSETTINGS_H

#include <QDialog>
#include <QString>
#include <QMessageBox>

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

        void setValues(QString vncPort, QString vncPassword, bool vncEnable,
                       QString smtpServer, int smtpPort, int smtpType, QString smtpUsername, QString smtpPassword);
        void getValues(QString &vncPort, QString &vncPassword, bool &vncEnable,
                       QString &smtpServer, int &smtpPort, int &smtpType, QString &smtpUsername, QString &smtpPassword);
        bool pendingReboot();

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
        void on_cbVncEnable_clicked();

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
            bool enable;
        } VNC_STATE;

        IP_STATE ip_state;
        SMTP_STATE smtp_state;
        VNC_STATE vnc_state;
        bool ip_addressing_changed{false};
        bool pending_reboot_{false};
        bool vnc_enable_change{false};
        bool vnc_port_change{false};
        bool vnc_password_change{false};

        Ui::FrmNetworkSettings *ui;
        void RunKeyboard(QString& text, bool numbersOnly, bool ipAddress = false);
        void HandleIpSettingClick(CClickableLabel& label);
        void updateUi();
        void SetLabelsForStaticAddressing();
        void SetLabelsForDhcpAddressing();
        void invalidPassword();
        int warnOnChange(QString const & title, QString const & body, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);




};

#endif // FRMNETWORKSETTINGS_H
