#ifndef FRMSELECTLOCKS_H
#define FRMSELECTLOCKS_H

#include <QDialog>

namespace Ui {
class CFrmSelectLocks;
}

class LockCabinetWidget;

class CFrmSelectLocks : public QDialog
{
    Q_OBJECT

public:
    explicit CFrmSelectLocks(QWidget *parent = 0);
    ~CFrmSelectLocks();

    void setLocks(QString locks);
    QString getLocks();

    signals:
    void NotifyOpenLockRequest(QString locks, bool is_user);

    public slots:
    void OnNotifyRequestLockOpen(QString lock);
    void OnNotifyLockOpenComplete();
    
    private slots:
    void on_pbClose_clicked();

    private:
    LockCabinetWidget& m_lock_cab;
    QStringList m_selected_locks;
    Ui::CFrmSelectLocks *ui;
};

#endif // FRMSELECTLOCKS_H
