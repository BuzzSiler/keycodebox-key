#include "frmadmininfo.h"
#include "ui_frmadmininfo.h"

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include "kcbcommon.h"
#include <QtGlobal>
#include <QDebug>
#include <QVariant>
#include <QMenu>
#include <QList>
#include <QStringList>
#include <QMultiMap>
#include <QMessageBox>
#include <QAction>
#include <QLabel>
#include <QDirModel>
#include <QFileInfo>
#include <QTouchEvent>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QFileDialog>
#include <QFile>
#include <QScreen>
#include <QProgressDialog>
#include <QColor>

#include "lockset.h"
#include "lockstate.h"
#include "menupushbutton.h"
#include <exception>
#include "encryption.h"
#include <unistd.h>
#include "selectlockswidget.h"
#include "kcbkeyboarddialog.h"
#include "reportcontrolwidget.h"
#include "kcbsystem.h"
#include "keycodeboxsettings.h"
#include "frmnetworksettings.h"

#include "xmlcodelistingreader.h"
#include "codeelement.h"
#include "codeexporter.h"
#include "codeimporter.h"
#include "logger.h"

#include "systemcontroller.h"
#include "lockcontroller.h"

#include "cabinetrowdelegate.h"
#include "autocodegenwidget.h"
#include "autocodegenstatic.h"


#define ADMIN_TAB_INDEX (0)
#define UTILITIES_TAB_INDEX (1)
#define REPORT_TAB_INDEX (2)
#define DOORS_TAB_INDEX (3)
#define CODES_TAB_INDEX (4)
#define CODE_HISTORY_TAB_INDEX (5)
#define AUTOCODE_TAB_INDEX (6)
#define SYSTEM_TAB_INDEX (7)

#define ACTION_INDEX_INSTALL_APP (0)
#define ACTION_INDEX_SET_BRANDING_IMAGE (1)
#define ACTION_INDEX_DEFAULT_BRANDING_IMAGE (2)
#define ACTION_INDEX_IMPORT_CODES (3)
#define ACTION_INDEX_EXPORT_CODES (4)
#define ACTION_INDEX_EXPORT_LOGS (5)

#define CODE_SELECTION_SINGLE_INDEX (0)
#define CODE_SELECTION_MULTI_INDEX (1)

static const QStringList INSTALL_APP_FILTER = {"Alpha*"};
static const QStringList BRANDING_IMAGE_FILTER = {"*.jpg", "*.jpeg"};
static const QStringList IMPORT_CODES_FILTER = {"*.xml", "*.json", "*.csv", "*.sql"};
static const QStringList IMPORT_CODES_XML_FILTER = {"*.xml"};
static const QStringList IMPORT_CODES_JSON_FILTER = {"*.json"};
static const QStringList IMPORT_CODES_CSV_FILTER = {"*.csv"};
static const QStringList IMPORT_CODES_SQL_FILTER = {"*.sql"};

static const char CMD_REMOVE_ALL_FP_FILES[] = "sudo rm -rf /home/pi/run/prints/*";
static const char CMD_LIST_SYSTEM_FLAGS[] = "ls /home/pi/run/* | grep 'flag'";
static const char CMD_READ_TIME_ZONE[] = "readlink /etc/localtime";
static const QString CMD_REMOVE_FP_FILE = "sudo rm -rf /home/pi/run/prints/\%1";

static const QString red("color: rgb(255, 0, 0);");
static const QString blue("color: rgb(0, 0, 255);");
static const QString med_sea_green("color: rgb(60,179,113);");
static const QString white("color: rgb(255, 255, 255);");

static const QString CAN_MULTICAST_STYLESHEET = QString("QLabel { background-color : rgb(60,179,113); color : blue; }");
static const QString CANNOT_MULTICAST_STYLESHEET = QString("QLabel { background-color : rgb(60,179,113): color : white; }");
static const QString IP_ADDRESS_ERROR = red;

static const int DISPLAY_POWER_DOWN_TIMEOUT[] = {
     0,             // None
    10 * 1000,      // 10 seconds
    30 * 1000,      // 30 seconds
    60 * 1000,      // 1 mins
    5 * 60 * 1000,  // 5 mins
    10 * 60 * 1000, // 10 mins
    30 * 60 * 1000, // 30 mins
};

void CFrmAdminInfo::ExtractCommandOutput(FILE *pF, std::string &rtnStr)
{
    char cChar = '\0';

    while(!feof(pF))
    {
        cChar = fgetc(pF);
        rtnStr += cChar;
    }
}

CFrmAdminInfo::CFrmAdminInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFrmAdminInfo), 
    _pworkingSet(0),
    _testEmail(EMAIL_INVALID),
    m_select_locks(* new SelectLocksWidget(this, SelectLocksWidget::ADMIN)),
    m_report(* new ReportControlWidget(this)),
    m_file_filter{0},
    m_util_action(UTIL_ACTION_INSTALL_APP),
    m_model(* new QStandardItemModel(0, 6)),
    m_autocodegen(* new AutoCodeGenWidget(this)),
    m_last_index(0)
{
    // KCB_DEBUG_ENTRY;
    ui->setupUi(this);

    setAttribute(Qt::WA_AcceptTouchEvents, true);


    kcb::SetWindowParams(this);

    // For whatever reason, the tabWidget will not take on the geometry of the parent (not wide enough).
    // Consequently, it is necessary to explicitly set the geometry to be the same as the parent.
    ui->tabWidget->setGeometry(parent->x(), parent->y(), parent->width(), parent->height());

    ui->pgbDiscoverHardware->setVisible(false);

    initialize();

    QStringList headers;
    headers << "Model" << "First Lock" << "Last Lock" << "Total Locks" << "Board Addr" << "SW Version";

    m_model.setHorizontalHeaderLabels(headers);
    ui->tvCabinets->setModel(&m_model);
    ui->tvCabinets->verticalHeader()->hide();
    ui->tvCabinets->verticalHeader()->setFixedWidth(60);
    ui->tvCabinets->horizontalHeader()->setFixedHeight(50);
    ui->tvCabinets->horizontalHeader()->setStretchLastSection(true);
    ui->tvCabinets->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->tvCabinets->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(&m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));

    ui->pbResetCabinetConfig->setDisabled(true);

    bool autocode_enabled = AutoCodeGeneratorStatic::IsEnabled();
    bool lockselection_enabled = KeyCodeBoxSettings::IsLockSelectionEnabled();
    bool lockselection_display = !autocode_enabled && lockselection_enabled;

    ui->cbAdminLockSelection->setEnabled(lockselection_display);
    if (KeyCodeBoxSettings::IsLockSelectionSingle())
    {
        ui->cbAdminLockSelection->setCurrentIndex(0);
    }
    else if (KeyCodeBoxSettings::IsLockSelectionMulti())
    {
        ui->cbAdminLockSelection->setCurrentIndex(1);
    }
    else
    {
        ui->cbAdminLockSelection->setDisabled(true);
    }

    ui->tblCodesList->setSortingEnabled(true);

    emit __UpdateCurrentAdmin(&_tmpAdminRec);
    // KCB_DEBUG_EXIT;
}

CFrmAdminInfo::~CFrmAdminInfo()
{
    delete ui;

    if(_pcopymodel) 
    {
        delete _pcopymodel;
    }

    if(_phistoryWorkingSet) 
    {
        _phistoryWorkingSet->clearSet();
        delete _phistoryWorkingSet;
        _phistoryWorkingSet = 0;
    }

    if(_pworkingSet) 
    {
        _pworkingSet->clearSet();
        delete _pworkingSet;
        _pworkingSet = nullptr;
    }
}

