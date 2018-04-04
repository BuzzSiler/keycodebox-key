#ifndef FRMCODEEDITMULTI_H
#define FRMCODEEDITMULTI_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
    class FrmCodeEditMulti;
}

class QSignalMapper;
class QPushButton;
class QTouchEvent;
class LockCabinetWidget;
class CLockState;
class CClickableLineEdit;

class FrmCodeEditMulti : public QDialog
{
        Q_OBJECT

    public:
        explicit FrmCodeEditMulti(QWidget *parent = 0);
        ~FrmCodeEditMulti();

        void setValues(CLockState * const state, const QStringList codes_in_use);
        void getValues(CLockState * const state);        
                       
    private slots:
        void on_pbClearCode1_clicked();
        void on_pbClearCode2_clicked();
        void on_pbClearUsername_clicked();
        void on_cbAccessType_currentIndexChanged(int index);
        void on_dtStartAccess_dateTimeChanged(const QDateTime &dateTime);
        void on_dtEndAccess_dateTimeChanged(const QDateTime &dateTime);
        void on_edCode1_clicked();
        void on_edCode2_clicked();
        void on_edUsername_clicked();
        void on_cbEnableCode2_stateChanged(int arg1);

        void OnNotifyLockSelected(QString lock, bool is_selected);

        void on_bbSaveCancel_accepted();

        void on_bbSaveCancel_rejected();

        void on_cbEnableQuestions_stateChanged(int arg1);

        void on_pbEditQuestions_clicked();

//        void OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *currEdit);

        void on_cbFingerprint_clicked();

    private:
        typedef struct _tag_code_state
        {
            QString code1;
            QString code2;
            bool    code2_enabled;
            QString username;
            QDateTime start_datetime;
            QDateTime end_datetime;
            bool questions_enabled;
            QString question1;
            QString question2;
            QString question3;
            bool fp_enabled;
            quint8 access_type;
            QString locks;

        } CODE_STATE;

        QVector<QString> m_questions;
        LockCabinetWidget& m_lock_cab;
        CClickableLineEdit *m_p_line_edit;
        CODE_STATE m_code_state;
        QStringList m_codes_in_use;
        Ui::FrmCodeEditMulti *ui;

        void updateAccessType(int index);
        void updateUi();
        bool isModified();
        void clrCodeState();
        void resetQuestions();
        void displayWarning(QWidget *p_widget, bool is_valid);


};

#endif // FRMCODEEDITMULTI_H
