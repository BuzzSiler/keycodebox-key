#include "frmselectlocks.h"
#include "ui_frmselectlocks.h"
#include <QObject>
#include <QDebug>
#include "selectlockswidget.h"


CFrmSelectLocks::CFrmSelectLocks(QWidget *parent) :
    QDialog(parent),
    m_select_locks(* new SelectLocksWidget(this, SelectLocksWidget::USER)),
    ui(new Ui::CFrmSelectLocks)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->vloSelectLocks->addWidget(&m_select_locks);

    connect(&m_select_locks, &SelectLocksWidget::NotifyClose, this, &CFrmSelectLocks::reject);
    connect(&m_select_locks, &SelectLocksWidget::NotifyOpen, this, &CFrmSelectLocks::accept);
}

CFrmSelectLocks::~CFrmSelectLocks()
{
    delete ui;
}

void CFrmSelectLocks::setLocks(QString locks)
{
    m_select_locks.setLocks(locks);
}

QString CFrmSelectLocks::getLocks()
{
    return m_select_locks.getLocks();
}
