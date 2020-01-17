#include "frmnetworksettings.h"
#include "ui_frmnetworksettings.h"

#include <QPushButton>
#include <QMessageBox>
#include <QList>
#include <QNetworkInterface>
#include <QPair>

#include "kcbcommon.h"
#include "kcbkeyboarddialog.h"
#include "kcbsystem.h"
#include "kcbutils.h"
#include "keycodeboxsettings.h"


static const int IP_ADDRESSING_STATIC_IP = 0;
static const int IP_ADDRESSING_DHCP = 1;

static QString const VNC_WARN_DIALOG_TITLE = QString(QObject::tr("Virtual Network Computing (VNC) Change"));
static QString const IP_WARN_DIALOG_TITLE = QString(QObject::tr("Network Addressing Changed"));


FrmNetworkSettings::FrmNetworkSettings(QWidget *parent) :
    QDialog(parent),
    ip_state({false, "", "", "", "", "0.0.0.0"}),
    smtp_state({"", "", 0, "", ""}),
    vnc_state({"", "", false}),
    ip_addressing_changed(false),
    ui(new Ui::FrmNetworkSettings)
{
    ui->setupUi(this);

    kcb::SetWindowParams(this);

    ui->lblMacAddressValue->setDisabled(true);
    ui->bbNetworkingOkCancel->button(QDialogButtonBox::Ok)->setDisabled(true);

    updateUi();
}

FrmNetworkSettings::~FrmNetworkSettings()
{
    // kcb::ClassAllocation::DestructorMsg(this);
    delete ui;
}

void FrmNetworkSettings::setValues(QString vncPort, QString vncPassword, bool vncEnable, QString smtpServer, int smtpPort, int smtpType, QString smtpUsername, QString smtpPassword)
{
    // IP Network Values
    ip_state.is_static = KeyCodeBoxSettings::StaticAddressingEnabled();
    ip_state.host = KeyCodeBoxSettings::GetHostAddress();
    ip_state.broadcast = KeyCodeBoxSettings::GetBcastAddress();
    ip_state.mask = KeyCodeBoxSettings::GetNetworkMask();
    ip_state.gateway = KeyCodeBoxSettings::GetGatewayAddress();
    ip_state.dns = KeyCodeBoxSettings::GetDnsAddress();

    ui->lblIpAddressValue->setText(ip_state.host);
    ui->lblIpBroadcastValue->setText(ip_state.broadcast);
    ui->lblIpMaskValue->setText(ip_state.mask);
    ui->lblIpGatewayValue->setText(ip_state.gateway);
    ui->lblIpDnsValue->setText(ip_state.dns);
    ui->lblMacAddressValue->setText(KeyCodeBoxSettings::GetMacAddress());

    // KCB_DEBUG_TRACE("is static" << ip_state.is_static);
    ui->cbAddressingType->setCurrentIndex(ip_state.is_static ? IP_ADDRESSING_STATIC_IP : IP_ADDRESSING_DHCP);

    // SMTP Values
    smtp_state.server = smtpServer;
    smtp_state.port = QString::number(smtpPort);
    smtp_state.type = smtpType;
    smtp_state.username = smtpUsername;
    smtp_state.password = smtpPassword;

    ui->lblSmtpServerValue->setText(smtp_state.server);
    ui->lblSmtpPortValue->setText(smtp_state.port);
    ui->cbxSmtpSecurityValue->setCurrentIndex(0);
    if (smtp_state.type >= 0 || smtp_state.type <= 2)
    {
        ui->cbxSmtpSecurityValue->setCurrentIndex(smtp_state.type);
    }
    ui->lblSmtpUsernameValue->setText(smtp_state.username);
    ui->lblSmtpPasswordValue->setText(smtp_state.password);

    // VNC Values
    vnc_state.port = vncPort;
    vnc_state.password = vncPassword;
    vnc_state.enable = vncEnable;

    ui->lblVncPortValue->setText(vnc_state.port);
    ui->lblVncPasswordValue->setText(vnc_state.password);
    ui->cbVncEnable->setChecked(vnc_state.enable);
    ui->cbVncEnable->setText(vnc_state.enable ? "VNC Enabled" : "VNC Disabled");

    updateUi();
}

