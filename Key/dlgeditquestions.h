#ifndef DLGEDITQUESTIONS_H
#define DLGEDITQUESTIONS_H

#include <QDialog>

namespace Ui {
class CDlgEditQuestions;
}

class CClickableLineEdit;

class CDlgEditQuestions : public QDialog
{
    Q_OBJECT

    public:
        explicit CDlgEditQuestions(QWidget *parent = 0);
        ~CDlgEditQuestions();

        void getValues(QVector<QString>& questions);
        void setValues(QVector<QString>& questions);





    private slots:

        void on_edtQuestion1_clicked();
        void on_edtQuestion2_clicked();
        void on_edtQuestion3_clicked();

        void on_buttonBoxQuestions_accepted();
        void on_buttonBoxQuestions_rejected();

        void on_clrQuestion1_clicked();
        void on_clrQuestion2_clicked();
        void on_clrQuestion3_clicked();

    private:
        Ui::CDlgEditQuestions *ui;

        void showKeyboard(CClickableLineEdit *p_lineEdit);
};

#endif // DLGEDITQUESTIONS_H
