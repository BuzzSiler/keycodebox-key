#include "keycodeboxmain.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <iostream>
#include <cstdlib>

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

MainWindow      *gpmainWindow;

void MainWindow::ExtractCommandOutput(FILE *pF, std::string &rtnStr)
{
    char cChar = '\0';

    while(!feof(pF))
    {
        cChar = fgetc(pF);
        rtnStr += cChar;
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _psystemController(new CSystemController(this))
{
    ui->setupUi(this);
    QMainWindow::showFullScreen();
    QMainWindow::activateWindow();
    QMainWindow::raise();
    initialize();

    setAttribute(Qt::WA_AcceptTouchEvents, true);

    _pscene = new QGraphicsScene(this);
    ui->graphicsView->setScene(_pscene);
    _pPixmap = new QPixmap("/home/pi/kcb-config/images/alpha_logo.jpg");

    if( _pPixmap->isNull() )
    {
        qDebug() << "Failed to load image, loading backup!";

        _pPixmap = new QPixmap("/home/pi/kcb-config/images/alpha_logo_touch.jpg");
        if(_pPixmap->isNull()) {
            qDebug() << "Failed to load image!";
        }
    }

    // Scale the image...
    _pPixmapItem = new CClickableGraphicsItem(_pPixmap->scaled(760, 390));
    _pscene->addItem(_pPixmapItem);
    ui->graphicsView->show();

    ui->graphicsView->setClickedFunc(&(this->OnImageClicked) );

    QDateTime currdt = QDateTime::currentDateTime();
    QDateTime dt = CEncryption::roundDateTime(10, currdt);

    connect(_psystemController, SIGNAL(__OnDisplayCodeDialog(QObject*)), this, SLOT(OnDisplayCodeDialog(QObject*)));
    connect(_psystemController, SIGNAL(__OnDisplayUserCodeTwoDialog(QObject*)), this, SLOT(OnDisplayUserCodeTwoDialog(QObject*)));
    connect(_psystemController, SIGNAL(__OnDisplayAdminPasswordDialog(QObject*)), this, SLOT(OnDisplayAdminPasswordDialog(QObject*)));
    connect(_psystemController, SIGNAL(__OnDisplayAdminMainDialog(QObject*)), this, SLOT(OnDisplayAdminMainDialog(QObject*)));

    connect(&_sysControlThread, SIGNAL(started()), _psystemController, SLOT(start()));

    connect(this, SIGNAL(__TouchScreenTouched()), _psystemController, SLOT(OnTouchScreenTouched()));
    connect(_psystemController, SIGNAL(__OnDisplayTimeoutScreen()), this, SLOT(OnDisplayTimeoutScreen()));

    connect(_psystemController, SIGNAL(__onUserCodeOne(QString)), this, SLOT(OnUserCodeOne(QString)));
    connect(_psystemController, SLOT(__onUserCodeTwo(QString)), this, SLOT(OnUserCodeTwo(QString)));

    connect(_psystemController, SIGNAL(__onUserFingerprintCodeOne(QString)), this, SLOT(OnUserFingerprintCodeOne(QString)));
    connect(_psystemController, SLOT(__onUserFingerprintCodeTwo(QString)), this, SLOT(OnUserFingerprintCodeTwo(QString)));

    _psystemController->setMainWindow(this);
    _sysControlThread.setSystemController(_psystemController);

    gpmainWindow = this;

    qDebug() << "MainWindow: moveToThread.";
    _psystemController->moveToThread(&_sysControlThread);

    _sysControlThread.start();
}

bool MainWindow::isInternetTime()
{
    FILE *pF;
    std::string sOutput = "";
    QString flagListCmd = "ls /home/pi/run/* | grep 'flag'";

    pF = popen(flagListCmd.toStdString().c_str(), "r");
    if(!pF)
    {
        qDebug() << "failed to list system flags";
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    qDebug() << "MainWindow::isInternetTime(), " << QString::fromStdString(sOutput);
    qDebug() << "MainWindow::isInternetTime(), " << QString::number(sOutput.find("internetTime"));

    if( sOutput.find("internetTime") != std::string::npos )
        return true;

    qDebug() << "MainWindow::isInternetTime(), internetTime FALSE";
    return false;
}

void MainWindow::initialize() {
    _pfUsercode = 0;
    _pfAdminPW = 0;
    _pfAdminInfo = 0;
    _pdFingerprint = 0;
    _pdFingerprintVerify = 0;
    _pQuestions = 0;

    QCursor::setPos(848, 480);
    QApplication::setOverrideCursor(Qt::BlankCursor);

    if( isInternetTime() )
    {
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), ntp ON";
        std::system("sudo /etc/init.d/ntp stop");

        QCoreApplication::processEvents();
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), ntpd -s";
        std::system("sudo ntpd -s");
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), ntp OFF";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _psystemController;
}

void MainWindow::OnImageClicked()
{
    qDebug() << "OnImageClicked";
    emit gpmainWindow->__TouchScreenTouched();
}

void MainWindow::OnDisplayTimeoutScreen()
{
    // hide any open screens to show the touch screen to start
    if(_pfUsercode) 
    {
        _pfUsercode->hide();
    }
    if(_pfAdminPW) 
    {
        _pfAdminPW->hide();
    }
    if(_pfAdminInfo) 
    {
        _pfAdminInfo->hide();
    }
    if(_pdFingerprint) 
    {
        _pdFingerprint->hide();
    }
    if(_pdFingerprintVerify) 
    {
        _pdFingerprintVerify->hide();
    }
    if(_pQuestions) 
    {
        _pQuestions->hide();
    }
}

void MainWindow::OnDisplayCodeDialog(QObject *psysController)
{
    if(!_pfUsercode)
    {
        qDebug() << "MainWindow::OnDisplayCodeDialog(): new CFrmUserCode";
        _pfUsercode = new CFrmUserCode();
        connect(psysController, SIGNAL(__OnNewMessage(QString)), _pfUsercode, SLOT(OnNewMessage(QString)));
        connect(psysController, SIGNAL(__OnClearEntry()), _pfUsercode, SLOT(OnClearCodeDisplay()));
        connect(psysController, SIGNAL(__OnEnableKeypad(bool)), _pfUsercode, SLOT(OnEnableKeyboard(bool)));
        connect(psysController, SIGNAL(__OnCodeMessage(QString)), _pfUsercode, SLOT(OnNewCodeMessage(QString)));

        connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), psysController, SLOT(OnUserCodeCancel()));
        connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), this, SLOT(OnUserCodeCancel()));
        connect(this, SIGNAL(__onCode(QString)), _pfUsercode, SLOT(OnSwipeCode(QString)));
        connect(this, SIGNAL(__onFingerprintCode(QString)), psysController, SLOT(OnFingerprintCodeEntered(QString)));

        connect(psysController, SIGNAL(__onEnrollFingerprintDialog(QString)), this, SLOT(OnEnrollFingerprintDialog(QString)));
        connect(psysController, SIGNAL(__onQuestionUserDialog(QString,QString,QString,QString)), this, SLOT(OnQuestionUserDialog(QString,QString,QString,QString)));

        connect(_pfUsercode, SIGNAL(__onVerifyFingerprint()), psysController, SLOT(OnVerifyFingerprint()));
        connect(_pfUsercode, SIGNAL(__onVerifyFingerprintDialog()), this, SLOT(OnVerifyFingerprintDialog()));
        connect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, SLOT(OnCodeEntered(QString)));


        if( !_pdFingerprint )
        {
            _pdFingerprint = new CDlgFingerprint();
            connect(_pdFingerprint, SIGNAL(__onEnrollFingerprintDialogCancel()), psysController, SLOT(EnrollFingerprintDialogCancel()));
            connect(psysController, SIGNAL(__onUpdateEnrollFingerprintDialog(int, int, QString)), _pdFingerprint, SLOT(OnUpdateEnrollFingerprintDialog(int, int, QString)));
            _pdFingerprint->hide();
        }

        if( !_pdFingerprintVerify )
        {
            _pdFingerprintVerify = new CDlgFingerprintVerify();
            connect(_pdFingerprintVerify, SIGNAL(__onVerifyFingerprintDialogCancel()), psysController, SLOT(OnVerifyFingerprintDialogCancel()));
            connect(psysController, SIGNAL(__onUpdateVerifyFingerprintDialog(bool, QString)), _pdFingerprintVerify, SLOT(OnUpdateVerifyFingerprintDialog(bool, QString)));
            _pdFingerprintVerify->hide();
        }

        if( !_pQuestions )
        {
            _pQuestions = new CDlgQuestions();
            _pQuestions->hide();
            connect(psysController, SIGNAL(__onQuestionUserDialog(QString,QString,QString,QString)), this, SLOT(OnQuestionUserDialog(QString,QString,QString,QString)));
            connect(_pQuestions, SIGNAL(__OnQuestionsCancel()), psysController, SLOT(QuestionUserCancel()));
            connect(_pQuestions, SIGNAL(__OnQuestionsSave(QString,QString,QString,QString)), psysController, SLOT(AnswerUserSave(QString,QString,QString,QString)));
            connect(_pQuestions, SIGNAL(__OnQuestionsClose()), this, SLOT(OnQuestionUserDialogClose()));
        }

        _pfUsercode->SetDisplayFingerprintButton(_psystemController->getDisplayFingerprintButton());
        _pfUsercode->SetDisplayShowHideButton(_psystemController->getDisplayShowHideButton());
    }
    else
    {
        hideFormsExcept(_pfUsercode);

        disconnect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, 0);
        connect(_pfUsercode, SIGNAL(__CodeEntered(QString)), psysController, SLOT(OnCodeEntered(QString)));

        disconnect(_pfUsercode, SIGNAL(__FingerprintCodeEntered(QString)), psysController, 0);
        connect(_pfUsercode, SIGNAL(__FingerprintCodeEntered(QString)), psysController, SLOT(OnFingerprintCodeEntered(QString)));

        qDebug() << "MainWindow::OnDisplayCodeDialog()";

        _pfUsercode->OnEnableKeyboard(true);
        _pfUsercode->SetDisplayFingerprintButton(_psystemController->getDisplayFingerprintButton());
        _pfUsercode->SetDisplayShowHideButton(_psystemController->getDisplayShowHideButton());
        _pfUsercode->show();
    }
}

