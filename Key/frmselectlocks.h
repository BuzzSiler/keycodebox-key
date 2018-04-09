#ifndef FRMSELECTLOCKS_H
#define FRMSELECTLOCKS_H

#include <QDialog>

namespace Ui {
class CFrmSelectLocks;
}

class QTimer;
class SelectLocksWidget;

class CFrmSelectLocks : public QDialog
{
    Q_OBJECT

    public:
        explicit CFrmSelectLocks(QWidget *parent = 0);
        ~CFrmSelectLocks();

        void setLocks(QString locks);
        QString getLocks();

    private:
        SelectLocksWidget& m_select_locks;
        QStringList m_selected_locks;
        QTimer& m_timer;

        Ui::CFrmSelectLocks *ui;

        void update();
};

#endif // FRMSELECTLOCKS_H
