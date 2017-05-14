#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "dlgfingerprintverify.h"
#include "ui_dlgfingerprintverify.h"

#include <QDebug>
#include "dlgfullkeyboard.h"
#include "clickablelabel.h"

CDlgFingerprintVerify::CDlgFingerprintVerify(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgFingerprintVerify)
{
  ui->setupUi(this);
  //  CDlgFingerprintVerify::showFullScreen();
  setAttribute(Qt::WA_AcceptTouchEvents);
}

CDlgFingerprintVerify::~CDlgFingerprintVerify()
{
}

void CDlgFingerprintVerify::shutdown()
{
}

void CDlgFingerprintVerify::on_buttonBoxCancel_clicked()
{
  // Rejected
  emit __onVerifyFingerprintDialogCancel();
  this->hide();
}

void CDlgFingerprintVerify::on_buttonBoxOk_clicked()
{
  // Accepted
  emit __onVerifyFingerprintDialogCancel();
  this->hide();
}

void CDlgFingerprintVerify::setValues(int nFingerprintPort, QString sPassword)
{
}

void CDlgFingerprintVerify::getValues(int &nFingerprintPort, QString &sPassword)
{
}

void CDlgFingerprintVerify::setMessage(QString message)
{
  ui->lblFPrintMessage2->setText(message);
}

void CDlgFingerprintVerify::OnUpdateVerifyFingerprintDialog(bool result, QString message)
{
  qDebug() << "CDlgFingerprintVerify::OnUpdateVerifyFingerprintDialog()";
  qDebug() << result;
  qDebug() << message;
  this->setMessage(message);
}
