#ifndef DLGFINGERPRINTVERIFY_H
#define DLGFINGERPRINTVERIFY_H

#include <QDialog>

namespace Ui {
  class CDlgFingerprintVerify;
}

class CDlgFingerprintVerify : public QDialog
{
    Q_OBJECT

    public:
        explicit CDlgFingerprintVerify(QWidget *parent = 0);
        ~CDlgFingerprintVerify();

        void setMessage(QString message);

    signals:
        void __onVerifyFingerprintDialogCancel();
        void __onUpdateVerifyFingerprintDialog(bool result, QString message);

    private slots:
        void on_bbClose_clicked();

    public slots:
        void OnUpdateVerifyFingerprintDialog(bool result, QString message);

    private:
        Ui::CDlgFingerprintVerify *ui;

};

#endif // DLGFINGERPRINTVERIFY_H
