#include "frmselectlocks.h"
#include "ui_frmselectlocks.h"
#include <QObject>
#include <QDebug>
#include <QTimer>
#include "selectlockswidget.h"
#include "kcbutils.h"


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
}

CFrmSelectLocks::~CFrmSelectLocks()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void CFrmSelectLocks::setLocks(QString locks)
{
    m_select_locks.setLocks(locks);
    m_timer.start(1000);
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
        reject();
    }

    // If time remaining is less than 11 change color to red
    if (time_remaining == 11)
    {
        QPalette palette;
        palette.setColor(QPalette::Window, Qt::red);
        palette.setColor(QPalette::WindowText, Qt::red);
        setAutoFillBackground(true);
        ui->lblTimeRemaining->setPalette(palette);
    }

    ui->lblTimeRemaining->setText(QString::number(time_remaining - 1));
    m_timer.start(1000);

}
