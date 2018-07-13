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

        void getValues(QString& question1, QString& question2, QString& question3);
        void setValues(QString lockNum, QString question1, QString question2, QString question3);

    signals:

        void __OnQuestionsClose();
        void __OnQuestionsCancel();
        void __OnQuestionsSave(QString lockNum, QString question1, QString question2, QString question3);

    private slots:

        void on_edtAnswer1_clicked();
        void on_edtAnswer2_clicked();
        void on_edtAnswer3_clicked();

        void on_clrAnswer1_clicked();
        void on_clrAnswer2_clicked();
        void on_clrAnswer3_clicked();

        void on_bbOkCancel_accepted();

        void on_bbOkCancel_rejected();

        void on_rbAnswer1Yes_clicked();
        void on_rbAnswer1No_clicked();
        void on_rbAnswer2Yes_clicked();
        void on_rbAnswer2No_clicked();
        void on_rbAnswer3Yes_clicked();
        void on_rbAnswer3No_clicked();

    private:
        Ui::CDlgQuestions *ui;
        QString _lockNum;
        QString _answer1;
        QString _answer2;
        QString _answer3;
#ifdef ENABLE_FLEETWAVE_INTERFACE
        bool _chevin_enabled{true};
#else
        bool _chevin_enabled{false};
#endif

        void RunKeyboard(QString& text);
        void enableOk();

};

#endif // DLGQUESTIONS_H
