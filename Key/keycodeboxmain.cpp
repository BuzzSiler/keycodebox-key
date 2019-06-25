#include "keycodeboxmain.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <cstdlib>

#include <QDateTime>
#include <QProcess>
#include <QScreen>
#include <QThread>

#include "usbcontroller.h"
#include "lockcontroller.h"
#include "usbhwkeypad.h"
#include "frmadmininfo.h"
#include "frmusercode.h"
#include "frmadminpassword.h"
#include "encryption.h"
#include "hidreader.h"
#include "magtekcardreader.h"
#include "systemcontroller.h"
#include "kcbcommon.h"
#include "logger.h"
#include "kcbsystem.h"
#include "kcbkeyboarddialog.h"
#include "keycodeboxsettings.h"

MainWindow      *gpmainWindow;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    kkd(*new KcbKeyboardDialog(this, true))
{
    ui->setupUi(this);

    LOG_TRACE_DEBUG("starting application");

    setAttribute(Qt::WA_AcceptTouchEvents, true);
    
    kcb::SetWindowParams(this);
    QRect ag;
    kcb::GetAvailableGeometry(ag);
    QCursor::setPos(ag.width(), ag.height());

    initialize();

    _pscene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_pscene);
    _pPixmap = new QPixmap("/home/pi/kcb-config/images/alpha_logo.jpg");

    if( _pPixmap->isNull() )
    {
        KCB_DEBUG_TRACE("Loading default branding image");

        _pPixmap = new QPixmap("/home/pi/kcb-config/images/alpha_logo_touch.jpg");
        if(_pPixmap->isNull()) 
        {
            KCB_DEBUG_TRACE("Failed to load default branding image");
        }
    }

    // Scale the image...
    _pPixmapItem = new CClickableGraphicsItem(_pPixmap->scaled(ag.width(), ag.height()));
    _pscene->addItem(_pPixmapItem);
    ui->graphicsView->show();

    ui->graphicsView->setClickedFunc(&(OnImageClicked) );
    ui->graphicsView->fitInView(ag);

    _pdisplayPowerDown = new QTimer();
   

    QDateTime currdt = QDateTime::currentDateTime();
    QDateTime dt = CEncryption::roundDateTime(10, currdt);

    // Note: This line is wrong according to Qt and results in this log entry:
    // default  WARN QObject::moveToThread: Cannot move objects with a parent
    // However, not passing 'this' to CSystemController results a threading failure
    // There is much that is wrong with this architecture, but until we address the
    // architecture, we are stuck passing 'this' in order to make things work.
    _psystemController = new CSystemController(this);

    connect(_psystemController, SIGNAL(__OnDisplayCodeDialog(QObject*)), this, SLOT(OnDisplayCodeDialog(QObject*)));
    connect(_psystemController, SIGNAL(__OnDisplayUserCodeTwoDialog(QObject*)), this, SLOT(OnDisplayUserCodeTwoDialog(QObject*)));
    connect(_psystemController, SIGNAL(__OnDisplayAdminPasswordDialog(QObject*)), this, SLOT(OnDisplayAdminPasswordDialog(QObject*)));
    connect(_psystemController, SIGNAL(__OnDisplayAdminMainDialog(QObject*)), this, SLOT(OnDisplayAdminMainDialog(QObject*)));

    connect(&_sysControlThread, SIGNAL(started()), _psystemController, SLOT(start()));

    // When the touch screen is touched, we want to 
    //     - notify the system controller
    //     - stop the display power down timer
    connect(this, SIGNAL(__TouchScreenTouched()), _psystemController, SLOT(OnTouchScreenTouched()));
    connect(this, SIGNAL(__TouchScreenTouched()), _pdisplayPowerDown, SLOT(stop()));
    // When the display is powered up, we want to start the display power down timer
    connect(this, SIGNAL(__DisplayPoweredOn()), this, SLOT(OnDisplayPoweredOn()));
    // When the display power down timer expires, we want to power down the display
    connect(_pdisplayPowerDown, SIGNAL(timeout()), this, SLOT(OnDisplayPowerDown()));

    connect(_psystemController, SIGNAL(__OnDisplayTimeoutScreen()), this, SLOT(OnDisplayTimeoutScreen()));

    connect(_psystemController, SIGNAL(__onUserCodeOne(QString)), this, SLOT(OnUserCodeOne(QString)));
    connect(_psystemController, SIGNAL(__onUserCodeTwo(QString)), this, SLOT(OnUserCodeTwo(QString)));

    connect(_psystemController, SIGNAL(__onUserFingerprintCodeOne(QString)), this, SLOT(OnUserFingerprintCodeOne(QString)));
    connect(_psystemController, SIGNAL(__onUserFingerprintCodeTwo(QString)), this, SLOT(OnUserFingerprintCodeTwo(QString)));

    _psystemController->setMainWindow(this);
    _sysControlThread.setSystemController(_psystemController);

    gpmainWindow = this;

    _psystemController->moveToThread(&_sysControlThread);
    SetupAdmin(_psystemController);
    _sysControlThread.start();

    kcb::TurnOnDisplay();
}

