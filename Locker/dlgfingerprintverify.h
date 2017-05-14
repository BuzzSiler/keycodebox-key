#ifndef DLGFINGERPRINTVERIFY_H
#define DLGFINGERPRINTVERIFY_H

#include <QDialog>
#include "clickablelabel.h"
#include "dlgfullkeyboard.h"

namespace Ui {
  class CDlgFingerprintVerify;
}

class CDlgFingerprintVerify : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgFingerprintVerify(QWidget *parent = 0);
    ~CDlgFingerprintVerify();

    void setValues(int nFingerprintPort, QString sPassword);
    void getValues(int &nFingerprintPort, QString &sPassword);
    void setMessage(QString message);

 signals:
    //void __onFingerprintDialogComplete(CDlgFingerprint*);
    void __onVerifyFingerprintDialogCancel();
    void __onUpdateVerifyFingerprintDialog(bool result, QString message);
    
private slots:
    void on_buttonBoxCancel_clicked();
    void on_buttonBoxOk_clicked();
    
public slots:
  void OnUpdateVerifyFingerprintDialog(bool result, QString message);
    
 private:
    Ui::CDlgFingerprintVerify *ui;
    void shutdown();

};

#endif // DLGFINGERPRINTVERIFY_H
