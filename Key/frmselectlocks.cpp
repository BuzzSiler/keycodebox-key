#include "frmselectlocks.h"
#include "ui_frmselectlocks.h"
#include <QObject>
#include <QDebug>
#include <QTimer>
#include "selectlockswidget.h"
#include "kcbutils.h"
#include "kcbcommon.h"

#define COUNT_DOWN_TIMEOUT (1000)

CFrmSelectLocks::CFrmSelectLocks(QWidget *parent) :
    QDialog(parent),
    m_select_locks(* new SelectLocksWidget(this, SelectLocksWidget::USER)),
    m_timer(* new QTimer(this)),
    ui(new Ui::CFrmSelectLocks)
{
    ui->setupUi(this);
    CFrmSelectLocks::showFullScreen();

    ui->vloSelectLocks->addWidget(&m_select_locks);

    connect(&m_select_locks, &SelectLocksWidget::NotifyClose, this, &CFrmSelectLocks::reject);
    connect(&m_select_locks, &SelectLocksWidget::NotifyOpen, this, &CFrmSelectLocks::accept);

    connect(&m_timer, &QTimer::timeout, this, &CFrmSelectLocks::update);
    m_timer.setSingleShot(true);
}

CFrmSelectLocks::~CFrmSelectLocks()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void CFrmSelectLocks::setLocks(QString locks)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE(locks);
    ui->lblTimeRemaining->setText("30");
    m_select_locks.setLocks(locks);
    m_timer.start(COUNT_DOWN_TIMEOUT);
    KCB_DEBUG_EXIT;
}

QString CFrmSelectLocks::getLocks()
{
    return m_select_locks.getLocks();
}

void CFrmSelectLocks::update()
{
    int time_remaining = ui->lblTimeRemaining->text().toInt();

    if (time_remaining == 1)
    {
        ui->lblTimeRemaining->setStyleSheet("color: black");
        ui->lblTimeRemaining->setText(QString::number(30));
        reject();
    }

    // Set color to red for last 10 seconds
    if (time_remaining == 11)
    {
        ui->lblTimeRemaining->setStyleSheet("color: red");
    }

    ui->lblTimeRemaining->setText(QString::number(time_remaining - 1));
    m_timer.start(COUNT_DOWN_TIMEOUT);
}
