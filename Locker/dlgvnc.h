#ifndef DLGVNC_H
#define DLGVNC_H

#include <QDialog>
#include "clickablelabel.h"
#include "dlgfullkeyboard.h"

namespace Ui {
class CDlgVNC;
}

class CDlgVNC : public QDialog
{
    Q_OBJECT

private:
    QString _sVNCServer;
    QString _sVNCPort;
    QString _sVNCType;
    QString _sUsername;
    QString _sPassword;

    CCurrentEdit        *_pcurrentLabelEdit = 0;
    CDlgFullKeyboard    *_pKeyboard = 0;

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
    
    void OnTextEntered(CDlgFullKeyboard *pkeyboard, CCurrentEdit *pCurrEdit);
    void OnCancelKeyboard(CDlgFullKeyboard *pkeyboard, CCurrentEdit *pCurrEdit);
    void OnCloseKeyboard(CDlgFullKeyboard *pkeyboard);
private:
    Ui::CDlgVNC *ui;
    void onStartEditLabel(QLabel *pLabel, QString sLabelText);
    void initializeKeyboard();
    void shutdown();
    void checkAndCreateCurrentLabelEdit();
};

#endif // DLGVNC_H
