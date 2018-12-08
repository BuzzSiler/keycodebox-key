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
#include "lockset.h"
#include "lockstate.h"
#include "menupushbutton.h"
#include <exception>
#include "encryption.h"
#include "linux/reboot.h"
#include <unistd.h>
#include "selectlockswidget.h"
#include "kcbkeyboarddialog.h"
#include "reportcontrolwidget.h"
#include "kcbsystem.h"
#include "frmnetworksettings.h"
#include "codeexporter.h"

#define ADMIN_TAB_INDEX (0)
#define REPORT_TAB_INDEX (2)
#define CODES_TAB_INDEX (4)
#define CODE_HISTORY_TAB_INDEX (5)

#define ACTION_INDEX_INSTALL_APP (0)
#define ACTION_INDEX_SET_BRANDING_IMAGE (1)
#define ACTION_INDEX_DEFAULT_BRANDING_IMAGE (2)
#define ACTION_INDEX_IMPORT_CODES (3)
#define ACTION_INDEX_EXPORT_CODES (4)
#define ACTION_INDEX_EXPORT_LOGS (5)

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
    // Explicit initialization needed to prevent spurious test emails
    _testEmail(false),
    m_select_locks(* new SelectLocksWidget(this, SelectLocksWidget::ADMIN)),
    m_report(* new ReportControlWidget(this))

{
    ui->setupUi(this);

    // For some reason, the admin form does not show full screen without the following
    // flags being set.  Maybe this should be don't at in the main so it gets
    // inherited?  Not sure.  Until this is resolved, just set these flags.
    CFrmAdminInfo::setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    CFrmAdminInfo::showFullScreen();

    ui->cbLockNum->setInsertPolicy(QComboBox::InsertAlphabetically);

    initialize();

    setAttribute(Qt::WA_AcceptTouchEvents, true);

    ui->cbUsbDrives->addItem("No Drive Inserted");
    m_file_filter = INSTALL_APP_FILTER;    
}

CFrmAdminInfo::~CFrmAdminInfo()
{
    delete ui;

    if(_pmodel) 
    {
        delete _pmodel;
    }

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
        _pworkingSet = 0;
    }
}

void CFrmAdminInfo::initializeConnections()
{
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

    connect(this, SIGNAL(__OnReadDoorLocksState()), _psysController, SLOT(OnReadLockStatus()));

    connect(_psysController, SIGNAL(__OnLockStatusUpdated(CLocksStatus*)), this, SLOT(OnLockStatusUpdated(CLocksStatus*)));

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

}

void CFrmAdminInfo::setSystemController(CSystemController *psysController)
{
    _psysController = psysController;

    initializeConnections();
    KCB_DEBUG_TRACE("emit __OnRequestCurrentAdmin");
    emit __OnRequestCurrentAdmin();
    emit __OnReadDoorLocksState();
}

void CFrmAdminInfo::show()
{
    QDialog::show();
    KCB_DEBUG_TRACE("Admin type is: " << _psysController->getAdminType());
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
        ui->vloReportSettings->addWidget(&m_report);
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
    FILE *pF;
    std::string sOutput = "";

    pF = popen(CMD_LIST_SYSTEM_FLAGS, "r");
    if(!pF)
    {
        qDebug() << "failed to list system flags";
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    qDebug() << "CFrmAdminInfo::isInternetTime(), " << QString::fromStdString(sOutput);
    qDebug() << "CFrmAdminInfo::isInternetTime(), " << QString::number(sOutput.find("internetTime"));

    if( sOutput.find("internetTime") != std::string::npos )
    {
        return true;
    }

    return false;
}

void CFrmAdminInfo::initialize()
{
    _pmodel = 0;
    _pcopymodel = 0;
    _bClose = false;
    ui->widgetEdit->setVisible(false);

    populateTimeZoneSelection(ui->cbTimeZone);
    startMediaTimer();

    ui->dtStartCodeList->setDisplayFormat(DATETIME_FORMAT);
    ui->dtEndCodeList->setDisplayFormat(DATETIME_FORMAT);
    ui->dtStartCodeHistoryList->setDisplayFormat(DATETIME_FORMAT);
    ui->dtEndCodeHistoryList->setDisplayFormat(DATETIME_FORMAT);
    
    ui->dtEndCodeList->setDateTime(QDateTime().currentDateTime());
    ui->dtEndCodeHistoryList->setDateTime(QDateTime().currentDateTime());

    setupCodeTableContextMenu();

    if( isInternetTime() )
    {
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
        ui->dtSystemTime->setDisabled(true);
        ui->cbInternetTime->setChecked(true);
    }

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabSelected(int)));
}

int CFrmAdminInfo::nthSubstr(int n, const std::string& s, const std::string& p)
{
    std::string::size_type i = s.find(p);     // Find the first occurrence

    int j;
    for (j = 1; j < n && i != std::string::npos; ++j)
    {
        i = s.find(p, i+1); // Find the next occurrence
    }

    if (j == n)
    {
        return(i);
    }
    else
    {
        return(-1);
    }
}