void MainWindow::OnDisplayUserCodeTwoDialog(QObject *psysController)
{
    if(!_pfUsercode) {
        qDebug() << "MainWindow::OnDisplayUserCodeTwoDialog()";
        _pfUsercode = new CFrmUserCode();
        connect(psysController, SIGNAL(__OnNewMessage(QString)), _pfUsercode, SLOT(OnNewMessage(QString)));
        connect(psysController, SIGNAL(__OnClearEntry()), _pfUsercode, SLOT(OnClearCodeDisplay()));
        connect(psysController, SIGNAL(__OnEnableKeypad(bool)), _pfUsercode, SLOT(OnEnableKeyboard(bool)));
        connect(psysController, SIGNAL(__OnCodeMessage(QString)), _pfUsercode, SLOT(OnNewCodeMessage(QString)));
        connect(psysController, SIGNAL(__OnDisplayFingerprintButton(bool)), _pfUsercode, SLOT(OnDisplayFingerprintButton(bool)));
        connect(psysController, SIGNAL(__OnDisplayShowHideButton(bool)), _pfUsercode, SLOT(OnDisplayShowHideButton(bool)));

        connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), psysController, SLOT(OnUserCodeCancel()));
        connect(_pfUsercode, SIGNAL(__OnUserCodeCancel()), this, SLOT(OnUserCodeCancel()));
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
    _pfUsercode->show();
}

