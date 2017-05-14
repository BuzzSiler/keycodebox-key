#ifndef FRMADMINPASSWORD_H
#define FRMADMINPASSWORD_H

#include <QDialog>

namespace Ui {
class CFrmAdminPassword;
}

class CFrmAdminPassword : public QDialog
{
    Q_OBJECT

public:
    explicit CFrmAdminPassword(QWidget *parent = 0);
    ~CFrmAdminPassword();

public slots:
    void OnClearCodeDisplay();
    void OnEnableKeyboard(bool bEnable);
    void OnNewMessage(QString sMsg, int nDurationMS=5000);

signals:
    void __TextEntered(QString sText);
    void __KeyPressed(char key);
    void __PasswordEntered(QString sPassword);
    void __OnAdminPasswordCancel();

private slots:
    void on_btn_Return_clicked();

    void on_btn_Close_clicked();

    void on_btn_Clear_clicked();

    void on_btn_Back_clicked();

    void on_btnA_clicked();
    void on_btnB_clicked();
    void on_btnC_clicked();
    void on_btnD_clicked();
    void on_btnE_clicked();
    void on_btnF_clicked();
    void on_btnG_clicked();
    void on_btnH_clicked();
    void on_btnI_clicked();
    void on_btnJ_clicked();

    void on_btnK_clicked();
    void on_btnL_clicked();
    void on_btnM_clicked();
    void on_btnN_clicked();
    void on_btnO_clicked();
    void on_btnP_clicked();
    void on_btnQ_clicked();
    void on_btnR_clicked();
    void on_btnS_clicked();
    void on_btnT_clicked();
    void on_btnU_clicked();
    void on_btnV_clicked();
    void on_btnW_clicked();
    void on_btnX_clicked();
    void on_btnY_clicked();
    void on_btnZ_clicked();


    void on_btn_At_clicked();

    void on_btn_Del_clicked();

    void on_btn_Space_clicked();

    void on_btnPeriod_clicked();

    void on_btn_Underscore_clicked();

    void on_btn_Dash_clicked();

    void on_btn_Exclamation_clicked();

    void on_btn_Pound_clicked();

    void on_btn_Caret_clicked();

    void on_btn_Plus_clicked();

    void on_btn_QuestionMark_clicked();

    void on_btn_Slash_clicked();

    void on_btn_Colon_clicked();

    void on_btn_Semicolon_clicked();

    void on_btn_Apostrophe_clicked();

    void on_btn_Quote_clicked();

    void on_btn_Splat_clicked();

    void on_btn_Ampersand_clicked();

    void on_btn_Dollar_clicked();

    void on_btn_Percent_clicked();

    void on_btn_LeftParen_clicked();

    void on_btn_RightParen_clicked();

    void on_btn_0_clicked();

    void on_btn_1_clicked();

    void on_btn_2_clicked();

    void on_btn_3_clicked();

    void on_btn_4_clicked();

    void on_btn_5_clicked();

    void on_btn_6_clicked();

    void on_btn_7_clicked();

    void on_btn_8_clicked();

    void on_btn_9_clicked();

    void on_btnShowPassword_clicked(bool checked);

    void ResetPlaceholderText();
    void on_btn_Cancel_clicked();

    void on_btn_Back_Slash_clicked();
    void on_btn_OpenApost_clicked();
    void on_btn_Tilde_clicked();
    void on_btn_LeftAngle_clicked();
    void on_btn_RightAngle_clicked();
    void on_btn_LeftCurly_clicked();
    void on_btn_RightCurly_clicked();
    void on_btn_LeftSquare_clicked();
    void on_btn_RightSquare_clicked();
    void on_btn_Upper_Lower_clicked(bool checked);

private:
    Ui::CFrmAdminPassword *ui;
    void onButtonClick(char key);
    void onPasswordEntered();
    void onBackSpace();
    void enableKeypad(bool bEnable);
};

#endif // FRMADMINPASSWORD_H
