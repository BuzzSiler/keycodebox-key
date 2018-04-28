#include "lockcabinetwidget.h"
#include "ui_lockcabinetwidget.h"

#include <QtGlobal>
#include <QDebug>
#include <QSignalMapper>
#include "kcbutils.h"
#include "kcbcommon.h"

static const QString css_unpushed = "QPushButton[checkable=true]:enabled {color:\"black\"; background-color: \"red\"; font-weight:bold;}"
                                    "QPushButton[checkable=true]:disabled{color:\"gray\"; font-weight:normal;}"
                                    "QPushButton[checkable=true]:checked {color:\"black\"; background-color: \"green\"; font-weight:bold; border:none;}";

static const QString css_pushed = "QPushButton[checkable=true]:enabled {color:\"black\"; font-weight:bold;}"
                                  "QPushButton[checkable=true]:disabled{color:\"gray\"; font-weight:normal;}"
                                  "QPushButton[checkable=true]:checked {color:\"black\"; background-color:\"green\"; font-weight:bold; border:5px;}";

LockCabinetWidget::LockCabinetWidget(QWidget *parent, quint8 num_cabs, quint8 locks_per_cab) :
    QWidget(parent),
    m_num_cabs(num_cabs),
    m_locks_per_cab(locks_per_cab),
    m_cabs(num_cabs),
    m_current_cab(0),
    m_max_locks(locks_per_cab * num_cabs),
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
                    arg(tr("Cabinet")).
                    arg(cab_num).
                    arg(start, 3).
                    arg(stop, 3));

        start += m_locks_per_cab;
    };
}

LockCabinetWidget::~LockCabinetWidget()
{
    Kcb::Utils::DestructorMsg(this);
    delete ui;
}

//-------------------------------------------------------------------------------------------------

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

void LockCabinetWidget::setSelectedCabinet(const QString& cab)
{
    m_current_cab = cab.toInt() - 1;
    Q_ASSERT_X(m_current_cab < m_cabs.count(),
               Q_FUNC_INFO,
               "m_current_cab out of range");
    updateUi();
}

void LockCabinetWidget::setSelectedLocks(QString locks)
{
    Q_ASSERT_X(locks != "", Q_FUNC_INFO, "invalid locks (empty string)");

    QVector<QString> locks_vtr;
    quint8  cab_index;
    quint16 lock_index;

    // qDebug() << "setSelectedLocks:" << locks;

    m_selected_locks.clear();
    clrAllLocks();
    StringToVector(locks, locks_vtr);

    foreach (auto lock, locks_vtr)
    {
        CalcLockCabIndecies(lock, cab_index, lock_index);
        m_cabs[cab_index].states[lock_index] = true;
        AddLockToSelected(lock);
    }

    updateUi();
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
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    quint8 cab_index;
    quint16 lock_index;

    CalcLockCabIndecies(lock, cab_index, lock_index);
    m_cabs[m_current_cab].states[lock_index] = false;
    RemoveLockFromSelected(lock);

    updateUi();
}

void LockCabinetWidget::setWarning()
{
    setStyleSheet(css_unpushed);
}

void LockCabinetWidget::clrWarning()
{
    setStyleSheet(css_pushed);
}

