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

        void on_bbSaveCancel_accepted();

        void on_bbSaveCancel_rejected();
        
        void on_edCode1_cursorPositionChanged(int arg1, int arg2);

    private:
        QVector<QString> m_questions;
        int m_access_type;
        LockCabinetWidget& m_lock_cab;
        Ui::FrmCodeEditMulti *ui;

        void updateAccessType(int index);

    protected:
        void touchEvent(QTouchEvent *ev);
        bool eventFilter(QObject *target, QEvent *event);
};

#endif // FRMCODEEDITMULTI_H
