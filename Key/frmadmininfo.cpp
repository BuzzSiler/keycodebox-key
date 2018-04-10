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
#include "lockset.h"
#include "lockstate.h"
#include "menupushbutton.h"
#include <exception>
#include "encryption.h"
#include "linux/reboot.h"
#include <unistd.h>
#include "selectlockswidget.h"
#include "kcbcommon.h"
#include "kcbkeyboarddialog.h"


static const char CMD_REMOVE_ALL_FP_FILES[] = "sudo rm -rf /home/pi/run/prints/*";
static const char CMD_LIST_SYSTEM_FLAGS[] = "ls /home/pi/run/* | grep 'flag'";
static const char CMD_READ_TIME_ZONE[] = "readlink /etc/localtime";
static const char CMD_REMOVE_FP_FILE[] = "sudo rm -rf /home/pi/run/prints/\%1";


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
    m_select_locks(* new SelectLocksWidget(this, SelectLocksWidget::ADMIN))

{
    ui->setupUi(this);

    ui->cbLockNum->setInsertPolicy(QComboBox::InsertAlphabetically);

    CFrmAdminInfo::showFullScreen();

    initialize();

    setAttribute(Qt::WA_AcceptTouchEvents, true);
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

    connect(&m_select_locks, &SelectLocksWidget::NotifyRequestLockOpen, this, &CFrmAdminInfo::OnOpenLockRequest);

}

void CFrmAdminInfo::setSystemController(CSystemController *psysController)
{
    _psysController = psysController;

    initializeConnections();
    qDebug() << "CFrmAdminInfo::setSystemController() -> emit __OnRequestCurrentAdmin()";
    emit __OnRequestCurrentAdmin();
    emit __OnReadDoorLocksState();
}

void CFrmAdminInfo::show()
{
    QDialog::show();
    qDebug() << "Admin type is: " + _psysController->getAdminType();
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
        // Force tabwidget to show administrator tab
        emit ui->tabWidget->currentChanged(0);
    }

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
    ui->widgetImmediateReport->setVisible(true);

    populateTimeZoneSelection(ui->cbTimeZone);
    startMediaTimer();

    ui->dtStartCodeList->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dtEndCodeList->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dtStartCodeHistoryList->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dtEndCodeHistoryList->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dtStartReport->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dtEndReport->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    
    ui->dtEndCodeList->setDateTime(QDateTime().currentDateTime());
    ui->dtEndCodeHistoryList->setDateTime(QDateTime().currentDateTime());
    ui->dtEndReport->setDateTime(QDateTime().currentDateTime());

    setupCodeTableContextMenu();

    if( isInternetTime() )
    {
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
        ui->dtSystemTime->setDisabled(true);
        ui->cbInternetTime->setChecked(true);
    }

    ui->btnCopyToggleSource->setText(tr("Source #1"));


    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabSelected(int)));

    ui->btnCopyFileBrandingImageReset->setEnabled(true);
}

int CFrmAdminInfo::nthSubstr(int n, const std::string& s, const std::string& p)
{
    std::string::size_type i = s.find(p);     // Find the first occurrence

    int j;
    for (j = 1; j < n && i != std::string::npos; ++j)
        i = s.find(p, i+1); // Find the next occurrence

    if (j == n)
        return(i);
    else
        return(-1);
}

