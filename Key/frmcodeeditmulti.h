#ifndef FRMCODEEDITMULTI_H
#define FRMCODEEDITMULTI_H

#include <QDialog>

namespace Ui {
    class FrmCodeEditMulti;
}

class QSignalMapper;
class QPushButton;
class QTouchEvent;
class LockCabinetWidget;
class CLockState;
class DlgNumberPad;
class CClickableLineEdit;

class FrmCodeEditMulti : public QDialog
{
        Q_OBJECT

    public:
        explicit FrmCodeEditMulti(QWidget *parent = 0);
        ~FrmCodeEditMulti();

        void setValues(CLockState * const state);
        void getValues(CLockState * const state);
                       
    private slots:
        void on_pbClearCode1_clicked();
        void on_pbClearCode2_clicked();
        void on_pbClearUsername_clicked();
        void on_pbQuestions_clicked();
        void on_cbAccessType_currentIndexChanged(int index);
        void on_dtStartAccess_dateTimeChanged(const QDateTime &dateTime);
        void on_dtEndAccess_dateTimeChanged(const QDateTime &dateTime);
        void on_edCode1_clicked();
        void on_edCode2_clicked();
        void on_edUsername_clicked();
        void on_cbEnableCode2_stateChanged(int arg1);

        void OnAccepted();
        void OnRejected();
        void OnNotifyLockSelected(QString lock, bool is_selected);

    private:
        QVector<QString> m_questions;
        int m_access_type;
        LockCabinetWidget& m_lock_cab;
        DlgNumberPad& m_num_pad;
        CClickableLineEdit *m_p_line_edit;
        Ui::FrmCodeEditMulti *ui;

        void updateAccessType(int index);
        void updateExitState();
        void updateUi();
};

#endif // FRMCODEEDITMULTI_H
