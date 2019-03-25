#include "frmnetworksettings.h"
#include "ui_frmnetworksettings.h"

#include <QPushButton>
#include <QMessageBox>

#include "kcbcommon.h"
#include "kcbkeyboarddialog.h"
#include "kcbsystem.h"
#include "kcbutils.h"


static const int IP_ADDRESSING_STATIC_IP = 0;
static const int IP_ADDRESSING_DHCP = 1;

FrmNetworkSettings::FrmNetworkSettings(QWidget *parent) :
    QDialog(parent),
    ip_state({false, "", "", "", "", "0.0.0.0"}),
    smtp_state({"", "", 0, "", ""}),
    vnc_state({"", ""}),
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
    delete ui;
}

void FrmNetworkSettings::setValues(int vncPort, QString vncPassword, QString smtpServer, int smtpPort, int smtpType, QString smtpUsername, QString smtpPassword)
{
    // IP Network Values
    ip_state.is_static = kcb::StaticAddressingEnabled();
    ip_state.host = kcb::GetHostAddress();
    ip_state.broadcast = kcb::GetBcastAddress();
    ip_state.mask = kcb::GetNetworkMask();
    ip_state.gateway = kcb::GetGatewayAddress();
    ip_state.dns = kcb::GetDnsAddress();

    ui->lblIpAddressValue->setText(ip_state.host);
    ui->lblIpBroadcastValue->setText(ip_state.broadcast);
    ui->lblIpMaskValue->setText(ip_state.mask);
    ui->lblIpGatewayValue->setText(ip_state.gateway);
    ui->lblIpDnsValue->setText(ip_state.dns);
    ui->lblMacAddressValue->setText(kcb::GetMacAddress());

    KCB_DEBUG_TRACE("is static" << ip_state.is_static);
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
    vnc_state.port = QString::number(vncPort);
    vnc_state.password = vncPassword;

    ui->lblVncPortValue->setText(vnc_state.port);
    ui->lblVncPasswordValue->setText(vnc_state.password);

    updateUi();
}

void FrmNetworkSettings::getValues(int& vncPort, QString& vncPassword, QString& smtpServer, int& smtpPort, int& smtpType, QString& smtpUsername, QString& smtpPassword)
{
    vncPort = ui->lblVncPortValue->text().toInt();
    vncPassword = ui->lblVncPasswordValue->text();

    smtpServer = ui->lblSmtpServerValue->text();
    smtpPort = ui->lblSmtpPortValue->text().toInt();
    smtpType = ui->cbxSmtpSecurityValue->currentIndex();
    smtpUsername = ui->lblSmtpUsernameValue->text();
    smtpPassword = ui->lblSmtpPasswordValue->text();
}

void FrmNetworkSettings::RunKeyboard(QString& text, bool numbersOnly, bool ipAddress)
{
    KcbKeyboardDialog kkd;

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
    QString text = ui->lblVncPortValue->text();
    RunKeyboard(text, true);
    ui->lblVncPortValue->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_lblVncPasswordValue_clicked()
{
    QString text = ui->lblVncPasswordValue->text();
    RunKeyboard(text, false);
    ui->lblVncPasswordValue->setText(text);

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
    KCB_DEBUG_ENTRY;

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
            kcb::EnableStaticAddressing();
        }
    }
    else
    {
        kcb::DisableStaticAddressing();

        ui->lblIpAddressValue->setText(kcb::GetHostAddress());
        ui->lblIpBroadcastValue->setText(kcb::GetBcastAddress());
        ui->lblIpMaskValue->setText(kcb::GetNetworkMask());
        ui->lblIpGatewayValue->setText(kcb::GetGatewayAddress());
        ui->lblIpDnsValue->setText(kcb::GetDnsAddress());
    }

    updateUi();

    KCB_DEBUG_EXIT;
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
    KCB_DEBUG_ENTRY;
    HandleIpSettingClick(*ui->lblIpDnsValue);
    KCB_DEBUG_EXIT;
}

void FrmNetworkSettings::HandleIpSettingClick(CClickableLabel& label)
{
    if (kcb::StaticAddressingEnabled())
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
    bool static_change = ip_state.is_static != kcb::StaticAddressingEnabled();
    bool host_change = ip_state.host != ui->lblIpAddressValue->text();
    bool broadcast_change = ip_state.broadcast != ui->lblIpBroadcastValue->text();
    bool mask_change = ip_state.mask != ui->lblIpMaskValue->text();
    bool gateway_change = ip_state.gateway != ui->lblIpGatewayValue->text();
    bool dns_change = ip_state.dns != ui->lblIpDnsValue->text();
    bool ip_change = (static_change || host_change || broadcast_change || mask_change || gateway_change || dns_change);

    bool server_change = smtp_state.server != ui->lblSmtpServerValue->text();
    bool port_change = smtp_state.port != ui->lblSmtpPortValue->text();
    bool type_change = smtp_state.type != ui->cbxSmtpSecurityValue->currentIndex();
    bool username_change = smtp_state.username != ui->lblSmtpUsernameValue->text();
    bool password_change = smtp_state.password != ui->lblSmtpPasswordValue->text();
    bool smtp_change = (server_change || port_change || type_change || username_change || password_change);

    port_change = vnc_state.port != ui->lblVncPortValue->text();
    password_change = vnc_state.password != ui->lblVncPasswordValue->text();
    bool vnc_change = (port_change || password_change);

    ui->bbNetworkingOkCancel->button(QDialogButtonBox::Ok)->setEnabled(smtp_change || vnc_change || ip_change);
    // KCB_DEBUG_EXIT;
}

void FrmNetworkSettings::on_bbNetworkingOkCancel_accepted()
{
    KCB_DEBUG_ENTRY;

    if (ui->cbAddressingType->currentIndex() == IP_ADDRESSING_STATIC_IP)
    {
        kcb::SetHostAddress(ui->lblIpAddressValue->text());
        kcb::SetBcastAddress(ui->lblIpBroadcastValue->text());
        kcb::SetNetworkMask(ui->lblIpMaskValue->text());
        kcb::SetGatewayAddress(ui->lblIpGatewayValue->text());
        kcb::SetDnsAddress(ui->lblIpDnsValue->text());

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

    int result = QMessageBox::warning(this,
                                        tr("Network Addressing Changed"), 
                                        tr("You have selected to change the network addressing. "
                                            "Continuing will change the IP address and associated parameters, requiring the system to reboot.\n"
                                            "If a VNC session is active, it will be terminated and must be restarted using the new IP address after the system reboots\n\n"
                                            "Do you wish to continue?"),
                                        QMessageBox::Ok, 
                                        QMessageBox::Cancel);
    if (result == QMessageBox::Ok)
    {
        kcb::RestartNetworkInterface();
        kcb::Reboot();
    }
    else
    {
        // Network addressing has been modified
        // We must revert the network addressing setting back to the initial setting
        ip_state.is_static ? kcb::EnableStaticAddressing() : kcb::DisableStaticAddressing();
    }

    KCB_DEBUG_EXIT;
}

void FrmNetworkSettings::on_bbNetworkingOkCancel_rejected()
{
    ip_state.is_static ? kcb::EnableStaticAddressing() : kcb::DisableStaticAddressing();

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

}

void FrmNetworkSettings::on_cbxSmtpSecurityValue_currentIndexChanged(int index)
{
    updateUi();
}
