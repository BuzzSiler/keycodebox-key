#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "dlgvnc.h"
#include "ui_dlgvnc.h"

#include <QDebug>
#include "dlgfullkeyboard.h"
#include "clickablelabel.h"

CDlgVNC::CDlgVNC(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgVNC)
{
    ui->setupUi(this);

    _pKeyboard = NULL;
    _pcurrentLabelEdit = NULL;
}

CDlgVNC::~CDlgVNC()
{
    shutdown();
    delete ui;
}

void CDlgVNC::initializeKeyboard()
{
    _pKeyboard = new CDlgFullKeyboard();

    connect(_pKeyboard, SIGNAL(__TextEntered(CDlgFullKeyboard*, CCurrentEdit *)), this, SLOT(OnTextEntered(CDlgFullKeyboard*, CCurrentEdit *)));
    connect(_pKeyboard, SIGNAL(__OnCancelKeyboard(CDlgFullKeyboard*,CCurrentEdit*)), this, SLOT(OnCancelKeyboard(CDlgFullKeyboard*, CCurrentEdit *)));
    connect(_pKeyboard, SIGNAL(__OnCloseKeyboard(CDlgFullKeyboard*)), this, SLOT(OnCloseKeyboard(CDlgFullKeyboard*)));
}

void CDlgVNC::shutdown()
{
    if(_pKeyboard) {
        delete _pKeyboard;
    }
    if( _pcurrentLabelEdit ) {
        delete _pcurrentLabelEdit;
    }
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


void CDlgVNC::onStartEditLabel(QLabel* pLabel, QString sLabelText)
{
    qDebug() << "onStartEditLabel()";
    if(!_pKeyboard) {
        qDebug() << "initializing keyboard()";
        initializeKeyboard();
    }
    qDebug() << "keyboard and CurrentEdit?";
    if(_pKeyboard && _pcurrentLabelEdit)
    {
        qDebug() << "...yes!";
        _pKeyboard->setCurrentEdit(_pcurrentLabelEdit);
        _pcurrentLabelEdit->setLabelToBeEdited(pLabel);
        _pcurrentLabelEdit->setOriginalTextToEdit(pLabel->text());
        _pcurrentLabelEdit->setLabelText(sLabelText);

        qDebug() << "onStartEditLabel(): Showing Keyboard";
        _pKeyboard->show();
    } else if(!_pKeyboard) {
        qDebug() << "No keyboard";
    } else if(!_pcurrentLabelEdit) {
        qDebug() << "No CurrentEdit";
    }
}

void CDlgVNC::OnTextEntered(CDlgFullKeyboard *pkeyboard, CCurrentEdit *pCurrEdit)
{
    Q_UNUSED(pkeyboard);
    qDebug() << "VNC OnTextEntered";
    pCurrEdit->getLabelBeingEdited()->setText(pCurrEdit->getNewText());
    _pKeyboard->hide();

}

void CDlgVNC::OnCancelKeyboard(CDlgFullKeyboard *pkeyboard, CCurrentEdit *pCurrEdit)
{
    // Do nothing
    Q_UNUSED(pkeyboard);
    Q_UNUSED(pCurrEdit);
}

void CDlgVNC::OnCloseKeyboard(CDlgFullKeyboard *pkeyboard)
{
    // Do nothing
    Q_UNUSED(pkeyboard);
}

void CDlgVNC::on_buttonBox_accepted()
{
    //do something
    emit __onVNCDialogComplete(this);
}

void CDlgVNC::on_buttonBox_rejected()
{
    // Rejected
}

void CDlgVNC::checkAndCreateCurrentLabelEdit()
{
    if(!_pcurrentLabelEdit) {
        qDebug() << "creating CurrentEdit";
        _pcurrentLabelEdit = new CCurrentEdit();
    }
}

void CDlgVNC::on_lblVNCPort_clicked()
{
  qDebug() << "liblVNCPort_clicked()";
    checkAndCreateCurrentLabelEdit();
    _pcurrentLabelEdit->setNumbersOnly();
    onStartEditLabel(ui->lblVNCPort, "VNC Server Port");
}

void CDlgVNC::on_lblPassword_clicked()
{
  qDebug() << "liblPassword_clicked()";
    checkAndCreateCurrentLabelEdit();
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblPassword, "Password");
}

void CDlgVNC::on_btnResetVNC_clicked()
{
  qDebug() << "btnRestVNC_clicked()";

  int nVNCPort = ui->lblVNCPort->text().toInt();
  QString sPassword = ui->lblPassword->text();
  std::string VNCPort = std::to_string(nVNCPort);
  std::string fullCmd = "/home/pi/scripts/start_remote_desktop.sh ";
  std::stringstream ss;
  ss << fullCmd << VNCPort << " /home/pi/etc/x11vnc.cfg " << sPassword.toStdString();
  std::string newCmd = ss.str();
  
  FILE *cmdResult;
  cmdResult = popen(newCmd.c_str(), "r");
  if(!cmdResult)
    {
      std::cout << "\tfailed to popen(fullCmd, 'r');\n";
    }
  
}
