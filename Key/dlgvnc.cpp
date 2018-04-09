#include <sstream>
#include <string>

#include "dlgvnc.h"
#include "ui_dlgvnc.h"

#include <QDebug>
#include "kcbkeyboarddialog.h"

CDlgVNC::CDlgVNC(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgVNC)
{
    ui->setupUi(this);
}

CDlgVNC::~CDlgVNC()
{
    delete ui;
}

void CDlgVNC::setValues(int nVNCPort, QString sPassword)
{
    _sVNCPort = QVariant(nVNCPort).toString();
    _sPassword = sPassword;

    ui->lblVNCPort->setText(QVariant(_sVNCPort).toString());
    ui->lblPassword->setText(_sPassword);
}

void CDlgVNC::getValues(int &nVNCPort, QString &sPassword)
{
    nVNCPort = ui->lblVNCPort->text().toInt();
    sPassword = ui->lblPassword->text();
}

void CDlgVNC::on_buttonBox_accepted()
{
    emit __onVNCDialogComplete(this);
}

void CDlgVNC::on_buttonBox_rejected()
{
    // Rejected
}

void CDlgVNC::RunKeyboard(QString& text, bool numbersOnly)
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(numbersOnly);
    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
}

void CDlgVNC::on_lblVNCPort_clicked()
{
    QString text = ui->lblVNCPort->text();
    RunKeyboard(text, true);
    ui->lblVNCPort->setText(text);
}

void CDlgVNC::on_lblPassword_clicked()
{
    QString text = ui->lblPassword->text();
    RunKeyboard(text);
    ui->lblPassword->setText(text);
}

/*
string GetEnv( const string & var ) {
     const char * val = ::getenv( var.c_str() );
     if ( val == 0 ) {
         return "";
     }
     else {
         return val;
     }
}
*/

void CDlgVNC::on_btnResetVNC_clicked()
{
    qDebug() << "btnRestVNC_clicked()";

    /* NOTE: the following would be better accessed via the KCB environment
       variables.  Above, is pasted a function for returning environment
       variables.  Consider creating a common or system module where all of
       the system ivocations and accesses can be placed.  Ideally, below
       should look more like:

       kcbsystem::update_password(vnc_port, password)
    */

    int nVNCPort = ui->lblVNCPort->text().toInt();
    QString sPassword = ui->lblPassword->text();
    std::string VNCPort = std::to_string(nVNCPort);
    std::string fullCmd = "/home/pi/kcb-config/scripts/kcb-remotedesktop.sh ";
    std::stringstream ss;
    ss << fullCmd << VNCPort << " /home/pi/kcb-config/config/x11vnc.cfg " << sPassword.toStdString();
    std::string newCmd = ss.str();

    FILE *cmdResult;
    cmdResult = popen(newCmd.c_str(), "r");
    if(!cmdResult)
    {
        qDebug() << "failed to popen(fullCmd, 'r');";
    }
  
}