void CFrmAdminInfo::getSystemIPAddressAndStatus()
{
    QList<QString> possibleMatches;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    static const QString red("color: rgb(255, 0, 0);");
    static const QString blue("color: rgb(0, 0, 255);");
    static const QString med_sea_green("color: rgb(60,179,113);");
    static const QString white("color: rgb(255, 255, 255);");

    if (!ifaces.isEmpty())
    {
        for(int i=0; i < ifaces.size(); i++)
        {
            unsigned int flags = ifaces[i].flags();
            bool isLoopback = (bool)(flags & QNetworkInterface::IsLoopBack);
            bool isP2P = (bool)(flags & QNetworkInterface::IsPointToPoint);
            bool isRunning = (bool)(flags & QNetworkInterface::IsRunning);
            bool checkSuccessful = false;
            QString pingAddress = "";
            QString parameter = "-r 0 -t 50";

            // If this interface isn't running, we don't care about it
            if ( !isRunning ) continue;
            // We only want valid interfaces that aren't loopback/virtual and not point to point
            if ( !ifaces[i].isValid() || isLoopback || isP2P ) continue;
            QList<QHostAddress> addresses = ifaces[i].allAddresses();
            for(int a=0; a < addresses.size(); a++)
            {
                // Ignore local host
                if ( addresses[a] == QHostAddress::LocalHost ) continue;

                // Ignore non-ipv4 addresses
                if ( !addresses[a].toIPv4Address() ) continue;

                QString ip = addresses[a].toString();
                if ( ip.isEmpty() ) continue;
                bool foundMatch = false;
                for (int j=0; j < possibleMatches.size(); j++) 
                    if ( ip == possibleMatches[j] ) 
                    { 
                        foundMatch = true; break; 
                    }

                if ( !foundMatch ) 
                { 
                    possibleMatches.push_back( ip );
                    //qDebug() << "possible address: " << ifaces[i].humanReadableName() << "->" << ip; 
                }
                ui->lblIPAddress->setText(ip);
                if((bool)(flags & QNetworkInterface::IsUp))
                {
                    if((bool)(flags & QNetworkInterface::CanMulticast))
                    {
                        ui->lblIPAddress->setStyleSheet("QLabel { background-color : rgb(60,179,113); color : blue; }");
                    } else {
                        ui->lblIPAddress->setStyleSheet("QLabel { background-color : rgb(60,179,113): color : white; }");
                    }

                    QString period = ".";
                    uint findN = nthSubstr(3, ip.toStdString(), period.toStdString());
                    //qDebug() << "CFrmAdminInfo::getSystemIPAddressAndState(), findN: " << QString::number(findN);
                    if(findN != std::string::npos)
                    {
                        pingAddress = ip.mid(0, findN);
                        if( pingAddress.at( pingAddress.length() - 1) != '.' )
                            pingAddress += ".1";
                        else
                            pingAddress += "1";

                        int exitCode = QProcess::execute("fping", QStringList() << parameter << pingAddress);
                        if( exitCode == 0 )
                            checkSuccessful = true;
                        else
                            ui->lblIPAddress->setVisible(false);
                    }
                }

                if( !checkSuccessful )
                    ui->lblIPAddress->setStyleSheet(red);
                else
                    ui->lblIPAddress->setVisible(true);
            }
        }
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

// trim from end
std::string CFrmAdminInfo::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
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
    foreach (QByteArray id, ids) {
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

    qDebug() << "sOutput: " << QString::fromStdString(sTimeZone);

    if( strcmp(sTimeZone.c_str(), "") )
    {
        char *tokString = strtok(const_cast<char*> (sTimeZone.c_str()), "\n");
        parsedString = tokString;
        qDebug() << "parsedString: " << QString::fromStdString(CMD_READ_TIME_ZONE);

        int index = cbox->findText(QString::fromStdString(CMD_READ_TIME_ZONE));
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
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
}

void CFrmAdminInfo::populateBoxListSelection(QComboBox *cbox)
{
    // Change this to retrieve a list of values from the locknums field
    // Rather than put all locks into a combo box, just put the entries that
    // are available in the database.

    cbox->clear();
    cbox->addItem(QString(tr("All Locks")));
}

void CFrmAdminInfo::populateCodeLockSelection()
{
//    populateBoxListSelection(ui->cbLockNum);
}

void CFrmAdminInfo::populateCodeLockHistorySelection()
{
//    populateBoxListSelection(ui->cbLockNumHistory);
}

void CFrmAdminInfo::populateFileWidget(QString sDirectory, QString sFilter)
{
    Q_UNUSED(sFilter);
    QString rp = "/media/pi";

    if(_pmodel) {
        delete _pmodel;
        _pmodel = 0;
    }

    if(!_pmodel)
        _pmodel = new QFileSystemModel();

    QStringList list;
    list << sDirectory << "KeycodeboxReports";

    _pmodel->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    _pmodel->setNameFilters(list);
    _pmodel->setNameFilterDisables(false);
    // Enable modifying file system
    _pmodel->setReadOnly(true);

    connect(_pmodel,
            SIGNAL(directoryLoaded(QString)),
            this,
            SLOT(onModelDirectoryLoaded(QString)));
    connect(_pmodel,
            SIGNAL(rootPathChanged(QString)),
            this,
            SLOT(onRootPathChanged(QString)));

    // Tie TreeView with DirModel
    // QTreeView::setModel(QAbstractItemModel * model)
    // Reimplemented from QAbstractItemView::setModel()
    ui->treeViewCopy->collapseAll();
    ui->treeView->setModel(_pmodel);

    _pmodel->setRootPath(rp);
    ui->treeView->setRootIndex(_pmodel->setRootPath(rp));
    ui->treeView->expandAll();
    resizeTreeViewColumns();
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

void CFrmAdminInfo::on_treeView_clicked(const QModelIndex &index)
{
    //    qDebug() << "Save to File" << path;
    _reportDirectory = _pmodel->rootPath() + "/" + index.data(Qt::DisplayRole).toString() + "/KeycodeboxReports";
    _tmpAdminRec.setReportDirectory(_reportDirectory);
    qDebug() << "Save to Directory" << _reportDirectory;
    QMessageBox::information(this, tr("Save Directory Set"), _reportDirectory);
}

void CFrmAdminInfo::populateFileCopyWidget(QString sDirectory, QString sFilter)
{
    Q_UNUSED(sFilter);
    QString rp = "/media/pi";

    if(_pcopymodel) {
        delete _pcopymodel;
        _pcopymodel = 0;
    }

    if(!_pcopymodel)
        _pcopymodel = new QFileSystemModel();

    QStringList list;
    list << sDirectory << "Alpha*" << "*.xml" << "*.jpg" << "*.jpeg";

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

    // Tie TreeView with DirModel
    // QTreeView::setModel(QAbstractItemModel * model)
    // Reimplemented from QAbstractItemView::setModel()
    ui->treeViewCopy->setModel(_pcopymodel);
    ui->treeViewCopy->collapseAll();

    _pcopymodel->setRootPath(rp);
    ui->treeViewCopy->setRootIndex(_pcopymodel->setRootPath(rp));

    ui->treeViewCopy->expandAll();
    resizeTreeViewColumns();
}

void CFrmAdminInfo::onCopyRootPathChanged(QString path)
{
    qDebug() << "Root path loaded" << path;
}

void CFrmAdminInfo::onCopyModelDirectoryLoaded(QString path)
{
    qDebug() << "loaded" << path;
    _pcopymodel->sort(0, Qt::AscendingOrder);
}

void CFrmAdminInfo::on_btnCopyToggleSource_clicked(bool checked)
{
    if(checked)
    {
        qDebug() << "CFrmAdminInfo::on_btnCopyToggleSource_clicked, checked, " << usbDevice1 << ", " << usbDevice0;
        populateFileCopyWidget(usbDevice1, usbDevice0);
        ui->btnCopyToggleSource->setText(tr("Source #2"));
    }
    else
    {
        qDebug() << "CFrmAdminInfo::on_btnCopyToggleSource_clicked, unchecked, " << usbDevice0 << ", " << usbDevice1;
        populateFileCopyWidget(usbDevice0, usbDevice1);
        ui->btnCopyToggleSource->setText(tr("Source #1"));
    }
}

void CFrmAdminInfo::resizeTreeViewColumns()
{
}

void CFrmAdminInfo::on_treeViewCopy_clicked(const QModelIndex &index)
{
    _copyDirectory = _pcopymodel->rootPath() + "/" + index.data(Qt::DisplayRole).toString();
    _copyDirectory = _pcopymodel->filePath ( index );

    if( QFileInfo(_copyDirectory).isFile())
    {
        ui->btnCopyFileBrandingImage->setEnabled(true);

        ui->btnCopyFileLoadCodes->setEnabled(true);
        ui->btnCopyFile->setEnabled(true);
    } else {
        ui->btnCopyFileBrandingImage->setEnabled(false);
        ui->btnCopyFileLoadCodes->setEnabled(false);
        ui->btnCopyFile->setEnabled(false);
    }

    //ui->treeViewCopy->setRootIndex(_pcopymodel->setRootPath(QString("/media/pi/")));
    qDebug() << "Copy from File" << _copyDirectory;

    resizeTreeViewColumns();
}

void CFrmAdminInfo::on_btnCopyFile_clicked()
{
    qDebug() << "Copy:" << _copyDirectory;
    QString cmd = "sudo cp '" + _copyDirectory + "' " + "/home/pi/kcb-config/bin/alpha_NEW";
    qDebug() << "CMD: " << cmd;
    std::system(cmd.toStdString().c_str());
    std::system("sudo chmod +x /home/pi/kcb-config/bin/alpha_NEW");

    QMessageBox::information(this, tr("File Copied"), tr("New KeyCodeBox application copied.\nSystem requires reboot to run."));
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

void CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked()
{
    qDebug() << "CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked()";

    xmlDocPtr doc = NULL;

    if( (doc = xmlReadFile(_copyDirectory.toStdString().c_str(), NULL, 0)) == NULL )
    {
        (void)QMessageBox::warning(this, tr("XML Parsing Failed"),
                                       tr("Can't parse given XML file\nPlease check syntax and integrity of your file."),
                                       QMessageBox::Ok);
        qDebug() << "CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked(), can't parse XML file: " << _copyDirectory;
        return;
    }

    if(!_pworkingSet)
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
}

void CFrmAdminInfo::on_btnCopyFileBrandingImage_clicked()
{
    qDebug() << "CFrmAdminInfo::on_btnCopyFileBrandingImage_clicked()";

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
}

void CFrmAdminInfo::on_btnCopyFileBrandingImageReset_clicked()
{
    qDebug() << "CFrmAdminInfo::on_btnCopyFileBrandingImageReset_clicked()";
    int nRC = QMessageBox::warning(this, tr("Verify Branding Image Reset"),
                                   tr("The branding image will be reset to the default.\nDo you want to continue?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) 
    {
        sync();
        std::system("cp /home/pi/kcb-config/images/alpha_logo_touch.jpg /home/pi/kcb-config/images/alpha_logo.jpg");
    }
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

void CFrmAdminInfo::on_lblAccessCode_clicked()
{
    QString text = ui->lblAccessCode->text();
    RunKeyboard(text);
    ui->lblAccessCode->setText(text);
}

void CFrmAdminInfo::on_lblPassword_clicked()
{
    QString text = ui->lblPassword->text();
    RunKeyboard(text);
    ui->lblPassword->setText(text);
}

void CFrmAdminInfo::on_lblAssistCode_clicked()
{
    QString text = ui->lblAssistCode->text();
    RunKeyboard(text);
    ui->lblAssistCode->setText(text);
}

void CFrmAdminInfo::on_lblAssistPassword_clicked()
{
    QString text = ui->lblAssistPassword->text();
    RunKeyboard(text);
    ui->lblAssistPassword->setText(text);
}

void CFrmAdminInfo::on_lblKey_clicked()
{
    QString text = ui->lblKey->text();
    RunKeyboard(text);
    ui->lblKey->setText(text);
}

void CFrmAdminInfo::on_btnSaveSettings_clicked()
{
    KCB_DEBUG_ENTRY;
    QDateTime   freq;
    bool    bNever = false;
    QString sFreq = ui->cbReportFreq->currentText();

    if(sFreq.compare(fNever) == 0)
    {
        freq = QDateTime(QDate(), QTime(0,0));
        bNever = true;
    }
    else if(sFreq.compare(fEach) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(0,0));
        bNever = false;
    }
    else if(sFreq.compare(fHourly) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(1,0));
    }
    else if(sFreq.compare(fEvery12Hours) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(12,0));
    }
    else if(sFreq.compare(fDaily) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(23,59));
    }
    else if(sFreq.compare(fWeekly) == 0)
    {
        freq = QDateTime(QDate(1,1,7), QTime(0,0));
    }
    else if(sFreq.compare(fMonthly) == 0)
    {
        freq = QDateTime(QDate(1,12,1), QTime(0,0));
    }
    // Update the Admin Info and close the dialog - syscontroller needs to switch
    _tmpAdminRec.setAdminName(ui->lblName->text());
    _tmpAdminRec.setAdminEmail(ui->lblEmail->text());
    _tmpAdminRec.setAdminPhone(ui->lblPhone->text());
    _tmpAdminRec.setDefaultReportFreq(freq);
    _tmpAdminRec.setDefaultReportStart(ui->dtStart->dateTime());
    _tmpAdminRec.setEmailReportActive(bNever);
    _tmpAdminRec.setPassword(ui->lblPassword->text());
    _tmpAdminRec.setAccessCode(ui->lblAccessCode->text());
    _tmpAdminRec.setAssistPassword(ui->lblAssistPassword->text());
    _tmpAdminRec.setAssistCode(ui->lblAssistCode->text());
    _tmpAdminRec.setDisplayFingerprintButton(ui->chkDisplayFingerprintButton->isChecked());
    _tmpAdminRec.setDisplayShowHideButton(ui->chkDisplayShowHideButton->isChecked());
    _tmpAdminRec.setUsePredictiveAccessCode(ui->chkUsePredictive->isChecked());
    _tmpAdminRec.setPredictiveKey(ui->lblKey->text());
    _tmpAdminRec.setPredictiveResolution(ui->spinCodeGenResolution->value());

    //_tmpAdminRec.setMaxLocks(ui->spinMaxLocks->value());

    _tmpAdminRec.setReportViaEmail(ui->cbReportViaEmail->isChecked());
    _tmpAdminRec.setReportToFile(ui->cbReportToFile->isChecked());
    _tmpAdminRec.setReportDirectory(_reportDirectory);

    _bClose = false;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
    KCB_DEBUG_EXIT;
}

void CFrmAdminInfo::on_btnDone_clicked()
{
    QDateTime   freq;
    bool    bNever = false;
    QString sFreq = ui->cbReportFreq->currentText();
    if(sFreq.compare(fNever) == 0)
    {
        freq = QDateTime(QDate(), QTime(0,0));
        bNever = true;
    }
    else if(sFreq.compare(fEach) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(0,0));
        bNever = false;
    }
    else if(sFreq.compare(fHourly) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(1,0));
    }
    else if(sFreq.compare(fEvery12Hours) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(12,0));
    }
    else if(sFreq.compare(fDaily) == 0)
    {
        freq = QDateTime(QDate(1,1,1), QTime(23,59));
    }
    else if(sFreq.compare(fWeekly) == 0)
    {
        freq = QDateTime(QDate(1,1,7), QTime(0,0));
    }
    else if(sFreq.compare(fMonthly) == 0)
    {
        freq = QDateTime(QDate(1,12,1), QTime(0,0));
    }
    // Update the Admin Info and close the dialog - syscontroller needs to switch
    _tmpAdminRec.setAdminName(ui->lblName->text());
    _tmpAdminRec.setAdminEmail(ui->lblEmail->text());
    _tmpAdminRec.setAdminPhone(ui->lblPhone->text());
    _tmpAdminRec.setDefaultReportFreq(freq);
    _tmpAdminRec.setDefaultReportStart(ui->dtStart->dateTime());
    _tmpAdminRec.setEmailReportActive(bNever);
    _tmpAdminRec.setPassword(ui->lblPassword->text());
    _tmpAdminRec.setAccessCode(ui->lblAccessCode->text());
    _tmpAdminRec.setAssistPassword(ui->lblAssistPassword->text());
    _tmpAdminRec.setAssistCode(ui->lblAssistCode->text());
    _tmpAdminRec.setDisplayFingerprintButton(ui->chkDisplayFingerprintButton->isChecked());
    _tmpAdminRec.setDisplayShowHideButton(ui->chkDisplayShowHideButton->isChecked());
    _tmpAdminRec.setUsePredictiveAccessCode(ui->chkUsePredictive->isChecked());
    _tmpAdminRec.setPredictiveKey(ui->lblKey->text());
    _tmpAdminRec.setPredictiveResolution(ui->spinCodeGenResolution->value());

    //_tmpAdminRec.setMaxLocks(ui->spinMaxLocks->value());

    _tmpAdminRec.setReportViaEmail(ui->cbReportViaEmail->isChecked());
    _tmpAdminRec.setReportToFile(ui->cbReportToFile->isChecked());
    _tmpAdminRec.setReportDirectory(_reportDirectory);

    _bClose = true;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
}

void CFrmAdminInfo::hideKeyboard(bool bHide) 
{
    ui->widgetEdit->setVisible(bHide);
}

void CFrmAdminInfo::onDelete()
{
    // Destructive - Not yet implemented
}

void CFrmAdminInfo::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    // New Admin info to display...
    // Display to the ui
    if(adminInfo)
    {
        qDebug() << "Admin Info received.";
        _tmpAdminRec = *adminInfo;

        _reportDirectory = _tmpAdminRec.getReportDirectory();
        ui->lblName->setText(adminInfo->getAdminName());
        ui->lblEmail->setText(adminInfo->getAdminEmail());
        ui->lblPhone->setText(adminInfo->getAdminPhone());
        ui->lblAccessCode->setText(adminInfo->getAccessCode());
        ui->lblPassword->setText(adminInfo->getPassword());
        ui->lblAssistCode->setText(adminInfo->getAssistCode());
        ui->lblAssistPassword->setText(adminInfo->getAssistPassword());
        ui->chkDisplayFingerprintButton->setChecked(adminInfo->getDisplayFingerprintButton());
        ui->chkDisplayShowHideButton->setChecked(adminInfo->getDisplayShowHideButton());

        QString sFreq;
        QDateTime dtFreq = adminInfo->getDefaultReportFreq();
        qDebug() << "AdminInfo freq date:" << dtFreq.toString("yyyy-MM-dd HH:mm:ss");
        if(dtFreq == QDateTime(QDate(), QTime(0,0))) {
            sFreq = fNever;
        }
        else if(dtFreq == QDateTime(QDate(1,1,1), QTime(0,0))) {
            sFreq = fEach;
        }
        else if(dtFreq == QDateTime(QDate(1,1,1), QTime(1,0))) {
            sFreq = fHourly;
        }
        else if(dtFreq == QDateTime(QDate(1,1,1), QTime(12,0))) {
            sFreq = fEvery12Hours;
        }
        else if(dtFreq == QDateTime(QDate(1,1,1), QTime(23,59))) {
            sFreq = fDaily;
        }
        else if(dtFreq == QDateTime(QDate(1,1,7), QTime(0,0))) {
            sFreq = fWeekly;
        }
        else if(dtFreq == QDateTime(QDate(1,12,1), QTime(0,0)))
        {
            sFreq = fMonthly;
        }
        ui->cbReportFreq->setCurrentText(sFreq);

        QDateTime dtStart = adminInfo->getDefaultReportStart();
        ui->dtStart->setDateTime(dtStart);
        ui->chkUsePredictive->setChecked(adminInfo->getUsePredictiveAccessCode());
        ui->lblKey->setText(adminInfo->getPredictiveKey());
        ui->spinCodeGenResolution->setValue(adminInfo->getPredictiveResolution());
        //ui->spinMaxLocks->setValue(adminInfo->getMaxLocks());

        qDebug() << "PRINT REPORT PRESSED";
        qDebug() << adminInfo->getReportViaEmail();
        qDebug() << adminInfo->getReportToFile();

        if(adminInfo->getReportViaEmail())
        {
            ui->cbReportViaEmail->setChecked(true);
        }
        if(adminInfo->getReportToFile())
        {
            ui->cbReportToFile->setChecked(true);
        }

        ui->tabWidget->setCurrentIndex(0);
        
    }
    else
    {
        qDebug() << "Admin Info received is blank.";
    }
}

