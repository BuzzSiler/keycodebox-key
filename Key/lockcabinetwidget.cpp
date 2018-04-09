#include "lockcabinetwidget.h"
#include "ui_lockcabinetwidget.h"

#include <QtGlobal>
#include <QDebug>
#include <QSignalMapper>
#include "kcbutils.h"
#include "kcbcommon.h"

LockCabinetWidget::LockCabinetWidget(QWidget *parent, quint8 num_cabs, quint8 locks_per_cab) :
    QWidget(parent),
    m_num_cabs(num_cabs),
    m_locks_per_cab(locks_per_cab),
    m_cabs(num_cabs),
    m_current_cab(0),
    m_mapper(* new QSignalMapper(this)),
    ui(new Ui::LockCabinetWidget)
{
    ui->setupUi(this);

    // Get a static list of the lock buttons on the widget
    m_lock_buttons = this->findChildren<QPushButton *>(QRegularExpression("pbLock.."), Qt::FindChildrenRecursively);

    // Map the click signal from the buttons to lockSelected slot via a mapper
    for (int ii = 0; ii < m_lock_buttons.length(); ++ii)
    {
        QPushButton *btn = m_lock_buttons[ii];
        connect(btn, SIGNAL(clicked()), &m_mapper, SLOT(map()));
        m_mapper.setMapping(btn, ii);
    }
    connect(&m_mapper, SIGNAL(mapped(int)), this, SLOT(lockSelected(int)));

    // Initialize the cabs/locks state
    int start = 1;
    for (int ii = 0; ii < m_cabs.length(); ++ii)
    {
        quint8 cab_num = ii + 1;
        quint8 stop = start + m_locks_per_cab - 1;

        m_cabs[ii].start = start;
        m_cabs[ii].states = QVector<bool>(m_lock_buttons.length(), false);
        m_cabs[ii].enabled = QVector<bool>(m_lock_buttons.length(), true);

        // Populate the cabinets dropdown
        ui->cbSelectedCabinet->addItem(
            QString("%1 - %2 (%4 .. %5)").
                    arg("Cabinet").
                    arg(cab_num).
                    arg(start, 3).
                    arg(stop, 3));

        start += m_locks_per_cab;
    };

    m_default_palette = m_lock_buttons[0]->palette();

}

LockCabinetWidget::~LockCabinetWidget()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

void LockCabinetWidget::StringToVector(QString str, QVector<int>& vtr)
{
    if (str.contains(','))
    {
        QStringList list = str.split(',');

        foreach (auto item, list)
        {
            vtr.append(item.toInt());
        }
    }
    else
    {
        if (str.length() >= 1)
        {
            vtr.append(str.toInt());
        }
    }
}

void LockCabinetWidget::VectorToString(QVector<int> vtr, QString& str)
{
    Q_ASSERT(vtr.count() >= 0);
    str = "";
    if (vtr.count() > 1)
    {
        QStringList list;
        foreach (auto item, vtr)
        {
            list << QString::number(item);
        }
        str = list.join(',');
    }
    else if (vtr.count() == 1)
    {
        str = QString::number(vtr[0]);
    }
}

void LockCabinetWidget::selectClearAllLocks(bool select_clear)
{
    m_selected_locks.clear();
    CAB_STATE* p_cab = &m_cabs[m_current_cab];
    for (int ii = 0; ii < p_cab->states.count(); ++ii)
    {
        if (p_cab->enabled[ii])
        {
            bool changed = p_cab->states[ii] != select_clear;
            p_cab->states[ii] = select_clear;
            m_lock_buttons[ii]->setChecked(select_clear);
            if (changed)
            {
                if (select_clear)
                {
                    m_selected_locks.append(ii + 1);
                }
                emit NotifyLockSelected(QString::number(ii + 1), select_clear);
            }
        }
    }

    updateUi();
}

void LockCabinetWidget::enableDisableLocksInCabinet(quint8 cab_index, bool enable_disable)
{
    CAB_STATE* p_cab = &m_cabs[cab_index];

    for (int ii = 0; ii < p_cab->enabled.count(); ++ii)
    {
        p_cab->enabled[ii] = enable_disable;
        m_lock_buttons[ii]->setEnabled(enable_disable);
    }

    updateUi();
}

/* Get selected locks and cabinet */
QString LockCabinetWidget::getSelectedCabinet()
{
    return QString::number(m_current_cab + 1);
}

QString LockCabinetWidget::getSelectedLocks()
{
    QString str;

    VectorToString(m_selected_locks, str);

    return str;
}

/* Set selected locks and cabinet */
void LockCabinetWidget::setSelectedCabinet(const QString& cab)
{
    m_current_cab = cab.toInt() - 1;
    Q_ASSERT_X(m_current_cab >= 0 && m_current_cab < m_cabs.count(),
               "LockCabinetWidget::setSelectedCabinet",
               "m_current_cab out of range");
    updateUi();
}

