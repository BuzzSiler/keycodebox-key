#include "selectlockswidget.h"
#include "ui_selectlockswidget.h"
#include <QDebug>
#include <QList>
#include <QAbstractButton>
#include <QSignalMapper>
#include <QVector>
#include "lockcabinetwidget.h"

SelectLocksWidget::SelectLocksWidget(QWidget *parent, Role role, int num_cabs) :
    QWidget(parent),
    m_role(role),
    m_cancel_open(false),
    m_lock_cab(* new LockCabinetWidget(this, num_cabs)),
    ui(new Ui::SelectLocksWidget)
{
    ui->setupUi(this);

    ui->vloLockCabinet->addWidget(&m_lock_cab);

    connect(&m_lock_cab, SIGNAL(NotifyLockSelected(QString, bool)), this, SLOT(OnNotifyLockSelected(QString, bool)));

    
    if (m_role == ADMIN)
    {
    }
}

SelectLocksWidget::~SelectLocksWidget()
{
    delete ui;
}

void SelectLocksWidget::setLocks(QString locks)
{
    Q_ASSERT(m_role == USER);
    Q_ASSERT(locks != "");

    qDebug() << "Locks:" << locks;

    m_lock_cab.disableAllLocks();
    m_lock_cab.setEnabledLocks(locks);
    m_lock_cab.setSelectedLocks(locks);
    ui->btnOpenSelected->setEnabled(locks.length() > 0 ? true : false);
}

void SelectLocksWidget::createLockListStr(QString cab, QString lock, QString& str)
{
    str = QString(tr("Cabinet %1 - Lock %2")).arg(cab, 3, '0').arg(lock, 3, '0');
    qDebug() << "Cabinet:" << cab << "Lock:" << lock << "Lock List Str:" << str;
}

void SelectLocksWidget::addLockToList(QString lock)
{
    QString item_str;

    qDebug() << "Cab Index:" << m_lock_cab.getSelectedCabinet() << "Lock Index:" << lock;

    createLockListStr(m_lock_cab.getSelectedCabinet(), lock, item_str);

    ui->lstSelectedLocks->addItem(item_str);
    ui->lstSelectedLocks->sortItems();
}

void SelectLocksWidget::removeLockFromList(QString lock)
{
    QString item_str;
    QList<QListWidgetItem *> items;
    int row;

    qDebug() << "Cab Index:" << m_lock_cab.getSelectedCabinet() << "Lock Index:" << lock;

    createLockListStr(m_lock_cab.getSelectedCabinet(), lock, item_str);

    items = ui->lstSelectedLocks->findItems(item_str, Qt::MatchExactly);
    Q_ASSERT(items.length() > 0);
    if (items.length() > 0)
    {
        row = ui->lstSelectedLocks->row(items[0]);
        delete ui->lstSelectedLocks->item(row);
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
    // Cabinet xxx - Lock yyy
    //    0     1  2  3    4
    QStringList cab_lock = str.split(' ', QString::SkipEmptyParts);
    QVector<QString> cab_lock_vtr = cab_lock.toVector();
    cab = cab_lock_vtr[1];
    lock = cab_lock_vtr[4];

    qDebug() << "Item String:" << str << "Cabinet:" << cab << "Lock:" << lock;
}

void SelectLocksWidget::openDoorTimer()
{
    QString cab;
    QString lock;
    QString str;

    qDebug() << "Opening" << ui->lstSelectedLocks->item(0)->text();

    str = ui->lstSelectedLocks->item(0)->text();
    delete ui->lstSelectedLocks->item(0);
    getCabinetLockFromStr(str, cab, lock);

    m_lock_cab.setSelectedCabinet(cab);
    m_lock_cab.clrSelectedLocks(lock);
    emit NotifyRequestLockOpen(lock);

    if (m_cancel_open || ui->lstSelectedLocks->count() == 0)
    {
        if (m_role == ADMIN)
        {
            ui->btnOpenSelected->setEnabled(ui->lstSelectedLocks->count() > 0 ? true : false);
            ui->btnCancelOpen->setEnabled(false);
        }
        else if (m_role == USER)
        {
            // If we are in user role, we're inside a dialog.
            // Let the dialog know that we are done.
            emit NotifyLockOpenComplete();
        }
    }
    else
    {
        QTimer::singleShot(2000, this, SLOT(openDoorTimer()));
    }
}

void SelectLocksWidget::on_btnOpenSelected_clicked()
{
    m_cancel_open = false;
    ui->btnOpenSelected->setEnabled(false);
    ui->btnCancelOpen->setEnabled(true);
    openDoorTimer();
}

void SelectLocksWidget::on_btnCancelOpen_clicked()
{
    m_cancel_open = true;
}