void CFrmAdminInfo::OnUpdatedCurrentAdmin(bool bSuccess)
{
    if( bSuccess )
    {
        qDebug() << "Succeeded in updating Current Admin Info";

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

    } else {
        qDebug() << "Failed to update Current Admin Info";
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
}

void CFrmAdminInfo::OnFoundNewStorageDevice(QString device0, QString device1)
{
    _pcopymodel = 0;
    qDebug() << "CFrmAdminInfo::OnFoundNewStorageDevice()";
    qDebug() << device0;
    qDebug() << device1;
    usbDevice0 = QString(device0);
    usbDevice1 = QString(device1);
    qDebug() << usbDevice0;
    qDebug() << usbDevice1;

    ui->btnCopyToggleSource->setText(tr("Source #1"));

    if( !ui->btnCopyToggleSource->text().compare(tr("Source #1")) )
    {
        populateFileCopyWidget(usbDevice0, usbDevice1);
        populateFileWidget(usbDevice0, usbDevice1);
    }
    if( !ui->btnCopyToggleSource->text().compare(tr("Source #2")) )
    {
        populateFileCopyWidget(usbDevice1, usbDevice0);
        populateFileWidget(usbDevice1, usbDevice0);
    }
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
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), ntp ON";
        std::system("sudo /etc/init.d/ntp stop");

        QCoreApplication::processEvents();
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), ntpd -s";
        std::system("sudo ntpd -s");
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), ntp OFF";

        QCoreApplication::processEvents();

        std::system("sudo /etc/init.d/ntp start");
        qDebug() << "CFrmAdminInfo::on_cbInternetTime_clicked(), setting datetime text";
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