void CFrmAdminInfo::initializeConnections()
{
    // KCB_DEBUG_ENTRY;
    connect(this, SIGNAL(__OnRequestCurrentAdmin()), _psysController, SLOT(OnRequestCurrentAdmin()));

    connect(_psysController, SIGNAL(__OnRequestedCurrentAdmin(CAdminRec*)), this, SLOT(OnRequestedCurrentAdmin(CAdminRec*)));
    connect(this, SIGNAL(__UpdateCurrentAdmin(CAdminRec*)), _psysController, SLOT(OnUpdateCurrentAdmin(CAdminRec*)));
    connect(_psysController, SIGNAL(__OnUpdatedCurrentAdmin(bool)), this, SLOT(OnUpdatedCurrentAdmin(bool)));

    connect(this, SIGNAL(__OnCloseFrmAdmin()), _psysController, SLOT(OnAdminDialogClosed()));

    connect(this, SIGNAL(__OnBrightnessChanged(int)), _psysController, SLOT(OnBrightnessChanged(int)));
    connect(this, SIGNAL(__OnReadLockSet(QString,QDateTime,QDateTime)), _psysController, SLOT(OnReadLockSet(QString,QDateTime,QDateTime)));
    connect(_psysController, SIGNAL(__OnLockSet(CLockSet*)), this, SLOT(OnLockSet(CLockSet*)));

    connect(this, SIGNAL(__OnReadLockHistorySet(QString,QDateTime,QDateTime)), _psysController, SLOT(OnReadLockHistorySet(QString,QDateTime,QDateTime)));
    connect(_psysController, SIGNAL(__OnLockHistorySet(CLockHistorySet*)), this, SLOT(OnLockHistorySet(CLockHistorySet*)));
    connect(this, SIGNAL(__OnImmediateReportRequest(QDateTime,QDateTime)), _psysController, SLOT(OnImmediateReportRequest(QDateTime,QDateTime)));

    connect(this, SIGNAL(__OnUpdateCodeState(CLockState*)), _psysController, SLOT(OnUpdateCodeState(CLockState*)));
    connect(_psysController, SIGNAL(__OnUpdatedCodeState(bool)), this, SLOT(OnUpdatedCodeState(bool)));

    connect(ui->tblCodesList, SIGNAL( cellClicked (int, int) ), this, SLOT( codeCellSelected( int, int ) ) );

    connect(ui->cbTimeZone, SIGNAL(currentIndexChanged(QString)), this, SLOT(setTimeZone()));

    connect(_psysController, SIGNAL(__OnFoundNewStorageDevice(QString, QString)), this, SLOT(OnFoundNewStorageDevice(QString, QString)));

    connect(ui->chkDisplayFingerprintButton, SIGNAL(toggled(bool)), this, SIGNAL(__OnDisplayFingerprintButton(bool)));
    connect(ui->chkDisplayShowHideButton, SIGNAL(toggled(bool)), this, SIGNAL(__OnDisplayShowHideButton(bool)));
    connect(ui->chkDisplayTakeReturnButtons, SIGNAL(toggled(bool)), this, SIGNAL(__OnDisplayTakeReturnButtons(bool)));

    connect(&m_select_locks, &SelectLocksWidget::NotifyRequestLockOpen, this, &CFrmAdminInfo::OnOpenLockRequest);

    connect(&m_report, &ReportControlWidget::NotifyGenerateReport, this, &CFrmAdminInfo::OnNotifyGenerateReport);

    connect( ui->tblHistory, SIGNAL( cellClicked(int, int) ), this, SLOT( codeHistoryTableCellSelected( int, int ) ) );
    connect( ui->cbUsbDrives, SIGNAL(currentIndexChanged(QString) ), this, SLOT(on_cbUsbDrives_currentIndexChanged(QString) ) );

    connect(_psysController, SIGNAL(DiscoverHardwareProgressUpdate(int)), this, SLOT(OnDiscoverHardwareProgressUpdate(int)));

    connect(&m_autocodegen, &AutoCodeGenWidget::RequestCodes1, this, &CFrmAdminInfo::OnRequestCodes1);

    connect(&m_autocodegen, &AutoCodeGenWidget::CommitCodes1, this, &CFrmAdminInfo::OnCommitCodes1);
    connect(&m_autocodegen, &AutoCodeGenWidget::CommitCodes2, this, &CFrmAdminInfo::OnCommitCodes2);

    connect(&m_autocodegen, &AutoCodeGenWidget::NotifyDisableLockSelection, this, &CFrmAdminInfo::OnNotifyDisableLockSelection);
    connect(&m_autocodegen, &AutoCodeGenWidget::NotifyEnableLockSelection, this, &CFrmAdminInfo::OnNotifyEnableLockSelection);

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::setSystemController(CSystemController *psysController)
{
    _psysController = psysController;

    connect(&m_autocodegen, &AutoCodeGenWidget::NotifyAutoCodeEnabled, _psysController, &CSystemController::OnNotifyAutoCodeEnabled);
    connect(&m_autocodegen, &AutoCodeGenWidget::NotifyAutoCodeDisabled, _psysController, &CSystemController::OnNotifyAutoCodeDisabled);
    connect(&m_autocodegen, &AutoCodeGenWidget::NotifyAutoCodeEmailUpdate, _psysController, &CSystemController::__OnAutoCodeEmail);

    initializeConnections();
    emit __OnRequestCurrentAdmin();
}

void CFrmAdminInfo::show()
{
    QDialog::show();
    // KCB_DEBUG_TRACE("Admin type is: " << _psysController->getAdminType());
    if(_psysController->getAdminType() == "Assist")
    {
        ui->tabUtilities->setEnabled(false);
        ui->tabWidget->setTabEnabled(1, false);
        ui->gpAdminInfo->setVisible(false);
    }
    else if(_psysController->getAdminType() == "Admin")
    {
        ui->tabUtilities->setEnabled(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->gpAdminInfo->setVisible(true);
        ui->vloSelectLocks->addWidget(&m_select_locks);
        ui->hloReportSettings->addWidget(&m_report);
        ui->hloAutoCodeGen->addWidget(&m_autocodegen);
        // Force tabwidget to show administrator tab
        emit ui->tabWidget->currentChanged(ADMIN_TAB_INDEX);
    }
}

int CFrmAdminInfo::getDisplayPowerDownTimeout()
{
    quint16 index = ui->cbDisplayPowerDownTimeout->currentIndex();
    Q_ASSERT_X(index < sizeof(DISPLAY_POWER_DOWN_TIMEOUT), Q_FUNC_INFO, "Display power down timeout index out of range");
    return DISPLAY_POWER_DOWN_TIMEOUT[index];
}

bool CFrmAdminInfo::isInternetTime()
{
    return KeyCodeBoxSettings::IsInternetTimeEnabled();
}

void CFrmAdminInfo::initialize()
{
    _pcopymodel = 0;
    _bClose = false;
    m_util_action = UTIL_ACTION_INSTALL_APP;
    
    populateTimeZoneSelection(ui->cbTimeZone);
    startMediaTimer();

    ui->cbLockNum->setInsertPolicy(QComboBox::InsertAlphabetically);
    ui->cbUsbDrives->addItem("No Drive Inserted");

    ui->dtStartCodeList->setDisplayFormat(DATETIME_FORMAT);
    ui->dtEndCodeList->setDisplayFormat(DATETIME_FORMAT);
    ui->dtStartCodeHistoryList->setDisplayFormat(DATETIME_FORMAT);
    ui->dtEndCodeHistoryList->setDisplayFormat(DATETIME_FORMAT);
    
    ui->dtEndCodeList->setDateTime(QDateTime().currentDateTime());
    ui->dtEndCodeHistoryList->setDateTime(QDateTime().currentDateTime());

    setupCodeTableContextMenu();

    bool is_internettime = isInternetTime();
    if( is_internettime )
    {
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
        ui->dtSystemTime->setDisabled(is_internettime);
        ui->cbInternetTime->setChecked(is_internettime);
        ui->btnSetTime->setDisabled(is_internettime);
    }

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabSelected(int)));

    ui->tabWidget->setCurrentIndex(0);
}

QString CFrmAdminInfo::GetIpStylesheet(bool can_ping, bool can_multicast)
{
    QString stylesheet(IP_ADDRESS_ERROR);

    if (can_ping)
    {
        if (can_multicast)
        {
            stylesheet = CAN_MULTICAST_STYLESHEET;
        }
        else
        {
            stylesheet = CANNOT_MULTICAST_STYLESHEET;
        }
    }

    return stylesheet;
}

void CFrmAdminInfo::SetSystemIPAddressAndStatus()
{
    QString display_text("No Connection");
    QString ip_address("");
    bool can_ping = false;
    bool can_multicast = false;
    
    kcb::GetIpAddressAndStatus(ip_address, can_ping, can_multicast);

    if (!ip_address.isEmpty())
    {
        display_text = ip_address;
    }

    ui->lblIPAddress->setText(display_text);
    ui->lblIPAddress->setStyleSheet(GetIpStylesheet(can_ping, can_multicast));
}

void CFrmAdminInfo::startMediaTimer()
{
    connect(&_timerMedia, SIGNAL(timeout()), this, SLOT(OnMediaCheckTimeout()));
    _timerMedia.setInterval(5000);
    _timerMedia.start();
}

void CFrmAdminInfo::OnMediaCheckTimeout()
{
    _timerMedia.stop();

    SetSystemIPAddressAndStatus();

    _timerMedia.start();
}

