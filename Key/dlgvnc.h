#ifndef DLGVNC_H
#define DLGVNC_H

#include <QDialog>

namespace Ui {
class CDlgVNC;
}

class CDlgVNC : public QDialog
{
    Q_OBJECT

    public:
        explicit CDlgVNC(QWidget *parent = 0);
        ~CDlgVNC();

        void setValues(int nVNCPort, QString sPassword);
        void getValues(int &nVNCPort, QString &sPassword);

    signals:
        void __onVNCDialogComplete(CDlgVNC*);
    private slots:
        void on_buttonBox_accepted();
        void on_buttonBox_rejected();

        void on_lblVNCPort_clicked();
        void on_lblPassword_clicked();
        void on_btnResetVNC_clicked();

    private:
        QString _sVNCServer;
        QString _sVNCPort;
        QString _sVNCType;
        QString _sUsername;
        QString _sPassword;
        Ui::CDlgVNC *ui;
        void RunKeyboard(QString& text, bool numbersOnly = false);
};

#endif // DLGVNC_H