void LockCabinetWidget::setSelectedLocks(QString locks)
{
    QVector<int> locks_int;

    qDebug() << "setSelectedLocks:" << locks;

    m_selected_locks.clear();
    clrAllLocks();
    StringToVector(locks, locks_int);

    foreach (auto lock, locks_int)
    {
        quint8 lock_index = (lock - 1) % m_locks_per_cab;
        quint8 cab_index = lock / m_locks_per_cab;
        Q_ASSERT_X(cab_index >= 0 && cab_index < m_num_cabs,
                   "LockCabinetWidget::setSelectedLocks",
                   "cab_index out of range");
        KCB_DEBUG_TRACE("Lock" << lock << "checked");                   
        m_cabs[cab_index].states[lock_index] = true;
        m_selected_locks.append(lock);
    }

    KCB_DEBUG_TRACE(m_selected_locks);
    updateUi();
}

/* Clear selected and all locks */
void LockCabinetWidget::clrLocksInCabinet(quint8 cab_index)
{
    Q_ASSERT_X(cab_index >= 0 && cab_index < m_num_cabs,
               "LockCabinetWidget::clrLocksInCabinet",
               "cab_index out of range");
    CAB_STATE* p_cab = &m_cabs[cab_index];
    for (int ii = 0; ii < p_cab->states.count(); ++ii)
    {
        p_cab->states[ii] = false;
    }
}

void LockCabinetWidget::clrAllLocks()
{
    for (int ii = 0; ii < m_cabs.count(); ++ii)
    {
        clrLocksInCabinet(ii);
    }

    updateUi();
}

void LockCabinetWidget::clrSelectedLocks(const QString& lock)
{
    quint8 lock_index = lock.toInt() - 1;
    Q_ASSERT_X(lock_index >= 0 && lock_index <= m_locks_per_cab,
               "LockCabinetWidget::clrSelectedLocks",
               "lock_index out of range");
    Q_ASSERT_X(m_current_cab >= 0 && m_current_cab < m_cabs.count(),
             "LockCabinetWidget::clrSelectedLocks",
             "m_current_cab out of range");
    m_cabs[m_current_cab].states[lock_index] = false;
    updateUi();
}

void LockCabinetWidget::setWarning()
{
    foreach (auto button, m_lock_buttons)
    {
        QPalette pal = button->palette();
        pal.setColor(QPalette::Button, QColor(Qt::red));
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
    }

}

void LockCabinetWidget::clrWarning()
{
    foreach (auto button, m_lock_buttons)
    {
        button->setPalette(m_default_palette);
    }
}

/* Set enabled locks */
void LockCabinetWidget::setEnabledLocks(QString locks)
{
    QVector<int> locks_int;

    StringToVector(locks, locks_int);

    foreach (auto lock, locks_int)
    {
        quint8 lock_index = (lock - 1) % m_locks_per_cab;
        quint8 cab_index = lock / m_locks_per_cab;
        Q_ASSERT_X(cab_index >= 0 && cab_index < m_num_cabs,
                   "LockCabinetWidget::setEnabledLocks",
                   "cab_index out of range");
        m_cabs[cab_index].enabled[lock_index] = true;
    }

    updateUi();
}

void LockCabinetWidget::enableAllLocks()
{
    for (int ii = 0; ii < m_cabs.count(); ++ii)
    {
        enableDisableLocksInCabinet(ii, true);
    }
}

void LockCabinetWidget::disableAllLocks()
{
    for (int ii = 0; ii < m_cabs.count(); ++ii)
    {
        enableDisableLocksInCabinet(ii, false);
    }
}

void LockCabinetWidget::on_pbSelectAll_clicked()
{
    selectClearAllLocks(true);
}

void LockCabinetWidget::on_pbClearAll_clicked()
{
    selectClearAllLocks(false);
}

void LockCabinetWidget::on_cbSelectedCabinet_currentIndexChanged(int index)
{
    Q_ASSERT_X(index >= 0 && index < m_num_cabs,
               "LockCabinetWidget::on_cbSelectedCabinet_currentIndexChanged",
               "index out of range");
    m_current_cab = index;
    updateUi();
}

void LockCabinetWidget::lockSelected(int lock_index)
{
    Q_ASSERT_X(lock_index >= 0 && lock_index < m_locks_per_cab,
               "LockCabinetWidget::lockSelected",
               "lock_index out of range");
    bool checked = m_lock_buttons[lock_index]->isChecked();
    quint8 cab_index = lock_index / m_locks_per_cab;
    m_cabs[cab_index].states[lock_index] = checked;
    quint8 lock = lock_index + 1;

    int index = m_selected_locks.indexOf(lock);

    if (checked)
    {
        Q_ASSERT(index == -1);
        m_selected_locks.append(lock);
    }
    else
    {
        Q_ASSERT(index >= 0);
        m_selected_locks.removeAt(index);
    }

    QString str;
    VectorToString(m_selected_locks, str);
    emit NotifyLockSelected(QString::number(lock), checked);
}

void LockCabinetWidget::updateUi()
{
    CAB_STATE* p_cab = &m_cabs[m_current_cab];
    for (int ii = 0; ii < m_locks_per_cab; ++ii)
    {
        m_lock_buttons[ii]->setChecked(p_cab->states[ii]);
        m_lock_buttons[ii]->setEnabled(p_cab->enabled[ii]);
        m_lock_buttons[ii]->setText(QString::number(p_cab->start + ii));
    }
}