void MainWindow::SetupAdmin(QObject *psysController)
{
    // KCB_DEBUG_ENTRY;
    if (!_pfAdminInfo)
    {
        _pfAdminInfo = new CFrmAdminInfo(this);
        _pfAdminInfo->hide();
        _pfAdminInfo->setSystemController((CSystemController*)psysController);
    }
    connect(_pfAdminInfo, SIGNAL(__OnOpenLockRequest(QString)), _psystemController, SLOT(OnOpenLockRequest(QString)));
    connect(_psystemController, SIGNAL(__onUserCodes(QString,QString)), _pfAdminInfo, SLOT(OnCodes(QString, QString)));
    connect(_pfAdminInfo, SIGNAL(__OnSendTestEmail(int)), _psystemController, SLOT(OnSendTestEmail(int)));
    connect(_psystemController, SIGNAL(__OnDisplayFingerprintButton(bool)), _pfAdminInfo, SLOT(OnDisplayFingerprintButton(bool)));
    connect(_psystemController, SIGNAL(__OnDisplayShowHideButton(bool)), _pfAdminInfo, SLOT(OnDisplayShowHideButton(bool)));
    connect(_psystemController, SIGNAL(__OnDisplayTakeReturnButtons(bool)), _pfAdminInfo, SLOT(OnDisplayTakeReturnButtons(bool)));
    connect(_psystemController, SIGNAL(__OnUpdateCodes()), _pfAdminInfo, SLOT(OnUpdateCodes()));
    // KCB_DEBUG_EXIT;
}

bool MainWindow::isInternetTime()
{
    return KeyCodeBoxSettings::IsInternetTimeEnabled();
}