void MainWindow::OnDisplayThankYouDialog(QObject *psysController)
{
    Q_UNUSED(psysController);
}

void MainWindow::hideFormsExcept(QDialog * pfrm) {
    if(_pfAdminPW && _pfAdminPW != pfrm)
    {
        _pfAdminPW->hide();
    }
    if(_pfUsercode && _pfUsercode != pfrm)
        _pfUsercode->hide();
    pfrm->activateWindow();
    pfrm->raise();
    pfrm->setFocus();
}

void MainWindow::OnDisplayAdminPasswordDialog(QObject *psysController)
{
    if(!_pfAdminPW) {
        qDebug() << "MainWindow::OnDisplayUserCodeTwoDialog()";
        _pfAdminPW = new CFrmAdminPassword();
    }
    hideFormsExcept(_pfAdminPW);
    connect(psysController, SIGNAL(__OnClearEntry()), _pfAdminPW, SLOT(OnClearCodeDisplay()));
    disconnect(_pfAdminPW, SIGNAL(__PasswordEntered(QString)), psysController, 0);
    connect(_pfAdminPW, SIGNAL(__PasswordEntered(QString)), psysController, SLOT(OnAdminPasswordEntered(QString)));
    connect(psysController, SIGNAL(__AdminSecurityCheckFailed()), this, SLOT(OnAdminSecurityCheckFailed()));
    connect(_pfAdminPW, SIGNAL(__OnAdminPasswordCancel()), this, SLOT(OnAdminPasswordCancel()));
    connect(_pfAdminPW, SIGNAL(__OnAdminPasswordCancel()), _psystemController, SLOT(OnAdminPasswordCancel()));
    _pfAdminPW->OnEnableKeyboard(true);
    _pfAdminPW->show();
}

