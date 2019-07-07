#ifndef SELECTLOCKSWIDGET_H
#define SELECTLOCKSWIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
    class SelectLocksWidget;
}

class QSignalMapper;
class QPushButton;
class LockCabinetWidget;
class QStringList;

class SelectLocksWidget : public QWidget
{
    Q_OBJECT


    public:
        typedef enum {ADMIN=1, USER=2} Role;

        explicit SelectLocksWidget(QWidget *parent, Role is_admin);
        ~SelectLocksWidget();

        void setLocks(QString locks);
        QString getLocks();
        void setTimer(int seconds);
        void startTimer();
        void updateCabinetConfig();

    signals:
        void NotifyRequestLockOpen(QString lockNum, bool takePicture);

        void NotifyClose();
        void NotifyOpen();
        void NotifyTimeout();

    private slots:
        void openDoorTimer();
        void OnNotifyLockSelected(QString lock, bool is_selected);
        void on_btnOpenSelected_clicked();
        void on_btnCancelOpen_clicked();

    private:
        /* SelectLocksWidget has two roles: 'Admin' and 'User'

        Admin Mode
           When in 'Admin' role, one or more locks are marked to be opened
           Locks are grouped into banks of 32.  The banks select list makes active the specified change of locks, 
           e.g., bank 1 => locks 1 .. 32, bank 2 => locks 33-64, etc.
           The selected locks list is a list of the locks to be opened.  Each entry in the list will include,
           bank # and lock #, e.g.,
                Bank 1, Lock 1
                Bank 2, Lock 17
                Bank 4, Lock 3
                etc.
            The array of 32 locks are toggle buttons.  When pressed, the lock and its bank will be added to the 
            selected list.  When unpressed, the lock and its bank will removed from the selected list.

            The select/clear all button when pressed will depress all of the lock buttons and add them to the selected list.
            The text of the select/clear all button will change to clear all.
            When the select/clear all button is unpressed, all of the lock buttons will be unpressed and the entire bank will
            be cleared from the selected list, and the text will change to select all.

            The open button will issue the open command for each lock in the selected list

            Selecting a different bank will update the array of locks for that bank.

        User Mode
            When in 'User' role, a list of authorized locks will be presented.
            The selected list will show all of the locks that user is authorized to open including bank # and lock #
            The array of locks will be disabled except for the locks that are authorized.  The locks that are 
            authorized will be depressed.
            The select/clear all button will be depressed and will display clear all.
            Clicking the select/clear all button will add/remove the selected locks from list and press/unpress the lock buttons.
            The text will change as appropriate.
            Selecting a different bank will update the array of locks

            Need to pass a list of locks
            From the list of locks we populate the available locks list and the available banks list
            Should we do this via the constructor or via a setter?
            Should we lay the widget on top of a separate dialog?

            What is the scenario?
                - User enters authorization codes
                - User is presented with a visualization of the available locks
                - User selects the appropriate locks and presses open

                - What happens if the user does not make a selection?
                    - Need timer to close dialog
                    - How long to wait?
                        - Multiple of the number of banks?
                    - Maybe add a timeout to the dialog that show how long before closing?


        */
        Role m_role;
        bool m_cancel_open;
        LockCabinetWidget& m_lock_cab;
        QStringList m_lock_list;
        int m_timeout;
        QTimer& m_timer;
        Ui::SelectLocksWidget *ui;

        void getCabinetLockFromStr(QString& str, QString& cab, QString& lock);
        void createLockListStr(QString cab, QString lock, QString& str);
        void addLockToList(QString lock);
        void addLocksToList(QString locks);
        void removeLockFromList(QString lock);
        void update();
};

#endif // SELECTLOCKSWIDGET_H
