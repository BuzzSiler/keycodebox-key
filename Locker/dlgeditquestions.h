#ifndef DLGEDITQUESTIONS_H
#define DLGEDITQUESTIONS_H

#include <QDialog>
#include <QAbstractButton>
#include <QDateTime>
#include "clickablelineedit.h"
#include "dlgfullkeyboard.h"
#include "currentedit.h"

namespace Ui {
class CDlgEditQuestions;
}

class CDlgEditQuestions : public QDialog
{
    Q_OBJECT


public:
    explicit CDlgEditQuestions(QWidget *parent = 0);
    ~CDlgEditQuestions();

    void getValues(QString *question1, QString *question2, QString *question3);
    void setValues(QString question1, QString question2, QString question3);

    void setMaxLocks(int nLocks);
    void setEditingRow(int nRow) { _nRow = nRow; }
    int getEditingRow() { return _nRow; }

private:
    Ui::CDlgEditQuestions *ui;

    CCurrentEdit    *_pcurrentLineEdit = 0;
    CDlgFullKeyboard *_pKeyboard = 0;

    int _id;
    int _nRow;

    void checkAndCreateCurrentLineEdit();
signals:
    
    void __OnQuestionEditClose();
    void __OnQuestionEditSave();
    
    void OnClose();
    void __OnCodes(QString code1);

private slots:

    void on_edtQuestion1_clicked();
    void on_edtQuestion2_clicked();
    void on_edtQuestion3_clicked();

    void on_buttonBoxQuestions_accepted();

    void on_buttonBoxQuestions_rejected();

    void on_buttonBoxQuestions_clicked(QAbstractButton *button);

    void OnAdminInfoCodes(QString code1, QString code2);

    void on_clrQuestion1_clicked();
    void on_clrQuestion2_clicked();
    void on_clrQuestion3_clicked();

public slots:
    void onStartEditLine(QLineEdit *pLine, QString sLabelText);
    void OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);
};

#endif // DLGEDITQUESTIONS_H