void LockCabinetWidget::setEnabledLocks(QString locks)
{
    Q_ASSERT_X(locks != "", Q_FUNC_INFO, "invalid locks (empty string)");

    QVector<QString> locks_vtr;
    quint8 cab_index;
    quint16 lock_index;

    StringToVector(locks, locks_vtr);

    foreach (auto lock, locks_vtr)
    {
        CalcLockCabIndecies(lock, cab_index, lock_index);
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

// Private Slots ----------------------------------------------------------------------------------

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
    Q_ASSERT_X(index >= 0 && index < m_num_cabs, Q_FUNC_INFO, "index out of range");
    m_current_cab = index;
    updateUi();
}

void LockCabinetWidget::lockSelected(int lock_index)
{
    Q_ASSERT_X(lock_index >= 0 && lock_index < m_locks_per_cab, Q_FUNC_INFO, "lock_index out of range");

    quint8 cab_index = lock_index / m_locks_per_cab;

    Q_ASSERT_X(cab_index < m_num_cabs, Q_FUNC_INFO, "cab_index out of range");

    bool checked = m_lock_buttons[lock_index]->isChecked();
    m_cabs[cab_index].states[lock_index] = checked;
    QString lock = QString::number(lock_index + 1);

    if (checked)
    {
        AddLockToSelected(lock);
    }
    else
    {
        KCB_DEBUG_TRACE("calling RemoveLockFromSelected" << Q_FUNC_INFO);
        RemoveLockFromSelected(lock);
    }

    QString str;
    VectorToString(m_selected_locks, str);
    emit NotifyLockSelected(lock, checked);
}

// Private Methods --------------------------------------------------------------------------------

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

void LockCabinetWidget::selectClearAllLocks(bool select_clear)
{
    CAB_STATE* p_cab = &m_cabs[m_current_cab];
    m_selected_locks.clear();

    for (int ii = 0; ii < p_cab->states.count(); ++ii)
    {
        // KCB_DEBUG_TRACE("Lock Enabled" << p_cab->enabled[ii]);
        if (p_cab->enabled[ii])
        {
            p_cab->states[ii] = select_clear;
            m_lock_buttons[ii]->setChecked(select_clear);
            QString lock = QString::number(ii + 1);
            if (select_clear)
            {
                AddLockToSelected(lock);
            }
            emit NotifyLockSelected(lock, select_clear);
        }
    }

    updateUi();
}

void LockCabinetWidget::enableDisableLocksInCabinet(quint8 cab_index, bool enable_disable)
{
    Q_ASSERT_X(cab_index < m_num_cabs, Q_FUNC_INFO, "cab_index out of range");
    CAB_STATE* p_cab = &m_cabs[cab_index];

    for (int ii = 0; ii < p_cab->enabled.count(); ++ii)
    {
        p_cab->enabled[ii] = enable_disable;
        m_lock_buttons[ii]->setEnabled(enable_disable);
    }

    updateUi();
}

void LockCabinetWidget::clrLocksInCabinet(quint8 cab_index)
{
    Q_ASSERT_X(cab_index < m_num_cabs, Q_FUNC_INFO, "cab_index out of range");
    CAB_STATE* p_cab = &m_cabs[cab_index];
    for (int ii = 0; ii < p_cab->states.count(); ++ii)
    {
        p_cab->states[ii] = false;
    }
}

void LockCabinetWidget::StringToVector(QString str, QVector<QString>& vtr)
{
    Q_ASSERT_X(str != "", Q_FUNC_INFO, "empty str parameter");

    if (str.contains(','))
    {
        QStringList list = str.split(',');

        foreach (auto item, list)
        {
            vtr.append(item);
        }
    }
    else
    {
        if (str.length() >= 1)
        {
            vtr.append(str);
        }
    }
}

void LockCabinetWidget::VectorToString(QVector<QString> vtr, QString& str)
{
    Q_ASSERT_X(vtr.count() >= 0, Q_FUNC_INFO, "empty vector parameter");

    str = "";
    if (vtr.count() > 1)
    {
        QStringList list;
        foreach (auto item, vtr)
        {
            Q_ASSERT_X(item != "", Q_FUNC_INFO, "empty string");
            list << item;
        }
        list.sort();
        str = list.join(',');
    }
    else if (vtr.count() == 1)
    {
        str = vtr[0];
    }
}

void LockCabinetWidget::AddLockToSelected(const QString lock)
{
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    int lock_index = m_selected_locks.indexOf(lock);
    if (lock_index == -1)
    {
        m_selected_locks.append(lock);
    }
}

void LockCabinetWidget::RemoveLockFromSelected(const QString lock)
{
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    int lock_index = m_selected_locks.indexOf(lock);
    if (lock_index > -1)
    {
        m_selected_locks.removeAt(lock_index);
    }
}

void LockCabinetWidget::CalcLockCabIndecies(const QString lock, quint8 &cab_index, quint16 &lock_index)
{
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    quint16 lock_int = lock.toInt();
    Q_ASSERT_X(lock_int >= 1 && lock_int <= m_max_locks, Q_FUNC_INFO, "invalid lock");

    lock_index = (lock_int - 1) % m_locks_per_cab;
    cab_index = lock_index / m_locks_per_cab;

    Q_ASSERT_X(lock_index < m_locks_per_cab, Q_FUNC_INFO, "lock_index out of range");
    Q_ASSERT_X(cab_index < m_num_cabs, Q_FUNC_INFO, "cab_index out of range");
}
