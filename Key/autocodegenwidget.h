#ifndef AUTOCODEGENWIDGET_H
#define AUTOCODEGENWIDGET_H

#include <QWidget>

#include "keycodeboxsettings.h"
#include "autocodegenerator.h"


namespace Ui {
    class AutoCodeGenWidget;
}

class LockCabinetWidget;

class AutoCodeGenWidget : public QWidget
{
    Q_OBJECT
    public:
    explicit AutoCodeGenWidget(QWidget *parent = 0);
    ~AutoCodeGenWidget();

    signals:
        void RequestCodes1(QStringList& codes);
        void RequestCodes2(QStringList& codes);
        void CommitCodes1(QMap<QString, QString> codes);
        void CommitCodes2(QMap<QString, QString> codes);
        void NotifyDisableLockSelection();
        void NotifyEnableLockSelection();
        void NotifyAutoCodeEnabled();
        void NotifyAutoCodeDisabled();
        void NotifyCodesUpdate(const QStringList& codes = QStringList());
        void NotifyAutoCodeEmailUpdate(const QString& key);

    public slots:
        void OnLockSelectionChanged();
        void UpdateCodes();

    private slots:
        void on_pbAutoCodeGenerate_clicked();
        void on_pbAutoCodeCommit_clicked();
        void on_cbAutoCodeMode_currentIndexChanged(int index);
        void on_gbAutoCodeEnableDisable_clicked(bool checked);
        void on_cbAutoCodePeriod_currentIndexChanged(int index);
        void on_leAutoCodePassword_clicked();
        void on_teAutoCodeStartOfDay_timeChanged(const QTime &time);
        void on_spAutoCodeCodeLength_valueChanged(int arg1);
        void on_cbAutoCodePeriodUnits_currentIndexChanged(int index);
        void on_cbAutoCodeEmailKey_clicked();
        void on_pbAutoCodeRandomPassword_clicked();

    private:
        LockCabinetWidget& m_lock_cabinet;
        AutoCodeSettings m_settings;
        bool m_init;
        AutoCodeParams m_params;
        Ui::AutoCodeGenWidget *ui;

        void EnableAutoCode();
        void DisableAutoCode();
        void InitLockCabinet();
        void InitControls();
        void updateUi();
        void LoadAutoCodeSettings();
        void StoreAutoCodeSettings();
        void ShowQrCode(const QString& data);
        int GetUnitIndex();
        bool WarnAutoCodeEnable();
        bool WarnAutoCodeDisable();
        void HandleAutoCodeEnable();
        void HandleAutoCodeDisable();
        void SendAutoCodeEmail();
        bool WarnAutoCodeCommit();
        void HandleAutoCodeCommit();
        void UpdateAutoCodeSettings();
        AutoCodeGenerator::CodeMap& ProcessCommittedAutoCodeSettings(AutoCodeGenerator::CodeMap& map);
        AutoCodeGenerator::CodeMap& ProcessEnabledAutoCodeSettings(AutoCodeGenerator::CodeMap& map);
        AutoCodeGenerator::CodeMap& RequestCodesFromDatabase(AutoCodeGenerator::CodeMap& map);
        AutoCodeGenerator::CodeMap& RequestActiveCodes(AutoCodeGenerator::CodeMap& map);
        bool IsCode1Mode();
        bool IsCode2Mode();
        QString RunKeyboard(const QString& text);
        QString GetHashedPassword(const QString& text);
        void UpdateAutoCodeParams();

};

#endif // AUTOCODEGENWIDGET_H
