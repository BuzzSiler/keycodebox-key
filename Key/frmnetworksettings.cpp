#include "frmnetworksettings.h"
#include "ui_frmnetworksettings.h"

#include "kcbcommon.h"
#include "kcbkeyboarddialog.h"
#include "kcbsystem.h"

FrmNetworkSettings::FrmNetworkSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrmNetworkSettings)
{
    ui->setupUi(this);

    kcb::SetWindowParams(this);


    ui->lblIpAddressValue->setText(kcb::GetHostAddress());
    ui->lblIpBroadcastValue->setText(kcb::GetBcastAddress());
    ui->lblIpMaskValue->setText(kcb::GetNetworkMask());
    ui->lblMacAddressValue->setText(kcb::GetMacAddress());
    ui->lblIpGatewayValue->setText(kcb::GetGatewayAddress());
}

FrmNetworkSettings::~FrmNetworkSettings()
{
    delete ui;
}

void FrmNetworkSettings::setValues(int vncPort, QString vncPassword, QString smtpServer, int smtpPort, int smtpType, QString smtpUsername, QString smtpPassword)
{
    ui->lblVncPortValue->setText(QString::number(vncPort));
    ui->lblVncPasswordValue->setText(vncPassword);

    ui->lblSmtpServerValue->setText(smtpServer);
    ui->lblSmtpPortValue->setText(QString::number(smtpPort));
    ui->cbxSmtpSecurityValue->setCurrentIndex(0);
    if (smtpType >= 0 || smtpType <= 2)
    {
        ui->cbxSmtpSecurityValue->setCurrentIndex(smtpType);
    }
    ui->lblSmtpUsernameValue->setText(smtpUsername);
    ui->lblSmtpPasswordValue->setText(smtpPassword);
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

void FrmNetworkSettings::on_lblVncPortValue_clicked()
{
    QString text = ui->lblVncPortValue->text();
    RunKeyboard(text, true);
    ui->lblVncPortValue->setText(text);
}

void FrmNetworkSettings::on_lblVncPasswordValue_clicked()
{
    QString text = ui->lblVncPasswordValue->text();
    RunKeyboard(text, false);
    ui->lblVncPasswordValue->setText(text);
}

void FrmNetworkSettings::on_lblSmtpServerValue_clicked()
{
    QString text = ui->lblSmtpServerValue->text();
    RunKeyboard(text, false);
    ui->lblSmtpServerValue->setText(text);
}

void FrmNetworkSettings::on_lblSmtpPortValue_clicked()
{
    QString text = ui->lblSmtpPortValue->text();
    RunKeyboard(text, true);
    ui->lblSmtpPortValue->setText(text);
}

void FrmNetworkSettings::on_lblSmtpUsernameValue_clicked()
{
    QString text = ui->lblSmtpUsernameValue->text();
    RunKeyboard(text, false);
    ui->lblSmtpUsernameValue->setText(text);
}

void FrmNetworkSettings::on_lblSmtpPasswordValue_clicked()
{
    QString text = ui->lblSmtpPasswordValue->text();
    RunKeyboard(text, false);
    ui->lblSmtpPasswordValue->setText(text);
}
