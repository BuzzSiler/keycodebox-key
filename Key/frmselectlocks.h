#ifndef FRMSELECTLOCKS_H
#define FRMSELECTLOCKS_H

#include <QDialog>

namespace Ui {
class CFrmSelectLocks;
}

class SelectLocksWidget;

class CFrmSelectLocks : public QDialog
{
    Q_OBJECT

    public:
        explicit CFrmSelectLocks(QWidget *parent = 0);
        ~CFrmSelectLocks();

        void setLocks(QString locks);
        QString getLocks();

//    public slots:
//        void OnNotifyRequestLockOpen(QString lock);
//        void OnNotifyLockOpenComplete();
    
    private:
        SelectLocksWidget& m_select_locks;
        QStringList m_selected_locks;
        Ui::CFrmSelectLocks *ui;
};

#endif // FRMSELECTLOCKS_H
