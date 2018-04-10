#include "dlgsmtp.h"
#include "ui_dlgsmtp.h"

#include <QDebug>
#include "kcbkeyboarddialog.h"

CDlgSMTP::CDlgSMTP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgSMTP)
{
    ui->setupUi(this);

}

CDlgSMTP::~CDlgSMTP()
{
    delete ui;
}

void CDlgSMTP::setValues(QString sSMTPServer, int nSMTPPort, int nSMTPType, QString sUsername, QString sPassword)
{
    _sSMTPServer = sSMTPServer;
    _sSMTPPort = QVariant(nSMTPPort).toString();
    _sUsername = sUsername;
    _sPassword = sPassword;

    ui->lblSMTPServer->setText(_sSMTPServer);
    ui->lblSMTPPort->setText(QVariant(_sSMTPPort).toString());
    ui->lblUsername->setText(_sUsername);
    ui->lblPassword->setText(_sPassword);

    ui->rbtnNone->setChecked(false);
    ui->rbtnSSL->setChecked(false);
    ui->rbtnTLS->setChecked(false);
    if(nSMTPType == 0)
    {
        ui->rbtnNone->setChecked(true);
    }
    else if(nSMTPType == 1)
    {
        ui->rbtnSSL->setChecked(true);
    }
    else if(nSMTPType == 2)
    {
        ui->rbtnTLS->setChecked(true);
    }
}

void CDlgSMTP::getValues(QString &sSMTPServer, int &nSMTPPort, int &nSMTPType, QString &sUsername, QString &sPassword)
{
    sSMTPServer = ui->lblSMTPServer->text();
    nSMTPPort = ui->lblSMTPPort->text().toInt();

    sUsername = ui->lblUsername->text();
    sPassword = ui->lblPassword->text();

    if(ui->rbtnNone->isChecked())
    {
        nSMTPType = 0;
    }
    else if(ui->rbtnSSL->isChecked())
    {
        nSMTPType = 1;
    }
    else if(ui->rbtnTLS->isChecked())
    {
        nSMTPType = 2;
    }

}

void CDlgSMTP::on_buttonBox_accepted()
{
    emit __onSMTPDialogComplete(this);
}

void CDlgSMTP::on_buttonBox_rejected()
{
    // Rejected
}

void CDlgSMTP::RunKeyboard(QString& text, bool numbersOnly)
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(numbersOnly);
    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
}

void CDlgSMTP::on_lblSMTPServer_clicked()
{
    QString text = ui->lblSMTPServer->text();
    RunKeyboard(text);
    ui->lblSMTPServer->setText(text);
}

void CDlgSMTP::on_lblSMTPPort_clicked()
{
    QString text = ui->lblSMTPPort->text();
    RunKeyboard(text, true);
    ui->lblSMTPPort->setText(text);
}

void CDlgSMTP::on_lblUsername_clicked()
{
    QString text = ui->lblUsername->text();
    RunKeyboard(text);
    ui->lblUsername->setText(text);
}

void CDlgSMTP::on_lblPassword_clicked()
{
    QString text = ui->lblPassword->text();
    RunKeyboard(text);
    ui->lblPassword->setText(text);
}
