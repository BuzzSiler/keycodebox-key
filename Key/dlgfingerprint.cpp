#include "dlgfingerprint.h"
#include "ui_dlgfingerprint.h"

#include <QDebug>

CDlgFingerprint::CDlgFingerprint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgFingerprint)
{
  ui->setupUi(this);
  setAttribute(Qt::WA_AcceptTouchEvents);
}

CDlgFingerprint::~CDlgFingerprint()
{
}

void CDlgFingerprint::shutdown()
{
}

void CDlgFingerprint::on_buttonBoxCancel_clicked()
{
    // Rejected
    emit __onEnrollFingerprintDialogCancel();
    this->hide();
}

void CDlgFingerprint::on_buttonBoxOk_clicked()
{
    // Accepted
    emit __onEnrollFingerprintDialogCancel();
    this->hide();
}

void CDlgFingerprint::setValues(int nFingerprintPort, QString sPassword)
{
    Q_UNUSED(nFingerprintPort);
    Q_UNUSED(sPassword);
}

void CDlgFingerprint::getValues(int &nFingerprintPort, QString &sPassword)
{
    Q_UNUSED(nFingerprintPort);
    Q_UNUSED(sPassword);
}

void CDlgFingerprint::setDefaultStage(int stage)
{
    ui->lblFPrintStageNumber->setText(QString::number(stage));
}

void CDlgFingerprint::setOkDisabled(bool disabled)
{
    ui->buttonBoxOk->setDisabled(disabled);
}

void CDlgFingerprint::setMessage(QString message)
{
    ui->lblFPrintMessage2->setText(message);
}

void CDlgFingerprint::OnUpdateEnrollFingerprintDialog(int current, int total, QString message)
{
    if( current > 1 )
    {
        ui->lblFPrintStageNumber->setText(QString::number(current));
    }
    else
    {
        ui->buttonBoxOk->setDisabled(false);
    }
    
    ui->lblFPrintMessage2->setText(message);
}
