#ifndef DLGQUESTIONS_H
#define DLGQUESTIONS_H

#include <QDialog>
#include <QAbstractButton>
#include <QDateTime>
#include "clickablelineedit.h"
#include "dlgfullkeyboard.h"
#include "currentedit.h"

namespace Ui {
class CDlgQuestions;
}

class CDlgQuestions : public QDialog
{
    Q_OBJECT


public:
    explicit CDlgQuestions(QWidget *parent = 0);
    ~CDlgQuestions();

    void getValues(QString *question1, QString *question2, QString *question3);
    void setValues(QString lockNum, QString question1, QString question2, QString question3);

    void setMaxLocks(int nLocks);
    void setEditingRow(int nRow) { _nRow = nRow; }
    int getEditingRow() { return _nRow; }

private:
    Ui::CDlgQuestions *ui;

    CCurrentEdit    *_pcurrentLineEdit = 0;
    CDlgFullKeyboard *_pKeyboard = 0;

    int _id;
    int _nRow;
    QString _lockNum;
    
    void checkAndCreateCurrentLineEdit();
signals:
    
    void __OnQuestionsClose();
    void __OnQuestionsCancel();
    void __OnQuestionsSave(QString lockNum, QString question1, QString question2, QString question3);
    
    void OnClose();
    void __OnCodes(QString code1);

private slots:

    void on_edtAnswer1_clicked();
    void on_edtAnswer2_clicked();
    void on_edtAnswer3_clicked();

    void on_buttonBoxQuestions_accepted();

    void on_buttonBoxQuestions_rejected();

    void on_buttonBoxQuestions_clicked(QAbstractButton *button);

    void OnAdminInfoCodes(QString code1, QString code2);

    void on_clrAnswer1_clicked();
    void on_clrAnswer2_clicked();
    void on_clrAnswer3_clicked();

public slots:
    void onStartEditLine(QLineEdit *pLine, QString sLabelText);
    void OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);
};

#endif // DLGQUESTIONS_H