void CFrmAdminInfo::OnCloseAdmin() {
    this->close();
}

void CFrmAdminInfo::OnLockStatusUpdated(CLocksStatus *locksStatus)
{
    qDebug() << "CFrmAdminInfo::OnLockStatusUpdate()";
    _pLocksStatus = locksStatus;
}

void CFrmAdminInfo::OnLockSet(CLockSet *pSet)
{
    KCB_DEBUG_TRACE("Count" << pSet->getLockMap()->size());

    displayInTable(pSet);

    pSet = nullptr;
}

void CFrmAdminInfo::OnLockHistorySet(CLockHistorySet *pSet)
{
    KCB_DEBUG_ENTRY;
    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");


    displayInHistoryTable(pSet);
    
    pSet = nullptr;
}

void CFrmAdminInfo::codeTableCellSelected(int nRow, int nCol)
{
    Q_UNUSED(nRow);
    Q_UNUSED(nCol);

}

void CFrmAdminInfo::codeHistoryTableCellSelected(int nRow, int nCol)
{
    Q_UNUSED(nRow);
    Q_UNUSED(nCol);
}

void CFrmAdminInfo::displayInTable(CLockSet *pSet)
{
    // KCB_DEBUG_ENTRY;

    QTableWidget *table = ui->tblCodesList;

    table->clear();
    table->setRowCount(pSet->getLockMap()->size());
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

    connect( table, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT( codeTableCellSelected( int, int ) ) );

    ui->cbLockNum->clear();
    ui->cbLockNum->addItem(QString(tr("All Locks")));

    CLockSet::Iterator itor;
    CLockState  *pState;
    _pworkingSet = pSet;
    int nRow = 0;
    int nCol = 0;
    QSet<QString> lock_items;

    for(itor = pSet->begin(); itor != pSet->end(); itor++)
    {
        pState = itor.value();
        // qDebug() << "Adding row of Lock State - Codes. Lock Nums:" << QVariant(pState->getLockNums());

        // Locks can be single or comma-separated.  We will add the comma-separated value as well as
        // the individual locks for filtering purposes
        QString locks = pState->getLockNums();
        lock_items.insert(locks);
        QStringList sl = locks.split(',');
        foreach (auto s, sl)
        {
            lock_items.insert(s);
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

    QStringList sl(lock_items.toList());
    sl.sort();
    ui->cbLockNum->addItems(sl);

    _codesInUse.clear();
    _psysController->getAllCodes1(_codesInUse);

    // KCB_DEBUG_EXIT();
}

void CFrmAdminInfo::setupCodeTableContextMenu() 
{
    QTableWidget    *table = ui->tblCodesList;

    /* Create a menu for adding, editing, and deleting codes */
    _pTableMenu = new QMenu(table);

    /* Find out what column we are and add an action for that specific column
           e.g., If we select a cell in the Lock # column then the menu should
           start with Edit Locks.  Selecting edit locks will bring up a dialog
           of bank, link, and 32 buttons popuplated with the locks that are 
           already selected.  User can choose the locks to be associated with 
           this 'code' or authorization.

           Note: this implies that duplicate codes should no longer be allowed.
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
    KCB_DEBUG_TRACE(pSet);

    Q_ASSERT_X(pSet != nullptr, Q_FUNC_INFO, "pSet is null");

    if (_phistoryWorkingSet) 
    {
        _phistoryWorkingSet->clearSet();
        delete _phistoryWorkingSet;
    }

    _phistoryWorkingSet = pSet;    // Hold onto the set for additional work.

    // Setup the table display

    QTableWidget    *table = ui->tblHistory;
    table->setRowCount(pSet->getLockHistoryMap()->size());
    table->setColumnCount(5);

    table->setColumnWidth(0, 80);
    table->setColumnWidth(1, 120);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 100);
    table->setColumnWidth(4, 350);
    
    table->verticalHeader()->hide();
    
    QStringList headers;
    headers<<tr("Lock #")<<tr("Username")<<tr("Code#1")<<tr("Code#2")<<tr("Accessed");
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setFixedWidth(60);
    table->horizontalHeader()->setFixedHeight(50);    
    table->horizontalHeader()->setStretchLastSection(true);

    table->setStyleSheet("QTableView {selection-background-color: lightblue;}");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect( table, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT( codeHistoryTableCellSelected( int, int ) ) );

    ui->cbLockNumHistory->clear();
    ui->cbLockNumHistory->addItem(QString(tr("All Locks")));

    int nRow = 0;
    int nCol = 0;
    CLockHistoryRec  *pState;
    QSet<QString> lock_items;
    
    auto itor = _phistoryWorkingSet->getIterator();

    while (itor.hasNext())
    {
        pState = itor.next();
        qDebug() << "Adding row of History Lock State - Codes. Lock Num:" << pState->getLockNums();
        
        // Locks can be single or comma-separated.  We will add the comma-separated value as well as
        // the individual locks for filtering purposes
        QString locks = pState->getLockNums();
        KCB_DEBUG_TRACE("Locks" << locks);
        lock_items.insert(locks);
        QStringList sl = locks.split(',');
        foreach (auto s, sl)
        {
            lock_items.insert(s);
        }
        
        nCol = 0;
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getLockNums()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getCode1()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getCode2()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getAccessTime().toString("yyyy-MM-dd HH:mm:ss AP")));
        nRow++;
    }

    QStringList sl(lock_items.toList());
    sl.sort();
    ui->cbLockNumHistory->addItems(sl);
    
}

void CFrmAdminInfo::codeDeleteSelection()
{
    qDebug() << "codeDeleteSelection";

    int nRC = QMessageBox::warning(this, tr("Verify Delete"),
                                   tr("Do you want to delete the selected code?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) 
    {
        deleteCodeByRow(_nRowSelected);
    }
}

void CFrmAdminInfo::codeAddNew()
{
    qDebug() << "codeAddNew";
    addCodeByRow();
}

void CFrmAdminInfo::codeInitNew()
{
    qDebug() << "codeInitNew";
    addCodeByRow();
}

void CFrmAdminInfo::codeEnableAll()
{
    qDebug() << "Enabling all Limited Use codes";

    /* Looping over all the codes in the working set
           if code access type is 'limited use' then
               reset using __OnUpdateCodeState
    */
    CLockSet::Iterator itor;
    int nRow = 0;
    if (_pState)
    {
        qDebug() << "Freeing _pState in CFrmAdminInfo::codeEnableAll";
        _pState = 0;
    }

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
    qDebug() << "codeEditSelection row:" << QVariant(_nRowSelected).toString();
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


void CFrmAdminInfo::on_dialBright_valueChanged(int value)
{
    if( value < 20 ) value = 20;
    if( value > 255) value = 255;
    emit __OnBrightnessChanged(value);
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

void CFrmAdminInfo::onSMTPDialogComplete(CDlgSMTP *dlg)
{
    _bClose = false;
    // Save
    QString smtpserver, smtpusername, smtppassword;
    int smtpport, smtptype;

    qDebug() << "Getting SMTP Values to save";
    dlg->getValues(smtpserver, smtpport, smtptype, smtpusername, smtppassword);
    delete dlg;
    qDebug() << "SMTP:" << smtpserver << ":" << smtpport << " user:" << smtpusername << " pw:" << smtppassword;
    _tmpAdminRec.setSMTPServer(smtpserver);
    _tmpAdminRec.setSMTPPort(smtpport);
    _tmpAdminRec.setSMTPType(smtptype);
    _tmpAdminRec.setSMTPUsername(smtpusername);
    _tmpAdminRec.setSMTPPassword(smtppassword);
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
}

void CFrmAdminInfo::onVNCDialogComplete(CDlgVNC *dlg)
{
    // Save
    _bClose = false;
    qDebug() << "onVNCDialogComplete";

    QString vncpassword = "KEYCODEBOXDEFAULT";
    int vncport = 5900;

    qDebug() << "Getting VNC Values to save";
    dlg->getValues(vncport, vncpassword);
    delete dlg;
    qDebug() << "VNC:" << QString::number(vncport) << " pw:" << vncpassword;
    _tmpAdminRec.setVNCPassword(vncpassword);
    _tmpAdminRec.setVNCPort(vncport);
    emit __UpdateCurrentAdmin(&_tmpAdminRec);

    FILE *pF;
    std::string sOutput = "";
    QString createCmd = "echo '|";
    createCmd += QString::number(vncport);
    createCmd += " ";
    createCmd += vncpassword;
    createCmd +="|' > /home/pi/run/vnc_creds.txt";

    pF = popen(createCmd.toStdString().c_str(), "r");
    if(!pF)
    {
        qDebug() << "failed to create vnc file";
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);
}

void CFrmAdminInfo::on_btnSetupSMTP_clicked()
{
    // Display Email Dialog for SMTP server settings
    CDlgSMTP    *dlgSMTP = new CDlgSMTP();

    this->setEnabled(false);
    qDebug() << "SMTP:" << _tmpAdminRec.getSMTPServer() << ":" << _tmpAdminRec.getSMTPPort();
    dlgSMTP->setValues(_tmpAdminRec.getSMTPServer(), _tmpAdminRec.getSMTPPort(), _tmpAdminRec.getSMTPType(),
                       _tmpAdminRec.getSMTPUsername(), _tmpAdminRec.getSMTPPassword() );
    connect(dlgSMTP, SIGNAL(__onSMTPDialogComplete(CDlgSMTP *)), this, SLOT(onSMTPDialogComplete(CDlgSMTP *)));
    dlgSMTP->show();
    this->setEnabled(true);
}

void CFrmAdminInfo::on_btnSetupVNC_clicked()
{
    // Display Email Dialog for VNC remote desktop  server settings
    CDlgVNC    *dlgVNC = new CDlgVNC();

    this->setEnabled(false);
    qDebug() << "VNC:" << _tmpAdminRec.getVNCServer() << ":" << QString::number(_tmpAdminRec.getVNCPort());
    dlgVNC->setValues(_tmpAdminRec.getVNCPort(), _tmpAdminRec.getVNCPassword() );
    connect(dlgVNC, SIGNAL(__onVNCDialogComplete(CDlgVNC *)), this, SLOT(onVNCDialogComplete(CDlgVNC *)));
    dlgVNC->show();
    this->setEnabled(true);
}

void CFrmAdminInfo::on_btnPrintReport_clicked()
{
    // Print report
    QDateTime   dtStart, dtEnd;
    dtStart = ui->dtStartReport->dateTime();
    dtEnd = ui->dtEndReport->dateTime();

    _tmpAdminRec.setReportViaEmail(ui->cbReportViaEmail->isChecked());
    _tmpAdminRec.setReportToFile(ui->cbReportToFile->isChecked());
    _bClose = false;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);

    emit __OnImmediateReportRequest(dtStart, dtEnd);
}

void CFrmAdminInfo::on_btnToggleSource_clicked(bool checked)
{
    if(checked) 
    {
        populateFileWidget(usbDevice1, usbDevice0);
        ui->btnToggleSource->setText(tr("Source #2"));
    } 
    else 
    {
        populateFileWidget(usbDevice0, usbDevice1);
        ui->btnToggleSource->setText(tr("Source #1"));
    }
}

void CFrmAdminInfo::setTime()
{
    // setTime
    QDateTime   dt = ui->dtSystemTime->dateTime();
    QString updateTime = "sudo ntpq -p";
    QString sDate = QString("sudo date ") + dt.toString("MMddhhmmyyyy.ss");
    std::system(updateTime.toStdString().c_str());
    std::system(sDate.toStdString().c_str());
    qDebug() << "system time: ";
    qDebug() << sDate;
    std::system("sudo hwclock --systohc");    // Set the hardware clock
}

void CFrmAdminInfo::on_tblCodesList_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void CFrmAdminInfo::on_tblCodesList_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

// Add a new code
void CFrmAdminInfo::on_tblCodesList_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void CFrmAdminInfo::OnCodeEditClose()
{
    qDebug() << "CFrmAdminInfo::OnCodeEditClose - NOT IMPLEMENTED";
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
    qDebug() << "CFrmAdminInfo::OnCodeEditAccept";
    if (_pFrmCodeEditMulti)
    {
        _pFrmCodeEditMulti->getValues(_pState);
        _pFrmCodeEditMulti->hide();
        HandleCodeUpdate();
    }
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

    if (access_type == ACCESS_TYPE_ALWAYS)
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

        qDebug() << "Item Text:" << item->text();


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

            qDebug() << "start time" << _pState->getStartTime().toString() << "end time" << _pState->getEndTime().toString();

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
    qDebug() << "eventFilter. Event type:" << QVariant(event->type()).toString();
    if(event->type() == QEvent::TouchBegin )
    {
        qDebug() << "TouchBegin";
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
    qDebug() << "CFrmAdminInfo::touchEvent";
    QList<QTouchEvent::TouchPoint>   lstPoints;
    switch (ev->type())
    {
    case QEvent::TouchBegin:
        qDebug() << "TouchBegin rowcount:" << QVariant(ui->tblCodesList->rowCount()).toString();
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
        qDebug() << "TouchEnd rowcount:" << QVariant(ui->tblCodesList->rowCount()).toString();
        if(ui->tblCodesList->rowCount() >= 0 )
        {
            _pTableMenuAdd->show();
            setTableMenuLocation(_pTableMenuAdd);
        }
        break;

    default:
        break;
    }
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

    qDebug() << "Created CodeEditForm";

    // Get line values
    CLockSet::Iterator itor;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }
    int nRow = 0;

    qDebug() << "before loop iterator cellClicked";

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
    checkAndCreateCodeEditForm();

    // Get line values
    CLockSet::Iterator itor;
    if (_pState)
    {
        KCB_DEBUG_TRACE("Freeing _pState");
        _pState = 0;
    }
    int nRow = 0;

    qDebug() << "before loop iterator cellClicked";

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

        qDebug() << "CFrmAdminInfo::deleteCodeByRow(), cmd: " << cmd;

        if( QDir(QString("/home/pi/")).exists() )
        {
            std::system( cmd.toStdString().c_str() );
        }
        emit __OnUpdateCodeState(_pState);
    }
}

void CFrmAdminInfo::purgeCodes()
{
    // Removes all codes
    qDebug() << "CFrmAdminInfo::purgeCodes()";

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

    qDebug() << "CFrmAdminInfo::purgeCodes():" << CMD_REMOVE_ALL_FP_FILES;
    std::system( CMD_REMOVE_ALL_FP_FILES );
}

void CFrmAdminInfo::on_tblCodesList_cellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void CFrmAdminInfo::on_btnRebootSystem_clicked()
{
    int nRC = QMessageBox::warning(this, tr("Verify System Reboot"),
                                   tr("System will be restarted.\nDo you want to restart?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes)
    {
        sync();
        std::system("sudo shutdown -r now");
    }
}

void CFrmAdminInfo::on_btnPurgeCodes_clicked()
{
    KCB_DEBUG_ENTRY;

    on_btnReadCodes_clicked();

    int nRC = QMessageBox::warning(this, tr("Verify Remove All Codes"),
                                   tr("All access codes will be removed from the system\nDo you want to continue?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes)
    {
        KCB_DEBUG_TRACE("Yes Selected");
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
    qDebug() << "CFrmAdminInfo::OnTabSelected" << index;

    QDateTime dt = QDateTime().currentDateTime();
    ui->dtEndCodeHistoryList->setDateTime(dt);
    ui->dtEndCodeList->setDateTime(dt);

    // Force the codes to be read into the codes tab
    emit ui->btnReadCodes->clicked();
    emit ui->btnRead->clicked();

}

void CFrmAdminInfo::on_btnTestEmail_clicked()
{
    qDebug() << "Testing admin send email";
    emit __OnSendTestEmail(1 /*ADMIN_SEND*/);
}

void CFrmAdminInfo::on_btnTestUserEmail_clicked()
{
    qDebug() << "Testing admin recv email";
    _testEmail = true;
    on_btnSaveSettings_clicked();
}

void CFrmAdminInfo::OnDisplayFingerprintButton(bool state)
{
    ui->chkDisplayFingerprintButton->setChecked(state);
}

void CFrmAdminInfo::OnDisplayShowHideButton(bool state)
{
    ui->chkDisplayShowHideButton->setChecked(state);
}

void CFrmAdminInfo::OnOpenLockRequest(QString lock, bool is_user)
{
    emit __OnOpenLockRequest(lock, is_user);
}
