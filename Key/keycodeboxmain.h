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

    CDlgQuestions *_pQuestions;
    
    QGraphicsScene      *_pscene;
    QPixmap             *_pPixmap;
    CClickableGraphicsItem  *_pPixmapItem;
    QTimer *_pdisplayPowerDown;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void OnImageClicked();
    void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
    bool isInternetTime();
    
    typedef enum {DISP_POWER_OFF, DISP_POWER_ON} DISP_POWER_STATE;
    static DISP_POWER_STATE display_power_state;

signals:
    void __TouchScreenTouched();
    void __DisplayPoweredOn();
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

    void OnQuestionUserDialog(QString lockNum, QString question1, QString question2, QString question3);
    void OnQuestionUserDialogClose();

    void OnDisplayPoweredOn();
    void OnDisplayPowerDown();

private:
    void initialize();
    void hideFormsExcept(QDialog *pfrm);

protected:
    void keyPressEvent(QKeyEvent *e);

};

#endif // KEYCODEBOXMAIN_H