void MainWindow::initialize() 
{
    _pfUsercode = 0;
    _pfAdminInfo = 0;
    _pdFingerprint = 0;
    _pQuestions = 0;

    QCursor::setPos(848, 480);
    QApplication::setOverrideCursor(Qt::BlankCursor);

    if( isInternetTime() )
    {
        kcb::EnableInternetTime();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _psystemController;
}

void MainWindow::OnDisplayPowerDown()
{
    kcb::TurnOffDisplay();
}

void MainWindow::OnDisplayPoweredOn()
{
    if (kcb::isDisplayPowerOn())
    {
        if (!_pfAdminInfo)
        {
            return;
        }

        int timeout = _pfAdminInfo->getDisplayPowerDownTimeout();
        if (timeout > 0)
        {
            _pdisplayPowerDown->start(timeout);
        }
    }
}

void MainWindow::OnImageClicked()
{
    // KCB_DEBUG_ENTRY;
    if (kcb::isDisplayPowerOn())
    {
        emit gpmainWindow->__TouchScreenTouched();
    }
    else
    {
        kcb::TurnOnDisplay();

        emit gpmainWindow->__DisplayPoweredOn();
    }
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnDisplayTimeoutScreen()
{
    // KCB_DEBUG_ENTRY;

    // hide any open screens to show the touch screen to start
    if(_pfUsercode) 
    {
        _pfUsercode->OnClearCodeDisplay();
        _pfUsercode->hide();
    }
    kkd.hide();
    if(_pfAdminInfo) 
    {
        _pfAdminInfo->hide();
    }
    if(_pdFingerprint) 
    {
        _pdFingerprint->hide();
    }
    if(_pQuestions) 
    {
        _pQuestions->hide();
    }

    OnDisplayPoweredOn();

    // KCB_DEBUG_EXIT;
}

void MainWindow::OnDisplayCodeDialog(QObject *psysController)
{
    // KCB_DEBUG_ENTRY;
    if(!_pfUsercode)
    {
        _pfUsercode = new CFrmUserCode(this);
        connect(psysController, SIGNAL(__OnClearEntry()), _pfUsercode, SLOT(OnClearCodeDisplay()));
        connect(psysController, SIGNAL(__OnCodeMessage(QString)), _pfUsercode, SLOT(OnNewCodeMessage(QString)));

        connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), psysController, SLOT(OnUserCodeCancel()));
        connect(_pfUsercode, SIGNAL(NotifyAdminRequested()), psysController, SLOT(OnNotifyAdminRequested()));
        connect(_pfUsercode, SIGNAL(NotifyAdminCancelled()), psysController, SLOT(OnNotifyAdminCancelled()));
        connect(this, SIGNAL(__onCode(QString)), _pfUsercode, SLOT(OnSwipeCode(QString)));
        connect(this, SIGNAL(__onFingerprintCode(QString)), psysController, SLOT(OnFingerprintCodeEntered(QString)));

        connect(psysController, SIGNAL(__onEnrollFingerprintDialog(QString)), this, SLOT(OnEnrollFingerprintDialog(QString)));
        connect(psysController, SIGNAL(__onQuestionUserDialog(QString,QString,QString,QString)), this, SLOT(OnQuestionUserDialog(QString,QString,QString,QString)));

        connect(_pfUsercode, SIGNAL(__onVerifyFingerprint()), psysController, SLOT(OnVerifyFingerprint()));
        connect(_pfUsercode, SIGNAL(__onVerifyFingerprintDialog()), psysController, SLOT(OnVerifyFingerprintDialog()));
        connect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, SLOT(OnCodeEntered(QString)));

        connect(_pfAdminInfo, SIGNAL(__OnDisplayFingerprintButton(bool)), _pfUsercode, SIGNAL(__OnDisplayFingerprintButton(bool)));
        connect(_pfAdminInfo, SIGNAL(__OnDisplayShowHideButton(bool)), _pfUsercode, SIGNAL(__OnDisplayShowHideButton(bool)));
        connect(_pfAdminInfo, SIGNAL(__OnDisplayTakeReturnButtons(bool)), _pfUsercode, SIGNAL(__OnDisplayTakeReturnButtons(bool)));    

        if( !_pdFingerprint )
        {
            _pdFingerprint = new CDlgFingerprint();
            connect(_pdFingerprint, SIGNAL(__onEnrollFingerprintDialogCancel()), psysController, SLOT(EnrollFingerprintDialogCancel()));
            connect(psysController, SIGNAL(__onUpdateEnrollFingerprintDialog(int, int, QString)), _pdFingerprint, SLOT(OnUpdateEnrollFingerprintDialog(int, int, QString)));
            _pdFingerprint->hide();
        }


        if( !_pQuestions )
        {
            _pQuestions = new CDlgQuestions(this);
            _pQuestions->hide();
            connect(psysController, SIGNAL(__onQuestionUserDialog(QString,QString,QString,QString)), this, SLOT(OnQuestionUserDialog(QString,QString,QString,QString)));
            connect(_pQuestions, SIGNAL(__OnQuestionsCancel()), psysController, SLOT(QuestionUserCancel()));
            connect(_pQuestions, SIGNAL(__OnQuestionsSave(QString,QString,QString,QString)), psysController, SLOT(AnswerUserSave(QString,QString,QString,QString)));
            connect(_pQuestions, SIGNAL(__OnQuestionsClose()), this, SLOT(OnQuestionUserDialogClose()));
        }

    }
    else
    {
        hideFormsExcept(_pfUsercode);

        disconnect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, 0);
        connect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, SLOT(OnCodeEntered(QString)));

        disconnect(_pfUsercode, SIGNAL(__FingerprintCodeEntered(QString)), psysController, 0);        
        connect(_pfUsercode, SIGNAL(__FingerprintCodeEntered(QString)), psysController, SLOT(OnFingerprintCodeEntered(QString)));
    }
    _pfUsercode->SetDisplayFingerprintButton(_psystemController->getDisplayFingerprintButton());
    _pfUsercode->SetDisplayShowHideButton(_psystemController->getDisplayShowHideButton());
    _pfUsercode->SetDisplayTakeReturnButtons(_psystemController->getDisplayTakeReturnButtons());
    _pfUsercode->show();
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnDisplayUserCodeTwoDialog(QObject *psysController)
{
    // KCB_DEBUG_ENTRY;
    if(!_pfUsercode) 
    {
        _pfUsercode = new CFrmUserCode(this);
        connect(psysController, SIGNAL(__OnClearEntry()), _pfUsercode, SLOT(OnClearCodeDisplay()));
        connect(psysController, SIGNAL(__OnCodeMessage(QString)), _pfUsercode, SLOT(OnNewCodeMessage(QString)));
        connect(psysController, SIGNAL(__OnDisplayFingerprintButton(bool)), _pfUsercode, SLOT(OnDisplayFingerprintButton(bool)));
        connect(psysController, SIGNAL(__OnDisplayShowHideButton(bool)), _pfUsercode, SLOT(OnDisplayShowHideButton(bool)));
        connect(psysController, SIGNAL(__OnDisplayTakeReturnButtons(bool)), _pfUsercode, SLOT(OnDisplayTakeReturnButtons(bool)));

        connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), psysController, SLOT(OnUserCodeCancel()));
        //connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), this, SLOT(OnUserCodeCancel()));
        connect(this, SIGNAL(__onCode(QString)), _pfUsercode, SLOT(OnSwipeCode(QString)));
        connect(this, SIGNAL(__onFingerprintCode(QString)), psysController, SLOT(OnFingerprintCodeEnteredTwo(QString)));
    }
    hideFormsExcept(_pfUsercode);

    disconnect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, 0);
    connect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, SLOT(OnCodeEnteredTwo(QString)));

    disconnect(_pfUsercode, SIGNAL(__FingerprintCodeEntered(QString)), psysController, 0);
    connect(_pfUsercode, SIGNAL(__FingerprintCodeEntered(QString)), psysController, SLOT(OnFingerprintCodeEnteredTwo(QString)));

    _pfUsercode->OnEnableKeyboard(true);
    _pfUsercode->SetDisplayFingerprintButton(_psystemController->getDisplayFingerprintButton());
    _pfUsercode->SetDisplayShowHideButton(_psystemController->getDisplayShowHideButton());    
    _pfUsercode->SetDisplayTakeReturnButtons(_psystemController->getDisplayTakeReturnButtons());  
    _pfUsercode->show();
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnDisplayThankYouDialog(QObject *psysController)
{
    Q_UNUSED(psysController);
}

