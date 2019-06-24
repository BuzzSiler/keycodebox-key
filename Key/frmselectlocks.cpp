#include "frmselectlocks.h"
#include "ui_frmselectlocks.h"

#include <QObject>
#include <QDebug>

#include "selectlockswidget.h"
#include "kcbutils.h"
#include "kcbcommon.h"
#include "kcbsystem.h"

CFrmSelectLocks::CFrmSelectLocks(QWidget *parent) :
    QDialog(parent),
    m_select_locks(* new SelectLocksWidget(this, SelectLocksWidget::USER)),
    ui(new Ui::CFrmSelectLocks)
{
    // KCB_DEBUG_ENTRY;
    ui->setupUi(this);

    kcb::SetWindowParams(this);

    m_select_locks.updateCabinetConfig();
    ui->vloSelectLocks->addWidget(&m_select_locks);

    connect(&m_select_locks, &SelectLocksWidget::NotifyClose, this, &CFrmSelectLocks::reject);
    connect(&m_select_locks, &SelectLocksWidget::NotifyOpen, this, &CFrmSelectLocks::accept);
    connect(&m_select_locks, &SelectLocksWidget::NotifyTimeout, this, &CFrmSelectLocks::reject);
    // KCB_DEBUG_EXIT;
}

CFrmSelectLocks::~CFrmSelectLocks()
{
    kcb::ClassAllocation::DestructorMsg(this);
    delete ui;
}

void CFrmSelectLocks::setLocks(QString locks)
{
    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE(locks);
    m_select_locks.setTimer(30);
    m_select_locks.setLocks(locks);
    m_select_locks.startTimer();
    // KCB_DEBUG_EXIT;
}

QString CFrmSelectLocks::getLocks()
{
    return m_select_locks.getLocks();
}