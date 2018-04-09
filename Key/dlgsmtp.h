#ifndef DLGSMTP_H
#define DLGSMTP_H

#include <QDialog>

namespace Ui {
class CDlgSMTP;
}

class CDlgSMTP : public QDialog
{
    Q_OBJECT

    public:
        explicit CDlgSMTP(QWidget *parent = 0);
        ~CDlgSMTP();

        void setValues(QString sSMTPServer, int nSMTPPort, int nSMTPType, QString sUsername, QString sPassword);
        void getValues(QString &sSMTPServer, int &nSMTPPort, int &nSMTPType, QString &sUsername, QString &sPassword);

    signals:
        void __onSMTPDialogComplete(CDlgSMTP*);
    private slots:
        void on_buttonBox_accepted();
        void on_buttonBox_rejected();

        void on_lblSMTPServer_clicked();
        void on_lblSMTPPort_clicked();
        void on_lblUsername_clicked();
        void on_lblPassword_clicked();

    private:
        QString _sSMTPServer;
        QString _sSMTPPort;
        QString _sSMTPType;
        QString _sUsername;
        QString _sPassword;
        Ui::CDlgSMTP *ui;
        void RunKeyboard(QString& text, bool numbersOnly = false);
};

#endif // DLGSMTP_H
