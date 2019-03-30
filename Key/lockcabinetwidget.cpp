#include "lockcabinetwidget.h"
#include "ui_lockcabinetwidget.h"

#include <QtGlobal>
#include <QDebug>
#include <QSignalMapper>
#include <QFontMetrics>

#include "kcbutils.h"
#include "kcbcommon.h"
#include "keycodeboxsettings.h"

static const QString css_unpushed = "QPushButton[checkable=true]:enabled {color:\"black\"; background-color: \"red\"; font-weight:bold;}"
                                    "QPushButton[checkable=true]:disabled{color:\"gray\"; font-weight:normal;}"
                                    "QPushButton[checkable=true]:checked {color:\"black\"; background-color: \"green\"; font-weight:bold; border:none;}";

static const QString css_pushed = "QPushButton[checkable=true]:enabled {color:\"black\"; font-weight:bold;}"
                                  "QPushButton[checkable=true]:disabled{color:\"gray\"; font-weight:normal;}"
                                  "QPushButton[checkable=true]:checked {color:\"black\"; background-color:\"green\"; font-weight:bold; border:5px;}";

LockCabinetWidget::LockCabinetWidget(QWidget *parent) :
    QWidget(parent),
    m_cabinet_info(0),
    m_num_cabs(-1),
    m_selected_locks(0),
    m_cabs(0),
    m_current_cab(-1),
    m_mapper(* new QSignalMapper(this)),
    m_default_stylesheet(""),
    m_is_configured(false),
    ui(new Ui::LockCabinetWidget)
{
    ui->setupUi(this);

    // Get a static list of the lock buttons on the widget
    m_lock_buttons = this->findChildren<QPushButton *>(QRegularExpression("pbLock.."), Qt::FindChildrenRecursively);

    // Map the click signal from the buttons to lockSelected slot via a mapper
    for (int ii = 0; ii < m_lock_buttons.length(); ++ii)
    {
        QPushButton *btn = m_lock_buttons[ii];
        btn->setDisabled(true);
        connect(btn, SIGNAL(clicked()), &m_mapper, SLOT(map()));
        m_mapper.setMapping(btn, ii);
    }
    connect(&m_mapper, SIGNAL(mapped(int)), this, SLOT(lockSelected(int)));

    ui->pbSelectAll->setDisabled(true);
    ui->pbClearAll->setDisabled(true);
    ui->cbSelectedCabinet->setDisabled(true);

    updateCabinetConfig();

}

LockCabinetWidget::~LockCabinetWidget()
{
    kcb::ClassAllocation::DestructorMsg(this);
    delete ui;
}

//-------------------------------------------------------------------------------------------------

bool LockCabinetWidget::isConfigured()
{
    return m_is_configured;
}

void LockCabinetWidget::updateCabinetConfig()
{
    KCB_DEBUG_ENTRY;

    m_is_configured = false;

    m_cabinet_info = KeyCodeBoxSettings::getCabinetsInfo();
    if (m_cabinet_info.count() == 0)
    {
        KCB_DEBUG_EXIT;
        return;
    }

    ui->cbSelectedCabinet->clear();
    ui->cbSelectedCabinet->setEnabled(true);

    m_num_cabs = m_cabinet_info.size();
    m_cabs.resize(m_num_cabs);
    m_current_cab = 0;

    for (int ii = 0; ii < m_cabs.length(); ++ii)
    {
        quint8 start = m_cabinet_info[ii].start;
        quint8 stop = m_cabinet_info[ii].stop;
        quint8 size = m_cabinet_info[ii].num_locks;

        m_cabs[ii].states = QVector<bool>(m_lock_buttons.length(), false);
        m_cabs[ii].enabled = QVector<bool>(m_lock_buttons.length(), false);

        for (int jj = 0; jj < size; ++jj)
        {
            m_cabs[ii].enabled[jj] = true;
        }

        // Populate the cabinets dropdown
        ui->cbSelectedCabinet->addItem(
            QString("%1 (%2 .. %3)").
                    arg(m_cabinet_info[ii].model).
                    arg(start, 3, 10, QChar('0')).
                    arg(stop, 3, 10, QChar('0')));
    };

    ui->pbSelectAll->setEnabled(true);
    ui->pbClearAll->setEnabled(true);

    m_is_configured = true;

    KCB_DEBUG_EXIT;
}

QString LockCabinetWidget::getSelectedCabinet()
{
    QString model("");

    if (m_current_cab >= 0)
    {
        model = m_cabinet_info[m_current_cab].model;
    }
    return model;
}

QString LockCabinetWidget::getSelectedLocks()
{
    QString str;

    VectorToString(m_selected_locks, str);

    return str;
}

void LockCabinetWidget::setSelectedCabinet(const QString& cab, const QString& lock)
{
    // Cabinet is a string representing the solenoid board model
    // Lock is a string representing the lock number (as a global number, i.e., 1 .. total num locks, as opposed to 1 .. 32 for a cabinet)
    // Compare both the cab (model) and the lock as only that combination is unique

    int cab_index = -1;
    foreach (auto entry, m_cabinet_info)
    {
        cab_index ++;

        if (cab == entry.model)
        {
            if (lock.toInt() >= entry.start && lock.toInt() <= entry.stop)
            {
                break;
            }
        }
    }

    m_current_cab = cab_index;
    Q_ASSERT_X(m_current_cab < m_num_cabs,
               Q_FUNC_INFO,
               "m_current_cab out of range");
    updateUi();
}

