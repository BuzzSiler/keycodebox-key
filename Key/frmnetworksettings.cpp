#include "frmnetworksettings.h"
#include "ui_frmnetworksettings.h"

#include "kcbkeyboarddialog.h"
#include "kcbcommon.h"

FrmNetworkSettings::FrmNetworkSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrmNetworkSettings),
    m_enable_dhcp(false),
    m_ip_address(""),
    m_ip_mask(""),
    m_ip_gateway(""),
    m_ip_dns(""),
    m_vnc_port(""),
    m_vnc_password(""),
    m_email_server(""),
    m_email_port(""),
    m_email_user(""),
    m_email_pw(""),
    m_email_security(0)

{
    ui->setupUi(this);

    FrmNetworkSettings::showFullScreen();
    FrmNetworkSettings::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->bbOkCancel->button(QDialogButtonBox::Ok)->setDisabled(true);
    ui->bbOkCancel->button(QDialogButtonBox::Cancel)->setEnabled(true);
}

FrmNetworkSettings::~FrmNetworkSettings()
{
    delete ui;
}

void FrmNetworkSettings::setValues(bool enableDhcp,
                                   QString ipAddress, QString ipMask, QString ipGateway, QString ipDns,
                                   QString vncPort, QString vncPassword,
                                   QString emailServer, QString emailPort, QString emailUsername,
                                   QString emailPassword, int emailSecurity)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE(vncPort << vncPassword);
    KCB_DEBUG_TRACE(emailServer << emailPort);
    KCB_DEBUG_TRACE(emailUsername << emailPassword << emailSecurity);

    m_enable_dhcp = enableDhcp;

    m_ip_address = ipAddress;
    m_ip_mask = ipMask;
    m_ip_gateway = ipGateway;
    m_ip_dns = ipDns;

    m_vnc_port = vncPort;
    m_vnc_password = vncPassword;

    m_email_server = emailServer;
    m_email_port = emailPort;
    m_email_user = emailUsername;
    m_email_pw = emailPassword;
    m_email_security = emailSecurity;

    ui->cbEnableDhcp->setChecked(enableDhcp);
    ui->edIpAddress->setText(m_ip_address);
    ui->edIpMask->setText(m_ip_mask);
    ui->edIpGateway->setText(m_ip_gateway);
    ui->edIpDns->setText(m_ip_dns);

    ui->edVncPort->setText(vncPort);
    ui->edVncPassword->setText(vncPassword);

    ui->edEmailServer->setText(emailServer);
    ui->edEmailPort->setText(emailPort);
    ui->edEmailAddress->setText(emailUsername);
    ui->edEmailPassword->setText(emailPassword);
    ui->cbSecurity->setCurrentIndex(emailSecurity);

    updateUi();

    KCB_DEBUG_EXIT;
}

void FrmNetworkSettings::getValues(bool& enableDhcp,
                                   QString& ipAddress, QString& ipMask, QString& ipGateway, QString& ipDns,
                                   QString& vncPort, QString& vncPassword,
                                   QString& emailServer, QString& emailPort, QString& emailUsername,
                                   QString& emailPassword, int& emailSecurity)
{
    enableDhcp = ui->cbEnableDhcp->isChecked();
    ipAddress = ui->edIpAddress->text();
    ipMask = ui->edIpMask->text();
    ipGateway = ui->edIpGateway->text();
    ipDns = ui->edIpDns->text();
    vncPort = ui->edVncPort->text();
    vncPassword = ui->edVncPassword->text();
    emailServer = ui->edEmailServer->text();
    emailPort = ui->edEmailPort->text().toInt();
    emailUsername = ui->edEmailAddress->text();
    emailPassword = ui->edEmailPassword->text();
    emailSecurity = ui->cbSecurity->currentIndex();
}

bool FrmNetworkSettings::isModified()
{
    bool enableDhcpChanged = (ui->cbEnableDhcp->checkState() == Qt::Checked) != m_enable_dhcp;
    bool ipAddressChanged = m_ip_address != ui->edIpAddress->text();
    bool ipMaskChanged = m_ip_mask != ui->edIpMask->text();
    bool ipGatewayChanged = m_ip_gateway != ui->edIpGateway->text();
    bool ipDnsChanged = m_ip_dns != ui->edIpDns->text();
    bool ipChanged = enableDhcpChanged || ipAddressChanged || ipMaskChanged || ipGatewayChanged || ipDnsChanged;

    bool vncPortChanged = m_vnc_port != ui->edVncPort->text();
    bool vncPasswordChanged = m_vnc_password != ui->edVncPassword->text();
    bool vncChanged = vncPortChanged || vncPasswordChanged;

    bool emailServerChanged = m_email_server != ui->edEmailServer->text();
    bool emailPortChanged = m_email_port != ui->edEmailPort->text();
    bool emailAddedChanged = m_email_user != ui->edEmailAddress->text();
    bool emailPasswordChanged = m_email_pw != ui->edEmailPassword->text();
    bool emailSecurityChanged = m_email_security != ui->cbSecurity->currentIndex();
    bool emailChanged = emailServerChanged || emailPortChanged || emailAddedChanged || emailPasswordChanged || emailSecurityChanged;

    return ipChanged || vncChanged || emailChanged;
}

void FrmNetworkSettings::updateUi()
{
    bool modified = isModified();

    bool dhcp_enabled = ui->cbEnableDhcp->checkState() == Qt::Checked;
    ui->edIpAddress->setDisabled(dhcp_enabled);
    ui->edIpMask->setDisabled(dhcp_enabled);
    ui->edIpGateway->setDisabled(dhcp_enabled);
    ui->edIpDns->setDisabled(dhcp_enabled);

    ui->bbOkCancel->button(QDialogButtonBox::Ok)->setEnabled(modified);
}

void FrmNetworkSettings::RunKeyboard(QString& text, bool numbersOnly)
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(numbersOnly);
    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
}

void FrmNetworkSettings::on_cbEnableDhcp_clicked()
{
    updateUi();
}

void FrmNetworkSettings::on_edIpAddress_clicked()
{
    QString text = ui->edIpAddress->text();
    RunKeyboard(text, false);
    ui->edIpAddress->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edIpMask_clicked()
{
    QString text = ui->edIpMask->text();
    RunKeyboard(text, false);
    ui->edIpMask->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edIpGateway_clicked()
{
    QString text = ui->edIpGateway->text();
    RunKeyboard(text, false);
    ui->edIpGateway->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edIpDns_clicked()
{
    QString text = ui->edIpDns->text();
    RunKeyboard(text, false);
    ui->edIpDns->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edVncPort_clicked()
{
    QString text = ui->edVncPort->text();
    RunKeyboard(text, true);
    ui->edVncPort->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edVncPassword_clicked()
{
    QString text = ui->edVncPassword->text();
    RunKeyboard(text, false);
    ui->edVncPassword->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edEmailServer_clicked()
{
    QString text = ui->edEmailServer->text();
    RunKeyboard(text, false);
    ui->edEmailServer->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edEmailPort_clicked()
{
    QString text = ui->edEmailPort->text();
    RunKeyboard(text, true);
    ui->edEmailPort->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edEmailAddress_clicked()
{
    QString text = ui->edEmailAddress->text();
    RunKeyboard(text, false);
    ui->edEmailAddress->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_edEmailPassword_clicked()
{
    QString text = ui->edEmailPassword->text();
    RunKeyboard(text, false);
    ui->edEmailPassword->setText(text);

    updateUi();
}

void FrmNetworkSettings::on_cbSecurity_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateUi();
}

void FrmNetworkSettings::on_pbEmailSend_clicked()
{
}