void CFrmAdminInfo::populateTimeZoneSelection(QComboBox *cbox)
{
    FILE *pF;
    std::string sOutput = "";
    std::string sTimeZone = "";
    std::string parsedString = "";

    cbox->clear();
    QList<QByteArray> ids = QTimeZone::availableTimeZoneIds();
    foreach (QByteArray id, ids) 
    {
        cbox->addItem(id);
    }

    pF = popen(CMD_READ_TIME_ZONE, "r");
    if(!pF)
    {
        KCB_DEBUG_TRACE("failed to parse timezone string");
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    sTimeZone = sOutput.substr(20);

    if( sOutput.length() == 0)
    {
        sTimeZone = "America/New_York";
    }

    if( strcmp(sTimeZone.c_str(), "") )
    {
        char *tokString = strtok(const_cast<char*> (sTimeZone.c_str()), "\n");
        parsedString = tokString;

        int index = cbox->findText(QString::fromStdString(parsedString));
        if (index != -1)
        {
            cbox->setCurrentIndex(index);
        }
    }
    ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
    _currentTime = QDateTime().currentDateTime();

}

void CFrmAdminInfo::setTimeZone()
{
    // KCB_DEBUG_ENTRY;

    kcb::SetTimeZone(ui->cbTimeZone->currentText());

    if(ui->cbInternetTime->isChecked())
    {
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::onRootPathChanged(QString path)
{
    Q_UNUSED(path);
}

void CFrmAdminInfo::populateFileCopyWidget(QString sDirectory, QStringList sFilter)
{
    // KCB_DEBUG_ENTRY;
    if(_pcopymodel)
    {
        delete _pcopymodel;
        _pcopymodel = 0;
    }

    if(!_pcopymodel)
    {
        _pcopymodel = new QFileSystemModel();
    }

    QStringList list;
    list << sDirectory;
    if (sFilter.isEmpty())
    {
        list << m_file_filter;
    }
    else
    {
        list << sFilter;
    }

    //KCB_DEBUG_TRACE("dir" << sDirectory << "filter" << m_file_filter << "filter override" << sFilter << "list" << list);

    // Enable modifying file system
    _pcopymodel->setReadOnly(true);
    _pcopymodel->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    _pcopymodel->setNameFilters(list);
    _pcopymodel->setNameFilterDisables(false);

    connect(_pcopymodel,
            SIGNAL(directoryLoaded(QString)),
            this,
            SLOT(onCopyModelDirectoryLoaded(QString)));

    connect(_pcopymodel,
            SIGNAL(rootPathChanged(QString)),
            this,
            SLOT(onCopyRootPathChanged(QString)));

    auto index = _pcopymodel->setRootPath(sDirectory);
    ui->treeViewCopy->setModel(_pcopymodel);
    ui->treeViewCopy->setRootIndex(index);
    ui->treeViewCopy->header()->setSectionResizeMode(QHeaderView::Fixed);
    ui->treeViewCopy->header()->resizeSection(0, 300);
    ui->treeViewCopy->header()->resizeSection(1, 70);
    ui->treeViewCopy->header()->resizeSection(2, 60);
    ui->treeViewCopy->header()->resizeSection(3, 75);
    ui->treeViewCopy->header()->setStretchLastSection(false);

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::onCopyRootPathChanged(QString path)
{
    Q_UNUSED(path);
}

void CFrmAdminInfo::onCopyModelDirectoryLoaded(QString path)
{
    Q_UNUSED(path);
    if (_pcopymodel)
    {
        _pcopymodel->sort(0, Qt::AscendingOrder);
    }
}

void CFrmAdminInfo::on_treeViewCopy_clicked(const QModelIndex &index)
{
    //KCB_DEBUG_ENTRY;
    _copyDirectory = _pcopymodel->rootPath() + "/" + index.data(Qt::DisplayRole).toString();
    _copyDirectory = _pcopymodel->filePath ( index );

    if( QFileInfo(_copyDirectory).isFile())
    {
        ui->btnActionExecute->setEnabled(true);
    }
    else
    {
       ui->btnActionExecute->setEnabled(false);
    }

    //KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnUtilActionInstallApp()
{
    // KCB_DEBUG_TRACE(_copyDirectory);

    bool result = kcb::UpdateAppFile(_copyDirectory);

    if (result)
    {
        QMessageBox::information(this, tr("File Copied"), tr("New KeyCodeBox application copied.\nSystem requires reboot to run."));
    }
    else
    {
        QMessageBox::warning(this, tr("Invalid File"), tr("The file selected is not a valid KeyCodeBox application.\nNo changes have been made."));
    }
}

void CFrmAdminInfo::insertCodes(CodeListing& codeListing)
{
    // KCB_DEBUG_ENTRY;

    //codeListing.print();

    if (!_pworkingSet)
    {
        _pworkingSet = new CLockSet();
    }

    CodeListing::Iterator iter;
    for(iter = codeListing.begin(); iter != codeListing.end(); iter++)
    {
        Code* code = *iter;

        //code->print();

        QDateTime starttime = QDateTime::fromString(code->starttime(), DATETIME_FORMAT);
        QDateTime endtime = QDateTime::fromString(code->endtime(), DATETIME_FORMAT);

        /* Initial start/end dates (if present) based on access type
            There are three access types:
                - Always
                - Timed
                - Limited Use
            Start/end dates are only valid for 'Timed' codes.  All other codes should be DEFAULT_DATETIME
            While they are initialized above, it is possible that a customer will put a start/end
            datetime in the XML file.
            For display purposes, only keep date/times for 'Timed' codes.               
        */
        if (code->accesstype() == ACCESS_TYPE_ALWAYS || code->accesstype() == ACCESS_TYPE_LIMITED_USE)
        {
            starttime = DEFAULT_DATETIME;
            endtime = DEFAULT_DATETIME;
        }

        QString code1 = code->code1();
        QString code2 = code->code2();
        if (codeListing.encrypted())
        {
            code1 = CEncryption::decryptString(code1);
            code2 = CEncryption::decryptString(code2);
        }

        // Check if the code has been designated as a fingerprint
        bool code1_fp = false;
        bool code2_fp = false;
        if (code1.toLower().contains("fp"))
        {
            code1_fp = true;
            code1 = code1.split(" ")[0];
        }
        if (code2.toLower().contains("fp"))
        {
            code2_fp = true;
            code2 = code2.split(" ")[0];
        }


        _pState = createNewLockState();
        setPStateValues(code->locks(), 
                        code1, 
                        code2, 
                        code->username(),
                        starttime, 
                        endtime,
                        code1_fp, code2_fp,
                        code->askquestion(), 
                        code->question1(), 
                        code->question2(), 
                        code->question3(),
                        code->accesstype());
        HandleCodeUpdate();

    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnUtilActionSetBrandingImage()
{
    // KCB_DEBUG_ENTRY;

    QString createCmd = " cp '";
    int nRC = QMessageBox::warning(this, tr("Set New Branding Image"),
                                   tr("The new branding image will be set to the selected file and scaled to 760x390.\nDo you want to continue?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) {
        sync();
        createCmd += _copyDirectory;
        createCmd += "' /home/pi/kcb-config/images/alpha_logo.jpg";
        std::system(createCmd.toStdString().c_str());
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnUtilActionDefaultBrandingImage()
{
    // KCB_DEBUG_ENTRY;
    int nRC = QMessageBox::warning(this, tr("Verify Branding Image Reset"),
                                   tr("The branding image will be reset to the default.\nDo you want to continue?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) 
    {
        sync();
        std::system("cp /home/pi/kcb-config/images/alpha_logo_touch.jpg /home/pi/kcb-config/images/alpha_logo.jpg");
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::RunKeyboard(QString& text, bool numbersOnly)
{
    KcbKeyboardDialog kkd;

    kkd.numbersOnly(numbersOnly);
    kkd.setValue(text);
    if (kkd.exec())
    {
        text = kkd.getValue();
    }
}

void CFrmAdminInfo::on_lblName_clicked()
{
    QString text = ui->lblName->text();
    RunKeyboard(text);
    ui->lblName->setText(text);
}

void CFrmAdminInfo::on_lblEmail_clicked()
{
    QString text = ui->lblEmail->text();
    RunKeyboard(text);
    ui->lblEmail->setText(text);
}

void CFrmAdminInfo::on_lblPhone_clicked()
{
    QString text = ui->lblPhone->text();
    RunKeyboard(text);
    ui->lblPhone->setText(text);
}

void CFrmAdminInfo::on_lblPassword_clicked()
{
    QString text = ui->lblPassword->text();
    RunKeyboard(text);
    ui->lblPassword->setText(text);
}

void CFrmAdminInfo::on_lblAssistPassword_clicked()
{
    QString text = ui->lblAssistPassword->text();
    RunKeyboard(text);
    ui->lblAssistPassword->setText(text);
}

void CFrmAdminInfo::updateTmpAdminRec()
{
    _tmpAdminRec.setAdminName(ui->lblName->text());
    _tmpAdminRec.setAdminEmail(ui->lblEmail->text());
    _tmpAdminRec.setAdminPhone(ui->lblPhone->text());
    _tmpAdminRec.setPassword(ui->lblPassword->text());
    _tmpAdminRec.setAssistPassword(ui->lblAssistPassword->text());
    _tmpAdminRec.setDisplayFingerprintButton(ui->chkDisplayFingerprintButton->isChecked());
    _tmpAdminRec.setDisplayShowHideButton(ui->chkDisplayShowHideButton->isChecked());
    _tmpAdminRec.setDisplayPowerDownTimeout(ui->cbDisplayPowerDownTimeout->currentIndex());
    _tmpAdminRec.setDisplayTakeReturnButtons(ui->chkDisplayTakeReturnButtons->isChecked());
}

void CFrmAdminInfo::on_btnDone_clicked()
{
    // KCB_DEBUG_ENTRY;
    kcb::TurnOnDisplay();

    // Update the Admin Info and close the dialog - syscontroller needs to switch
    updateTmpAdminRec();
    _bClose = true;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    // KCB_DEBUG_ENTRY;

    if (adminInfo)
    {
        KCB_DEBUG_TRACE("Admin Info received.");
        _tmpAdminRec = *adminInfo;

        ui->lblName->setText(adminInfo->getAdminName());
        ui->lblEmail->setText(adminInfo->getAdminEmail());
        ui->lblPhone->setText(adminInfo->getAdminPhone());
        ui->lblPassword->setText(adminInfo->getPassword());
        ui->lblAssistPassword->setText(adminInfo->getAssistPassword());
        ui->chkDisplayFingerprintButton->setChecked(adminInfo->getDisplayFingerprintButton());
        ui->chkDisplayShowHideButton->setChecked(adminInfo->getDisplayShowHideButton());
        ui->cbDisplayPowerDownTimeout->setCurrentIndex(adminInfo->getDisplayPowerDownTimeout());
        ui->chkDisplayTakeReturnButtons->setChecked(adminInfo->getDisplayTakeReturnButtons());

        // Temporary to complete report widget funcationality
        _tmpAdminRec.setDefaultReportDeleteFreq(MONTHLY);

        m_report.setValues(_tmpAdminRec);

        // KCB_DEBUG_TRACE("Setting Current Index to 0");
        // ui->tabWidget->setCurrentIndex(0);

    }
    else
    {
        KCB_DEBUG_TRACE("Admin Info received is blank.");
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnUpdatedCurrentAdmin(bool bSuccess)
{
    // KCB_DEBUG_ENTRY;
    if( bSuccess )
    {
        // KCB_DEBUG_TRACE("Succeeded in updating Current Admin Info");

        /* Note: This is such an ugly hack.  I hate it. 
           I think the better way is to separate "Save and Close" so that
           the user can "Save" without closing and then you can enable
           the "Test Email" button once it is saved.
        */
        if (_testEmail == EMAIL_ADMIN_RECV || _testEmail == EMAIL_ADMIN_SEND)
        {
            emit __OnSendTestEmail((int) _testEmail);
        }
        _testEmail = EMAIL_INVALID;
    } 
    else 
    {
        KCB_DEBUG_TRACE("Failed to update Current Admin Info");
    }

    if(_bClose)
    {
        emit __OnCloseFrmAdmin();
        this->close();
    }
    else
    {
        emit __OnRequestCurrentAdmin();
    }

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnFoundNewStorageDevice(QString device0, QString device1)
{
    // KCB_DEBUG_ENTRY;
    _pcopymodel = 0;
    usbDevice0 = QString(device0);
    usbDevice1 = QString(device1);

    QStringList list;

    if (!usbDevice0.isEmpty())
    {
        list << QString("/media/pi/%1").arg(usbDevice0);
    }
    
    if (!usbDevice1.isEmpty())
    {
        list << QString("/media/pi/%1").arg(usbDevice1);
    }

    // KCB_DEBUG_TRACE("USBDevices" << list);
    m_report.OnNotifyUsbDrive(list);
    OnNotifyUsbDrive(list);    
}

void CFrmAdminInfo::OnUpdatedCodeState(bool bSuccess)
{
    if(bSuccess)
    {
        _pState->clearModified();
        if(_pFrmCodeEditMulti)
        {
            _pFrmCodeEditMulti->hide();
        }
        displayInTable(_pworkingSet);
    } 
    else 
    {
        KCB_DEBUG_TRACE("unsuccessful code state update");
    }
    on_btnReadCodes_clicked();
}

void CFrmAdminInfo::on_cbInternetTime_clicked()
{
    bool is_checked = ui->cbInternetTime->isChecked();
    ui->btnSetTime->setDisabled(is_checked);
    ui->dtSystemTime->setDisabled(is_checked);
    ui->cbTimeZone->setDisabled(is_checked);

    if (ui->cbInternetTime->isChecked())
    {
        KeyCodeBoxSettings::EnableInternetTime();

        kcb::EnableInternetTime();
        setTimeZone();
    }
    else
    {
        KeyCodeBoxSettings::DisableInternetTime();
        kcb::DisableInternetTime();
    }

    ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
}

void CFrmAdminInfo::on_btnSetTime_clicked()
{
    setTime();
    setTimeZone();
}

void CFrmAdminInfo::OnCloseAdmin() 
{
    this->close();
}

void CFrmAdminInfo::OnLockSet(CLockSet *pSet)
{
    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");
    // if (pSet != nullptr)
    // {
    //     KCB_DEBUG_TRACE("Count" << pSet->getLockMap()->size());
    // }

    displayInTable(pSet);
    
    pSet = nullptr;
}

void CFrmAdminInfo::OnLockHistorySet(CLockHistorySet *pSet)
{
    // KCB_DEBUG_ENTRY;

    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");

    displayInHistoryTable(pSet);
    
    pSet = nullptr;

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::createCodeTableHeader()
{
    // KCB_DEBUG_ENTRY;

    QTableWidget *table = ui->tblCodesList;

    // KCB_DEBUG_TRACE("table width" << table->horizontalHeader()->width());

    int hh_width = table->horizontalHeader()->width();
    int delta = hh_width / 20;

    table->clear();
    table->setColumnCount(7);

    table->setColumnWidth(0, 1 * delta);
    table->setColumnWidth(1, 2 * delta);
    table->setColumnWidth(2, 4 * delta);
    table->setColumnWidth(3, 3 * delta);
    table->setColumnWidth(4, 3 * delta);
    table->setColumnWidth(5, 4 * delta);
    table->setColumnWidth(6, 4 * delta);

    table->verticalHeader()->hide();
    
    QStringList headers;
    headers << "Line" << "Locks" << "Username" << "Code#1" << "Code#2" << "Start Time" << "End Time";

    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setFixedWidth(60);
    table->horizontalHeader()->setFixedHeight(50);
    table->horizontalHeader()->setStretchLastSection(true);

    table->setStyleSheet("QTableView {selection-background-color: gray;}");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    // KCB_DEBUG_EXIT;
}

QStringList CFrmAdminInfo::FormatLocks(const QString& locks)
{
    QStringList formatted_locks;

    foreach (const auto& lock, locks.split(','))
    {
        if (!lock.isEmpty())
        {
            formatted_locks.append(QString("%0").arg(lock, 3, QChar('0')));
        }
    }
    return formatted_locks;
}

void CFrmAdminInfo::displayInTable(CLockSet *pSet)
{
    // KCB_DEBUG_ENTRY;

    createCodeTableHeader();

    ui->cbLockNum->clear();
    ui->cbLockNum->addItem(QString(tr("All Locks")));

    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");
    if (pSet == nullptr)
    {
        KCB_DEBUG_TRACE("pSet is null so there are no codes to display");
        return;
    }

    CLockSet::Iterator itor;
    CLockState  *pState;
    _pworkingSet = pSet;
    int nRow = 0;
    int nCol = 0;
    QStringList lock_filter_items;
    QTableWidget *table = ui->tblCodesList;
    bool code1mode = AutoCodeGeneratorStatic::IsCode1Mode();
    bool code2mode = AutoCodeGeneratorStatic::IsCode2Mode();

    table->setRowCount(pSet->getLockMap()->size());

    ui->tblCodesList->setSortingEnabled(false);

    _codes1InUse.clear();
    _codes2InUse.clear();

    QStringList annotations;

    for(itor = pSet->begin(); itor != pSet->end(); itor++)
    {
        pState = itor.value();

        // pState->show();

        QString locks = pState->getLockNums();
        QStringList sl = FormatLocks(locks);
        // Note: lock items are collected during the iteration of the codes
        // to populate the locks filter drop down (see after iteration loop)
        lock_filter_items.append(sl);

        nCol = 0;
        QString row_value = QString("%0").arg(QVariant(nRow + 1).toString(), 3, QChar('0'));
        table->setItem(nRow, nCol++, new QTableWidgetItem(row_value));
        table->setItem(nRow, nCol++, new QTableWidgetItem(sl.join(',')));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription()));


        annotations.clear();
        QString code1 = pState->getCode1();
        bool code1_duplicate = _codes1InUse.contains(code1);
        _codes1InUse.append(code1);
        if (code1_duplicate)
        {
            annotations.append("Dup");
        }

        if (pState->getFingerprint1())
        {
            annotations.append("FP");
        }
        else if (pState->getAutoCode())
        {
            if (code1mode)
            {
                annotations.append("AC");
            }
        }

        if (annotations.count())
        {
            code1 += QString(" (%1)").arg(annotations.join(","));
        }

        auto code1_item = new QTableWidgetItem(code1);
        table->setItem(nRow, nCol++, code1_item);

        annotations.clear();
        QString code2 = pState->getCode2();
        bool code2_duplicate = false;
        if (!code2.isEmpty())
        {
            code2_duplicate = _codes2InUse.contains(code2);
            _codes2InUse.append(code2);
            if (code2_duplicate)
            {
                annotations.append("Dup");
            }

            if (pState->getAutoCode())
            {
                if (code2mode)
                {
                    annotations.append("AC");
                }
            }

            if (pState->getAskQuestions())
            {
                annotations.append("Q");
            }
        }

        if (annotations.count())
        {
            code2 += QString(" (%1)").arg(annotations.join(","));
        }

        auto code2_item = new QTableWidgetItem(code2);
        table->setItem(nRow, nCol++, code2_item);

        if (pState->getAccessType() == ACCESS_TYPE_ALWAYS)
        {
            table->setItem(nRow, nCol++, new QTableWidgetItem(tr("ALWAYS")));
            table->setItem(nRow, nCol++, new QTableWidgetItem(tr("ACTIVE")));
        }
        else if (pState->getAccessType() == ACCESS_TYPE_TIMED)
        {
            table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getStartTime().toString("MMM dd yyyy hh:mm:ss AP")));
            table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getEndTime().toString("MMM dd yyyy hh:mm:ss AP")));
        }
        else if (pState->getAccessType() == ACCESS_TYPE_LIMITED_USE)
        {
            table->setItem(nRow, nCol++, new QTableWidgetItem(QString("%1 (%2) %3").arg(tr("LIMITED USE"), QString::number(pState->getRemainingUses()), tr("uses remaining"))));
            table->setItem(nRow, nCol++, new QTableWidgetItem(pState->isActive() ? tr("ACTIVE") : tr("DISABLED")));
        }

        if (code1_duplicate || code2_duplicate)
        {
            for (int col = 0; col < table->columnCount(); ++col)
            {
                table->item(nRow, col)->setBackground(QColor(255,255,0));
            }
        }

        nRow++;
    }

    ui->tblCodesList->setSortingEnabled(true);

    // Note: We want the entries in the combo box to be in numeric order
    // We gather the locks using a set to remove duplicates.  Unfortunately
    // sets cannot be sorted, so we convert to a list and add to the
    // combo box.
    QSet<QString> unique_filter_items = QSet<QString>::fromList(lock_filter_items);
    QList<QString> sorted_filter_items(unique_filter_items.toList());
    qSort(sorted_filter_items.begin(), sorted_filter_items.end());
    ui->cbLockNum->addItems(QStringList(sorted_filter_items));


    // KCB_DEBUG_EXIT();
}

void CFrmAdminInfo::updateCodeTableContextMenu()
{
    _pTableMenu->clear();
    if (AutoCodeGeneratorStatic::IsCode1Mode())
    {
        _pTableMenu->addAction(tr("Edit Code"), this, SLOT(codeEditSelection()));
    }
    else if (AutoCodeGeneratorStatic::IsCode2Mode())
    {
        _pTableMenu->addAction(tr("Edit Code"), this, SLOT(codeEditSelection()));
        _pTableMenu->addAction(tr("Add Code"), this, SLOT(codeInitNew()));
        _pTableMenu->addAction(tr("Delete"), this, SLOT(codeDeleteSelection()));
    }
    else
    {
        _pTableMenu->addAction(tr("Edit Code"), this, SLOT(codeEditSelection()));
        _pTableMenu->addAction(tr("Add Code"), this, SLOT(codeInitNew()));
        _pTableMenu->addAction(tr("Delete"), this, SLOT(codeDeleteSelection()));
    }
}

void CFrmAdminInfo::setupCodeTableContextMenu()
{
    // KCB_DEBUG_ENTRY;
    _pTableMenu = new QMenu(ui->tblCodesList);
    connect(ui->tblCodesList, SIGNAL(cellClicked(int,int)), this, SLOT(OnRowSelected(int, int)));
    updateCodeTableContextMenu();
    _pTableMenuAdd = new QMenu(ui->tblCodesList);
    connect(ui->tblCodesList->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnHeaderSelected(int)));
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::displayInHistoryTable(CLockHistorySet *pSet)
{
    // KCB_DEBUG_ENTRY;
    // KCB_DEBUG_TRACE(pSet);

    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");

    if (_phistoryWorkingSet) 
    {
        _phistoryWorkingSet->clearSet();
        delete _phistoryWorkingSet;
    }

    _phistoryWorkingSet = pSet;    // Hold onto the set for additional work.

    // Setup the table display

    QTableWidget    *table = ui->tblHistory;

    int hh_width = table->horizontalHeader()->width();
    int delta = hh_width / 20;

    table->clearContents();
    table->setRowCount(pSet->getLockHistoryMap()->size());
    table->setColumnCount(6);

    table->setColumnWidth(0, 2 * delta);
    table->setColumnWidth(1, 6 * delta);
    table->setColumnWidth(2, 2 * delta);
    table->setColumnWidth(3, 2 * delta);
    table->setColumnWidth(4, 6 * delta);
    table->setColumnWidth(5, 2 * delta);
    
    table->verticalHeader()->hide();
    
    QStringList headers;
    headers<<tr("Lock #")<<tr("Username")<<tr("Code#1")<<tr("Code#2")<<tr("Accessed")<<tr("Image");
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setFixedWidth(50);
    table->horizontalHeader()->setFixedHeight(50);   
    table->horizontalHeader()->setStretchLastSection(true);

    table->setStyleSheet("QTableView {selection-background-color: lightblue;}");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);


    ui->cbLockNumHistory->clear();
    ui->cbLockNumHistory->addItem(QString(tr("All Locks")));

    int nRow = 0;
    int nCol = 0;
    CLockHistoryRec  *pState;
    QSet<int> lock_items;
    

    auto itor = _phistoryWorkingSet->getIterator();

    table->setSortingEnabled(false);
    
    while (itor.hasNext())
    {
        pState = itor.next();
        
        QString locks = pState->getLockNums();
        QStringList sl = locks.split(',');
        foreach (auto s, sl)
        {
            lock_items.insert(s.toInt());
        }
        
        //KCB_DEBUG_TRACE("code1(e)" << pState->getCode1() << "code1(c)" << CEncryption::decryptString(pState->getCode1()));

        nCol = 0;
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getLockNums()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(CEncryption::decryptString(pState->getCode1())));
        table->setItem(nRow, nCol++, new QTableWidgetItem(CEncryption::decryptString(pState->getCode2())));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getAccessTime().toString("yyyy-MM-dd HH:mm:ss AP")));
		
        QByteArray ba = pState->getImage();
        if (ba.count() > 0)
        {
            QPixmap pm;
            pm.loadFromData(ba);
            QLabel *image_label = new QLabel();
            image_label->setPixmap(pm.scaled(60, 60, Qt::KeepAspectRatio));
            image_label->setAlignment(Qt::AlignHCenter);
            table->setCellWidget(nRow, nCol, image_label);
        }
        else
        {
            table->setItem(nRow, nCol, new QTableWidgetItem("No Image"));
        }
        nRow++;
    }

    table->setSortingEnabled(true);

    // Note: We want the entries in the combo box to be in numeric order
    // We gather the locks using a set to remove duplicates.  Unfortunately
    // sets cannot be sorted, so we convert to a list, so and add to the
    // combo box.
    QList<int> lock_items_list(lock_items.toList());
    qSort(lock_items_list.begin(), lock_items_list.end());
    foreach (auto i, lock_items_list)
    {
        ui->cbLockNumHistory->addItem(QString::number(i));
    }

    // KCB_DEBUG_EXIT;
    
}

void CFrmAdminInfo::codeDeleteSelection()
{
    KCB_DEBUG_ENTRY;

    int nRC = QMessageBox::warning(this, tr("Verify Delete"),
                                   tr("Do you want to delete the selected code?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) 
    {
        deleteCodeByRow(_nRowSelected);
    }
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::codeInitNew()
{
    KCB_DEBUG_ENTRY;
    addCodeByRow();
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::codeEnableAll()
{
    // KCB_DEBUG_TRACE("Enabling all Limited Use codes");

    /* Looping over all the codes in the working set
           if code access type is 'limited use' then
               reset using __OnUpdateCodeState
    */
    CLockSet::Iterator itor;
    int nRow = 0;
    if (_pState)
    {
        // KCB_DEBUG_TRACE("Freeing _pState in CFrmAdminInfo::codeEnableAll");
        _pState = 0;
    }

    if(!_pworkingSet) 
    {
        _pworkingSet = new CLockSet();
    }

    on_btnReadCodes_clicked();
    displayInTable(_pworkingSet);

    // KCB_DEBUG_TRACE("before loop iterator cellClicked");

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        // itor is our man!
        _pState = itor.value();

        if(_pState)
        {
            if (_pState->getAccessType() == ACCESS_TYPE_LIMITED_USE && _pState->getAccessCount() > 0)
            {
                _pState->setMarkForReset();
                emit __OnUpdateCodeState(_pState);
            }
        }
        nRow++;
    }
    
}

void CFrmAdminInfo::codeEditSelection()
{
    editCodeByRow(_nRowSelected);
}

void CFrmAdminInfo::codeCellSelected( int row, int col) 
{
    CLockSet::Iterator itor;
    CLockState  *pState;
    int nRow = 0;

    Q_UNUSED(col);
    _nRowSelected = row;

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        if(nRow == row)
        {
            // itor is our man!
            pState = itor.value();
            Q_UNUSED(pState);
        }

        nRow++;
    }
}


void CFrmAdminInfo::on_btnReadCodes_clicked()
{
    QDateTime dtStart = ui->dtStartCodeList->dateTime();
    QDateTime dtEnd = ui->dtEndCodeList->dateTime();
    QString locks = ui->cbLockNum->currentText();
    if (locks == tr("All Locks"))
    {
        locks = "*";
    }
    else if (locks.contains(QChar(',')))
    {
        QStringList sl;
        foreach (const auto& l, locks.split(','))
        {
            sl.append(QString::number(l.toInt()));
        }
        locks = sl.join(',');
    }
    else
    {
        locks = QString::number(locks.toInt());
    }

    emit __OnReadLockSet(locks, dtStart, dtEnd);
}

void CFrmAdminInfo::on_btnRead_clicked()
{
    QDateTime dtStart = ui->dtStartCodeHistoryList->dateTime();
    QDateTime dtEnd = ui->dtEndCodeHistoryList->dateTime();
    QString locks = ui->cbLockNumHistory->currentText();
    
    if (locks == tr("All Locks"))
    {
        locks = "*";
    }
    
    emit __OnReadLockHistorySet(locks, dtStart, dtEnd);
}


void CFrmAdminInfo::OnNotifyGenerateReport()
{
    QDateTime start;
    QDateTime end;
    m_report.getValues(_tmpAdminRec, start, end);
    _bClose = false;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
    emit __OnImmediateReportRequest(start, end);
}

void CFrmAdminInfo::setTime()
{
    kcb::SetDateTime(ui->dtSystemTime->dateTime());
}

void CFrmAdminInfo::OnCodeEditClose()
{
    KCB_DEBUG_TRACE("NOT IMPLEMENTED");
}

void CFrmAdminInfo::OnCodeEditReject()
{
    if (_pFrmCodeEditMulti)
    {
        _pFrmCodeEditMulti->hide();
    }
}

void CFrmAdminInfo::OnCodeEditAccept()
{
    // KCB_DEBUG_ENTRY;
    if (_pFrmCodeEditMulti)
    {
        _pState->setAutoCode(AutoCodeGeneratorStatic::IsCode1Mode() || AutoCodeGeneratorStatic::IsCode2Mode());
        _pFrmCodeEditMulti->getValues(_pState);
        _pFrmCodeEditMulti->hide();
        HandleCodeUpdate();
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::setPStateValues(QString lockNums, 
                                    QString sAccessCode,
                                    QString sSecondCode, 
                                    QString sUsername,
                                    QDateTime dtStart, 
                                    QDateTime dtEnd, 
                                    bool fingerprint1, 
                                    bool fingerprint2,
                                    bool askQuestions, 
                                    QString question1, 
                                    QString question2, 
                                    QString question3,
                                    int access_type)
{
    _pState->setLockNums(lockNums);
    _pState->setCode1(sAccessCode);
    _pState->setCode2(sSecondCode);
    _pState->setDescription(sUsername);
    _pState->setStartTime(dtStart);
    _pState->setEndTime(dtEnd);

    if(fingerprint1)
    {
        _pState->setFingerprint1();
    }
    else
    {
        _pState->clearFingerprint1();
    }

    if (fingerprint2)
    {
        _pState->setFingerprint2();
    }
    else
    {
        _pState->clearFingerprint2();
    }

    _pState->setAskQuestions(askQuestions);
    _pState->setQuestion1(question1);
    _pState->setQuestion2(question2);
    _pState->setQuestion3(question3);

    _pState->setMaxAccess(-1); 
    _pState->setAccessCount(0);
    _pState->setAccessType(access_type);

    if (access_type == ACCESS_TYPE_LIMITED_USE)
    {
        _pState->setMaxAccess(2);
    }
}

void CFrmAdminInfo::HandleCodeUpdate()
{
    // _pState->show();

    if(_pState->isNew())
    {
        // KCB_DEBUG_TRACE("new _pState");
        if(_pworkingSet)
        {
            // KCB_DEBUG_TRACE("adding _pState to _pworkingSet");
            _pworkingSet->addToSet(_pState);
        }
        // KCB_DEBUG_TRACE("emitting __OnUpdateCodeState");
        emit __OnUpdateCodeState(_pState);
    } 
    else 
    {
        int nCol = 1;
        QTableWidget     *table = ui->tblCodesList;
        QTableWidgetItem *item;
        item = table->item(_nRowSelected, nCol++);

        // KCB_DEBUG_TRACE("Item Text:" << item->text());


        /* Note: The following code seems to assume dtStart/dtEnd will be datetime, but what about ALWAYS? */
        // KCB_DEBUG_TRACE("not new _pState");

        QString locks = _pState->getLockNums();

        if (item) 
        {
            item->setText(locks);
            item = table->item(_nRowSelected, nCol++);
            item->setText(_pState->getDescription());
            item = table->item(_nRowSelected, nCol++);
            item->setText(_pState->getCode1());
            item = table->item(_nRowSelected, nCol++);
            item->setText(_pState->getCode2());
            item = table->item(_nRowSelected, nCol++);

            // KCB_DEBUG_TRACE("start time" << _pState->getStartTime().toString() << "end time" << _pState->getEndTime().toString());

            item->setText(_pState->getStartTime().toString("MMM dd yyyy hh:mm AP"));
            item = table->item(_nRowSelected, nCol++);
            item->setText(_pState->getEndTime().toString("MMM dd yyyy hh:mm AP"));
            item=table->item(_nRowSelected, nCol++);
        }

        _pState->setModified();
        emit __OnUpdateCodeState(_pState);
    }
}

CLockState* CFrmAdminInfo::createNewLockState()
{
    CLockState  *pState;
    pState = new CLockState();
    pState->setID(-1);
    pState->setLockNums("");
    pState->setCode1("");
    pState->setCode2("");
    pState->setDescription("");
    pState->setStartTime(QDateTime().currentDateTime());
    pState->setEndTime(QDateTime().currentDateTime());
    pState->setAskQuestions(false);

    /* Set the access type: default is ACCESS_ALWAYS */
    pState->setAccessType(0 /*ACCESS_ALWAYS*/);
    pState->setMaxAccess(-1);

    pState->clearFingerprint1();
    pState->clearFingerprint2();

    pState->setNew();
    pState->clearMarkForDeletion();
    pState->clearMarkForReset();
    pState->clearModified();

    return pState;
}

void CFrmAdminInfo::setTableMenuLocation(QMenu *pmenu)
{
    pmenu->setGeometry(_lastTouchPos.x(),_lastTouchPos.y(),pmenu->width(), pmenu->height());
}

void CFrmAdminInfo::OnRowSelected(int row, int column) 
{
    _nRowSelected = row;
    updateCodeTableContextMenu();
    _pTableMenu->show();

    QPoint widgetPoint = QWidget::mapFromGlobal(QCursor::pos());
    _pTableMenu->setGeometry(widgetPoint.x(), widgetPoint.y(), _pTableMenu->width(), _pTableMenu->height());
}

bool CFrmAdminInfo::eventFilter(QObject *target, QEvent *event)
{
    //KCB_DEBUG_TRACE("eventFilter. Event type:" << QVariant(event->type()).toString());
    if(event->type() == QEvent::TouchBegin )
    {
        KCB_DEBUG_TRACE("TouchBegin");
        touchEvent(static_cast<QTouchEvent*>(event));

        if(ui->tblCodesList->rowCount() == 0 )
        {
            return true;
        }
    }

    return QDialog::eventFilter(target, event);
}

void CFrmAdminInfo::OnCodes(QString code1, QString code2)
{
    // KCB_DEBUG_TRACE(code1 << code2);
    emit __OnAdminInfoCodes(code1, code2);
}

void CFrmAdminInfo::touchEvent(QTouchEvent *ev)
{
    KCB_DEBUG_ENTRY;
    QList<QTouchEvent::TouchPoint>   lstPoints;
    switch (ev->type())
    {
    case QEvent::TouchBegin:
        KCB_DEBUG_TRACE("TouchBegin rowcount:" << QVariant(ui->tblCodesList->rowCount()).toString());
        lstPoints = ev->touchPoints();
        _lastTouchPos = lstPoints.at(0).screenPos().toPoint();

        if(ui->tblCodesList->rowCount() == 0 )
        {
            _pTableMenuAdd->show();
            setTableMenuLocation(_pTableMenuAdd);
        }

        if(ui->tblCodesList->rowCount() > 0 )
        {
            _pTableMenu->show();
            setTableMenuLocation(_pTableMenu);
        }
        break;

    case QEvent::TouchEnd:
        KCB_DEBUG_TRACE("TouchEnd rowcount:" << QVariant(ui->tblCodesList->rowCount()).toString());
        if(ui->tblCodesList->rowCount() >= 0 )
        {
            _pTableMenuAdd->show();
            setTableMenuLocation(_pTableMenuAdd);
        }
        break;

    default:
        break;
    }

    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnHeaderSelected(int nHeader) 
{
    if (nHeader == 5)
    {
        _pTableMenuAdd->clear();
        _pTableMenuAdd->addAction(tr("Enable All"), this, SLOT(codeEnableAll()));
        _pTableMenuAdd->show();
        QPoint widgetPoint = QWidget::mapFromGlobal(QCursor::pos());
        _pTableMenuAdd->setGeometry(widgetPoint.x(), widgetPoint.y(), _pTableMenuAdd->width(), _pTableMenuAdd->height());
    }
    else
    {
        if (ui->tblCodesList->rowCount() == 0)
        {
            _pTableMenuAdd->clear();
            _pTableMenuAdd->addAction(tr("Add"), this, SLOT(codeInitNew()));
            _pTableMenuAdd->show();
            QPoint widgetPoint = QWidget::mapFromGlobal(QCursor::pos());
            _pTableMenuAdd->setGeometry(widgetPoint.x(), widgetPoint.y(), _pTableMenuAdd->width(), _pTableMenuAdd->height());
        }
    }
}

void CFrmAdminInfo::checkAndCreateCodeEditForm()
{
    if(!_pFrmCodeEditMulti)
    {
        _pFrmCodeEditMulti = new FrmCodeEditMulti(this);
        connect(_pFrmCodeEditMulti, SIGNAL(rejected()), this, SLOT(OnCodeEditReject()));
        connect(_pFrmCodeEditMulti, SIGNAL(accepted()), this, SLOT(OnCodeEditAccept()));
        connect(this, SIGNAL(__OnAdminInfoCodes(QString,QString)), _pFrmCodeEditMulti, SIGNAL(__OnAdminInfoCodes(QString,QString)));
        connect(this, &CFrmAdminInfo::__NotifyLockSelectionChanged, _pFrmCodeEditMulti, &FrmCodeEditMulti::OnLockSelectionChanged);
    }
}

void CFrmAdminInfo::addCodeByRow()
{
    // KCB_DEBUG_ENTRY;
    checkAndCreateCodeEditForm();

    // Get line values
    CLockSet::Iterator itor;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }

    _pState = createNewLockState();
    if(!_pworkingSet) 
    {
        _pworkingSet = new CLockSet();
    }

    // _pState->show();

    _pFrmCodeEditMulti->setValues(_pState, _codes1InUse, _codes2InUse);
    _pFrmCodeEditMulti->show();
    // KCB_DEBUG_EXIT;
}

QString CFrmAdminInfo::StripAnnotations(const QString& text)
{
    // Code Annotations
    //    xx...xx (...)
    // Find the left parenthesis index
    // Take the left N characters where N is index-1
    int index = text.indexOf(QChar('('));
    return text.left(index - 1);
}

void CFrmAdminInfo::editCodeByRow(int row)
{
    checkAndCreateCodeEditForm();

    CLockSet::Iterator itor;
    if (_pState)
    {
        _pState = 0;
    }

    QTableWidgetItem* username_item = ui->tblCodesList->item(row, 2);
    QTableWidgetItem* code1_item = ui->tblCodesList->item(row, 3);
    QTableWidgetItem* code2_item = ui->tblCodesList->item(row, 4);

    QString code1 = StripAnnotations(code1_item->text());
    QString code2 = StripAnnotations(code2_item->text());

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        _pState = itor.value();
        if (_pState->getDescription() == username_item->text() &&
            _pState->getCode1() == code1 &&
            _pState->getCode2() == code2)
        {
            break;
        }
    }

    if(_pState)
    {
        _pFrmCodeEditMulti->setValues(_pState, _codes1InUse, _codes2InUse);
        _pFrmCodeEditMulti->show();
    }
    else
    {
        // Q: Does this branch ever occurr?  How is it possible to edit a row for which
        // there is no code entry?
        KCB_WARNING_TRACE("Editing code that is not part of working set");
        _pState = createNewLockState();
        _pworkingSet->addToSet(_pState);

        _pFrmCodeEditMulti->setValues(_pState, _codes1InUse, _codes2InUse);
        _pFrmCodeEditMulti->show();
    }
}

void CFrmAdminInfo::deleteCodeByRow(int row)
{
    KCB_DEBUG_ENTRY;
    checkAndCreateCodeEditForm();

    CLockSet::Iterator itor;
    if (_pState)
    {
        _pState = 0;
    }

    QTableWidgetItem* username_item = ui->tblCodesList->item(row, 2);
    QTableWidgetItem* code1_item = ui->tblCodesList->item(row, 3);
    QTableWidgetItem* code2_item = ui->tblCodesList->item(row, 4);

    QString code1 = StripAnnotations(code1_item->text());
    QString code2 = StripAnnotations(code2_item->text());

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        _pState = itor.value();
        if (_pState->getDescription() == username_item->text() &&
            _pState->getCode1() == code1 &&
            _pState->getCode2() == code2)
        {
            break;
        }
    }

    if(_pState)
    {
        _pState->setMarkForDeletion();

        QString cmd = QString("%1%2").arg(QString(CMD_REMOVE_FP_FILE)).arg(_pState->getCode1());

        // KCB_DEBUG_TRACE("cmd: " << cmd);

        if( QDir(QString("/home/pi/")).exists() )
        {
            std::system( cmd.toStdString().c_str() );
        }
        emit __OnUpdateCodeState(_pState);
    }
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::purgeCodes()
{
    // KCB_DEBUG_ENTRY;

    CLockSet::Iterator itor;
    if (_pState)
    {
        // KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }
    int nRow = 0;

    if(!_pworkingSet)
    {
        _pworkingSet = new CLockSet();
    }

    on_btnReadCodes_clicked();
    displayInTable(_pworkingSet);

    // KCB_DEBUG_TRACE("before loop iterator cellClicked");

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        _pState = itor.value();

        if(_pState)
        {
            _pState->setMarkForDeletion();
            emit __OnUpdateCodeState(_pState);
        }
        nRow++;
    }

    // KCB_DEBUG_TRACE(CMD_REMOVE_ALL_FP_FILES);
    std::system( CMD_REMOVE_ALL_FP_FILES );
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnRebootSystem_clicked()
{
    int nRC = QMessageBox::warning(this, tr("Verify System Reboot"),
                                   tr("System will be restarted.\nDo you want to restart?"),
                                  QMessageBox::Ok, QMessageBox::Cancel);
    if(nRC == QMessageBox::Ok)
    {
        if(_pcopymodel)
        {
            _pcopymodel->disconnect();
            delete _pcopymodel;
            _pcopymodel = 0;
        }
        QFileInfo fi(_copyDirectory);
        QString path = fi.absolutePath();
        kcb::UnmountUsb(path);        
        kcb::Reboot();
    }
}

void CFrmAdminInfo::on_btnPurgeCodes_clicked()
{
    // KCB_DEBUG_ENTRY;

    int nRC = QMessageBox::warning(this, tr("Verify Remove All Codes"),
                                   tr("All access codes will be removed from the system\nDo you want to continue?"),
                                   QMessageBox::Ok, QMessageBox::Cancel);
    if(nRC == QMessageBox::Ok)
    {
        _psysController->clearAllCodes();
        on_btnReadCodes_clicked();

        nRC = QMessageBox::warning(this, tr("Code Removal Success"),
                                   tr("Code Removal was successful!!"
                                   "\n\n"
                                   "Please give the codes list a moment to update."),
                                   QMessageBox::Ok);
    }
}

void CFrmAdminInfo::UpdateAutoCodeDisplay()
{
    // KCB_DEBUG_ENTRY;
    if (AutoCodeGeneratorStatic::IsCode1Mode())
    {
        // KCB_DEBUG_TRACE("updating code1");
        QStringList codes;
        _psysController->getAllCodes1(codes);
        emit m_autocodegen.NotifyCodesUpdate(codes);
    }
    else if (AutoCodeGeneratorStatic::IsCode2Mode())
    {
        // KCB_DEBUG_TRACE("updating code2");
        emit m_autocodegen.NotifyCodesUpdate(QStringList());
    }
    else
    {
        // Nothing else to do at this time
        // KCB_DEBUG_TRACE("nothing to do");
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnTabSelected(int index)
{
    if (index == CODES_TAB_INDEX)
    {
        QDateTime dt = QDateTime().currentDateTime();
        ui->dtEndCodeList->setDateTime(dt);
        // Force the codes to be read into the codes tab
        emit ui->btnReadCodes->clicked();
    }
    else if (index == CODE_HISTORY_TAB_INDEX)
    {
        QDateTime dt = QDateTime().currentDateTime();
        ui->dtEndCodeHistoryList->setDateTime(dt);
        // Force the codes to be read into the codes tab
        emit ui->btnRead->clicked();
    }
    else if (index == SYSTEM_TAB_INDEX)
    {
        SetCabinetInfo();
    }
    else if (index == AUTOCODE_TAB_INDEX)
    {
        UpdateAutoCodeDisplay();
    }
    else if (index == DOORS_TAB_INDEX)
    {
    }
    else if (index == UTILITIES_TAB_INDEX)
    {
    }

    if (m_last_index != REPORT_TAB_INDEX && index == REPORT_TAB_INDEX)
    {
        // If we have selected the Report Tab, we want to populate with the latest information
        m_report.setValues(_tmpAdminRec);
    }

    if (m_last_index == REPORT_TAB_INDEX && index != m_last_index)
    {
        // If we are changing away from the Report Tab then we want to update to the latest values
        m_report.getValues(_tmpAdminRec);
    }

    m_last_index = index;
}

void CFrmAdminInfo::updateAdminForEmail(EMAIL_ADMIN_SELECT email_select)
{
    // Note: the mechanism for sending a test email is as follows:
    //    1. set the selection type, i.e., send admin test email or send report test email
    //    2. update the admin settings, i.e., if we changed the admin email or other email related parameters
    //    3. clear the 'close' variable because we aren't closing the admin screen
    //    4. update the admin
    // After the admin update signal is sent, the admin will be updated in the database and a signal issued
    // which will arrive in at slot OnUpdatedCurrentAdmin.  In OnUpdatedCurrentAdmin, the _testEmail variable
    // will be checked and if properly set, the appropriate test email will be requested from the system
    // controller.
    _testEmail = email_select;
    _bClose = false;
    updateTmpAdminRec();
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
}

void CFrmAdminInfo::on_btnTestEmail_clicked()
{
    // KCB_DEBUG_TRACE("Testing admin send email");
    updateAdminForEmail(EMAIL_ADMIN_SEND);
}

void CFrmAdminInfo::on_btnTestUserEmail_clicked()
{
    // KCB_DEBUG_TRACE("Testing admin recv email");
    updateAdminForEmail(EMAIL_ADMIN_RECV);
}

void CFrmAdminInfo::OnDisplayFingerprintButton(bool state)
{
    ui->chkDisplayFingerprintButton->setChecked(state);
}

void CFrmAdminInfo::OnDisplayShowHideButton(bool state)
{
    ui->chkDisplayShowHideButton->setChecked(state);
}

void CFrmAdminInfo::OnDisplayTakeReturnButtons(bool state)
{
    ui->chkDisplayTakeReturnButtons->setChecked(state);
}

void CFrmAdminInfo::OnOpenLockRequest(QString lock, bool take_picture)
{
    emit __OnOpenLockRequest(lock, take_picture);
}

void CFrmAdminInfo::on_pbNetworkSettings_clicked()
{
    FrmNetworkSettings ns;

    ns.setValues(_tmpAdminRec.getVNCPort(), _tmpAdminRec.getVNCPassword(),
                 _tmpAdminRec.getSMTPServer(), _tmpAdminRec.getSMTPPort(), _tmpAdminRec.getSMTPType(), 
                 _tmpAdminRec.getSMTPUsername(), _tmpAdminRec.getSMTPPassword()
                 );
    if (ns.exec())
    {
        int vncPort;
        QString vncPassword;
        QString smtpServer;
        int smtpPort;
        int smtpType;
        QString smtpUsername;
        QString smtpPassword;

        ns.getValues(vncPort, vncPassword,
                     smtpServer, smtpPort, smtpType, smtpUsername, smtpPassword);

        _tmpAdminRec.setVNCPort(vncPort);
        _tmpAdminRec.setVNCPassword(vncPassword);
        _tmpAdminRec.setSMTPServer(smtpServer);
        _tmpAdminRec.setSMTPPort(smtpPort);
        _tmpAdminRec.setSMTPType(smtpType);
        _tmpAdminRec.setSMTPUsername(smtpUsername);
        _tmpAdminRec.setSMTPPassword(smtpPassword);

        // Notify the world about the changes
        emit __UpdateCurrentAdmin(&_tmpAdminRec);

        KeyCodeBoxSettings::SetVncCredentials(QString::number(vncPort), vncPassword);
    }
}

void CFrmAdminInfo::codeHistoryTableCellSelected(int row, int col)
{
    // KCB_DEBUG_TRACE("row" << row << "col" << col);

    if (col == 5)
    {     
        const QVector<CLockHistoryRec*>* set = _phistoryWorkingSet->getLockHistoryMap();

        CLockHistoryRec* rec = (*set)[row];
        QByteArray ba = rec->getImage();
        QPixmap pm;
        pm.loadFromData(ba);
        // KCB_DEBUG_TRACE("pm size" << pm.size());
        if (pm.size() != QSize(0, 0))
        {
            QMessageBox mb;
            mb.setWindowTitle("Access Image");
            mb.setStandardButtons(QMessageBox::Ok);
            mb.setIconPixmap(pm);
            mb.exec();
        }
    }
}

void CFrmAdminInfo::on_cbActionsSelect_currentIndexChanged(int index)
{
    // KCB_DEBUG_ENTRY;
    ui->cbFileFormat->setDisabled(true);
    ui->cbSecurity->setDisabled(true);
    ui->btnActionExecute->setDisabled(true);

    switch (index)
    {
        case ACTION_INDEX_INSTALL_APP:
            m_util_action = UTIL_ACTION_INSTALL_APP;
            m_file_filter = INSTALL_APP_FILTER;
            break;

        case ACTION_INDEX_SET_BRANDING_IMAGE:
            m_util_action = UTIL_ACTION_SET_BRANDING_IMAGE;
            m_file_filter = BRANDING_IMAGE_FILTER;
            break;

        case ACTION_INDEX_DEFAULT_BRANDING_IMAGE:
            m_util_action = UTIL_ACTION_DEFAULT_BRANDING_IMAGE;
            m_file_filter = BRANDING_IMAGE_FILTER;
            break;

        case ACTION_INDEX_IMPORT_CODES:
        case ACTION_INDEX_EXPORT_CODES:
            setFileFilterFromFormatSelection(ui->cbFileFormat->currentText());
            if (index == ACTION_INDEX_IMPORT_CODES)
            {
                ui->cbFileFormat->setEnabled(true);
                int index = ui->cbFileFormat->findText("XML");
                ui->cbFileFormat->setCurrentIndex(index);
                m_util_action = UTIL_ACTION_IMPORT_CODES;
            }
            else if (index == ACTION_INDEX_EXPORT_CODES)
            {
                ui->btnActionExecute->setEnabled(true);
                ui->cbFileFormat->setEnabled(true);
                ui->cbSecurity->setEnabled(true);
                m_util_action = UTIL_ACTION_EXPORT_CODES;
            }
            break;

        case ACTION_INDEX_EXPORT_LOGS:
            ui->btnActionExecute->setEnabled(true);
            m_util_action = UTIL_ACTION_EXPORT_LOGS;
            m_file_filter = QStringList() << "*.log";
            break;        

        default:
            // KCB_DEBUG_TRACE("Unknown action index" << index);
            break;
    }

    populateFileCopyWidget(ui->cbUsbDrives->currentText());

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnActionExecute_clicked()
{
    switch (m_util_action)
    {
        case UTIL_ACTION_INSTALL_APP:
            OnUtilActionInstallApp();
            break;

        case UTIL_ACTION_SET_BRANDING_IMAGE:
            OnUtilActionSetBrandingImage();
            break;

        case UTIL_ACTION_DEFAULT_BRANDING_IMAGE:
            OnUtilActionDefaultBrandingImage();
            break;

        case UTIL_ACTION_IMPORT_CODES:
            {
                int nRC = QMessageBox::warning(this, 
                                               tr("Code Import"),
                                               QString(tr("You have selected import codes in %1 format.\n"
                                                          "Continuing will import all of the codes into the database.\n"
                                                          "If there are duplicate codes, they must be removed manually.\n"
                                                          "Do you want to continue?"
                                                         )).arg("XML"),
                                               QMessageBox::Yes, QMessageBox::No);
                if (nRC == QMessageBox::Yes)                                        
                {
                    CodeImportExportUtil::CODEFORMAT format = CodeImportExportUtil::StringToFormat(ui->cbFileFormat->currentText());
                    CodeImportExportUtil::CODESECURITY security = CodeImportExportUtil::NOTSPECIFIED_SECURITY; // handles the case where security is specified in the file format

                    // If the format is CSV then we take the security from the security combo box
                    if (format == CodeImportExportUtil::CSV_FORMAT)
                    {
                        security = CodeImportExportUtil::StringToSecurity(ui->cbSecurity->currentText());                        
                    }

                    CodeListing codeListing;
                    CodeImporter importer(format, _copyDirectory, security);
                    bool result = importer.import(codeListing);
                    if (result)
                    {
                        insertCodes(codeListing);
                        (void) QMessageBox::information(this, 
                                                        tr("Bulk Code Upload Complete"),
                                                        tr("Please check the 'Codes' tab to see that your codes were added successfully."),
                                                        QMessageBox::Ok);
                    }
                    else
                    {
                        QString format_str = CodeImportExportUtil::FormatToString(format).toUpper();
                        (void) QMessageBox::warning(this, 
                                                    tr("%1 Parsing Failed").arg(format_str),
                                                    tr("Can't parse given %1 file\nPlease check syntax and integrity of your file.").arg(format_str),
                                                    QMessageBox::Ok);
                    }
                }
            }
            break;

        case UTIL_ACTION_EXPORT_CODES:
            {
                CodeImportExportUtil::CODEFORMAT format = CodeImportExportUtil::StringToFormat(ui->cbFileFormat->currentText());
                CodeImportExportUtil::CODESECURITY security = CodeImportExportUtil::StringToSecurity(ui->cbSecurity->currentText());
                bool clear_or_encrypted = security == CodeImportExportUtil::CLEAR_SECURITY ? true : false;
                QString root_path = ui->cbUsbDrives->currentText();
                CLockSet *lockset;                
            
                // A CLockSet object was allocated by this call to readAllCodes
                // It must be delete before leaving this code block.
                _psysController->readAllCodes(&lockset, clear_or_encrypted);
                if (lockset && lockset->isValid())
                {
                    CodeExporter exporter(format, root_path, *lockset, security);
                    bool result = exporter.Export();
                    Q_ASSERT_X(result, Q_FUNC_INFO, "exporter export failed");
                    if (!result)
                    {
                        KCB_WARNING_TRACE("Failure exporting codes");
                    }
                }
                else
                {
                    (void) QMessageBox::information(this, tr("Code Export"),
                                            QString(tr("There are no codes available to be exported")),
                                            QMessageBox::Ok);
                }
            }
            break;

        case UTIL_ACTION_EXPORT_LOGS:
            {
                // Copy the log file from /home/pi/kcb-config/logs/messages.log to USB drive
                QStringList nameFilter;
                nameFilter << "messages*.log" << "stderr.log";
                QDir dir("/home/pi/kcb-config/logs");
                QStringList log_list = dir.entryList(nameFilter, QDir::Files); 

                foreach (auto entry, log_list)
                {
                    QString source = QString("%1/%2").arg("/home/pi/kcb-config/logs").arg(entry);
                    QString target = QString("%1/%2").arg(ui->cbUsbDrives->currentText()).arg(entry);
                    bool result = QFile::copy(source, target);
                    if (!result)
                    {
                        // KCB_DEBUG_TRACE("failed to copy logs");
                    }
                }
            }
            break;

        default:
            KCB_DEBUG_TRACE("Unknown utility action" << m_util_action);
            break;
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnNotifyUsbDrive(QStringList list)
{    
    ui->cbUsbDrives->clear();
    if (list.count() > 0)
    {
        ui->cbUsbDrives->addItems(list);
        ui->cbUsbDrives->setCurrentIndex(0);
        ui->cbActionsSelect->setEnabled(true);
        ui->cbActionsSelect->setCurrentIndex(0);
        ui->pbUtilUnmountDrive->setEnabled(true);
    }
    else
    {
        ui->treeViewCopy->setModel(nullptr);
        ui->cbUsbDrives->addItem("No Drive Inserted");
        ui->cbActionsSelect->setDisabled(true);
        ui->pbUtilUnmountDrive->setDisabled(true);
        ui->btnActionExecute->setDisabled(true);
        ui->cbFileFormat->setDisabled(true);
        ui->cbSecurity->setDisabled(true);
    }
}

void CFrmAdminInfo::on_cbUsbDrives_currentIndexChanged(const QString &arg1)
{
    // KCB_DEBUG_ENTRY;

    populateFileCopyWidget(arg1);

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_cbFileFormat_currentIndexChanged(const QString &arg1)
{
    // KCB_DEBUG_ENTRY;
    
    bool is_import = ui->cbActionsSelect->currentText() == "Import Codes";
    bool is_export = ui->cbActionsSelect->currentText() == "Export Codes";
    bool is_csv = ui->cbFileFormat->currentText() == "CSV";

    // KCB_DEBUG_TRACE("export" << is_export << "import" << is_import << "csv" << is_csv);
    ui->cbSecurity->setEnabled(is_export || (is_import && is_csv));

    setFileFilterFromFormatSelection(arg1);
    populateFileCopyWidget(ui->cbUsbDrives->currentText());

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::setFileFilterFromFormatSelection(const QString filter)
{
    m_file_filter = IMPORT_CODES_FILTER;
    
    if (filter.toLower() == "xml" )
    {
        m_file_filter.clear();
        m_file_filter << IMPORT_CODES_XML_FILTER;
    }
    else if (filter.toLower() == "json")
    {
        m_file_filter.clear();
        m_file_filter << IMPORT_CODES_JSON_FILTER;
    }
    else if (filter.toLower() == "csv")
    {
        m_file_filter.clear();
        m_file_filter << IMPORT_CODES_CSV_FILTER;
    }
    else if (filter.toLower() == "sql")
    {
        m_file_filter.clear();
        m_file_filter << IMPORT_CODES_SQL_FILTER;
    }
}

void CFrmAdminInfo::on_pbUtilUnmountDrive_clicked()
{
    // KCB_DEBUG_ENTRY;

    // Set the model to null to disconnect the watcher signals
    ui->treeViewCopy->setModel(nullptr);

    // If there is an active model, then disconnect and delete
    if(_pcopymodel)
    {
        _pcopymodel->disconnect();
        delete _pcopymodel;
        _pcopymodel = 0;
    }

    QString path = ui->cbUsbDrives->currentText();
    // KCB_DEBUG_TRACE("Unmounting" << path);
    kcb::UnmountUsb(path);

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_cbLogLevel_currentIndexChanged(const QString &arg1)
{
    kcb::Logger::LOG_LEVEL level = kcb::Logger::LEVEL_INFO;

    if (arg1 == "INFO")
    {
        level = kcb::Logger::LEVEL_INFO;
    }
    else if (arg1 == "DEBUG")
    {
        level = kcb::Logger::LEVEL_DEBUG;
    }
    else if (arg1 == "WARNING")
    {
        level = kcb::Logger::LEVEL_WARNING;
    }
    else if (arg1 == "CRITICAL")
    {
        level = kcb::Logger::LEVEL_CRITICAL;
    }
    else if (arg1 == "FATAL")
    {
        level = kcb::Logger::LEVEL_FATAL;
    }

    kcb::Logger::setLevel(level);
}

void CFrmAdminInfo::on_pbDiscoverHardware_clicked()
{
    // KCB_DEBUG_ENTRY;
    // signal the lockcontroller to start detecting the hardware
    // it would seem to be a good idea to do that in a separate thread so
    // the ui remains responsive, but should we allow the user to do anything
    // else while we are detecting hardware?
    ui->pbResetCabinetConfig->setDisabled(true);
    ui->pbApplyChanges->setDisabled(true);
    ui->pgbDiscoverHardware->setValue(0);
    ui->pgbDiscoverHardware->setVisible(true);
    ui->pbDiscoverHardware->setDisabled(true);
    _psysController->DiscoverHardware();
    ui->pgbDiscoverHardware->setValue(100);

    SetCabinetInfo();

    invokeUpdateCabinetConfig();

    ui->pbDiscoverHardware->setEnabled(true);
    ui->pgbDiscoverHardware->setVisible(false);
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnDiscoverHardwareProgressUpdate(int value)
{
    // KCB_DEBUG_ENTRY;
    ui->pgbDiscoverHardware->setValue(value);
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::ClearCabinetInfo()
{
    // KCB_DEBUG_ENTRY;
    m_model.removeRows(0, m_model.rowCount());
    ui->pbResetCabinetConfig->setDisabled(true);
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::SetCabinetInfo()
{
    // KCB_DEBUG_ENTRY;
    CABINET_VECTOR cabinets = KeyCodeBoxSettings::getCabinetsInfo();

    ClearCabinetInfo();

    foreach (auto cab, cabinets)
    {
        m_model.insertRow(m_model.rowCount(),
                          QList<QStandardItem *>() << new QStandardItem(cab.model)
                                                   << new QStandardItem(QString::number(cab.start))
                                                   << new QStandardItem(QString::number(cab.stop))
                                                   << new QStandardItem(QString::number(cab.num_locks))
                                                   << new QStandardItem(cab.addr)
                                                   << new QStandardItem(cab.sw_version));
        ui->tvCabinets->setItemDelegateForRow(m_model.rowCount() - 1, new CabinetRowDelegate(this));
    }

    ui->pbResetCabinetConfig->setEnabled(cabinets.count() > 0);
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnItemChanged(QStandardItem* item)
{
    int col = item->column();
    int row = item->row();

    ui->pbApplyChanges->setEnabled(col == 3 || col == 2 || col == 1);

    // Note: This code works in a cascading fashion.  By changing the total locks column,
    // this code will be called to notify there was a change.  Each time this function
    // is called it will use the current column to make an update to the next row (if one exists)
    // which will in turn force this function to be called again.  Each time, a single change is
    // made to the model.

    if (col == 3)
    {
        QStandardItemModel *model = static_cast<QStandardItemModel *>(ui->tvCabinets->model());
        int total = model->data(model->index(row, col)).toInt();
        int first = model->data(model->index(row, col-2)).toInt();
        model->setData(model->index(row, col-1), first + total - 1);
    }
    else if (col == 2)
    {
        QStandardItemModel *model = static_cast<QStandardItemModel *>(ui->tvCabinets->model());
        if (model->rowCount() > (row + 1))
        {
            int last = model->data(model->index(row, col)).toInt();
            model->setData(model->index(row+1, col-1), last + 1);
        }
    }
    else if (col == 1)
    {
        QStandardItemModel *model = static_cast<QStandardItemModel *>(ui->tvCabinets->model());
        int first = model->data(model->index(row, col)).toInt();
        int total = model->data(model->index(row, col+2)).toInt();
        model->setData(model->index(row, col+1), first + total - 1);
    }

}

void CFrmAdminInfo::invokeUpdateCabinetConfig()
{
    // KCB_DEBUG_ENTRY;
    m_select_locks.updateCabinetConfig();
    if (_pFrmCodeEditMulti)
    {
        _pFrmCodeEditMulti->updateCabinetConfig();
    }
    m_autocodegen.updateCabinetConfig();
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_pbApplyChanges_clicked()
{
    // KCB_DEBUG_ENTRY;

    ui->pbApplyChanges->setDisabled(true);
    KeyCodeBoxSettings::ClearCabinetConfig();

    for (int rr = 0; rr < m_model.rowCount(); ++rr)
    {
        QString model = m_model.data(m_model.index(rr, 0)).toString();
        int start = m_model.data(m_model.index(rr, 1)).toInt();
        int stop = m_model.data(m_model.index(rr, 2)).toInt();
        int num_locks = m_model.data(m_model.index(rr, 3)).toInt();
        QString addr = m_model.data(m_model.index(rr, 4)).toString();
        QString sw_version = m_model.data(m_model.index(rr, 5)).toString();

        // KCB_DEBUG_TRACE("model" << model << "first" << start << "last" << stop << "total" << num_locks << "addr" << addr << "sw" << sw_version);
        KeyCodeBoxSettings::AddCabinet({model, num_locks, start, stop, sw_version, addr});
    }

    _psysController->UpdateLockRanges();

    invokeUpdateCabinetConfig();

    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_pbResetCabinetConfig_clicked()
{
    ui->pbApplyChanges->setDisabled(true);
    ui->pbResetCabinetConfig->setDisabled(true);
    ui->pbDiscoverHardware->setDisabled(true);
    ClearCabinetInfo();
    KeyCodeBoxSettings::ClearCabinetConfig();
    invokeUpdateCabinetConfig();
    ui->pbDiscoverHardware->setEnabled(true);
}

void CFrmAdminInfo::on_cbAdminLockSelection_currentIndexChanged(int index)
{
    // KCB_DEBUG_TRACE("index" << index);
    if (index == CODE_SELECTION_SINGLE_INDEX)
    {
        KeyCodeBoxSettings::SetLockSelectionSingle();
        emit __NotifyLockSelectionChanged();
    }
    else if (index == CODE_SELECTION_MULTI_INDEX)
    {
        KeyCodeBoxSettings::SetLockSelectionMulti();
        emit __NotifyLockSelectionChanged();
    }
    else
    {
        KCB_DEBUG_TRACE("invalid index" << index);
        return;
    }
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnRequestCodes1(QStringList& codes)
{
    _psysController->getAllCodes1(codes);
}

void CFrmAdminInfo::setLockStateDefaults(CLockState& state)
{
    state.setLockNums("");
    state.setCode1("");
    state.setCode2("");
    state.setDescription("");
    state.setStartTime(DEFAULT_DATETIME);
    state.setEndTime(DEFAULT_DATETIME);
    state.clearFingerprint1();
    state.clearFingerprint2();
    state.setAskQuestions(false);
    state.setQuestion1("");
    state.setQuestion2("");
    state.setQuestion3("");
    state.setMaxAccess(-1); 
    state.setAccessCount(0);
    state.setAccessType(0);
    state.setAutoCode(AutoCodeGeneratorStatic::IsCode1Mode() || AutoCodeGeneratorStatic::IsCode2Mode());
}

void CFrmAdminInfo::OnCommitCodes1(QMap<QString, QString> codes)
{
    // KCB_DEBUG_TRACE("OnCommitCodes1 called");

    for (int ii = 0; ii < 10; ++ii)
    {
        QApplication::processEvents();
    }
    QProgressDialog progress("Committing codes ...", "", 1, codes.keys().count()+3, this);
    progress.setCancelButton(nullptr);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("AutoCode Commit");

    int count = 1;
    progress.setValue(count);
    count++;
    progress.show();
    QApplication::processEvents();
    progress.setValue(count);
    count++;
    _psysController->clearAllCodes();
    QApplication::processEvents();
    progress.setValue(count);
    count++;

    foreach (auto key, codes.keys())
    {
        progress.setValue(count);
        count++;

        CLockState state;
        setLockStateDefaults(state);
        state.setLockNums(key);
        state.setCode1(codes[key]);
        state.setAutoCode(true);

        _psysController->addCode(state);

    }
    progress.setValue(codes.keys().count());
}

void CFrmAdminInfo::OnCommitCodes2(QMap<QString, QString> codes)
{
    Q_UNUSED(codes);
    _psysController->clearLockAndCode2ForAllCodes();
}

void CFrmAdminInfo::OnNotifyDisableLockSelection()
{
    // KCB_DEBUG_ENTRY;
    ui->cbAdminLockSelection->setDisabled(true);
    if (KeyCodeBoxSettings::IsLockSelectionSingle())
    {
        ui->cbAdminLockSelection->setCurrentIndex(0);
    }
    else if (KeyCodeBoxSettings::IsLockSelectionMulti())
    {
        ui->cbAdminLockSelection->setCurrentIndex(0);
    }
    emit __NotifyLockSelectionChanged();
    // KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnNotifyEnableLockSelection()
{
    ui->cbAdminLockSelection->setEnabled(true);
    if (KeyCodeBoxSettings::IsLockSelectionSingle())
    {
        ui->cbAdminLockSelection->setCurrentIndex(0);
    }
    else if (KeyCodeBoxSettings::IsLockSelectionMulti())
    {
        ui->cbAdminLockSelection->setCurrentIndex(0);
    }
    emit __NotifyLockSelectionChanged();
}

void CFrmAdminInfo::OnUpdateCodes()
{
    // The code table has been updated.
    //     - We need to read the codes for the code display
    on_btnReadCodes_clicked();
    //     - We need to read the codes to populate the autocode locks
    m_autocodegen.UpdateCodes();
}

void CFrmAdminInfo::OnNotifyUpdateCabinetConfig()
{
    // KCB_DEBUG_ENTRY;
    invokeUpdateCabinetConfig();
    // KCB_DEBUG_EXIT;
}