void LockCabinetWidget::setSelectedLocks(QString locks)
{
    Q_ASSERT_X(locks != "", Q_FUNC_INFO, "invalid locks (empty string)");

    if (!isConfigured())
    {
        KCB_DEBUG_TRACE("Lock Cabinet is not configured");
        return;
    }

    QVector<QString> locks_vtr;
    int  cab_index;
    int lock_index;

    clrAllLocks();
    StringToVector(locks, locks_vtr);

    foreach (auto lock, locks_vtr)
    {
        CalcLockCabIndecies(lock, cab_index, lock_index);
        if (cab_index >= 0 && lock_index >= 0)
        {
            m_cabs[cab_index].states[lock_index] = true;
            AddLockToSelected(lock);
        }
    }

    updateUi();
}

void LockCabinetWidget::clrAllLocks()
{
    for (int ii = 0; ii < m_cabs.count(); ++ii)
    {
        clrLocksInCabinet(ii);
    }

    m_selected_locks.clear();

    updateUi();
}

void LockCabinetWidget::clrSelectedLocks(const QString& lock)
{
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    int cab_index;
    int lock_index;

    CalcLockCabIndecies(lock, cab_index, lock_index);
    if (cab_index >= 0 && lock_index >= 0)
    {
        m_cabs[m_current_cab].states[lock_index] = false;
        RemoveLockFromSelected(lock);
    }

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
    KCB_DEBUG_ENTRY;
    Q_ASSERT_X(locks != "", Q_FUNC_INFO, "invalid locks (empty string)");

    QVector<QString> locks_vtr;
    int cab_index;
    int lock_index;

    StringToVector(locks, locks_vtr);

    foreach (auto lock, locks_vtr)
    {
        CalcLockCabIndecies(lock, cab_index, lock_index);
        KCB_DEBUG_TRACE("lock" << lock << "cab_index" << cab_index << "lock_index" << lock_index);
        if (cab_index >= 0 && lock_index >= 0)
        {
            m_cabs[cab_index].enabled[lock_index] = true;
        }
    }

    updateUi();
    KCB_DEBUG_EXIT;
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
    KCB_DEBUG_ENTRY;
    for (int ii = 0; ii < m_cabs.count(); ++ii)
    {
        enableDisableLocksInCabinet(ii, false);
    }
    KCB_DEBUG_EXIT;
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
    Q_ASSERT_X(lock_index >= 0 && lock_index < MAX_NUM_LOCKS_PER_CABINET, Q_FUNC_INFO, "lock_index out of range");

    if (lock_index < 0 || lock_index > MAX_NUM_LOCKS_PER_CABINET)
    {
        KCB_WARNING_TRACE("Index" << lock_index << "out of range");
        return;
    }

    bool checked = m_lock_buttons[lock_index]->isChecked();
    m_cabs[m_current_cab].states[lock_index] = checked;
    
    QString lock = QString::number(m_cabinet_info[m_current_cab].start + lock_index);

    if (checked)
    {
        KCB_DEBUG_TRACE("calling AddLockToSelected");
        AddLockToSelected(lock);
    }
    else
    {
        KCB_DEBUG_TRACE("calling RemoveLockFromSelected");
        RemoveLockFromSelected(lock);
    }

    QString str;
    VectorToString(m_selected_locks, str);
    emit NotifyLockSelected(lock, checked);
}

// Private Methods --------------------------------------------------------------------------------

void LockCabinetWidget::updateUi()
{
    if (m_current_cab < 0)
    {
        return;
    }
    
    CAB_STATE* p_cab = &m_cabs[m_current_cab];
    quint8 start = m_cabinet_info[m_current_cab].start;
    quint8 stop = m_cabinet_info[m_current_cab].stop;
    for (int ii = 0; ii < MAX_NUM_LOCKS_PER_CABINET; ++ii)
    {
        m_lock_buttons[ii]->setText("");

        if (start + ii <= stop)
        {
            m_lock_buttons[ii]->setChecked(p_cab->states[ii]);
            m_lock_buttons[ii]->setEnabled(p_cab->enabled[ii]);
            m_lock_buttons[ii]->setText(QString::number(start + ii));
        }
        else
        {
            m_lock_buttons[ii]->setChecked(false);
            m_lock_buttons[ii]->setEnabled(false);
        }
    }
}