void MainWindow::hideFormsExcept(QDialog * pfrm) 
{
    // KCB_DEBUG_ENTRY;
    if (&kkd != pfrm)
    {
        kkd.hide();
    }
    if(_pfUsercode && _pfUsercode != pfrm)
    {
        _pfUsercode->SetDisplayCodeEntryControls(false);   
        _pfUsercode->hide();
    }
    if(_pQuestions && _pQuestions != pfrm) 
    {
        _pQuestions->hide();
    }

    pfrm->activateWindow();
    pfrm->raise();
    pfrm->setFocus();
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnDisplayAdminPasswordDialog(QObject *psysController)
{
    // KCB_DEBUG_ENTRY;
    bool result = kcb::isVncConnectionActive();
    if (result)
    {
        kcb::TurnOffDisplay();
    }

    connect(psysController, SIGNAL(__OnClearEntry()), &kkd, SLOT(ClearText()));
    connect(&kkd, SIGNAL(NotifyEntered(QString)), psysController, SLOT(OnAdminPasswordEntered(QString)));
    connect(&kkd, SIGNAL(NotifyCancelled()), this, SLOT(OnAdminPasswordCancel()));
    connect(&kkd, SIGNAL(NotifyCancelled()), _psystemController, SLOT(OnAdminPasswordCancel()));
    connect(psysController, SIGNAL(__AdminSecurityCheckFailed()), this, SLOT(OnAdminSecurityCheckFailed()));

    hideFormsExcept(&kkd);
    kkd.resetPassword();  
    kkd.show();
    // KCB_DEBUG_EXIT;
}

/**
 * @brief MainWindow::OnAdminSecurityCheckFailed
 * Clears the text, adds and displays the message for 5000ms
 * After which the message is reset to <Enter Password> and the keyboard is reenabled automatically
 */
void MainWindow::OnAdminSecurityCheckFailed()
{
    // KCB_DEBUG_ENTRY;

    kkd.invalidPassword();
    kkd.show();
    hideFormsExcept(&kkd);

    QTimer::singleShot(5000, this, SLOT(ResetKeyboard()));
    // KCB_DEBUG_EXIT;
}

void MainWindow::ResetKeyboard()
{
    // KCB_DEBUG_ENTRY;
    kkd.resetPassword();
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnDisplayAdminMainDialog(QObject *psysController)
{
    if(!_pfAdminInfo)
    {
        SetupAdmin((CSystemController *)psysController);
    }

    hideFormsExcept(_pfAdminInfo);

    _pfAdminInfo->show();
}

void MainWindow::OnAdminPasswordCancel()
{
    // KCB_DEBUG_ENTRY;
    kcb::TurnOnDisplay();
    kkd.hide();
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnUserCodeCancel()
{
    // KCB_DEBUG_ENTRY;
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnUserCodeOne(QString sCode1)
{
    emit __onCode(sCode1);
}

void MainWindow::OnUserCodeTwo(QString sCode2)
{
    emit __onCode(sCode2);
}

void MainWindow::OnUserFingerprintCodeOne(QString sCode1)
{
    emit __onFingerprintCode(sCode1);
}

void MainWindow::OnUserFingerprintCodeTwo(QString sCode2)
{
    emit __onFingerprintCode(sCode2);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
}

void MainWindow::OnEnrollFingerprintDialog(QString sCode)
{
    Q_UNUSED(sCode);

    // KCB_DEBUG_ENTRY;

    _pdFingerprint->show();
    _pdFingerprint->setDefaultStage(1);
    _pdFingerprint->setMessage("");
    _pdFingerprint->setOkDisabled(true);

    // KCB_DEBUG_EXIT;
}

void MainWindow::OnQuestionUserDialog(QString lockNum, QString question1, QString question2, QString question3)
{
    // KCB_DEBUG_ENTRY;
    // Needed to force all other windows hidden.  This is a kludge -- hopefully temporary
    hideFormsExcept(_pQuestions);
    _pQuestions->setValues(lockNum, question1, question2, question3);
    // Needed to force the questions dialog to the top
    _pQuestions->raise();
    _pQuestions->show();
    // KCB_DEBUG_EXIT;
}

void MainWindow::OnQuestionUserDialogClose()
{
    // KCB_DEBUG_ENTRY;
    _pQuestions->hide();
    // KCB_DEBUG_EXIT;
}
