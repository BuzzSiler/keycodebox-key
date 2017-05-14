#ifndef DLGFINGERPRINT_H
#define DLGFINGERPRINT_H

#include <QDialog>
#include "clickablelabel.h"
#include "dlgfullkeyboard.h"

namespace Ui {
  class CDlgFingerprint;
}

class CDlgFingerprint : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgFingerprint(QWidget *parent = 0);
    ~CDlgFingerprint();

    void setValues(int nFingerprintPort, QString sPassword);
    void getValues(int &nFingerprintPort, QString &sPassword);
    void setDefaultStage(int stage);
    void setMessage(QString message);
    void setOkDisabled(bool disabled);

 signals:
    //void __onFingerprintDialogComplete(CDlgFingerprint*);
    void __onEnrollFingerprintDialogCancel();
    void __onUpdateEnrollFingerprintDialog(int current, int total);
    
private slots:
    void on_buttonBoxCancel_clicked();
    void on_buttonBoxOk_clicked();
    
public slots:
  void OnUpdateEnrollFingerprintDialog(int current, int total, QString message);
    
 private:
    Ui::CDlgFingerprint *ui;
    void shutdown();

};

#endif // DLGFINGERPRINT_H
