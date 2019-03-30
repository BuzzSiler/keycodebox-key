#ifndef LOCKCABINETWIDGET_H
#define LOCKCABINETWIDGET_H

#include <QWidget>
#include "keycodeboxsettings.h"

namespace Ui {
    class LockCabinetWidget;
}

class QSignalMapper;
class QPushButton;

class LockCabinetWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LockCabinetWidget(QWidget *parent=0);
        ~LockCabinetWidget();

        void updateCabinetConfig();
        QString getSelectedLocks();
        void setSelectedLocks(QString locks);
        void clrAllLocks();
        QString getSelectedCabinet();
        void setEnabledLocks(QString locks);
        void enableAllLocks();
        void disableAllLocks();
        void setSelectedCabinet(const QString& cab, const QString& lock);
        void clrSelectedLocks(const QString& lock);
        void setWarning();
        void clrWarning();
        bool isConfigured();

    signals:
        void NotifyLockSelected(QString lock, bool is_selected);

    private slots:
        void on_pbSelectAll_clicked();
        void on_pbClearAll_clicked();
        void on_cbSelectedCabinet_currentIndexChanged(int index);
        void lockSelected(int lock_index);
        QString elideText(QString& text, const QFont& font, const int width);

    private:
        typedef struct _tag_cab_state
        {
            QVector<bool> states;
            QVector<bool> enabled;
        } CAB_STATE;

        CABINET_VECTOR m_cabinet_info;
        qint8 m_num_cabs;
        QVector<QString> m_selected_locks;
        QVector<CAB_STATE> m_cabs;        
        qint8 m_current_cab;
        QList<QPushButton *> m_lock_buttons;

        QSignalMapper& m_mapper;
        QString m_default_stylesheet;
        bool m_is_configured;
        Ui::LockCabinetWidget *ui;

        void updateUi();
        void selectClearAllLocks(bool select_clear);
        void enableDisableLocksInCabinet(qint8 cab_index, bool enable_disable);
        void clrLocksInCabinet(qint8 cab_index);
        void StringToVector(QString str, QVector<QString>& vtr);
        void VectorToString(QVector<QString> vtr, QString& str);
        void AddLockToSelected(const QString lock);
        void RemoveLockFromSelected(const QString lock);
        void CalcLockCabIndecies(const QString lock, int& cab_index, int& lock_index);
};

#endif // LOCKCABINETWIDGET_H
