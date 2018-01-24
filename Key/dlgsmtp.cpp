#include "dlgsmtp.h"
#include "ui_dlgsmtp.h"

#include <QDebug>
#include "dlgfullkeyboard.h"
#include "clickablelabel.h"

CDlgSMTP::CDlgSMTP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgSMTP)
{
    ui->setupUi(this);

    _pKeyboard = NULL;
    _pcurrentLabelEdit = NULL;
}

CDlgSMTP::~CDlgSMTP()
{
    shutdown();
    delete ui;
}

void CDlgSMTP::initializeKeyboard()
{
    _pKeyboard = new CDlgFullKeyboard();

    connect(_pKeyboard, SIGNAL(__TextEntered(CDlgFullKeyboard*, CCurrentEdit *)), this, SLOT(OnTextEntered(CDlgFullKeyboard*, CCurrentEdit *)));
    connect(_pKeyboard, SIGNAL(__OnCancelKeyboard(CDlgFullKeyboard*,CCurrentEdit*)), this, SLOT(OnCancelKeyboard(CDlgFullKeyboard*, CCurrentEdit *)));
    connect(_pKeyboard, SIGNAL(__OnCloseKeyboard(CDlgFullKeyboard*)), this, SLOT(OnCloseKeyboard(CDlgFullKeyboard*)));
}

void CDlgSMTP::shutdown()
{
    if(_pKeyboard) {
        delete _pKeyboard;
    }
    if( _pcurrentLabelEdit ) {
        delete _pcurrentLabelEdit;
    }
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
    if(nSMTPType == 0) {
        ui->rbtnNone->setChecked(true);
    }
    else if(nSMTPType == 1) {
        ui->rbtnSSL->setChecked(true);
    }
    else if(nSMTPType == 2) {
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
        nSMTPType = 0;
    else if(ui->rbtnSSL->isChecked())
        nSMTPType = 1;
    else if(ui->rbtnTLS->isChecked())
        nSMTPType = 2;

}


void CDlgSMTP::onStartEditLabel(QLabel* pLabel, QString sLabelText)
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

void CDlgSMTP::OnTextEntered(CDlgFullKeyboard *pkeyboard, CCurrentEdit *pCurrEdit)
{
    Q_UNUSED(pkeyboard);
    qDebug() << "SMTP OnTextEntered";
    pCurrEdit->getLabelBeingEdited()->setText(pCurrEdit->getNewText());
    _pKeyboard->hide();

}

void CDlgSMTP::OnCancelKeyboard(CDlgFullKeyboard *pkeyboard, CCurrentEdit *pCurrEdit)
{
    // Do nothing
    Q_UNUSED(pkeyboard);
    Q_UNUSED(pCurrEdit);
}

void CDlgSMTP::OnCloseKeyboard(CDlgFullKeyboard *pkeyboard)
{
    // Do nothing
    Q_UNUSED(pkeyboard);
}

void CDlgSMTP::on_buttonBox_accepted()
{
    //do something
    emit __onSMTPDialogComplete(this);
}

void CDlgSMTP::on_buttonBox_rejected()
{
    // Rejected
}

void CDlgSMTP::checkAndCreateCurrentLabelEdit()
{
    if(!_pcurrentLabelEdit) {
        qDebug() << "creating CurrentEdit";
        _pcurrentLabelEdit = new CCurrentEdit();
    }
}

void CDlgSMTP::on_lblSMTPServer_clicked()
{
    //
    qDebug() << "lblSMTPServer clicked";
    checkAndCreateCurrentLabelEdit();
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblSMTPServer, "SMTP Server Address");
}

void CDlgSMTP::on_lblSMTPPort_clicked()
{
    //
    checkAndCreateCurrentLabelEdit();
    _pcurrentLabelEdit->setNumbersOnly();
    onStartEditLabel(ui->lblSMTPPort, "SMTP Server Port");
}

void CDlgSMTP::on_lblUsername_clicked()
{
    //
    checkAndCreateCurrentLabelEdit();
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblUsername, "User Name");
}

void CDlgSMTP::on_lblPassword_clicked()
{
    //
    checkAndCreateCurrentLabelEdit();
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblPassword, "Password");
}