void FrmNetworkSettings::getValues(QString& vncPort, QString& vncPassword, bool& vncEnable, QString& smtpServer, int& smtpPort, int& smtpType, QString& smtpUsername, QString& smtpPassword)
{
    vncPort = ui->lblVncPortValue->text();
    vncPassword = ui->lblVncPasswordValue->text();
    vncEnable = ui->cbVncEnable->isChecked();

    smtpServer = ui->lblSmtpServerValue->text();
    smtpPort = ui->lblSmtpPortValue->text().toInt();
    smtpType = ui->cbxSmtpSecurityValue->currentIndex();
    smtpUsername = ui->lblSmtpUsernameValue->text();
    smtpPassword = ui->lblSmtpPasswordValue->text();
}

void FrmNetworkSettings::RunKeyboard(QString& text, bool numbersOnly, bool ipAddress)
{
    KcbKeyboardDialog kkd;

    KCB_DEBUG_TRACE("text" << text << ", numbers" << numbersOnly << ", ip" << ipAddress);
    kkd.numbersOnly(numbersOnly);
    kkd.ipAddress(ipAddress);
    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
}

void FrmNetworkSettings::on_lblVncPortValue_clicked()
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(true);
    kkd.setValue(ui->lblVncPortValue->text());
    if (kkd.exec())
    {
        ui->lblVncPortValue->setText(kkd.getValue());
    }

    updateUi();
}

void FrmNetworkSettings::invalidPassword()
{
    (void) QMessageBox::critical(this,
                                 tr("Invalid Password"),
                                 tr("The password must be 6 or more characters.\n"
                                    "Please re-enter the password."),
                                 QMessageBox::Close);
}

void FrmNetworkSettings::on_lblVncPasswordValue_clicked()
{
    KcbKeyboardDialog kkd;

    kkd.setValue(ui->lblVncPasswordValue->text());
    if (kkd.exec())
    {
        QString password = kkd.getValue();
        if (password.isEmpty() || password.length() < 6)
        {
            invalidPassword();
        }
        else
        {
            ui->lblVncPasswordValue->setText(password);
        }
    }

    updateUi();
}

