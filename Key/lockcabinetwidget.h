#ifndef LOCKCABINETWIDGET_H
#define LOCKCABINETWIDGET_H

#include <QWidget>

namespace Ui {
    class LockCabinetWidget;
}

class QSignalMapper;
class QPushButton;

class LockCabinetWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LockCabinetWidget(QWidget *parent=0, quint8 num_cabs=1, quint8 locks_per_cab=32);
        ~LockCabinetWidget();

        QString getSelectedLocks();
        void setSelectedLocks(QString locks);
        void clrAllLocks();
        QString getSelectedCabinet();
        void setEnabledLocks(QString locks);
        void enableAllLocks();
        void disableAllLocks();
        void setSelectedCabinet(const QString& cab);
        void clrSelectedLocks(const QString& lock);
        void setWarning();
        void clrWarning();

    signals:
        void NotifyLockSelected(QString lock, bool is_selected);

    private slots:
        void on_pbSelectAll_clicked();
        void on_pbClearAll_clicked();
        void on_cbSelectedCabinet_currentIndexChanged(int index);
        void lockSelected(int lock_index);

    private:
        typedef struct _tag_cab_state
        {
            int number;
            int start;
            QVector<bool> states;
            QVector<bool> enabled;
        } CAB_STATE;

        quint8 m_num_cabs;
        quint8 m_locks_per_cab;
        QVector<int> m_selected_locks;
        QVector<CAB_STATE> m_cabs;
        quint8 m_current_cab;
        QList<QPushButton *> m_lock_buttons;

        QSignalMapper& m_mapper;
        QPalette m_default_palette;
        Ui::LockCabinetWidget *ui;
        void selectClearAllLocks(bool select_clear);
        void updateUi();
        void enableDisableLocksInCabinet(quint8 cab_index, bool enable_disable);
        void clrLocksInCabinet(quint8 cab_index);
        void StringToVector(QString str, QVector<int>& vtr);
        void VectorToString(QVector<int> vtr, QString& str);
};

#endif // LOCKCABINETWIDGET_H