void LockCabinetWidget::selectClearAllLocks(bool select_clear)
{
    if (m_current_cab < 0)
    {
        return;
    }
    
    CAB_STATE* p_cab = &m_cabs[m_current_cab];
    quint8 start = m_cabinet_info[m_current_cab].start;
    quint8 stop = m_cabinet_info[m_current_cab].stop;

    for (int ii = 0; ii < p_cab->states.count(); ++ii)
    {
        if (start + ii <= stop)
        {
            //KCB_DEBUG_TRACE("Lock" << ii << "Enabled" << p_cab->enabled[ii] << "SelectClear" << select_clear);
            if (p_cab->enabled[ii])
            {
                p_cab->states[ii] = select_clear;
                m_lock_buttons[ii]->setChecked(select_clear);
                QString lock = QString::number(start + ii);
                if (select_clear)
                {
                    AddLockToSelected(lock);
                }
                else
                {
                    RemoveLockFromSelected(lock);
                }
                emit NotifyLockSelected(lock, select_clear);
            }
        }
        else
        {
            p_cab->enabled[ii] = false;
        }
    }

    updateUi();
}

void LockCabinetWidget::enableDisableLocksInCabinet(qint8 cab_index, bool enable_disable)
{
    KCB_DEBUG_ENTRY;

    Q_ASSERT_X(cab_index < m_num_cabs, Q_FUNC_INFO, "cab_index out of range");
    if (m_num_cabs < 0 || cab_index < 0 || cab_index > (m_num_cabs - 1))
    {
        KCB_DEBUG_EXIT;
        return;
    }
    
    CAB_STATE* p_cab = &m_cabs[cab_index];
    quint8 start = m_cabinet_info[cab_index].start;
    quint8 stop = m_cabinet_info[cab_index].stop;

    for (int ii = 0; ii < p_cab->enabled.count(); ++ii)
    {
        if (start + ii <= stop)
        {
            p_cab->enabled[ii] = enable_disable;
            m_lock_buttons[ii]->setEnabled(enable_disable);
        }
        else
        {
            p_cab->enabled[ii] = false;
            m_lock_buttons[ii]->setEnabled(false);
        }
    }

    updateUi();
    KCB_DEBUG_EXIT;
}

void LockCabinetWidget::clrLocksInCabinet(qint8 cab_index)
{
    Q_ASSERT_X(cab_index < m_num_cabs, Q_FUNC_INFO, "cab_index out of range");
    if (m_num_cabs < 0 || cab_index < 0 || cab_index > (m_num_cabs - 1))
    {
        return;
    }

    m_cabs[cab_index].states = QVector<bool>(MAX_NUM_LOCKS_PER_CABINET, false);
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
        QList<int> int_list;
        foreach (auto item, vtr)
        {
            Q_ASSERT_X(item != "", Q_FUNC_INFO, "empty string");
            int_list << item.toInt();
        }
        qSort(int_list);

        QStringList str_list;
        foreach (auto item, int_list)
        {
            str_list << QString::number(item);
        }
        str = str_list.join(',');
    }
    else if (vtr.count() == 1)
    {
        str = vtr[0];
    }
}

QString LockCabinetWidget::elideText(QString& text, const QFont& font, const int width)
{
    QFontMetrics metrix(font);
    QString clippedText = metrix.elidedText(text, Qt::ElideMiddle, width);
    return clippedText;
}

void LockCabinetWidget::AddLockToSelected(const QString lock)
{
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    int lock_index = m_selected_locks.indexOf(lock);
    if (lock_index == -1)
    {
        m_selected_locks.append(lock);
        QString sorted_string;
        VectorToString(m_selected_locks, sorted_string);
        sorted_string = elideText(sorted_string, ui->lblCurrentSelectedLocks->font(), ui->lblCurrentSelectedLocks->width());
        ui->lblCurrentSelectedLocks->setText(sorted_string);
    }
}

void LockCabinetWidget::RemoveLockFromSelected(const QString lock)
{
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    int lock_index = m_selected_locks.indexOf(lock);
    if (lock_index > -1)
    {
        m_selected_locks.removeAt(lock_index);
        QString sorted_string;
        VectorToString(m_selected_locks, sorted_string);
        sorted_string = elideText(sorted_string, ui->lblCurrentSelectedLocks->font(), ui->lblCurrentSelectedLocks->width());
        ui->lblCurrentSelectedLocks->setText(sorted_string);
    }
}

void LockCabinetWidget::CalcLockCabIndecies(const QString lock, int &cab_index, int &lock_index)
{
    KCB_DEBUG_ENTRY;
    Q_ASSERT_X(lock != "", Q_FUNC_INFO, "empty string");

    quint16 lock_int = lock.toInt();
    Q_ASSERT_X(lock_int >= 1 && lock_int <= MAX_NUM_LOCKS_PER_CABINET, Q_FUNC_INFO, "invalid lock");

    if (m_num_cabs < 0)
    {
        KCB_DEBUG_EXIT;
        return;
    }

    cab_index = -1;
    lock_index = -1;

    for (int ii = 0; ii < m_num_cabs; ++ii)
    {
        quint8 start = m_cabinet_info[ii].start;
        quint8 stop = m_cabinet_info[ii].stop;

        KCB_DEBUG_TRACE("ii" << ii << "start" << start << "lock_int" << lock_int << "stop" << stop);
        if (lock_int >= start && lock_int <= stop)
        {
            cab_index = ii;
            lock_index = (lock_int - 1) % MAX_NUM_LOCKS_PER_CABINET;
            break;
        }
    }
    KCB_DEBUG_EXIT;
}