void FrmNetworkSettings::on_lblSmtpServerValue_clicked()
{
    QString text = ui->lblSmtpServerValue->text();
    RunKeyboard(text, false);
    ui->lblSmtpServerValue->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_lblSmtpPortValue_clicked()
{
    QString text = ui->lblSmtpPortValue->text();
    RunKeyboard(text, true);
    ui->lblSmtpPortValue->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_lblSmtpUsernameValue_clicked()
{
    QString text = ui->lblSmtpUsernameValue->text();
    RunKeyboard(text, false);
    ui->lblSmtpUsernameValue->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_lblSmtpPasswordValue_clicked()
{
    QString text = ui->lblSmtpPasswordValue->text();
    RunKeyboard(text, false);
    ui->lblSmtpPasswordValue->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_cbAddressingType_currentIndexChanged(int index)
{
    // KCB_DEBUG_ENTRY;

    bool is_static = index == IP_ADDRESSING_STATIC_IP;

    ui->lblIpBroadcastValue->setEnabled(is_static);
    ui->lblIpAddressValue->setEnabled(is_static);
    ui->lblIpMaskValue->setEnabled(is_static);
    ui->lblIpGatewayValue->setEnabled(is_static);
    ui->lblIpDnsValue->setEnabled(is_static);

    if (index == IP_ADDRESSING_STATIC_IP)
    {
        bool enable_static = true;

        // If the initial setting is DHCP and we are changing to static then we need to warn the user
        // If the initial setting was static and we are returning to static then there is no need to warn the user (they got the warning the first time)

        if (!ip_state.is_static)
        {
            int result = QMessageBox::warning(this,
                                                tr("Static Network Addressing"), 
                                                tr("You have enabled static network addressing. This is an advanced feature and should be enabled only by knowledgeable IT personnel.\n"
                                                   "WARNING: Incorrectly configuring network addressing can disable network communication.\n"
                                                   "Do you wish to continue?"),
                                                QMessageBox::Ok, 
                                                QMessageBox::Cancel);
            if (result == QMessageBox::Cancel)
            {
                enable_static = false;
                ui->cbAddressingType->setCurrentIndex(IP_ADDRESSING_DHCP);
            }
        }

        if (enable_static)
        {
            KeyCodeBoxSettings::EnableStaticAddressing();
        }
    }
    else
    {
        KeyCodeBoxSettings::DisableStaticAddressing();

        ui->lblIpAddressValue->setText(KeyCodeBoxSettings::GetHostAddress());
        ui->lblIpBroadcastValue->setText(KeyCodeBoxSettings::GetBcastAddress());
        ui->lblIpMaskValue->setText(KeyCodeBoxSettings::GetNetworkMask());
        ui->lblIpGatewayValue->setText(KeyCodeBoxSettings::GetGatewayAddress());
        ui->lblIpDnsValue->setText(KeyCodeBoxSettings::GetDnsAddress());
    }

    updateUi();

    // KCB_DEBUG_EXIT;
}

void FrmNetworkSettings::on_lblIpAddressValue_clicked()
{
    HandleIpSettingClick(*ui->lblIpAddressValue);
}

void FrmNetworkSettings::on_lblIpMaskValue_clicked()
{
    HandleIpSettingClick(*ui->lblIpMaskValue);
}

void FrmNetworkSettings::on_lblIpBroadcastValue_clicked()
{
    HandleIpSettingClick(*ui->lblIpBroadcastValue);
}

void FrmNetworkSettings::on_lblIpGatewayValue_clicked()
{
    HandleIpSettingClick(*ui->lblIpGatewayValue);
}

void FrmNetworkSettings::on_lblIpDnsValue_clicked()
{
    // KCB_DEBUG_ENTRY;
    HandleIpSettingClick(*ui->lblIpDnsValue);
    // KCB_DEBUG_EXIT;
}

void FrmNetworkSettings::HandleIpSettingClick(CClickableLabel& label)
{
    if (KeyCodeBoxSettings::StaticAddressingEnabled())
    {
        QString text = label.text();
        RunKeyboard(text, true, true);
        label.setText(text);
    }

    updateUi();
}

void FrmNetworkSettings::updateUi()
{
    // KCB_DEBUG_ENTRY;
    bool static_change = ip_state.is_static != KeyCodeBoxSettings::StaticAddressingEnabled();
    bool host_change = ip_state.host != ui->lblIpAddressValue->text();
    bool broadcast_change = ip_state.broadcast != ui->lblIpBroadcastValue->text();
    bool mask_change = ip_state.mask != ui->lblIpMaskValue->text();
    bool gateway_change = ip_state.gateway != ui->lblIpGatewayValue->text();
    bool dns_change = ip_state.dns != ui->lblIpDnsValue->text();
    ip_addressing_changed = (static_change || host_change || broadcast_change || mask_change || gateway_change || dns_change);

    bool server_change = smtp_state.server != ui->lblSmtpServerValue->text();
    bool smtp_port_change = smtp_state.port != ui->lblSmtpPortValue->text();
    bool type_change = smtp_state.type != ui->cbxSmtpSecurityValue->currentIndex();
    bool username_change = smtp_state.username != ui->lblSmtpUsernameValue->text();
    bool smtp_password_change = smtp_state.password != ui->lblSmtpPasswordValue->text();
    bool smtp_change = (server_change || smtp_port_change || type_change || username_change || smtp_password_change);

    vnc_port_change = vnc_state.port != ui->lblVncPortValue->text();
    vnc_password_change = vnc_state.password != ui->lblVncPasswordValue->text();
    vnc_enable_change = vnc_state.enable != ui->cbVncEnable->isChecked();

    bool vnc_changed = (vnc_port_change || vnc_password_change || vnc_enable_change);

    ui->bbNetworkingOkCancel->button(QDialogButtonBox::Ok)->setEnabled(smtp_change || vnc_changed || ip_addressing_changed);
}

int FrmNetworkSettings::warnOnChange(QString const & title, QString const & body, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    return QMessageBox::warning(this, title, body, buttons, defaultButton);
}

void FrmNetworkSettings::on_bbNetworkingOkCancel_accepted()
{
    // KCB_DEBUG_ENTRY;

    if (ui->cbAddressingType->currentIndex() == IP_ADDRESSING_STATIC_IP)
    {
        KeyCodeBoxSettings::SetHostAddress(ui->lblIpAddressValue->text());
        KeyCodeBoxSettings::SetBcastAddress(ui->lblIpBroadcastValue->text());
        KeyCodeBoxSettings::SetNetworkMask(ui->lblIpMaskValue->text());
        KeyCodeBoxSettings::SetGatewayAddress(ui->lblIpGatewayValue->text());
        KeyCodeBoxSettings::SetDnsAddress(ui->lblIpDnsValue->text());

        QString cidr_address = kcb::IpAddrSubnetMaskToCidr(ui->lblIpAddressValue->text(), ui->lblIpMaskValue->text());

        QString ip_address_str = QString("static ip_address=%1").arg(cidr_address);
        QString routers_str = QString("static routers=%1").arg(ui->lblIpGatewayValue->text());
        QString gateway_str = QString("static gateway=%1").arg(ui->lblIpGatewayValue->text());
        QString broadcast_str = QString("static broadcast=%1").arg(ui->lblIpBroadcastValue->text());
        QString dns_str = QString("static domain_name_servers=%1").arg(ui->lblIpDnsValue->text());
        QString address_settings = QString("interface eth0\n%1\n%2\n%4\n%5\n%6\n").arg(ip_address_str).arg(routers_str).arg(gateway_str).arg(broadcast_str).arg(dns_str);

        kcb::AddStaticAddressing(address_settings);
    }
    else
    {
        kcb::RemoveStaticAddressing();
    }

    if (ip_addressing_changed)
    {
        QString body(tr("You have selected to change the network addressing. "
                        "Continuing will change the IP address and associated parameters, requiring the system to restart.\n"
                        "If a VNC session is active, it will be terminated and must be restarted using the new IP address after the system restarts\n\n"
                        "Do you wish to continue?"));
        int result = warnOnChange(IP_WARN_DIALOG_TITLE, body, QMessageBox::Ok, QMessageBox::Cancel);
        if (result == QMessageBox::Ok)
        {
            kcb::RestartNetworkInterface();
            pending_reboot_ = true;
        }
        else
        {
            // Network addressing has been modified
            // We must revert the network addressing setting back to the initial setting
            ip_state.is_static ? KeyCodeBoxSettings::EnableStaticAddressing() : KeyCodeBoxSettings::DisableStaticAddressing();
        }
    }

    if (vnc_enable_change)
    {
        bool enable_disable = ui->cbVncEnable->isChecked();
        QString body(tr("VNC has been %0. "
                        "Continuing requires the system to restart.\n\n"
                        "Do you wish to continue?").arg(enable_disable ? "enabled" : "disabled"));
        int result = warnOnChange(VNC_WARN_DIALOG_TITLE, body, QMessageBox::Ok, QMessageBox::Cancel);
        if (result == QMessageBox::Ok)
        {
            if (enable_disable)
            {
                kcb::setVncEnabled();
            }
            else
            {
                kcb::setVncDisabled();
            }
            pending_reboot_ = true;
        }
        else
        {
            ui->cbVncEnable->setChecked(vnc_state.enable);
            ui->cbVncEnable->setText(vnc_state.enable ? "VNC Enabled" : "VNC Disabled");
            ui->lblVncPortValue->setText(vnc_state.port);
            ui->lblVncPasswordValue->setText(vnc_state.password);
        }
        
    }
    else if (vnc_port_change)
    {
        QString body(tr("The VNC port has been changed. "
                        "Continuing requires the system to restart.\n\n"
                        "Do you wish to continue?"));
        int result = warnOnChange(VNC_WARN_DIALOG_TITLE, body, QMessageBox::Ok, QMessageBox::Cancel);
        if (result == QMessageBox::Ok)
        {
            pending_reboot_ = true;
        }
        else
        {
            ui->lblVncPortValue->setText(vnc_state.port);
        }
    }
    else if (vnc_password_change)
    {
        QString body(tr("The VNC password has been changed. "
                        "Continuing requires the system to restart.\n\n"
                        "Do you wish to continue?"));
        int result = warnOnChange(VNC_WARN_DIALOG_TITLE, body, QMessageBox::Ok, QMessageBox::Cancel);
        if (result == QMessageBox::Ok)
        {
            pending_reboot_ = true;
        }
        else
        {
            ui->lblVncPasswordValue->setText(vnc_state.password);
        }
    }

}

void FrmNetworkSettings::on_bbNetworkingOkCancel_rejected()
{
    ip_state.is_static ? KeyCodeBoxSettings::EnableStaticAddressing() : KeyCodeBoxSettings::DisableStaticAddressing();

    ui->lblSmtpServerValue->setText(smtp_state.server);
    ui->lblSmtpPortValue->setText(smtp_state.port);
    if (smtp_state.type >= 0 || smtp_state.type <= 2)
    {
        ui->cbxSmtpSecurityValue->setCurrentIndex(smtp_state.type);
    }
    else
    {
        ui->cbxSmtpSecurityValue->setCurrentIndex(0);
    }
    ui->lblSmtpUsernameValue->setText(smtp_state.username);
    ui->lblSmtpPasswordValue->setText(smtp_state.password);

    ui->lblVncPortValue->setText(vnc_state.port);
    ui->lblVncPasswordValue->setText(vnc_state.password);
    ui->cbVncEnable->setChecked(vnc_state.enable);

}

void FrmNetworkSettings::on_cbxSmtpSecurityValue_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateUi();
}

bool FrmNetworkSettings::pendingReboot()
{
    bool active = pending_reboot_;
    pending_reboot_ = false;
    return active;
}

void FrmNetworkSettings::on_cbVncEnable_clicked()
{
    bool enable = false;
    QString port = "5901";
    QString password = "keycodebox";
    QString enable_text = "VNC Enabled";
    QString disable_text = "VNC Disabled";
    QString cb_text = disable_text;

    if (ui->cbVncEnable->isChecked())
    {
        KCB_DEBUG_TRACE(enable_text);
        enable = true;
        cb_text = enable_text;
        VncSettings vs = KeyCodeBoxSettings::getVncSettings();
        if (!vs.port.isEmpty())
        {
            port = vs.port;
        }

        if (!vs.password.isEmpty())
        {
            password = vs.password;
        }
    }
    else
    {
        KCB_DEBUG_TRACE(disable_text);
        enable = false;
        cb_text = disable_text;
        port = "";
        password = "";
    }

    ui->lblVncPortValue->setText(port);
    ui->lblVncPasswordValue->setText(password);
    ui->cbVncEnable->setText(cb_text);
    ui->lblVncPortValue->setEnabled(enable);
    ui->lblVncPort->setEnabled(enable);
    ui->lblVncPasswordValue->setEnabled(enable);
    ui->lblVncPassword->setEnabled(enable);

    updateUi();
}
