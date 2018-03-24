#include "frmselectlocks.h"
#include "ui_frmselectlocks.h"
#include <QObject>
#include <QDebug>
#include "lockcabinetwidget.h"


CFrmSelectLocks::CFrmSelectLocks(QWidget *parent) :
    QDialog(parent),
    m_lock_cab(* new LockCabinetWidget(this)),
    ui(new Ui::CFrmSelectLocks)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->vloLockCabinet->addWidget(&m_lock_cab);

    // Connect to the lock cabinet widget to get the selected
    // locks.

    //connect(&m_selectLocks, SIGNAL(NotifyRequestLockOpen(QString)), this, SLOT(OnNotifyRequestLockOpen(QString)));
    //connect(&m_selectLocks, SIGNAL(NotifyLockOpenComplete()), this, SLOT(OnNotifyLockOpenComplete()));
}

CFrmSelectLocks::~CFrmSelectLocks()
{
    delete ui;
}

void CFrmSelectLocks::setLocks(QString locks)
{
    //m_selectLocks.setLocks(locks);
    m_selected_locks.clear();
}

QString CFrmSelectLocks::getLocks()
{
    return m_selected_locks.join(',');
}

void CFrmSelectLocks::OnNotifyRequestLockOpen(QString locks)
{
    // emit signal to let system control know that the lock(s) have been opened
    m_selected_locks.append(locks);
    emit NotifyOpenLockRequest(locks, false);
}

void CFrmSelectLocks::on_pbClose_clicked()
{
    done(QDialog::Rejected);
}

void CFrmSelectLocks::OnNotifyLockOpenComplete()
{
    // Open is complete, return Accepted
    done(QDialog::Accepted);
}