/**
 * @brief MainWindow::OnAdminSecurityCheckFailed
 * Clears the text, adds and displays the message for 5000ms
 * After which the message is reset to <Enter Password> and the keyboard is reenabled automatically
 */
void MainWindow::OnAdminSecurityCheckFailed()
{
    _pfAdminPW->OnNewMessage(tr("Incorrect Password"), 5000);
}

void MainWindow::OnDisplayAdminMainDialog(QObject *psysController)
{
    if(!_pfAdminInfo)
    {
        _pfAdminInfo = new CFrmAdminInfo();
        _pfAdminInfo->setSystemController((CSystemController*)psysController);
    }
    connect(_pfAdminInfo, SIGNAL(__OnOpenLockRequest(QString, bool)), _psystemController, SLOT(OnOpenLockRequest(QString, bool)));
    connect(_psystemController, SIGNAL(__onUserCodes(QString,QString)), _pfAdminInfo, SLOT(OnCodes(QString, QString)));
    connect(_pfAdminInfo, SIGNAL(__OnDisplayFingerprintButton(bool)), _pfUsercode, SIGNAL(__OnDisplayFingerprintButton(bool)));
    connect(_pfAdminInfo, SIGNAL(__OnDisplayShowHideButton(bool)), _pfUsercode, SIGNAL(__OnDisplayShowHideButton(bool)));
    connect(_pfAdminInfo, SIGNAL(__OnSendTestEmail(int)), _psystemController, SLOT(OnSendTestEmail(int)));
    connect(_psystemController, SIGNAL(__OnDisplayFingerprintButton(bool)), _pfAdminInfo, SLOT(OnDisplayFingerprintButton(bool)));
    connect(_psystemController, SIGNAL(__OnDisplayShowHideButton(bool)), _pfAdminInfo, SLOT(OnDisplayShowHideButton(bool)));

    hideFormsExcept(_pfAdminInfo);

    _pfAdminInfo->show();
}

void MainWindow::OnAdminPasswordCancel()
{
    qDebug() << "MainWindow::OnAdminPasswordCancel()";
    _pfAdminPW->hide();
    _pfAdminPW->OnEnableKeyboard(true);
}

void MainWindow::OnUserCodeCancel()
{
    qDebug() << "MainWindow::OnAdminPasswordCancel()";

}

void MainWindow::OnUserCodeOne(QString sCode1)
{
    qDebug() << "MainWindow::OnUserCodeTwo" << sCode1;
    emit __onCode(sCode1);
}

void MainWindow::OnUserCodeTwo(QString sCode2)
{
    qDebug() << "MainWindow::OnUserCodeTwo" << sCode2;
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

    qDebug() << "MainWindow::OnEnrollFingerprintDialog()";

    _pdFingerprint->show();
    _pdFingerprint->setDefaultStage(1);
    _pdFingerprint->setMessage("");
    _pdFingerprint->setOkDisabled(true);
}

void MainWindow::OnQuestionUserDialog(QString lockNum, QString question1, QString question2, QString question3)
{
    KCB_DEBUG_ENTRY;
    _pQuestions->setValues(lockNum, question1, question2, question3);
    _pQuestions->show();
    KCB_DEBUG_EXIT;
}

void MainWindow::OnQuestionUserDialogClose()
{
    _pQuestions->hide();
}

void MainWindow::OnVerifyFingerprintDialog()
{
    KCB_DEBUG_ENTRY;
    _pdFingerprintVerify->show();
    _pdFingerprintVerify->setMessage("");
    KCB_DEBUG_EXIT;
}
