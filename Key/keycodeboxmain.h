#ifndef KEYCODEBOXMAIN_H
#define KEYCODEBOXMAIN_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QGraphicsPixmapItem>

#include <QGraphicsWidget>

#include "systemcontroller.h"
#include "systemcontrollerthread.h"
#include "frmadmininfo.h"
#include "frmadminpassword.h"
#include "dlgfingerprint.h"
#include "dlgfingerprintverify.h"
#include "dlgquestions.h"

class CClickableGraphicsItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    //    QThread myObjThread;
    Ui::MainWindow *ui;
    CSystemControllerThread     _sysControlThread;
    QGraphicsWidget     *_pGWidget;
    CSystemController   *_psystemController;

    CFrmUserCode        *_pfUsercode;
    CFrmAdminPassword   *_pfAdminPW;
    CFrmAdminInfo       *_pfAdminInfo;

    CDlgFingerprint *_pdFingerprint;
    CDlgFingerprintVerify *_pdFingerprintVerify;

    CDlgQuestions *_pQuestions;
    
    QGraphicsScene      *_pscene;
    QPixmap             *_pPixmap;
    CClickableGraphicsItem  *_pPixmapItem;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void OnImageClicked();
    void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
    bool isInternetTime();
    
signals:
    void __TouchScreenTouched();
    void __onCode(QString sCode);

    void __onFingerprintCode(QString sCode);

    void __onCodes(QString sCode1, QString sCode2);
    void __onFingerprintEnrollment(int code);

    void __StopUserTimeout();
    
private slots:

    void OnDisplayTimeoutScreen();
    void OnDisplayCodeDialog(QObject *psysController);
    void OnDisplayUserCodeTwoDialog(QObject *psysController);
    void OnDisplayThankYouDialog(QObject *psysController);
    void OnDisplayAdminPasswordDialog(QObject *psysController);
    void OnDisplayAdminMainDialog(QObject *psysController);

    void OnAdminPasswordCancel();

    void OnAdminSecurityCheckFailed();
    void OnUserCodeCancel();

    void OnUserCodeOne(QString sCode1);
    void OnUserCodeTwo(QString sCode2);

    void OnUserFingerprintCodeOne(QString sCode1);
    void OnUserFingerprintCodeTwo(QString sCode2);

    void OnEnrollFingerprintDialog(QString sCode);
    void OnVerifyFingerprintDialog();

    void OnQuestionUserDialog(int doorNum, QString question1, QString question2, QString question3);
    void OnQuestionUserDialogClose();
    
private:
    void initialize();
    void hideFormsExcept(QDialog *pfrm);

protected:
    void keyPressEvent(QKeyEvent *e);

};

#endif // KEYCODEBOXMAIN_H
