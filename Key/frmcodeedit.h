#ifndef FRMCODEEDIT_H
#define FRMCODEEDIT_H

#include <QDialog>
#include <QAbstractButton>
#include <QDateTime>
#include "clickablelineedit.h"
#include "dlgfullkeyboard.h"
#include "currentedit.h"
#include "dlgeditquestions.h"

namespace Ui {
class CFrmCodeEdit;
}

class CFrmCodeEdit : public QDialog
{
    Q_OBJECT


public:
    explicit CFrmCodeEdit(QWidget *parent = 0);
    ~CFrmCodeEdit();

    void getValues(int *pId, int *pLockNum, QString *psAccessCode, QString *psSecondCode, QString *psDesc,
		   QDateTime *pdtStart, QDateTime *pdtEnd, bool *pFingerprint1, bool *pFingerprint2);
    
    void setValues(int id, int nLockNum, QString sAccessCode, QString sSecondCode, QString sDesc,
		   QDateTime dtStart, QDateTime dtEnd, bool fingerprint1, bool fingerprint2,
		   bool askQuestions, std::string question1, std::string question2, std::string question3);

    void setMaxLocks(int nLocks);
    void setEditingRow(int nRow);
    int getEditingRow() { return _nRow; }

private:
    Ui::CFrmCodeEdit *ui;

    CDlgEditQuestions *_pDlgEditQuestions = 0;
    CCurrentEdit    *_pcurrentLineEdit = 0;
    CDlgFullKeyboard *_pKeyboard = 0;

    int _id;
    int _nRow;
    bool _askQuestions;
    QString _question1;
    QString _question2;
    QString _question3;

    void checkAndCreateCurrentLineEdit();
signals:
    void OnDoneSave(int nRow, int nId, int nLockNum, QString sAccessCode, QString sSecondCode, QString sDescription, QDateTime dtStart, QDateTime dtEnd, bool fingerprint1, bool fingerprint2, bool askQuestions, QString question1, QString question2, QString question3);
    void OnClose();
    void __OnCodes(QString code1);
    void __OnQuestionEditSave();
    void __OnQuestionEditClose();
    
private slots:
  
    void on_edtAccessCode_clicked();
    void on_edtSecondCode_clicked();
    void on_edtDescription_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_chkAllAccess_clicked(bool checked);
    void on_chkFingerPrint1_clicked(bool checked);
    void on_chkQuestions_clicked(bool checked);
    void on_chkFingerPrint2_clicked(bool checked);
    
    void OnAdminInfoCodes(QString code1, QString code2);

    void on_clrAccessCode_clicked();
    void on_clrSecondCode_clicked();
    void on_clrDescription_clicked();
    
public slots:
    void onStartEditLine(QLineEdit *pLine, QString sLabelText);
    void OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);
    void OnQuestionEditSave();
    void OnQuestionEditClose();
};

#endif // FRMCODEEDIT_H
