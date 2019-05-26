#ifndef FRMCODEEDITMULTI_H
#define FRMCODEEDITMULTI_H

#include <QDialog>
#include <QDateTime>
#include <QLabel>

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
        void updateCabinetConfig();
        
    signals:
        void __OnAdminInfoCodes(QString,QString);

    public slots:
        void OnLockSelectionChanged();

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
        void on_cbEnableCode2_stateChanged(int state);
        void on_bbSaveCancel_accepted();
        void on_bbSaveCancel_rejected();
        void on_cbEnableQuestions_stateChanged(int state);
        void on_pbEditQuestions_clicked();
        void on_cbFingerprint_clicked();

        void OnNotifyLockSelected(QString lock, bool is_selected);

        void on_pbCode1Random_clicked();

        void on_pbCode2Random_clicked();

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
            bool fp_deleted;
            quint8 access_type;
            QString locks;
        } CODE_STATE;

        QVector<QString> m_questions;
        LockCabinetWidget& m_lock_cab;
        CClickableLineEdit *m_p_line_edit;
        CODE_STATE m_code_state;
        QStringList m_codes_in_use;
        bool m_initialized;
        Ui::FrmCodeEditMulti *ui;
        QLabel& m_access_type_label;
        QLabel& m_access_state_label;

        void updateAccessType(int index);
        void updateUi();
        bool isModified();
        void clrCodeState();
        void resetQuestions();
        void disableCode2();
        void disableQuestions();



};

#endif // FRMCODEEDITMULTI_H
