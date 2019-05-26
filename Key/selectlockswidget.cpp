#include "selectlockswidget.h"
#include "ui_selectlockswidget.h"
#include <QDebug>
#include <QList>
#include <QAbstractButton>
#include <QSignalMapper>
#include <QVector>
#include <QTimer>
#include "lockcabinetwidget.h"
#include "kcbutils.h"
#include "kcbcommon.h"

static int const TIMEOUT_PERIOD = 1000;

SelectLocksWidget::SelectLocksWidget(QWidget *parent, Role role) :
    QWidget(parent),
    m_role(role),
    m_cancel_open(false),
    m_lock_cab(* new LockCabinetWidget(this)),
    m_lock_list{},
    m_timeout(0),
    m_timer(* new QTimer(this)),
    ui(new Ui::SelectLocksWidget)
{
    ui->setupUi(this);

    ui->lblTimeRemaining->setVisible(false);
    ui->lblTimeTitle->setVisible(false);

    ui->vloLockCabinet->addWidget(&m_lock_cab);

    connect(&m_lock_cab, SIGNAL(NotifyLockSelected(QString, bool)), this, SLOT(OnNotifyLockSelected(QString, bool)));

    if (m_role == USER)
    {
        ui->btnCancelOpen->setEnabled(true);
    }

    connect(&m_timer, &QTimer::timeout, this, &SelectLocksWidget::update);
    m_timer.setSingleShot(true);

    if (KeyCodeBoxSettings::IsLockSelectionSingle())
    {
        m_lock_cab.OnNotifySingleLockSelection();
        ui->gbSelectedLocks->setVisible(false);
    }
    else
    {
        m_lock_cab.OnNotifyMultiLockSelection();
        ui->gbSelectedLocks->setVisible(true);
    }
}

SelectLocksWidget::~SelectLocksWidget()
{
    kcb::ClassAllocation::DestructorMsg(this);
    delete ui;
}

void SelectLocksWidget::updateCabinetConfig()
{
    KCB_DEBUG_ENTRY;
    m_lock_cab.updateCabinetConfig();
    KCB_DEBUG_EXIT;
}

void SelectLocksWidget::setLocks(QString locks)
{
    KCB_DEBUG_ENTRY;
    Q_ASSERT(m_role == USER);
    Q_ASSERT(locks != "");

    if (!m_lock_cab.isConfigured())
    {
        KCB_DEBUG_TRACE("Lock Cabinet is not configured");
        return;
    }

    KCB_DEBUG_TRACE("Locks:" << locks);

    m_lock_cab.disableAllLocks();
    m_lock_cab.setEnabledLocks(locks);

    KCB_DEBUG_TRACE("after disable/enable locks");

    ui->lstSelectedLocks->clear();
    ui->btnOpenSelected->setEnabled(false);
    KCB_DEBUG_EXIT;
}

QString SelectLocksWidget::getLocks()
{
    KCB_DEBUG_ENTRY;
    if (!m_lock_cab.isConfigured())
    {
        KCB_DEBUG_TRACE("Lock Cabinet is not configured");
        return QString("");
    }

    QStringList locks;
    QString lock;
    QString str;
    QString cab;

    int num_items = ui->lstSelectedLocks->count();

    for (int ii = 0; ii < num_items; ++ii)
    {
        str = ui->lstSelectedLocks->item(ii)->text();
        getCabinetLockFromStr(str, cab, lock);
        locks.append(QString::number(lock.toInt()));
    }

    KCB_DEBUG_EXIT;
    return locks.join(",");
}

void SelectLocksWidget::setTimer(int seconds)
{
    m_timeout = seconds;
    ui->lblTimeRemaining->setVisible(true);
    ui->lblTimeTitle->setVisible(true);
    ui->lblTimeRemaining->setText(QString::number(seconds));
}

void SelectLocksWidget::startTimer()
{
    if (m_timeout > 0)
    {
        m_timer.start(TIMEOUT_PERIOD);
    }
}

void SelectLocksWidget::createLockListStr(QString cab, QString lock, QString& str)
{
    str = QString(tr("%1 - Lock %2")).arg(cab, 3, '0').arg(lock, 3, '0');
}

void SelectLocksWidget::addLockToList(QString lock)
{
    QString item_str;

    createLockListStr(m_lock_cab.getSelectedCabinet(), lock, item_str);

    m_lock_list.append(item_str);
    m_lock_list.removeDuplicates();
    m_lock_list.sort();

    ui->lstSelectedLocks->clear();
    ui->lstSelectedLocks->addItems(m_lock_list);
}