void CFrmAdminInfo::getSystemIPAddressAndStatus()
{
    QList< QPair<QString, unsigned int> > possibleMatches;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    if (ifaces.isEmpty())
    {
        return;
    }

    QList<QNetworkInterface> qualified_ifaces;

    for (int i=0; i < ifaces.size(); i++)
    {
        unsigned int flags = ifaces[i].flags();
        bool isLoopback = (bool)(flags & QNetworkInterface::IsLoopBack);
        bool isP2P = (bool)(flags & QNetworkInterface::IsPointToPoint);
        bool isRunning = (bool)(flags & QNetworkInterface::IsRunning);
        QString name = ifaces[i].name();

        /* The Omnikey5427 HID reader shows up on a USB network when plugged in
           I'm not sure if that is the reader or the pcscd application that is
           doing it.  Either way we aren't interested in USB networks
         */
        if ( name.contains("usb") )
        {
            continue;
        }

        // If this interface isn't running, we don't care about it
        if ( !isRunning ) 
        {
            continue;
        }

        // We only want valid interfaces that aren't loopback/virtual and not point to point
        if ( !ifaces[i].isValid() || isLoopback || isP2P ) 
        {
            continue;
        }

        qualified_ifaces.append(ifaces[i]);
    }

    foreach (auto iface, qualified_ifaces)
    {
        QList<QNetworkAddressEntry> addresses = iface.addressEntries();

        foreach (auto addrEntry, addresses)
        {
            QHostAddress addr = addrEntry.ip();

            // Ignore local host
            if ( addr == QHostAddress::LocalHost ) 
            {
                continue;
            }

            // Ignore non-ipv4 addresses
            if ( !addr.toIPv4Address() ) 
            {
                continue;
            }

            QString ip = addr.toString();
            if ( ip.isEmpty() ) 
            {
                continue;
            }

            possibleMatches.append(QPair<QString, unsigned int>(ip, iface.flags()));
        }
    }

    QPair<QString,unsigned int> ip_display(QString(tr("No Valid Address")), 0);

    if (possibleMatches.length() == 1)
    {
        bool checkSuccessful = false;
        QPair<QString, unsigned int>ip_display(possibleMatches[0]);
    
        if (ip_display.second & QNetworkInterface::IsUp)
        {
            QString stylesheet = (ip_display.second & QNetworkInterface::CanMulticast) ? CAN_MULTICAST_STYLESHEET : CANNOT_MULTICAST_STYLESHEET;
            ui->lblIPAddress->setStyleSheet(stylesheet);

            QString gateway = kcb::GetGatewayAddress();
            checkSuccessful = kcb::FPingAddress(gateway);
        }

        ui->lblIPAddress->setText(ip_display.first);
        ui->lblIPAddress->setVisible(checkSuccessful);
        if( !checkSuccessful )
        {
            ui->lblIPAddress->setStyleSheet(IP_ADDRESS_ERROR);
        }
    }
    else
    {
        ui->lblIPAddress->setText(ip_display.first);
        ui->lblIPAddress->setStyleSheet(IP_ADDRESS_ERROR);
    }
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

    getSystemIPAddressAndStatus();

    _timerMedia.start();
}

