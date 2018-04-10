#ifndef DLGQUESTIONS_H
#define DLGQUESTIONS_H

#include <QDialog>

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

    signals:

        void __OnQuestionsClose();
        void __OnQuestionsCancel();
        void __OnQuestionsSave(QString lockNum, QString question1, QString question2, QString question3);

    private slots:

        void on_edtAnswer1_clicked();
        void on_edtAnswer2_clicked();
        void on_edtAnswer3_clicked();

        void on_buttonBoxQuestions_accepted();
        void on_buttonBoxQuestions_rejected();

        void on_clrAnswer1_clicked();
        void on_clrAnswer2_clicked();
        void on_clrAnswer3_clicked();

    private:
        QString _lockNum;
        Ui::CDlgQuestions *ui;
        void RunKeyboard(QString& text);


};

#endif // DLGQUESTIONS_H