void SelectLocksWidget::addLocksToList(QString locks)
{
    if (locks.contains(","))
    {
        QStringList sl = locks.split(",");

        foreach (auto s, sl)
        {
            addLockToList(s);
        }
    }
    else
    {
        addLockToList(locks);
    }
}

void SelectLocksWidget::removeLockFromList(QString lock)
{
    QString item_str;

    createLockListStr(m_lock_cab.getSelectedCabinet(), lock, item_str);

    int index = m_lock_list.indexOf(item_str);
    if (index >= 0)
    {
        m_lock_list.removeAt(index);
        m_lock_list.sort();

        ui->lstSelectedLocks->clear();
        ui->lstSelectedLocks->addItems(m_lock_list);
    }
}

void SelectLocksWidget::OnNotifyLockSelected(QString lock, bool is_selected)
{
    if (is_selected)
    {
        addLockToList(lock);
    }
    else
    {
        removeLockFromList(lock);
    }

    ui->btnOpenSelected->setEnabled(ui->lstSelectedLocks->count() > 0 ? true : false);
}

void SelectLocksWidget::getCabinetLockFromStr(QString& str, QString& cab, QString& lock)
{
    // mmmm - Lock yyy
    //  0   1  2    3
    QStringList cab_lock = str.split(' ', QString::SkipEmptyParts);
    QVector<QString> cab_lock_vtr = cab_lock.toVector();
    cab = cab_lock_vtr[0];
    lock = QString::number(cab_lock_vtr[3].toInt());
}

void SelectLocksWidget::openDoorTimer()
{
    QString cab;
    QString lock;
    QString str;

    str = ui->lstSelectedLocks->item(0)->text();
    delete ui->lstSelectedLocks->item(0);
    getCabinetLockFromStr(str, cab, lock);

    m_lock_cab.setSelectedCabinet(cab, lock);
    m_lock_cab.clrSelectedLocks(lock);
    emit NotifyRequestLockOpen(QString::number(lock.toInt()), false);

    int lock_index = m_lock_list.indexOf(str);
    if (lock_index > -1)
    {
        m_lock_list.removeAt(lock_index);
    }  

    if (m_cancel_open || ui->lstSelectedLocks->count() == 0)
    {
        ui->btnOpenSelected->setEnabled(ui->lstSelectedLocks->count() > 0 ? true : false);
        ui->btnCancelOpen->setEnabled(false);
    }
    else
    {
        QTimer::singleShot(500, this, SLOT(openDoorTimer()));
    }
}


void SelectLocksWidget::on_btnCancelOpen_clicked()
{
    if (m_role == ADMIN)
    {
        m_cancel_open = true;
    }
    else // m_role == USER
    {
        emit NotifyClose();
    }
}

void SelectLocksWidget::on_btnOpenSelected_clicked()
{
    if (m_role == ADMIN)
    {
        m_cancel_open = false;
        ui->btnOpenSelected->setEnabled(false);
        ui->btnCancelOpen->setEnabled(true);
        openDoorTimer();
    }
    else // m_role == USER
    {
        emit NotifyOpen();
    }
}

void SelectLocksWidget::update()
{
    if (m_timeout == 0)
    {
        ui->lblTimeRemaining->setStyleSheet("color: black");
        //ui->lblTimeRemaining->setText(QString::number(30));
        emit NotifyTimeout();
    }

    // Set color to red for last 10 seconds
    if (m_timeout == 11)
    {
        ui->lblTimeRemaining->setStyleSheet("color: red");
    }

    if (m_timeout > 0)
    {
        m_timeout = m_timeout - 1;
        ui->lblTimeRemaining->setText(QString::number(m_timeout));
        m_timer.start(TIMEOUT_PERIOD);
    }
}

void SelectLocksWidget::OnLockSelectionChanged()
{
    KCB_DEBUG_ENTRY;
    if (KeyCodeBoxSettings::IsLockSelectionSingle())
    {
        KCB_DEBUG_TRACE("setting for single selection");
        m_lock_cab.OnNotifySingleLockSelection();
        ui->gbSelectedLocks->setVisible(false);
    }
    else if (KeyCodeBoxSettings::IsLockSelectionMulti())
    {
        KCB_DEBUG_TRACE("setting for multi selection");
        m_lock_cab.OnNotifyMultiLockSelection();
        ui->gbSelectedLocks->setVisible(true);
    }
    else
    {
        KCB_DEBUG_TRACE("setting for disable selection");
        m_lock_cab.OnNotifyDisableLockSelection();
        ui->gbSelectedLocks->setVisible(false);
    }
    KCB_DEBUG_EXIT;
}