void CFrmAdminInfo::populateTimeZoneSelection(QComboBox *cbox)
{
    FILE *pF;
    std::string sOutput = "";
    std::string sTimeZone = "";
    std::string parsedString = "";

    cbox->clear();
    // Fill in combo box.
    QList<QByteArray> ids = QTimeZone::availableTimeZoneIds();
    foreach (QByteArray id, ids) 
    {
        cbox->addItem(id);
    }

    qDebug() << "DFrmAdminInfo::populateTimeZoneSelection(...): setting current timezone";

    pF = popen(CMD_READ_TIME_ZONE, "r");
    if(!pF)
    {
        qDebug() << "failed to parse timezone string";
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
    qDebug() << "CFrmAdminInfo::setTimeZone()";
    qDebug() << ui->cbTimeZone->currentText();

    // setTimeZone

    QString unlink = QString("sudo unlink /etc/localtime");
    QString link = QString("sudo ln -s /usr/share/zoneinfo/") + ui->cbTimeZone->currentText() + QString(" /etc/localtime");

    qDebug() << "timezone change: ";
    qDebug() << unlink;
    qDebug() << link;

    std::system(unlink.toStdString().c_str());
    std::system(link.toStdString().c_str());

    if(ui->cbInternetTime->isChecked())
    {
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
    }
}

void CFrmAdminInfo::onRootPathChanged(QString path)
{
    qDebug() << "Root path loaded" << path;
}

void CFrmAdminInfo::onModelDirectoryLoaded(QString path)
{
    qDebug() << "loaded" << path;
    _pmodel->sort(0, Qt::AscendingOrder);
}

void CFrmAdminInfo::populateFileCopyWidget(QString sDirectory, QStringList sFilter)
{
    //KCB_DEBUG_ENTRY;
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
    ui->treeViewCopy->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeViewCopy->header()->resizeSection(1, 60);
    ui->treeViewCopy->header()->resizeSection(2, 60);
    ui->treeViewCopy->header()->setStretchLastSection(false);

    //KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::onCopyRootPathChanged(QString path)
{
    qDebug() << "Root path loaded" << path;
}

void CFrmAdminInfo::onCopyModelDirectoryLoaded(QString path)
{
    KCB_DEBUG_TRACE("loaded" << path);
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

    //KCB_DEBUG_TRACE("Copy from File" << _copyDirectory);
    //KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnCopyFile_clicked()
{
    KCB_DEBUG_TRACE(_copyDirectory);

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


void parseCode (xmlNodePtr cur,
                QString& locks, QString &code1, QString &code2, QString &username, 
                QString &question1, QString &question2, QString &question3, 
                QDateTime &dtStart, QDateTime &dtEnd, int *access_type) 
{
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"locks")))
        {
            locks = QString::fromLatin1((char *)xmlNodeGetContent(cur));
            qDebug() << "Locks" << locks;
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"code1")))
        {
            code1 = QString::fromLatin1((char *)xmlNodeGetContent(cur));
            qDebug() << "code1" << code1;
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"code2")))
        {
            code2 = QString::fromLatin1((char *)xmlNodeGetContent(cur));  
            qDebug() << "code2" << code2;            
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
        {
            username = QString::fromLatin1((char *)xmlNodeGetContent(cur));  
            qDebug() << "username" << username;            
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"question1")))
        {
            question1 = QString::fromLatin1((char *)xmlNodeGetContent(cur));
            qDebug() << "question1" << question1;
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"question2")))
        {
            question2 = QString::fromLatin1((char *)xmlNodeGetContent(cur));  
            qDebug() << "question2" << question2;
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"question3")))
        {
            question3 = QString::fromLatin1((char *)xmlNodeGetContent(cur));  
            qDebug() << "question3" << question3;
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"startDT")))
        {
            dtStart = QDateTime::fromString(QString::fromLatin1((char *)xmlNodeGetContent(cur)),Qt::ISODate);  
            qDebug() << "startDT" << dtStart.toString();
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"endDT")))
        {
            dtEnd = QDateTime::fromString(QString::fromLatin1((char *)xmlNodeGetContent(cur)),Qt::ISODate);
            qDebug() << "endDT" << dtEnd.toString();
        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"accesstype")))
        {
            *access_type = atoi((char *)xmlNodeGetContent(cur));
            qDebug() << "accesstype" << *access_type;  
        }
        cur = cur->next;
    }
}

