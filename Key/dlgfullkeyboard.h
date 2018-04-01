#ifndef DLGFULLKEYBOARD_H
#define DLGFULLKEYBOARD_H

#include <QDialog>
#include <QWidget>
#include <QSignalMapper>


#include "currentedit.h"

namespace Ui {
class CDlgFullKeyboard;
}

class CDlgFullKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgFullKeyboard(QWidget *parent = 0);
    ~CDlgFullKeyboard();

    void setCurrentEdit(CCurrentEdit *pEdit);
    CCurrentEdit* getCurrentEdit() { return _pcurrentEdit; }

    void numbersOnly(bool state);

private:
    Ui::CDlgFullKeyboard *ui;

    CCurrentEdit    *_pcurrentEdit;
    QSignalMapper   *_pmapper;

    void initialize();
    void onButtonClick(char key);
    void onBackSpace();
    void onTextEntered();
    void hideKeyboard(bool bHide);
    void onDelete();
    void makeLower();
    void makeUpper();

public:
    const QLineEdit &getLineEdit();
    const QLabel &getLabel();
    void setActive();

public slots:
    void OnClearCodeDisplay();

signals:
    void __TextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);
    void __KeyPressed(char key);
    void __OnCloseKeyboard(CDlgFullKeyboard *keyboard);
    void __OnCancelKeyboard(CDlgFullKeyboard *keyboard, CCurrentEdit *pcurrEdit);

private slots:
    void onSetCurrentEdit(CCurrentEdit *pEdit) {
        connect(pEdit, SIGNAL(__onNumbersOnly(bool)), this, SLOT(onNumbersOnly(bool)));
        setCurrentEdit(pEdit);
    }

    void onNumbersOnly(bool bSet) {
        highlightNumbers(bSet);
    }

    void highlightNumbers(bool bSet);

    void buttonClicked(QWidget *btn);

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
    void on_btn_Back_clicked();
    void on_btn_Del_clicked();
    void on_btn_Space_clicked();
    void on_btn_Return_clicked();
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
    void on_btn_Close_clicked();
    void on_btn_Clear_clicked();
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
    void keyPressEvent(QKeyEvent *event);
};

#endif // DLGFULLKEYBOARD_H