void CFrmAdminInfo::importAsXml()
{
    KCB_DEBUG_ENTRY;

    xmlDocPtr doc = NULL;

    if ( (doc = xmlReadFile(_copyDirectory.toStdString().c_str(), NULL, 0)) == NULL )
    {
        (void)QMessageBox::warning(this, tr("XML Parsing Failed"),
                                       tr("Can't parse given XML file\nPlease check syntax and integrity of your file."),
                                       QMessageBox::Ok);
        qDebug() << "CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked(), can't parse XML file: " << _copyDirectory;
        return;
    }

    if (!_pworkingSet)
    {
        _pworkingSet = new CLockSet();
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);

    if (cur == NULL) 
    {
        qDebug() << "empty document";
        xmlFreeDoc(doc);
        return;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "codeListing")) 
    {
        qDebug() << "document of the wrong type, root node != codeListing";
        xmlFreeDoc(doc);
        return;
    }    

    QString locks = "";
    QString code1 = "";
    QString code2 = "";    
    QString username = "";
    QString question1 = "";
    QString question2 = "";
    QString question3 = "";
    int access_type = 0;    

    cur = cur->xmlChildrenNode;
    while (cur != NULL) 
    {
        // Default start/end date/time to today's date
        QDateTime dtStart = QDateTime::currentDateTime();
        QDateTime dtEnd = QDateTime::currentDateTime();

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"code")))
        {
            qDebug() << "Parsing code node";
            parseCode (cur, locks, code1, code2, username, question1, question2, question3,
                        dtStart, dtEnd, &access_type);
            qDebug() << "Parse complete";

            /* Initial start/end dates (if present) based on access type
               There are three access types:
                   - Always
                   - Timed
                   - Limited Use
               Start/end dates are only valid for 'Timed' codes.  All other codes should be DEFAULT_DATETIME
               While they are initialized above, it is possible that a customer will put a start/end
               datetime in the XML file.               
            */
            if (access_type == ACCESS_TYPE_ALWAYS || access_type == ACCESS_TYPE_LIMITED_USE)
            {
                dtStart = DEFAULT_DATETIME;
                dtEnd = DEFAULT_DATETIME;
            }

            _pState = createNewLockState();

            setPStateValues(locks, code1, code2, username,
							dtStart, dtEnd, false, false, false, 
            				question1, question2, question3,
                            access_type);
            HandleCodeUpdate();
        }
        cur = cur->next;
    }
    
    xmlFreeDoc(doc);
    xmlCleanupParser();

    (void) QMessageBox::warning(this, tr("Bulk Code Upload Complete"),
                                   tr("Please check the 'Codes' tab to see that your codes were added successfully."),
                                   QMessageBox::Ok);
    //KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnCopyFileBrandingImage_clicked()
{
    KCB_DEBUG_ENTRY;

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
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnCopyFileBrandingImageReset_clicked()
{
    KCB_DEBUG_ENTRY;
    int nRC = QMessageBox::warning(this, tr("Verify Branding Image Reset"),
                                   tr("The branding image will be reset to the default.\nDo you want to continue?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) 
    {
        sync();
        std::system("cp /home/pi/kcb-config/images/alpha_logo_touch.jpg /home/pi/kcb-config/images/alpha_logo.jpg");
    }
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::onStopEdit()
{
    ui->tabWidget->show();
    ui->widgetEdit->hide();
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

void CFrmAdminInfo::on_btnDone_clicked()
{
    KCB_DEBUG_ENTRY;
    /* This slot is called when we are leaving the admin interface.
       It seems like the most obvious place to re-enable display power
       It would be preferrable to have all power control in the same
       module -- maybe one day :-)
    */
    system(qPrintable("vcgencmd display_power 1"));

    // Update the Admin Info and close the dialog - syscontroller needs to switch
    _tmpAdminRec.setAdminName(ui->lblName->text());
    _tmpAdminRec.setAdminEmail(ui->lblEmail->text());
    _tmpAdminRec.setAdminPhone(ui->lblPhone->text());
    _tmpAdminRec.setPassword(ui->lblPassword->text());
    _tmpAdminRec.setAssistPassword(ui->lblAssistPassword->text());
    _tmpAdminRec.setDisplayFingerprintButton(ui->chkDisplayFingerprintButton->isChecked());
    _tmpAdminRec.setDisplayShowHideButton(ui->chkDisplayShowHideButton->isChecked());
    _tmpAdminRec.setDisplayPowerDownTimeout(ui->cbDisplayPowerDownTimeout->currentIndex());
    _tmpAdminRec.setDisplayTakeReturnButtons(ui->chkDisplayTakeReturnButtons->isChecked());

    _bClose = true;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::hideKeyboard(bool bHide) 
{
    ui->widgetEdit->setVisible(bHide);
}

void CFrmAdminInfo::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    // New Admin info to display...
    // Display to the ui
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
}

void CFrmAdminInfo::OnUpdatedCurrentAdmin(bool bSuccess)
{
    KCB_DEBUG_ENTRY;
    if( bSuccess )
    {
        KCB_DEBUG_TRACE("Succeeded in updating Current Admin Info");

        /* Note: This is such an ugly hack.  I hate it. 
           I think the better way is to separate "Save and Close" so that
           the user can "Save" without closing and then you can enable
           the "Test Email" button once it is saved.
        */
        if (_testEmail)
        {
            emit __OnSendTestEmail(2 /*ADMIN_RECV*/);
            _testEmail = false;
        }

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
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::OnFoundNewStorageDevice(QString device0, QString device1)
{
    KCB_DEBUG_ENTRY;
    _pcopymodel = 0;
    qDebug() << device0;
    qDebug() << device1;
    usbDevice0 = QString(device0);
    usbDevice1 = QString(device1);
    qDebug() << usbDevice0;
    qDebug() << usbDevice1;

    QStringList list;

    if (!usbDevice0.isEmpty())
    {
        list << QString("/media/pi/%1").arg(usbDevice0);
    }
    
    if (!usbDevice1.isEmpty())
    {
        list << QString("/media/pi/%1").arg(usbDevice1);
    }

    KCB_DEBUG_TRACE("USBDevices" << list);
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
        //TODO: reset the values...didn't save
        KCB_DEBUG_TRACE("unsuccessful code state update");
    }
    on_btnReadCodes_clicked();
}

void CFrmAdminInfo::on_cbInternetTime_clicked()
{
    // Internet time set checked
    if(ui->cbInternetTime->isChecked())
    {
        //we check this flag in keycodeboxmain.cpp
        std::system("touch /home/pi/run/internetTime.flag");

        setTimeZone();
        ui->dtSystemTime->setDisabled(true);
        // update system time here
        KCB_DEBUG_TRACE("ntp ON");
        std::system("sudo /etc/init.d/ntp stop");

        QCoreApplication::processEvents();
        KCB_DEBUG_TRACE("ntpd -s");
        std::system("sudo ntpd -s");
        KCB_DEBUG_TRACE("ntp OFF");

        QCoreApplication::processEvents();

        std::system("sudo /etc/init.d/ntp start");
        KCB_DEBUG_TRACE("setting datetime text");
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
    }
    else
    {
        std::system("rm -rf /home/pi/run/internetTime.flag");
        ui->dtSystemTime->setDisabled(false);
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
    }
}

void CFrmAdminInfo::on_btnSetTime_clicked()
{
    if( !isInternetTime() )
    {
        setTime();
        setTimeZone();
    }
}

void CFrmAdminInfo::OnCloseAdmin() 
{
    this->close();
}

void CFrmAdminInfo::OnLockStatusUpdated(CLocksStatus *locksStatus)
{
    KCB_DEBUG_ENTRY;
    _pLocksStatus = locksStatus;
}

void CFrmAdminInfo::OnLockSet(CLockSet *pSet)
{
    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");
    if (pSet != nullptr)
    {
        KCB_DEBUG_TRACE("Count" << pSet->getLockMap()->size());
    }

    displayInTable(pSet);
    
    pSet = nullptr;
}

void CFrmAdminInfo::OnLockHistorySet(CLockHistorySet *pSet)
{
    KCB_DEBUG_ENTRY;

    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");

    displayInHistoryTable(pSet);
    
    pSet = nullptr;

    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::createCodeTableHeader()
{
    KCB_DEBUG_ENTRY;

    QTableWidget *table = ui->tblCodesList;

    table->clear();
    table->setColumnCount(7);

    table->setColumnWidth(0, 40);
    table->setColumnWidth(1, 80);
    table->setColumnWidth(2, 150);
    table->setColumnWidth(3, 110);
    table->setColumnWidth(4, 110);
    table->setColumnWidth(5, 155);
    table->setColumnWidth(6, 155);

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

    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::displayInTable(CLockSet *pSet)
{
    KCB_DEBUG_ENTRY;

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
    QSet<int> lock_items;
    QTableWidget *table = ui->tblCodesList;

    table->setRowCount(pSet->getLockMap()->size());

    for(itor = pSet->begin(); itor != pSet->end(); itor++)
    {
        pState = itor.value();

        // Locks can be single or comma-separated.
        QString locks = pState->getLockNums();
        QStringList sl = locks.split(',');
        foreach (auto s, sl)
        {
            lock_items.insert(s.toInt());
        }

        nCol = 0;
        table->setItem(nRow, nCol++, new QTableWidgetItem(QVariant(nRow + 1).toString()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getLockNums()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription()));
        QString code1 = pState->getCode1();
        if (pState->getFingerprint1())
        {
            code1 += tr(" (FP)");
        }
        table->setItem(nRow, nCol++, new QTableWidgetItem(code1));
        QString code2 = pState->getCode2();
        if (pState->getAskQuestions())
        {
            code2 += tr(" (Q)");
        }
        table->setItem(nRow, nCol++, new QTableWidgetItem(code2));

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

        nRow++;
    }

    // Note: We want the entries in the combo box to be in numeric order
    // We gather the locks using a set to remove duplicates.  Unfortunately
    // sets cannot be sorted, so we convert to a list, so and add to the
    // combo box.
    QList<int> lock_items_list(lock_items.toList());
    qSort(lock_items_list.begin(), lock_items_list.end());
    foreach (auto i, lock_items_list)
    {
        ui->cbLockNum->addItem(QString::number(i));
    }

    _codesInUse.clear();
    _psysController->getAllCodes1(_codesInUse);

    // KCB_DEBUG_EXIT();
}

void CFrmAdminInfo::setupCodeTableContextMenu() 
{
    QTableWidget *table = ui->tblCodesList;

    /* Create a menu for adding, editing, and deleting codes */
    _pTableMenu = new QMenu(table);

    /* Find out what column we are and add an action for that specific column
           e.g., If we select a cell in the Lock # column then the menu should
           start with Edit Locks.  Selecting edit locks will bring up a dialog
           of bank, link, and 32 buttons popuplated with the locks that are 
           already selected.  User can choose the locks to be associated with 
           this 'code' or authorization.

           Note: Duplicate codes are not allowed.
    */

    _pTableMenu->addAction(tr("Edit Code"), this, SLOT(codeEditSelection()));
    _pTableMenu->addAction(tr("Add Code"), this, SLOT(codeInitNew()));
    _pTableMenu->addAction(tr("Delete"), this, SLOT(codeDeleteSelection()));
    connect(table,SIGNAL(cellClicked(int,int)),this,SLOT(OnRowSelected(int, int)));

    /* Create a menu for adding codes and enabling 'limited use' access type codes */
    _pTableMenuAdd = new QMenu(table);
    connect(table->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnHeaderSelected(int)));
}

void CFrmAdminInfo::displayInHistoryTable(CLockHistorySet *pSet)
{
    KCB_DEBUG_ENTRY;
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
    table->clearContents();
    table->setRowCount(pSet->getLockHistoryMap()->size());
    table->setColumnCount(6);

    table->setColumnWidth(0, 80);
    table->setColumnWidth(1, 120);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 100);
    table->setColumnWidth(4, 350);
    table->setColumnWidth(5, 50);
    
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

    while (itor.hasNext())
    {
        pState = itor.next();
        
        QString locks = pState->getLockNums();
        QStringList sl = locks.split(',');
        foreach (auto s, sl)
        {
            lock_items.insert(s.toInt());
        }
        
        nCol = 0;
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getLockNums()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getCode1()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getCode2()));
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
    KCB_DEBUG_EXIT;
    
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

void CFrmAdminInfo::codeAddNew()
{
    KCB_DEBUG_ENTRY;
    addCodeByRow();
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
    KCB_DEBUG_TRACE("Enabling all Limited Use codes");

    /* Looping over all the codes in the working set
           if code access type is 'limited use' then
               reset using __OnUpdateCodeState
    */
    CLockSet::Iterator itor;
    int nRow = 0;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState in CFrmAdminInfo::codeEnableAll");
        _pState = 0;
    }

    if(!_pworkingSet) 
    {
        _pworkingSet = new CLockSet();
    }

    on_btnReadCodes_clicked();
    displayInTable(_pworkingSet);

    KCB_DEBUG_TRACE("before loop iterator cellClicked");

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
    KCB_DEBUG_TRACE(QVariant(_nRowSelected).toString());
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
    KCB_DEBUG_ENTRY;

    QDateTime dtStart = ui->dtStartCodeList->dateTime();
    QDateTime dtEnd = ui->dtEndCodeList->dateTime();
    QString locks = ui->cbLockNum->currentText();

    KCB_DEBUG_TRACE(locks);

    if (locks == tr("All Locks"))
    {
        locks = "*";
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
    // setTime
    QDateTime   dt = ui->dtSystemTime->dateTime();
    QString updateTime = "sudo ntpq -p";
    QString sDate = QString("sudo date ") + dt.toString("MMddhhmmyyyy.ss");
    std::system(updateTime.toStdString().c_str());
    std::system(sDate.toStdString().c_str());
    KCB_DEBUG_TRACE("system time:" << sDate);
    std::system("sudo hwclock --systohc");    // Set the hardware clock
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
    KCB_DEBUG_ENTRY;
    if (_pFrmCodeEditMulti)
    {
        _pFrmCodeEditMulti->getValues(_pState);
        _pFrmCodeEditMulti->hide();
        HandleCodeUpdate();
    }
    KCB_DEBUG_EXIT;
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
    _pState->show();

    if(_pState->isNew())
    {
        KCB_DEBUG_TRACE("new _pState");
        if(_pworkingSet)
        {
            KCB_DEBUG_TRACE("adding _pState to _pworkingSet");
            _pworkingSet->addToSet(_pState);
        }
        KCB_DEBUG_TRACE("emitting __OnUpdateCodeState");
        emit __OnUpdateCodeState(_pState);
    } 
    else 
    {
        int nCol = 1;
        QTableWidget     *table = ui->tblCodesList;
        QTableWidgetItem *item;
        item = table->item(_nRowSelected, nCol++);

        KCB_DEBUG_TRACE("Item Text:" << item->text());


        /* Note: The following code seems to assume dtStart/dtEnd will be datetime, but what about ALWAYS? */
        KCB_DEBUG_TRACE("not new _pState");

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

            KCB_DEBUG_TRACE("start time" << _pState->getStartTime().toString() << "end time" << _pState->getEndTime().toString());

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
    Q_UNUSED(column);
    _nRowSelected = row;
    KCB_DEBUG_TRACE(_nRowSelected);
    _pTableMenu->show();

    QPoint widgetPoint = QWidget::mapFromGlobal(QCursor::pos());
    _pTableMenu->setGeometry(widgetPoint.x(), widgetPoint.y(), _pTableMenu->width(), _pTableMenu->height());
}

bool CFrmAdminInfo::eventFilter(QObject *target, QEvent *event)
{
    KCB_DEBUG_TRACE("eventFilter. Event type:" << QVariant(event->type()).toString());
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
    KCB_DEBUG_TRACE(code1 << code2);
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
    /* Originally, the actions were added in setupCodeTableContextMenu but when they were displayed
       here, the menu was the size of the _pTableMenu.  I couldn't track down why, so I decided to
       just clear the menu and add what was needed here.  I like the just-in-time approach better
    */

    _pTableMenuAdd->clear();
    _pTableMenuAdd->addAction(tr("Add Code"), this, SLOT(codeInitNew()));
    if (nHeader == 5)
    {
        _pTableMenuAdd->addAction(tr("Enable All"), this, SLOT(codeEnableAll()));
    }
    _pTableMenuAdd->show();

    QPoint widgetPoint = QWidget::mapFromGlobal(QCursor::pos());
    _pTableMenuAdd->setGeometry(widgetPoint.x(), widgetPoint.y(), _pTableMenuAdd->width(), _pTableMenuAdd->height());
}

void CFrmAdminInfo::checkAndCreateCodeEditForm()
{
    if(!_pFrmCodeEditMulti)
    {
        _pFrmCodeEditMulti = new FrmCodeEditMulti(this);
        connect(_pFrmCodeEditMulti, SIGNAL(rejected()), this, SLOT(OnCodeEditReject()));
        connect(_pFrmCodeEditMulti, SIGNAL(accepted()), this, SLOT(OnCodeEditAccept()));
        connect(this, SIGNAL(__OnAdminInfoCodes(QString,QString)), _pFrmCodeEditMulti, SIGNAL(__OnAdminInfoCodes(QString,QString)));
    }
}

void CFrmAdminInfo::addCodeByRow()
{
    KCB_DEBUG_ENTRY;
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

    _pFrmCodeEditMulti->setValues(_pState, _codesInUse);
    _pFrmCodeEditMulti->show();
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::editCodeByRow(int row)
{
    KCB_DEBUG_TRACE(row);
    
    checkAndCreateCodeEditForm();

    KCB_DEBUG_TRACE("Created CodeEditForm");

    // Get line values
    CLockSet::Iterator itor;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }
    int nRow = 0;

    KCB_DEBUG_TRACE("before loop iterator cellClicked");

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        if(nRow == row)
        {
            // itor is our man!
            _pState = itor.value();
            break;
        }
        nRow++;
    }

    if(_pState)
    {
        _pFrmCodeEditMulti->setValues(_pState, _codesInUse);
        _pFrmCodeEditMulti->show();
    }
    else
    {
        _pState = createNewLockState();
        _pworkingSet->addToSet(_pState);

        _pFrmCodeEditMulti->setValues(_pState, _codesInUse);
        _pFrmCodeEditMulti->show();
    }
}

void CFrmAdminInfo::deleteCodeByRow(int row)
{
    KCB_DEBUG_ENTRY;
    checkAndCreateCodeEditForm();

    // Get line values
    CLockSet::Iterator itor;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }
    int nRow = 0;

    KCB_DEBUG_TRACE("before loop iterator cellClicked");

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        if(nRow == row)
        {
            // itor is our man!
            _pState = itor.value();
            break;
        }
        nRow++;
    }

    if(_pState)
    {
        _pState->setMarkForDeletion();

        QString cmd = QString("%1%2").arg(QString(CMD_REMOVE_FP_FILE)).arg(_pState->getCode1());

        KCB_DEBUG_TRACE("cmd: " << cmd);

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
    // Removes all codes
    KCB_DEBUG_ENTRY;

    // Get line values
    CLockSet::Iterator itor;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }
    int nRow = 0;

    if(!_pworkingSet)
    {
        _pworkingSet = new CLockSet();
    }

    on_btnReadCodes_clicked();
    displayInTable(_pworkingSet);

    qDebug() << "before loop iterator cellClicked";

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        // itor is our man!
        _pState = itor.value();

        if(_pState)
        {
            _pState->setMarkForDeletion();
            emit __OnUpdateCodeState(_pState);
        }
        nRow++;
    }

    KCB_DEBUG_TRACE(CMD_REMOVE_ALL_FP_FILES);
    std::system( CMD_REMOVE_ALL_FP_FILES );
    KCB_DEBUG_EXIT;
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
    KCB_DEBUG_ENTRY;

    on_btnReadCodes_clicked();

    int nRC = QMessageBox::warning(this, tr("Verify Remove All Codes"),
                                   tr("All access codes will be removed from the system\nDo you want to continue?"),
                                   QMessageBox::Ok, QMessageBox::Cancel);
    if(nRC == QMessageBox::Ok)
    {
        KCB_DEBUG_TRACE("Ok Selected");
        purgeCodes();
        usleep(50000);

        on_btnReadCodes_clicked();

        nRC = QMessageBox::warning(this, tr("Code Removal Success"),
                                   tr("Code Removal is successful!!\nPlease give the codes list a moment to update."),
                                   QMessageBox::Ok);
    }
}

void CFrmAdminInfo::OnTabSelected(int index)
{
    static int last_index = 0;
    // Note: Index is new selected tab

    KCB_DEBUG_TRACE(last_index << index);

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

    if (last_index != REPORT_TAB_INDEX && index == REPORT_TAB_INDEX)
    {
        // If we have selected the Report Tab, we want to populate with the latest information
        m_report.setValues(_tmpAdminRec);
    }

    if (last_index == REPORT_TAB_INDEX && index != last_index)
    {
        // If we are changing away from the Report Tab then we want to update to the latest values
        m_report.getValues(_tmpAdminRec);
    }

    last_index = index;
}

void CFrmAdminInfo::on_btnTestEmail_clicked()
{
    KCB_DEBUG_TRACE("Testing admin send email");
    emit __OnSendTestEmail(1 /*ADMIN_SEND*/);
}

void CFrmAdminInfo::on_btnTestUserEmail_clicked()
{
    KCB_DEBUG_TRACE("Testing admin recv email");
    _testEmail = true;
    emit __OnSendTestEmail(2 /*ADMIN_RECV*/);
    _testEmail = false;
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

void CFrmAdminInfo::OnOpenLockRequest(QString lock, bool is_user)
{
    Q_UNUSED(is_user);
    // Only allow this path if we are admin, i.e., not user
    // Note: It should never be the case that we are not admin
    Q_ASSERT_X(is_user == false, Q_FUNC_INFO, "We are not admin");
    emit __OnOpenLockRequest(lock);
}

void CFrmAdminInfo::on_pbNetworkSettings_clicked()
{
    FrmNetworkSettings ns;

    ns.setValues(_tmpAdminRec.getVNCPort(), _tmpAdminRec.getVNCPassword(),
                 _tmpAdminRec.getSMTPServer(), _tmpAdminRec.getSMTPPort(), _tmpAdminRec.getSMTPType(), _tmpAdminRec.getSMTPUsername(), _tmpAdminRec.getSMTPPassword()
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

        // Write VNC credentials to file
        FILE *pF;
        std::string sOutput = "";
        QString createCmd = "echo '|";
        createCmd += QString::number(vncPort);
        createCmd += " ";
        createCmd += vncPassword;
        createCmd +="|' > /home/pi/run/vnc_creds.txt";

        pF = popen(createCmd.toStdString().c_str(), "r");
        if(!pF)
        {
            qDebug() << "failed to create vnc file";
        }

        ExtractCommandOutput(pF, sOutput);
        fclose(pF);

    }
}

void CFrmAdminInfo::codeHistoryTableCellSelected(int row, int col)
{
    KCB_DEBUG_TRACE("row" << row << "col" << col);

    if (col == 5)
    {     
        const QVector<CLockHistoryRec*>* set = _phistoryWorkingSet->getLockHistoryMap();

        CLockHistoryRec* rec = (*set)[row];
        QByteArray ba = rec->getImage();
        QPixmap pm;
        pm.loadFromData(ba);
        KCB_DEBUG_TRACE("pm size" << pm.size());
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
    KCB_DEBUG_ENTRY;
    ui->cbFileFormat->setEnabled(false);
    ui->cbEncryptedClear->setEnabled(false);
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
                // Only XML format is supported
                //      - set format to xml
                //      - disable format and security control
                int index = ui->cbFileFormat->findText("XML");
                KCB_DEBUG_TRACE("found XML at" << index);
                ui->cbFileFormat->setCurrentIndex(index);
                ui->cbFileFormat->setDisabled(true);
                ui->cbEncryptedClear->setDisabled(true);                
                m_util_action = UTIL_ACTION_IMPORT_CODES;
            }
            else if (index == ACTION_INDEX_EXPORT_CODES)
            {
                ui->btnActionExecute->setEnabled(true);
                ui->cbFileFormat->setEnabled(true);
                ui->cbEncryptedClear->setEnabled(true);
                m_util_action = UTIL_ACTION_EXPORT_CODES;
            }
            break;

        case ACTION_INDEX_EXPORT_LOGS:
            ui->btnActionExecute->setEnabled(true);
            m_util_action = UTIL_ACTION_EXPORT_LOGS;
            m_file_filter = QStringList() << "*.log";
            break;        

        default:
            KCB_DEBUG_TRACE("Unknown action index" << index);
            break;
    }

    populateFileCopyWidget(ui->cbUsbDrives->currentText());

    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnActionExecute_clicked()
{
    switch (m_util_action)
    {
        case UTIL_ACTION_INSTALL_APP:
            on_btnCopyFile_clicked();
            break;

        case UTIL_ACTION_SET_BRANDING_IMAGE:
            on_btnCopyFileBrandingImage_clicked();
            break;

        case UTIL_ACTION_DEFAULT_BRANDING_IMAGE:
            on_btnCopyFileBrandingImageReset_clicked();
            break;

        case UTIL_ACTION_IMPORT_CODES:
            {
                QString security = ui->cbEncryptedClear->currentText().toLower();
                QString format = ui->cbFileFormat->currentText().toLower();
                int nRC = QMessageBox::warning(this, tr("Code Import"),
                                            QString(tr("You have selected import codes in %1 format (%2)\n"
                                                       "Continuing will import all of the codes into the database.\n"
                                                       "If there are duplicate codes, they must be removed manually.\n"
                                                       "Do you want to continue?"
                                                      )).arg("XML").arg(security == "clear" ? "unencrypted" : "encrypted"),
                                            QMessageBox::Yes, QMessageBox::No);
                if (nRC == QMessageBox::Yes)                                        
                {
                    importAsXml();
                }
            }
            break;

        case UTIL_ACTION_EXPORT_CODES:
            {
                CLockSet *lockset;                
                QString format = ui->cbFileFormat->currentText().toLower();
                QString security = ui->cbEncryptedClear->currentText().toLower();

                bool clear_or_encrypted = security == "clear" ? true : false;
                QString root_path = ui->cbUsbDrives->currentText();
            
                // A CLockSet object was allocated by this call to readAllCodes
                // It must be delete before leaving this code block.
                _psysController->readAllCodes(&lockset, clear_or_encrypted);
                if (lockset && lockset->isValid())
                {
                    CodeExporter exporter(CodeExporter::StringToFormat(format), root_path, *lockset, clear_or_encrypted);
                    bool result = exporter.Export();
                    //delete lockset;
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
                QString source("/home/pi/kcb-config/logs/stderr.log");
                QString target(QString("%1/messages_%2.log").arg(ui->cbUsbDrives->currentText()).arg(QDateTime::currentDateTime().toString(REPORT_FILE_FORMAT)));
                bool result = QFile::copy(source, target);
                if (!result)
                {
                    KCB_DEBUG_TRACE("failed to copy logs");
                }
            }
            break;

        default:
            KCB_DEBUG_TRACE("Unknown utility action" << m_util_action);
            break;
    }
    KCB_DEBUG_EXIT;    
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
    }
    else
    {
        ui->cbUsbDrives->addItem("No Drive Inserted");
        ui->cbActionsSelect->setDisabled(true);
        ui->treeViewCopy->setModel(nullptr);
    }
}

void CFrmAdminInfo::on_cbUsbDrives_currentIndexChanged(const QString &arg1)
{
    KCB_DEBUG_ENTRY;

    populateFileCopyWidget(arg1);

    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_cbFileFormat_currentIndexChanged(const QString &arg1)
{
    KCB_DEBUG_ENTRY;

    setFileFilterFromFormatSelection(arg1);
    populateFileCopyWidget(ui->cbUsbDrives->currentText());

    KCB_DEBUG_EXIT;
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

//-------------------------------------------------------------------------------------------------
// EOF