#include "frmadmininfo.h"
#include "ui_frmadmininfo.h"

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>

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
#include "frmcodeedit.h"

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
    ui(new Ui::CFrmAdminInfo), _pworkingSet(0)
{
    ui->setupUi(this);
    CFrmAdminInfo::showFullScreen();
    _pcurrentLabelEdit = 0;

    initialize();

    setAttribute(Qt::WA_AcceptTouchEvents, true);
}

CFrmAdminInfo::~CFrmAdminInfo()
{
    delete ui;
    if(_pcurrentLabelEdit) {
        delete _pcurrentLabelEdit;
    }
    if(_pmodel) {
        delete _pmodel;
    }
    if(_pcopymodel) {
        delete _pcopymodel;
    }
    if(_phistoryWorkingSet) {
        _phistoryWorkingSet->clearSet();
        delete _phistoryWorkingSet;
        _phistoryWorkingSet = 0;
    }
    if(_pworkingSet) {
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
    connect(this, SIGNAL(__OnReadLockSet(int,QDateTime,QDateTime)), _psysController, SLOT(OnReadLockSet(int,QDateTime,QDateTime)));
    connect(_psysController, SIGNAL(__OnLockSet(CLockSet*)), this, SLOT(OnLockSet(CLockSet*)));

    connect(this, SIGNAL(__OnReadLockHistorySet(int,QDateTime,QDateTime)), _psysController, SLOT(OnReadLockHistorySet(int,QDateTime,QDateTime)));
    connect(_psysController, SIGNAL(__OnLockHistorySet(CLockHistorySet*)), this, SLOT(OnLockHistorySet(CLockHistorySet*)));
    connect(this, SIGNAL(__OnImmediateReportRequest(QDateTime,QDateTime,int)), _psysController, SLOT(OnImmediateReportRequest(QDateTime,QDateTime,int)));

    connect(this, SIGNAL(__LocalOnReadLockSet(int,QDateTime,QDateTime)), this, SLOT(LocalReadLockSet(int,QDateTime,QDateTime)));
    connect(this, SIGNAL(__LocalOnReadLockHistorySet(int, QDateTime, QDateTime)), this, SLOT(LocalReadLockHistorySet(int, QDateTime, QDateTime)));

    connect(this, SIGNAL(__OnReadDoorLocksState()), _psysController, SLOT(OnReadLockStatus()));

    connect(_psysController, SIGNAL(__OnLockStatusUpdated(CLocksStatus*)), this, SLOT(OnLockStatusUpdated(CLocksStatus*)));

    connect(this, SIGNAL(__OnUpdateCodeState(CLockState*)), _psysController, SLOT(OnUpdateCodeState(CLockState*)));
    connect(_psysController, SIGNAL(__OnUpdatedCodeState(bool)), this, SLOT(OnUpdatedCodeState(bool)));

    connect(ui->tblCodesList, SIGNAL( cellClicked (int, int) ), this, SLOT( codeCellSelected( int, int ) ) );

    connect(ui->cbTimeZone, SIGNAL(currentIndexChanged(QString)), this, SLOT(setTimeZone()));

    connect(_psysController, SIGNAL(__OnFoundNewStorageDevice(QString, QString)), this, SLOT(OnFoundNewStorageDevice(QString, QString)));
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
    }
}

bool CFrmAdminInfo::isInternetTime()
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

    qDebug() << "CFrmAdminInfo::isInternetTime(), " << QString::fromStdString(sOutput);
    qDebug() << "CFrmAdminInfo::isInternetTime(), " << QString::number(sOutput.find("internetTime"));

    if( sOutput.find("internetTime") != std::string::npos )
        return true;

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
    populateCodeLockSelection();
    populateCodeLockHistorySelection();
    startMediaTimer();

    ui->dtEndCodeList->setDateTime(QDateTime().currentDateTime());
    ui->dtEndCodeHistoryList->setDateTime(QDateTime().currentDateTime());
    ui->dtEndReport->setDateTime(QDateTime().currentDateTime());

    setupCodeTableContextMenu();

    ui->tblCodesList->setAttribute(Qt::WA_AcceptTouchEvents);

    if( isInternetTime() )
    {
        ui->dtSystemTime->setDateTime(QDateTime().currentDateTime());
        ui->dtSystemTime->setDisabled(true);
        ui->cbInternetTime->setChecked(true);
    }

    ui->btnCopyToggleSource->setText("Source #1");

    connect(this, SIGNAL(__OnDoneSave(int,int,int,QString,QString,QString,QDateTime,QDateTime,bool,bool,bool,QString,QString,QString)),
            this, SLOT(OnCodeEditDoneSave(int,int,int,QString,QString,QString,QDateTime,QDateTime,bool,bool,bool,QString,QString,QString)));

    ui->btnCopyFileBrandingImageReset->setEnabled(true);
}

int CFrmAdminInfo::nthSubstr(int n, const std::string& s, const std::string& p) {
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
            QString parameter = "-r0 -t 50";

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
                for (int j=0; j < possibleMatches.size(); j++) if ( ip == possibleMatches[j] ) { foundMatch = true; break; }
                if ( !foundMatch ) { possibleMatches.push_back( ip );
                    qDebug() << "possible address: " << ifaces[i].humanReadableName() << "->" << ip; }
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
                    qDebug() << "CFrmAdminInfo::getSystemIPAddressAndState(), findN: " << QString::number(findN);
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
    QString parseString = "readlink /etc/localtime";

    pF = popen(parseString.toStdString().c_str(), "r");
    if(!pF)
    {
        qDebug() << "failed to parse timezone string";
    }

    ExtractCommandOutput(pF, sOutput);
    fclose(pF);

    sTimeZone = sOutput.substr(20);

    if( sOutput.length() == 0)
        sTimeZone = "America/New_York";

    qDebug() << "sOutput: " << QString::fromStdString(sTimeZone);

    if( strcmp(sTimeZone.c_str(), "") )
    {
        char *tokString = strtok(const_cast<char*> (sTimeZone.c_str()), "\n");
        parsedString = tokString;
        qDebug() << "parsedString: " << QString::fromStdString(parsedString);

        int index = cbox->findText(QString::fromStdString(parsedString.c_str()));
        if (index != -1) {
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
    cbox->clear();
    cbox->addItem(QString("All Locks"));
    for(int i=1;i<65;i++)
    {
        cbox->addItem(QString("Lock #" + QVariant(i).toString()));
    }
}

void CFrmAdminInfo::populateCodeLockSelection()
{
    populateBoxListSelection(ui->cbLockNum);
}

void CFrmAdminInfo::populateCodeLockHistorySelection()
{
    populateBoxListSelection(ui->cbLockNumHistory);
}

void CFrmAdminInfo::populateFileWidget(QString sDirectory, QString sFilter)
{
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
    _tmpAdminRec.setReportDirectory(_reportDirectory.toStdString());
    qDebug() << "Save to Directory" << _tmpAdminRec.getReportDirectory().c_str();
    QMessageBox::information(this, "Save Directory Set", _tmpAdminRec.getReportDirectory().c_str());
}

void CFrmAdminInfo::populateFileCopyWidget(QString sDirectory, QString sFilter)
{
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
    //static_cast<QTreeView*>(ui->treeViewCopy)->selectionModel()->clearSelection();
    //ui->treeViewCopy->collapseAll();
    if(checked)
    {
        qDebug() << "CFrmAdminInfo::on_btnCopyToggleSource_clicked, checked, " << usbDevice1 << ", " << usbDevice0;
        populateFileCopyWidget(usbDevice1, usbDevice0);
        ui->btnCopyToggleSource->setText("Source #2");
    }
    else
    {
        qDebug() << "CFrmAdminInfo::on_btnCopyToggleSource_clicked, unchecked, " << usbDevice0 << ", " << usbDevice1;
        populateFileCopyWidget(usbDevice0, usbDevice1);
        ui->btnCopyToggleSource->setText("Source #1");
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
    QString cmd = "sudo cp '" + _copyDirectory + "' " + "/home/pi/bin/Alpha_NEW";
    qDebug() << "CMD: " << cmd;
    std::system(cmd.toStdString().c_str());
    std::system("sudo chmod +x /home/pi/bin/Alpha_NEW");

    QMessageBox::information(this, "File Copied", tr("New SafePak application copied.\nSystem requires reboot to run."));
}

void CFrmAdminInfo::printElementNames(xmlNode *aNode)
{
    xmlNode *curNode = NULL;

    /*
    <?xml version="1.0"?>
    <codeListing title="20161213 Codes">
      <code>
        <lock>4</lock>
        <code1>88888888</code1>
        <code2>999999999</code2>
        <user>This is an exciting code!</user>
        <startDT>2008-10-31T15:07:38.6875000-05:00</startDT>
        <endDT>2008-10-31T15:07:38.6875000-05:00</endDT>
      </code>
      <code>
        <lock>5</lock>
        <code1>88888888</code1>
        <code2>999999999</code2>
        <user>This is another exciting code!</user>
        <startDT>2008-10-31T15:07:38.6875000-05:00</startDT>
        <endDT>2008-10-31T15:07:38.6875000-05:00</endDT>
      </code>
    </codeListing>
   */

    int nLockNum = 0;
    QString sAccessCode = "";
    QString sSecondCode = "";
    QString description = "";
    QString question1 = "";
    QString question2 = "";
    QString question3 = "";
    QDateTime dtStart;
    QDateTime dtEnd;

    for (curNode = aNode; curNode; curNode=curNode->next)
    {
        if (curNode->type == XML_ELEMENT_NODE)
        {
            if( !strcmp((char *)curNode->name, "lock") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording lock'";
                nLockNum = atoi((char *)xmlNodeGetContent(curNode));
                qDebug() << nLockNum;
            }
            if( !strcmp((char *)curNode->name, "code1") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording code1'";
                sAccessCode = QString::fromLatin1((char *)xmlNodeGetContent(curNode));
                qDebug() << sAccessCode;
            }
            if( !strcmp((char *)curNode->name, "code2") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording code2'";
                sSecondCode = QString::fromLatin1((char *)xmlNodeGetContent(curNode));
                qDebug() << sSecondCode;
            }
            if( !strcmp((char *)curNode->name, "username") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording user'";
                description = QString::fromLatin1((char *)xmlNodeGetContent(curNode));
                qDebug() << description;
            }
            if( !strcmp((char *)curNode->name, "question1") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording question1'";
                question1 = QString::fromLatin1((char *)xmlNodeGetContent(curNode));
                qDebug() << question1;
            }
            if( !strcmp((char *)curNode->name, "question2") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording question2'";
                question2 = QString::fromLatin1((char *)xmlNodeGetContent(curNode));
                qDebug() << question2;
            }
            if( !strcmp((char *)curNode->name, "question3") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording question3'";
                question3 = QString::fromLatin1((char *)xmlNodeGetContent(curNode));
                qDebug() << question3;
            }

            if( !strcmp((char *)curNode->name, "startDT") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording start daytime'";
                dtStart = QDateTime::fromString(QString::fromLatin1((char *)xmlNodeGetContent(curNode)),Qt::ISODate);
                qDebug() << dtStart.toString();
            }
            if( !strcmp((char *)curNode->name, "endDT") )
            {
                qDebug() << "CFrmAdminInfo::printElementNames(), recording end daytime'";
                dtEnd = QDateTime::fromString(QString::fromLatin1((char *)xmlNodeGetContent(curNode)),Qt::ISODate);
                qDebug() << dtEnd.toString();


                _pState = createNewLockState();
                if(!_pworkingSet) {
                    _pworkingSet = new CLockSet();
                }
                qDebug() << "CFrmAdminInfo::printElementNames(), ADD CODE";

                // NEED TO ADD XML PARSING FOR QUESTIONS

                emit(__OnDoneSave(-1, -1, nLockNum, sAccessCode, sSecondCode,
                                  description, dtStart, dtEnd,
                                  false, false, false, question1, question2, question3));
            }
        }
        printElementNames(curNode->children);
    }
}

void CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked()
{
    qDebug() << "CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked()";

    xmlDoc *doc = NULL;
    xmlNode *rootElement = NULL;

    if( (doc = xmlReadFile(_copyDirectory.toStdString().c_str(), NULL, 0)) == NULL )
    {
        qDebug() << "CFrmAdminInfo::on_btnCopyFileLoadCodes_clicked(), can't parse XML file: " << _copyDirectory;
        return;
    }

    rootElement = xmlDocGetRootElement(doc);
    printElementNames(rootElement);
    xmlFreeDoc(doc);
    xmlCleanupParser();
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
        createCmd += "' /home/pi/dev/keycodebox/alpha/images/alpha_logo.jpg";
        std::system(createCmd.toStdString().c_str());
    }
}

void CFrmAdminInfo::on_btnCopyFileBrandingImageReset_clicked()
{
    qDebug() << "CFrmAdminInfo::on_btnCopyFileBrandingImageReset_clicked()";
    int nRC = QMessageBox::warning(this, tr("Verify Branding Image Reset"),
                                   tr("The branding image will be reset to the default.\nDo you want to continue?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) {
        sync();
        std::system("cp /home/pi/dev/keycodebox/alpha/images/alpha_logo_touch.jpg /home/pi/dev/keycodebox/alpha/images/alpha_logo.jpg");
    }
}

void CFrmAdminInfo::open_single_door(int nLockNum)
{
    emit __OnOpenLockRequest(nLockNum);
}

void CFrmAdminInfo::populateAvailableDoors() {
    QPushButton *pb;
    QIcon       icon("/home/pi/SafePak/images/vault.png");
    QList<QPushButton*> lstWidgets;
    QList<QPushButton*>::Iterator   itor;
    lstWidgets = ui->tabWidget->findChildren<QPushButton *>();
    int nLockNum = 0;

    for(itor = lstWidgets.begin(); itor != lstWidgets.end(); itor++)
    {
        pb = (QPushButton*)(*itor);

        qDebug() << "Widget Classname:" << pb->metaObject()->className();
        qDebug() << "objectName:" << pb->objectName();

        if((QString(pb->metaObject()->className()) == QString("QPushButton")) &&
                (pb->objectName().mid(0, 5) == "cbBox"))
            qDebug() << "cbBox";
        {
            nLockNum = parseLockNumFromObjectName(pb->objectName());
            if(_pLocksStatus->isLock(nLockNum)) {
                qDebug() << "Lock on " << nLockNum;
                pb->setIcon(icon);
            } else {
                qDebug() << "No lock on " << nLockNum;
                pb->setIcon(QIcon());
            }
        }
    }
}

void CFrmAdminInfo::createLockMenus()
{
    qDebug() << "CFrmAdminInfo::createLockMenus()";
    QPushButton *pb;
    QList<QPushButton*> lstWidgets;
    QList<QPushButton*>::Iterator   itor;

    lstWidgets = ui->tabWidget->findChildren<QPushButton*>();
    int nCount = lstWidgets.count();

    qDebug() << "Count widgets:" << nCount;

    for(itor = lstWidgets.begin(); itor != lstWidgets.end(); itor++)
    {
        pb = (QPushButton*)(*itor);

        if((QString(pb->metaObject()->className()) == QString("QPushButton")) &&
                (pb->objectName().mid(0, 5) == "cbBox"))
        {
            qDebug() << "createLockMenus cbBox";
            return;

            if(pb->menu() == NULL) {
                QMenu* pmenu = new QMenu();
                pmenu->setMinimumWidth(150);
                if(_pLocksStatus->isLock(parseLockNumFromObjectName(pb->objectName()))) {
                    QAction* pAction1 = new QAction("Open", pb);
                    pAction1->setData("Open");
                    pAction1->setData((*itor)->objectName());
                    pmenu->addAction(pAction1);
                }
                QAction* pAction2 = new QAction("Codes", pb);
                pAction2->setData("Codes");
                QAction* pAction3 = new QAction("History", pb);
                pAction3->setData("History");
                pAction2->setData((*itor)->objectName());
                pAction3->setData((*itor)->objectName());
                pmenu->addAction(pAction2);
                pmenu->addAction(pAction3);
                pb->setMenu(pmenu);
                connect(pmenu, SIGNAL(triggered(QAction*)), this, SLOT(menuSelection(QAction*)));
            } else {
                QMenu *pmenu = pb->menu();
                bool bFound = false;
                if(_pLocksStatus->isLock(parseLockNumFromObjectName(pb->objectName()))) {
                    QList<QAction*> list = pmenu->actions();
                    for(QList<QAction*>::Iterator itor = list.begin(); itor != list.end(); itor++)
                    {
                        if((*itor)->text() == "Open")
                        {
                            bFound = true;
                        }
                    }
                    if(!bFound) {
                        QAction* pAction1 = new QAction("Open", pb);
                        pAction1->setData("Open");
                        pAction1->setData((*itor)->objectName());
                        pmenu->insertAction(pmenu->actions().first(), pAction1);
                    }
                } else {
                    QList<QAction*> list = pmenu->actions();
                    for(QList<QAction*>::Iterator itor = list.begin(); itor != list.end(); itor++)
                    {
                        if((*itor)->text() == "Open")
                        {
                            pb->menu()->removeAction(*itor);
                        }
                    }
                }
                connect(pmenu, SIGNAL(triggered(QAction*)), this, SLOT(menuSelection(QAction*)));
            }
        }
    }
}

int CFrmAdminInfo::parseLockNumFromObjectName(QString objectName) {
    if(objectName.mid(0,5) == "cbBox")
    {
        try {
            int nVal = QString(objectName.right(objectName.length()-5)).toInt();
            qDebug() << "Parse lock number from objectName:" + QVariant(nVal).toString();
            return nVal;
        } catch(exception &e) {
            qDebug() << "Parsing lock number from object name failed!";
        }
    }
    return 0;
}

void CFrmAdminInfo::menuSelection(QAction *action)
{
    int nLockNum = parseLockNumFromObjectName(action->parentWidget()->objectName());
    qDebug() << "Object: " << action->parentWidget()->objectName() << " Action!" << action->text();
    if( action->text() == QString("Open"))
    {
        qDebug() << "Open Lock Num:" << QVariant(nLockNum).toString();
        emit __OnOpenLockRequest(nLockNum);
    } else if(action->text() == QString("Codes")) {
        emit __LocalOnReadLockSet(nLockNum,
                                  QDateTime::fromString(_DATENONE, "yyyy-MM-dd HH:mm:ss"),
                                  QDateTime::fromString(_DATENONE, "yyyy-MM-dd HH:mm:ss"));
        // Switch tabs
        ui->tabWidget->setCurrentIndex(3);
    } else if(action->text() == QString("History")) {
        emit __LocalOnReadLockHistorySet(nLockNum,
                                         QDateTime::fromString(_DATENONE, "yyyy-MM-dd HH:mm:ss"),
                                         QDateTime::fromString(_DATENONE, "yyyy-MM-dd HH:mm:ss"));
        ui->tabWidget->setCurrentIndex(4);
    }
}

void CFrmAdminInfo::onStartEditLabel(QLabel* pLabel, QString sLabelText)
{
    //    ui->tabWidget->hide();
    if(!_pKeyboard) {
        _pKeyboard = new CDlgFullKeyboard();
        connect(_pKeyboard, SIGNAL(__TextEntered(CDlgFullKeyboard*,CCurrentEdit*)),
                this, SLOT(OnKeyboardTextEntered(CDlgFullKeyboard*, CCurrentEdit*)));
    }
    if(!_pcurrentLabelEdit) {
        _pcurrentLabelEdit = new CCurrentEdit();
    }
    if(_pKeyboard && _pcurrentLabelEdit) {
        _pKeyboard->setCurrentEdit(_pcurrentLabelEdit);
        _pcurrentLabelEdit->setLabelToBeEdited(pLabel);
        _pcurrentLabelEdit->setOriginalTextToEdit(pLabel->text());
        _pcurrentLabelEdit->setLabelText(sLabelText);

        _pKeyboard->setActive();
    }
}

void CFrmAdminInfo::OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *currEdit)
{
    currEdit->getLabelBeingEdited()->setText(currEdit->getNewText());
    keyboard->hide();
}

void CFrmAdminInfo::onStopEdit()
{
    ui->tabWidget->show();
    ui->widgetEdit->hide();
}

void CFrmAdminInfo::checkAndCreateCurrentLabelEdit()
{
    if(!_pcurrentLabelEdit) {
        _pcurrentLabelEdit = new CCurrentEdit();
    }
}

void CFrmAdminInfo::on_lblName_clicked()
{
    checkAndCreateCurrentLabelEdit();

    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblName, "Administrator");

    qDebug() << "on_lblName_clicked()";
}

void CFrmAdminInfo::on_lblEmail_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblEmail_clicked()";
    _pcurrentLabelEdit->setEmailFilter();
    onStartEditLabel(ui->lblEmail, "Administrator Email Address");
}

void CFrmAdminInfo::on_lblPhone_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblPhone_clicked()";
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblPhone, "Administrator Phone");
}

void CFrmAdminInfo::on_lblAccessCode_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblAccessCode_clicked()";
    _pcurrentLabelEdit->setNumbersOnly();
    onStartEditLabel(ui->lblAccessCode, "Administrator Code #1");
}

void CFrmAdminInfo::on_lblPassword_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblPassword_clicked()";
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblPassword, "Administrator Password");
}

void CFrmAdminInfo::on_lblAssistCode_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblAssistCode_clicked()";
    _pcurrentLabelEdit->setNumbersOnly();
    onStartEditLabel(ui->lblAssistCode, "Assistant Code");
}

void CFrmAdminInfo::on_lblAssistPassword_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblAssistPassword_clicked()";
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblAssistPassword, "Assistant Password");
}

void CFrmAdminInfo::on_lblKey_clicked()
{
    checkAndCreateCurrentLabelEdit();
    qDebug() << "on_lblKey_clicked()";
    _pcurrentLabelEdit->clearInputMasks();
    onStartEditLabel(ui->lblKey, "Predictive Key");
}

void CFrmAdminInfo::on_btnSaveSettings_clicked()
{
    qDebug() << "on_btnSaveSettings_clicked()";
    QDateTime   freq;
    bool    bNever = false;
    QString sFreq = ui->cbReportFreq->currentText();
    if(sFreq.compare(fNever) == 0) {
        freq = QDateTime(QDate(), QTime(0,0));
        bNever = true;
    } else if(sFreq.compare(fEach) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(0,0));
        bNever = false;
    } else if(sFreq.compare(fHourly) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(1,0));
    } else if(sFreq.compare(fEvery12Hours) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(12,0));
    } else if(sFreq.compare(fDaily) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(23,59));
    } else if(sFreq.compare(fWeekly) == 0) {
        freq = QDateTime(QDate(1,1,7), QTime(0,0));
    } else if(sFreq.compare(fMonthly) == 0) {
        freq = QDateTime(QDate(1,12,1), QTime(0,0));
    }
    // Update the Admin Info and close the dialog - syscontroller needs to switch
    _tmpAdminRec.setAdminName(ui->lblName->text().toStdString());
    _tmpAdminRec.setAdminEmail(ui->lblEmail->text().toStdString());
    _tmpAdminRec.setAdminPhone(ui->lblPhone->text().toStdString());
    _tmpAdminRec.setDefaultReportFreq(freq);
    _tmpAdminRec.setDefaultReportStart(ui->dtStart->dateTime());
    _tmpAdminRec.setEmailReportActive(bNever);
    _tmpAdminRec.setPassword(ui->lblPassword->text().toStdString());
    _tmpAdminRec.setAccessCode(ui->lblAccessCode->text().toStdString());
    _tmpAdminRec.setAssistPassword(ui->lblAssistPassword->text().toStdString());
    _tmpAdminRec.setAssistCode(ui->lblAssistCode->text().toStdString());
    _tmpAdminRec.setShowFingerprint(ui->chkShowFingerprint->isChecked());
    _tmpAdminRec.setUsePredictiveAccessCode(ui->chkUsePredictive->isChecked());
    _tmpAdminRec.setPredictiveKey(ui->lblKey->text().toStdString());
    _tmpAdminRec.setPredictiveResolution(ui->spinCodeGenResolution->value());

    _tmpAdminRec.setMaxLocks(ui->spinMaxLocks->value());

    _tmpAdminRec.setReportViaEmail(ui->cbReportViaEmail->isChecked());
    _tmpAdminRec.setReportToFile(ui->cbReportToFile->isChecked());
    _tmpAdminRec.setReportDirectory(_reportDirectory.toStdString());

    _bClose = false;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
}

void CFrmAdminInfo::on_btnDone_clicked()
{
    QDateTime   freq;
    bool    bNever = false;
    QString sFreq = ui->cbReportFreq->currentText();
    if(sFreq.compare(fNever) == 0) {
        freq = QDateTime(QDate(), QTime(0,0));
        bNever = true;
    } else if(sFreq.compare(fEach) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(0,0));
        bNever = false;
    } else if(sFreq.compare(fHourly) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(1,0));
    } else if(sFreq.compare(fEvery12Hours) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(12,0));
    } else if(sFreq.compare(fDaily) == 0) {
        freq = QDateTime(QDate(1,1,1), QTime(23,59));
    } else if(sFreq.compare(fWeekly) == 0) {
        freq = QDateTime(QDate(1,1,7), QTime(0,0));
    } else if(sFreq.compare(fMonthly) == 0) {
        freq = QDateTime(QDate(1,12,1), QTime(0,0));
    }
    // Update the Admin Info and close the dialog - syscontroller needs to switch
    _tmpAdminRec.setAdminName(ui->lblName->text().toStdString());
    _tmpAdminRec.setAdminEmail(ui->lblEmail->text().toStdString());
    _tmpAdminRec.setAdminPhone(ui->lblPhone->text().toStdString());
    _tmpAdminRec.setDefaultReportFreq(freq);
    _tmpAdminRec.setDefaultReportStart(ui->dtStart->dateTime());
    _tmpAdminRec.setEmailReportActive(bNever);
    _tmpAdminRec.setPassword(ui->lblPassword->text().toStdString());
    _tmpAdminRec.setAccessCode(ui->lblAccessCode->text().toStdString());
    _tmpAdminRec.setAssistPassword(ui->lblAssistPassword->text().toStdString());
    _tmpAdminRec.setAssistCode(ui->lblAssistCode->text().toStdString());
    _tmpAdminRec.setShowFingerprint(ui->chkShowFingerprint->isChecked());
    _tmpAdminRec.setUsePredictiveAccessCode(ui->chkUsePredictive->isChecked());
    _tmpAdminRec.setPredictiveKey(ui->lblKey->text().toStdString());
    _tmpAdminRec.setPredictiveResolution(ui->spinCodeGenResolution->value());

    _tmpAdminRec.setMaxLocks(ui->spinMaxLocks->value());

    _tmpAdminRec.setReportViaEmail(ui->cbReportViaEmail->isChecked());
    _tmpAdminRec.setReportToFile(ui->cbReportToFile->isChecked());
    _tmpAdminRec.setReportDirectory(_reportDirectory.toStdString());

    _bClose = true;
    emit __UpdateCurrentAdmin(&_tmpAdminRec);
}

void CFrmAdminInfo::hideKeyboard(bool bHide) {
    ui->widgetEdit->setVisible(bHide);
}

/**
 * @brief CFrmAdminInfo::onDelete
 * forward delete
 */
void CFrmAdminInfo::onDelete()
{
    // Destructive - Not yet implemented
}

void CFrmAdminInfo::OnRequestedCurrentAdmin(CAdminRec *adminInfo)
{
    // New Admin info to display...
    // Display to the ui
    if(adminInfo) {
        qDebug() << "Admin Info received.";
        _tmpAdminRec = *adminInfo;

        _reportDirectory = _tmpAdminRec.getReportDirectory().c_str();
        ui->lblName->setText(adminInfo->getAdminName().c_str());
        ui->lblEmail->setText(adminInfo->getAdminEmail().c_str());
        ui->lblPhone->setText(adminInfo->getAdminPhone().c_str());
        ui->lblAccessCode->setText(adminInfo->getAccessCode().c_str());
        ui->lblPassword->setText(adminInfo->getPassword().c_str());
        ui->lblAssistCode->setText(adminInfo->getAssistCode().c_str());
        ui->lblAssistPassword->setText(adminInfo->getAssistPassword().c_str());
        ui->chkShowFingerprint->setChecked(adminInfo->getShowFingerprint());

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
        ui->lblKey->setText(adminInfo->getPredictiveKey().c_str());
        ui->spinCodeGenResolution->setValue(adminInfo->getPredictiveResolution());
        ui->spinMaxLocks->setValue(adminInfo->getMaxLocks());

        qDebug() << "PRINT REPORT PRESSED";
        qDebug() << adminInfo->getReportViaEmail();
        qDebug() << adminInfo->getReportToFile();

        if(adminInfo->getReportViaEmail())
            ui->cbReportViaEmail->setChecked(true);
        if(adminInfo->getReportToFile())
            ui->cbReportToFile->setChecked(true);
    } else {
        qDebug() << "Admin Info received is blank.";
    }
}

void CFrmAdminInfo::OnUpdatedCurrentAdmin(bool bSuccess)
{
    if( bSuccess ) {
        qDebug() << "Succeeded in updating Current Admin Info";
    } else {
        qDebug() << "Failed to update Current Admin Info";
    }

    if(_bClose) {
        emit __OnCloseFrmAdmin();
        this->close();
    } else {
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

    ui->btnCopyToggleSource->setText("Source #1");

    if( !ui->btnCopyToggleSource->text().compare("Source #1") )
    {
        populateFileCopyWidget(usbDevice0, usbDevice1);
        populateFileWidget(usbDevice0, usbDevice1);
    }
    if( !ui->btnCopyToggleSource->text().compare("Source #2") )
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
        if(_pFrmCodeEdit)
        {
            _pFrmCodeEdit->hide();
        }
        displayInTable(_pworkingSet);
    } else {
        //TODO: reset the values...didn't save
    }
    on_btnReadCodes_clicked();
}

void CFrmAdminInfo::on_cbInternetTime_clicked()
{
    // Internet time set checked
    if(ui->cbInternetTime->isChecked())
    {
        //we check this flag in safepakmain.cpp
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

void CFrmAdminInfo::on_btnOpenDoor_clicked()
{
    // Open selected door
}

void CFrmAdminInfo::on_cbBox1_clicked()
{
    qDebug() << "on_cbBox1_clicked()";

    emit __OnOpenLockRequest(1);
}

void CFrmAdminInfo::on_cbBox2_clicked()
{
    qDebug() << "on_cbBox2_clicked()";

    emit __OnOpenLockRequest(2);
}

void CFrmAdminInfo::on_cbBox3_clicked()
{
    qDebug() << "on_cbBox3_clicked()";

    emit __OnOpenLockRequest(3);
}

void CFrmAdminInfo::on_cbBox4_clicked()
{
    qDebug() << "on_cbBox4_clicked()";

    emit __OnOpenLockRequest(4);
}

void CFrmAdminInfo::on_cbBox5_clicked()
{
    qDebug() << "on_cbBox1_clicked()";

    emit __OnOpenLockRequest(5);
}

void CFrmAdminInfo::on_cbBox6_clicked()
{
    qDebug() << "on_cbBox6_clicked()";

    emit __OnOpenLockRequest(6);
}

void CFrmAdminInfo::on_cbBox7_clicked()
{
    qDebug() << "on_cbBox7_clicked()";

    emit __OnOpenLockRequest(7);
}

void CFrmAdminInfo::on_cbBox8_clicked()
{
    qDebug() << "on_cbBox8_clicked()";

    emit __OnOpenLockRequest(8);
}

void CFrmAdminInfo::on_cbBox9_clicked()
{
    qDebug() << "on_cbBox9_clicked()";

    emit __OnOpenLockRequest(9);
}

void CFrmAdminInfo::on_cbBox10_clicked()
{
    qDebug() << "on_cbBox10_clicked()";

    emit __OnOpenLockRequest(10);
}

void CFrmAdminInfo::on_cbBox11_clicked()
{
    qDebug() << "on_cbBox11_clicked()";

    emit __OnOpenLockRequest(11);
}

void CFrmAdminInfo::on_cbBox12_clicked()
{
    qDebug() << "on_cbBox12_clicked()";

    emit __OnOpenLockRequest(12);
}

void CFrmAdminInfo::on_cbBox13_clicked()
{
    qDebug() << "on_cbBox13_clicked()";

    emit __OnOpenLockRequest(13);
}

void CFrmAdminInfo::on_cbBox14_clicked()
{
    qDebug() << "on_cbBox14_clicked()";

    emit __OnOpenLockRequest(14);
}

void CFrmAdminInfo::on_cbBox15_clicked()
{
    qDebug() << "on_cbBox15_clicked()";

    emit __OnOpenLockRequest(15);
}

void CFrmAdminInfo::on_cbBox16_clicked()
{
    qDebug() << "on_cbBox16_clicked()";

    emit __OnOpenLockRequest(16);
}

void CFrmAdminInfo::on_cbBox17_clicked()
{
    qDebug() << "on_cbBox17_clicked()";

    emit __OnOpenLockRequest(17);
}

void CFrmAdminInfo::on_cbBox18_clicked()
{
    qDebug() << "on_cbBox18_clicked()";

    emit __OnOpenLockRequest(18);
}

void CFrmAdminInfo::on_cbBox19_clicked()
{
    qDebug() << "on_cbBox19_clicked()";

    emit __OnOpenLockRequest(19);
}

void CFrmAdminInfo::on_cbBox20_clicked()
{
    qDebug() << "on_cbBox20_clicked()";

    emit __OnOpenLockRequest(20);
}

void CFrmAdminInfo::on_cbBox21_clicked()
{
    qDebug() << "on_cbBox21_clicked()";

    emit __OnOpenLockRequest(21);
}

void CFrmAdminInfo::on_cbBox22_clicked()
{
    qDebug() << "on_cbBox22_clicked()";

    emit __OnOpenLockRequest(22);
}

void CFrmAdminInfo::on_cbBox23_clicked()
{
    qDebug() << "on_cbBox23_clicked()";

    emit __OnOpenLockRequest(23);
}

void CFrmAdminInfo::on_cbBox24_clicked()
{
    qDebug() << "on_cbBox24_clicked()";

    emit __OnOpenLockRequest(24);
}

void CFrmAdminInfo::on_cbBox25_clicked()
{
    qDebug() << "on_cbBox25_clicked()";

    emit __OnOpenLockRequest(25);
}

void CFrmAdminInfo::on_cbBox26_clicked()
{
    qDebug() << "on_cbBox26_clicked()";

    emit __OnOpenLockRequest(26);
}

void CFrmAdminInfo::on_cbBox27_clicked()
{
    qDebug() << "on_cbBox27_clicked()";

    emit __OnOpenLockRequest(27);
}

void CFrmAdminInfo::on_cbBox28_clicked()
{
    qDebug() << "on_cbBox28_clicked()";

    emit __OnOpenLockRequest(28);
}

void CFrmAdminInfo::on_cbBox29_clicked()
{
    qDebug() << "on_cbBox29_clicked()";

    emit __OnOpenLockRequest(29);
}

void CFrmAdminInfo::on_cbBox30_clicked()
{
    qDebug() << "on_cbBox30_clicked()";

    emit __OnOpenLockRequest(30);
}

void CFrmAdminInfo::on_cbBox31_clicked()
{
    qDebug() << "on_cbBox31_clicked()";

    emit __OnOpenLockRequest(31);
}

void CFrmAdminInfo::on_cbBox32_clicked()
{
    qDebug() << "on_cbBox32_clicked()";

    emit __OnOpenLockRequest(32);
}

void CFrmAdminInfo::on_cbBox33_clicked()
{
    qDebug() << "on_cbBox33_clicked()";

    emit __OnOpenLockRequest(33);
}

void CFrmAdminInfo::on_cbBox34_clicked()
{
    qDebug() << "on_cbBox34_clicked()";

    emit __OnOpenLockRequest(34);
}

void CFrmAdminInfo::on_cbBox35_clicked()
{
    qDebug() << "on_cbBox35_clicked()";

    emit __OnOpenLockRequest(35);
}

void CFrmAdminInfo::on_cbBox36_clicked()
{
    qDebug() << "on_cbBox36_clicked()";

    emit __OnOpenLockRequest(36);
}

void CFrmAdminInfo::on_cbBox37_clicked()
{
    qDebug() << "on_cbBox37_clicked()";

    emit __OnOpenLockRequest(37);
}

void CFrmAdminInfo::on_cbBox38_clicked()
{
    qDebug() << "on_cbBox38_clicked()";

    emit __OnOpenLockRequest(38);
}

void CFrmAdminInfo::on_cbBox39_clicked()
{
    qDebug() << "on_cbBox39_clicked()";

    emit __OnOpenLockRequest(39);
}

void CFrmAdminInfo::on_cbBox40_clicked()
{
    qDebug() << "on_cbBox40_clicked()";

    emit __OnOpenLockRequest(40);
}

void CFrmAdminInfo::on_cbBox41_clicked()
{
    qDebug() << "on_cbBox41_clicked()";

    emit __OnOpenLockRequest(41);
}

void CFrmAdminInfo::on_cbBox42_clicked()
{
    qDebug() << "on_cbBox42_clicked()";

    emit __OnOpenLockRequest(42);
}

void CFrmAdminInfo::on_cbBox43_clicked()
{
    qDebug() << "on_cbBox43_clicked()";

    emit __OnOpenLockRequest(43);
}

void CFrmAdminInfo::on_cbBox44_clicked()
{
    qDebug() << "on_cbBox44_clicked()";

    emit __OnOpenLockRequest(44);
}

void CFrmAdminInfo::on_cbBox45_clicked()
{
    qDebug() << "on_cbBox45_clicked()";

    emit __OnOpenLockRequest(45);
}

void CFrmAdminInfo::on_cbBox46_clicked()
{
    qDebug() << "on_cbBox46_clicked()";

    emit __OnOpenLockRequest(46);
}

void CFrmAdminInfo::on_cbBox47_clicked()
{
    qDebug() << "on_cbBox47_clicked()";

    emit __OnOpenLockRequest(47);
}

void CFrmAdminInfo::on_cbBox48_clicked()
{
    qDebug() << "on_cbBox48_clicked()";

    emit __OnOpenLockRequest(48);
}

void CFrmAdminInfo::on_cbBox49_clicked()
{
    qDebug() << "on_cbBox49_clicked()";

    emit __OnOpenLockRequest(49);
}

void CFrmAdminInfo::on_cbBox50_clicked()
{
    qDebug() << "on_cbBox50_clicked()";

    emit __OnOpenLockRequest(50);
}

void CFrmAdminInfo::on_cbBox51_clicked()
{
    qDebug() << "on_cbBox51_clicked()";

    emit __OnOpenLockRequest(51);
}

void CFrmAdminInfo::on_cbBox52_clicked()
{
    qDebug() << "on_cbBox52_clicked()";

    emit __OnOpenLockRequest(52);
}

void CFrmAdminInfo::on_cbBox53_clicked()
{
    qDebug() << "on_cbBox53_clicked()";

    emit __OnOpenLockRequest(53);
}

void CFrmAdminInfo::on_cbBox54_clicked()
{
    qDebug() << "on_cbBox54_clicked()";

    emit __OnOpenLockRequest(54);
}

void CFrmAdminInfo::on_cbBox55_clicked()
{
    qDebug() << "on_cbBox55_clicked()";

    emit __OnOpenLockRequest(55);
}

void CFrmAdminInfo::on_cbBox56_clicked()
{
    qDebug() << "on_cbBox56_clicked()";

    emit __OnOpenLockRequest(56);
}

void CFrmAdminInfo::on_cbBox57_clicked()
{
    qDebug() << "on_cbBox57_clicked()";

    emit __OnOpenLockRequest(57);
}

void CFrmAdminInfo::on_cbBox58_clicked()
{
    qDebug() << "on_cbBox58_clicked()";

    emit __OnOpenLockRequest(58);
}

void CFrmAdminInfo::on_cbBox59_clicked()
{
    qDebug() << "on_cbBox59_clicked()";

    emit __OnOpenLockRequest(59);
}

void CFrmAdminInfo::on_cbBox60_clicked()
{
    qDebug() << "on_cbBox60_clicked()";

    emit __OnOpenLockRequest(60);
}

void CFrmAdminInfo::on_cbBox61_clicked()
{
    qDebug() << "on_cbBox61_clicked()";

    emit __OnOpenLockRequest(61);
}

void CFrmAdminInfo::on_cbBox62_clicked()
{
    qDebug() << "on_cbBox62_clicked()";

    emit __OnOpenLockRequest(62);
}

void CFrmAdminInfo::on_cbBox63_clicked()
{
    qDebug() << "on_cbBox63_clicked()";

    emit __OnOpenLockRequest(63);
}

void CFrmAdminInfo::on_cbBox64_clicked()
{
    qDebug() << "on_cbBox64_clicked()";

    emit __OnOpenLockRequest(64);
}

void CFrmAdminInfo::on_btnReadDoorLocks_2_clicked()
{
}

void CFrmAdminInfo::OnLockStatusUpdated(CLocksStatus *locksStatus)
{
    qDebug() << "CFrmAdminInfo::OnLockStatusUpdate()";
    _pLocksStatus = locksStatus;

    // Update the view
    // Populate the Available doors combobox
    populateAvailableDoors();
}

void CFrmAdminInfo::OnLockSet(CLockSet *pSet)
{
    // Display
    qDebug() << "CFrmAdminInfo::OnLockSet(). Count:" << pSet->getLockMap()->size();

    displayInTable(pSet);

    pSet = NULL;
}

void CFrmAdminInfo::OnLockHistorySet(CLockHistorySet *pSet)
{
    qDebug() << "OnLockHistorySet()";
    displayInHistoryTable(pSet);
    pSet = NULL;
}

/*
 * const int fLockNum = 0;
const int fDesc = 1;
const int fCode1 = 2;
const int fCode2 = 3;
const int fStart = 4;
const int fEnd = 5;
*/
void CFrmAdminInfo::codeTableCellSelected(int nRow, int nCol)
{
    if(_pworkingSet)
    {
        CLockSet::Iterator itor;
        CLockState  *pState;
        itor = _pworkingSet->begin();
        for(int i=0; i<nRow; i++, itor++) {
        }

        pState = itor.value();  // This should be the correct state

        // Pull up code entry for edit
    }
}

void CFrmAdminInfo::codeHistoryTableCellSelected(int nRow, int nCol)
{
    if(_pworkingSet)
    {
        CLockSet::Iterator itor;
        CLockState  *pState;
        itor = _pworkingSet->begin();
        for(int i=0;i<nRow;i++,itor++) {
        }

        pState = itor.value();  // This should be the correct state

        // Pull up code entry for edit
    }
}

void CFrmAdminInfo::displayInTable(CLockSet *pSet)
{
    QTableWidget    *table = ui->tblCodesList;

    table->clear();
    table->setRowCount(pSet->getLockMap()->size());
    table->setColumnCount(7);

    QStringList headers, vheader;

    table->setColumnWidth(0, 40);
    table->setColumnWidth(1, 80);
    table->setColumnWidth(2, 130);
    table->setColumnWidth(3, 110);
    table->setColumnWidth(4, 110);
    table->setColumnWidth(5, 160);
    table->setColumnWidth(6, 160);

    table->verticalHeader()->hide();
    headers<<"Line"<<"Lock #"<<"Username"<<"Code#1"<<"Code#2"<<"Start Time"<<"End Time";
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setFixedWidth(60);
    table->horizontalHeader()->setFixedHeight(50);

    CLockSet::Iterator itor;
    CLockState  *pState;

    _pworkingSet = pSet;    // Hold onto the set for additional work.

    connect( table, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT( codeTableCellSelected( int, int ) ) );

    table->setStyleSheet("QTableView {selection-background-color: gray;}");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    int nRow = 0;
    int nCol = 0;

    QDateTime alwaysActiveStart = QDateTime(QDate(1990, 1, 1), QTime(0, 0, 0));
    QDateTime alwaysActiveEnd = QDateTime(QDate(1990, 1, 1), QTime(0, 0, 0));

    for(itor = pSet->begin(); itor != pSet->end(); itor++)
    {
        pState = itor.value();
        qDebug() << "Adding row of Lock State - Codes. Lock Num:" << QVariant(pState->getLockNum()).toString();
        nCol = 0;
        table->setItem(nRow, nCol++, new QTableWidgetItem(QVariant(nRow + 1).toString()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(QVariant(pState->getLockNum()).toString()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription().c_str()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getCode1().c_str()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getCode2().c_str()));

        if( (pState->getStartTime().secsTo(alwaysActiveStart) == 0) && (pState->getEndTime().secsTo(alwaysActiveEnd) == 0) )
        {
            table->setItem(nRow, nCol++, new QTableWidgetItem("ALWAYS"));
            table->setItem(nRow, nCol++, new QTableWidgetItem("ACTIVE"));
        }
        else
        {
            table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getStartTime().toString("MMM dd yyyy hh:mm:ss AP")));
            table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getEndTime().toString("MMM dd yyyy hh:mm:ss AP")));
        }
        nRow++;
    }
}

void CFrmAdminInfo::setupCodeTableContextMenu() {
    QTableWidget    *table = ui->tblCodesList;

    _pTableMenu = new QMenu(table);
    _pTableMenu->addAction("Edit Code", this, SLOT(codeEditSelection()));
    _pTableMenu->addAction("Add Code", this, SLOT(codeInitNew()));
    _pTableMenu->addAction("Delete", this, SLOT(codeDeleteSelection()));
    connect(table,SIGNAL(cellClicked(int,int)),this,SLOT(OnRowSelected(int, int)));

    _pTableMenuAdd = new QMenu(table);
    _pTableMenuAdd->addAction("Add Code", this, SLOT(codeInitNew()));
    connect(table->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnHeaderSelected(int)));
}

void CFrmAdminInfo::displayInHistoryTable(CLockHistorySet *pSet)
{
    QTableWidget    *table = ui->tblHistory;
    table->setRowCount(pSet->getLockHistoryMap()->size());
    table->setColumnCount(5);

    QStringList headers;
    table->setColumnWidth(0, 80);
    table->setColumnWidth(1, 120);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 100);
    table->setColumnWidth(4, 350);
    headers<<"Lock #"<<"Username"<<"Code#1"<<"Code#2"<<"Accessed";
    table->setHorizontalHeaderLabels(headers);

    CLockHistorySet::Iterator itor;
    CLockHistoryRec  *pState;
    std::string unencCode1, unencCode2;

    if(_phistoryWorkingSet) {
        _phistoryWorkingSet->clearSet();
        delete _phistoryWorkingSet;
    }

    _phistoryWorkingSet = pSet;    // Hold onto the set for additional work.

    connect( table, SIGNAL( cellDoubleClicked (int, int) ), this, SLOT( codeHistoryTableCellSelected( int, int ) ) );

    table->setStyleSheet("QTableView {selection-background-color: lightblue;}");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int nRow = 0;
    int nCol = 0;
    for(itor = pSet->begin(); itor != pSet->end(); itor++)
    {
        pState = itor.value();
        qDebug() << "Adding row of History Lock State - Codes. Lock Num:" << QVariant(pState->getLockNum()).toString();
        nCol = 0;
        table->setItem(nRow, nCol++, new QTableWidgetItem(QVariant(pState->getLockNum()).toString()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getDescription().c_str()));
        unencCode1 = pState->getCode1();
        unencCode2 = pState->getCode2();

        qDebug() << "  >>>History>> Code1:" << unencCode1.c_str() << "  code2:" << unencCode2.c_str();

        table->setItem(nRow, nCol++, new QTableWidgetItem(unencCode1.c_str()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(unencCode2.c_str()));
        table->setItem(nRow, nCol++, new QTableWidgetItem(pState->getAccessTime().toString("yyyy-MM-dd HH:mm:ss AP")));
        nRow++;
    }
}

void CFrmAdminInfo::codeDeleteSelection()
{
    qDebug() << "codeDeleteSelection";

    int nRC = QMessageBox::warning(this, tr("Verify Delete"),
                                   tr("Do you want to delete the selected code?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) {
        deleteCodeByRow(_nRowSelected);
    }
}

void CFrmAdminInfo::codeAddNew()
{
    qDebug() << "codeAddNew";
    addCodeByRow(_nRowSelected);
}

void CFrmAdminInfo::codeInitNew()
{
    qDebug() << "codeInitNew";

    int nLock = ui->cbLockNum->currentIndex();
    qDebug() << " " << QString::number(nLock) << "\n";
    if( nLock == -1 )
        nLock = 1;
    addCodeByRow(nLock);
}

void CFrmAdminInfo::codeEditSelection()
{
    qDebug() << "codeEditSelection row:" << QVariant(_nRowSelected).toString();
    editCodeByRow(_nRowSelected);
}

void CFrmAdminInfo::codeCellSelected( int row, int col) {
    CLockSet::Iterator itor;
    CLockState  *pState;
    int nRow = 0;
    int nCol = 0;
    QTableWidget    *table = ui->tblCodesList;

    _nRowSelected = row;

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        if(nRow == row) {
            // itor is our man!
            pState = itor.value();
        }

        nRow++;
    }
}

/**
 * @brief CFrmAdminInfo::isLock
 * here for efficiency only
 * @param nLockNum
 * @return
 */
int CFrmAdminInfo::isLock(uint16_t nLockNum)
{
    // Test
    uint64_t    un64Mask = 0x0000000000000001;
    uint64_t    un64LockNum = (unsigned long long)nLockNum - 1LL;  // Set Lock num to zero based number
    if ((_un64LockLocks & (un64Mask << un64LockNum)) != 0 )
    {
        return 1;
    } else {
        return 0;
    }
}

void CFrmAdminInfo::openAllDoors()
{
    qDebug() << "CFrmAdminInfo::openAllDoors()";

    // Loop to open all doors
    for(uint i = 1; i <= _tmpAdminRec.getMaxLocks() && !_bStopOpen; i++)
    {
        while(!_bContinueOpenLoop && !_bStopOpen) {
            QCoreApplication::processEvents();
        }
        _tmpDoorOpen = i;
        _bContinueOpenLoop = false;
        QTimer::singleShot(2000, this, SLOT(OpenDoorTimer()));
    }
}

void CFrmAdminInfo::OpenDoorTimer()
{
    if(!_bStopOpen) {
        int nDoor = _tmpDoorOpen;
        if(_pLocksStatus->isLock(nDoor))
        {
            emit __OnOpenLockRequest(nDoor);
        }
        _bContinueOpenLoop = true;
    }
}

void CFrmAdminInfo::on_dialBright_valueChanged(int value)
{
    if( value < 20 ) value = 20;
    if( value > 255) value = 255;
    emit __OnBrightnessChanged(value);  //
}

void CFrmAdminInfo::on_btnReadCodes_clicked()
{
    QDateTime dtStart = ui->dtStartCodeList->dateTime();
    QDateTime dtEnd = ui->dtEndCodeList->dateTime();
    int nLock = ui->cbLockNum->currentIndex();
    if( nLock == 0 ) {
        nLock = -1;
    }
    emit __OnReadLockSet(nLock, dtStart, dtEnd);
}

void CFrmAdminInfo::LocalReadLockSet(int nLock, QDateTime dtStart, QDateTime dtEnd)
{
    ui->cbLockNum->setCurrentIndex( nLock );
    ui->dtStartCodeList->setDateTime(dtStart);
    ui->dtEndCodeList->setDateTime(dtEnd);
    emit __OnReadLockSet(nLock, QDateTime::fromString(_DATENONE), QDateTime::fromString(_DATENONE));
}

void CFrmAdminInfo::LocalReadLockHistorySet(int nLock, QDateTime dtStart, QDateTime dtEnd)
{
    ui->cbLockNumHistory->setCurrentIndex( nLock );
    ui->dtStartCodeHistoryList->setDateTime(dtStart);
    ui->dtEndCodeHistoryList->setDateTime(dtEnd);
    emit __OnReadLockSet(nLock, QDateTime::fromString(_DATENONE), QDateTime::fromString(_DATENONE));
}

void CFrmAdminInfo::on_btnReadCodes_2_clicked()
{
    //
    QDateTime dtStart = ui->dtStartCodeHistoryList->dateTime();
    QDateTime dtEnd = ui->dtEndCodeHistoryList->dateTime();
    int nLock = ui->cbLockNumHistory->currentIndex();
    if( nLock == 0 ) {
        nLock = -1;
    }
    emit __OnReadLockHistorySet(nLock, dtStart, dtEnd);
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
    qDebug() << "SMTP:" << smtpserver << ":" << QVariant(smtpport).toString() << " user:" << smtpusername << " pw:" << smtppassword;
    _tmpAdminRec.setSMTPServer(smtpserver.toStdString());
    _tmpAdminRec.setSMTPPort(smtpport);
    _tmpAdminRec.setSMTPType(smtptype);
    _tmpAdminRec.setSMTPUsername(smtpusername.toStdString());
    _tmpAdminRec.setSMTPPassword(smtppassword.toStdString());
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
    qDebug() << "VNC:" << QVariant(vncport).toString() << " pw:" << vncpassword;
    _tmpAdminRec.setVNCPassword(vncpassword.toStdString());
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
    qDebug() << "SMTP:" << _tmpAdminRec.getSMTPServer().c_str() << ":" << QVariant(_tmpAdminRec.getSMTPPort()).toString();
    dlgSMTP->setValues(_tmpAdminRec.getSMTPServer().c_str(), _tmpAdminRec.getSMTPPort(), _tmpAdminRec.getSMTPType(),
                       _tmpAdminRec.getSMTPUsername().c_str(), _tmpAdminRec.getSMTPPassword().c_str() );
    connect(dlgSMTP, SIGNAL(__onSMTPDialogComplete(CDlgSMTP *)), this, SLOT(onSMTPDialogComplete(CDlgSMTP *)));
    dlgSMTP->show();
    this->setEnabled(true);
}

void CFrmAdminInfo::on_btnSetupVNC_clicked()
{
    // Display Email Dialog for VNC remote desktop  server settings
    CDlgVNC    *dlgVNC = new CDlgVNC();

    this->setEnabled(false);
    qDebug() << "VNC:" << _tmpAdminRec.getVNCServer().c_str() << ":" << QVariant(_tmpAdminRec.getVNCPort()).toString();
    dlgVNC->setValues(_tmpAdminRec.getVNCPort(), _tmpAdminRec.getVNCPassword().c_str() );
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

    emit __OnImmediateReportRequest(dtStart, dtEnd, -1);
}

void CFrmAdminInfo::on_btnOpenAllDoors_2_clicked(bool checked)
{
    if( checked ) {
        _bContinueOpenLoop = true;
        _bStopOpen = false;
        openAllDoors();
    } else {
        // Stop
        _bContinueOpenLoop = true;
        _bStopOpen = true;
    }
}

void CFrmAdminInfo::on_btnToggleSource_clicked(bool checked)
{
    if(checked) {
        populateFileWidget(usbDevice1, usbDevice0);
        ui->btnToggleSource->setText("Source #2");
    } else {
        populateFileWidget(usbDevice0, usbDevice1);
        ui->btnToggleSource->setText("Source #1");
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
}

void CFrmAdminInfo::on_tblCodesList_cellDoubleClicked(int row, int column)
{
}

// Add a new code
void CFrmAdminInfo::on_tblCodesList_clicked(const QModelIndex &index)
{
}

void CFrmAdminInfo::OnCodeEditClose() {
    //
    if(_pFrmCodeEdit)
    {
        _pFrmCodeEdit->hide();
    }
}

void CFrmAdminInfo::OnCodeEditDoneSave(int nRow, int nId, int nLockNum, QString sAccessCode,
                                       QString sSecondCode, QString sDescription, QDateTime dtStart, QDateTime dtEnd, bool fingerprint1, bool fingerprint2,
                                       bool askQuestions, QString question1, QString question2, QString question3)
{
    qDebug() << "CFrmAdminInfo::OnCodeEditDoneSave(), nRow: " << QString::number(nRow) << " nId: " << QString::number(nId);
    _pState->setLockNum(nLockNum);
    _pState->setCode1(sAccessCode.toStdString());
    _pState->setCode2(sSecondCode.toStdString());
    _pState->setDescription(sDescription.toStdString());
    _pState->setStartTime(dtStart);
    _pState->setEndTime(dtEnd);

    if(fingerprint1)
        _pState->setFingerprint1();
    else
        _pState->clearFingerprint1();

    qDebug() << "------------------------QUESTION 1: " << question1;
    qDebug() << "------------------------QUESTION 2: " << question2;
    qDebug() << "------------------------QUESTION 3: " << question3;

    _pState->setAskQuestions(askQuestions);
    _pState->setQuestion1(question1.toStdString());
    _pState->setQuestion2(question2.toStdString());
    _pState->setQuestion3(question3.toStdString());

    if(_pState->isNew())
    {
        if(_pworkingSet)
        {
            _pworkingSet->addToSet(_pState);
        }
        emit __OnUpdateCodeState(_pState);
    } else {
        nRow = 0;
        int nCol = 0;
        QTableWidget    *table = ui->tblCodesList;
        QTableWidgetItem    *item;
        item = table->item(nRow, nCol++);
        if(item) {
            item->setText(QVariant(nLockNum).toString());
            item = table->item(nRow, nCol++);
            item->setText(sDescription);
            item = table->item(nRow, nCol++);
            item->setText(sAccessCode);
            item = table->item(nRow, nCol++);
            item->setText(sSecondCode);
            item = table->item(nRow, nCol++);
            item->setText(dtStart.toString("MMM dd yyyy hh:mm AP"));
            item = table->item(nRow, nCol++);
            item->setText(dtEnd.toString("MMM dd yyyy hh:mm AP"));
            item=table->item(nRow, nCol++);
        }
        _pState->setModified();
        emit __OnUpdateCodeState(_pState);
    }
}

CLockState* CFrmAdminInfo::createNewLockState()
{
    int lockNum = ui->cbLockNum->currentIndex();
    if( lockNum == -1 )
        lockNum = 1;

    CLockState  *pState;
    pState = new CLockState();
    pState->setID(-1);
    pState->setLockNum(lockNum);
    pState->setCode1("");
    pState->setCode2("");
    pState->setDescription("");
    pState->setStartTime(QDateTime().currentDateTime());
    pState->setEndTime(QDateTime().currentDateTime());

    pState->clearFingerprint1();
    pState->clearFingerprint2();

    pState->setNew();
    pState->clearMarkForDeletion();
    pState->clearModified();

    return pState;
}

void CFrmAdminInfo::setTableMenuLocation(QMenu *pmenu)
{
    pmenu->setGeometry(_lastTouchPos.x(),_lastTouchPos.y(),pmenu->width(), pmenu->height());
}

void CFrmAdminInfo::OnRowSelected(int row, int column) {
    _nRowSelected = row;
    qDebug() << "Row Selected:" << QVariant(_nRowSelected).toString();
    _pTableMenu->show();

    //MANUAL GEOMETRY
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
            return true;
    }

    return QDialog::eventFilter(target, event);
}

void CFrmAdminInfo::OnCodes(QString code1, QString code2)
{
    QString str = code1;
    if( code2.size() > 0 )
    {
        str += " : " + code2;
    }
    ui->edInfo->setText(str);

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
            qDebug() << "Show Menu";
            _pTableMenuAdd->show();
            setTableMenuLocation(_pTableMenuAdd);
        }

        if(ui->tblCodesList->rowCount() > 0 )
        {
            qDebug() << "Show Menu";
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

void CFrmAdminInfo::OnHeaderSelected(int nHeader) {
    _pTableMenuAdd->show();

    //MANUAL GEOMETRY
    QPoint widgetPoint = QWidget::mapFromGlobal(QCursor::pos());
    _pTableMenuAdd->setGeometry(widgetPoint.x(), widgetPoint.y(), _pTableMenu->width(), _pTableMenu->height());
}

void CFrmAdminInfo::checkAndCreateCodeEditForm()
{
    if(!_pFrmCodeEdit) {
        _pFrmCodeEdit = new CFrmCodeEdit(this);
        _pFrmCodeEdit->setMaxLocks(_tmpAdminRec.getMaxLocks());
        connect(_pFrmCodeEdit, SIGNAL(OnClose()), this, SLOT(OnCodeEditClose()));
        connect(_pFrmCodeEdit, SIGNAL(OnDoneSave(int,int,int,QString,QString,QString,QDateTime,QDateTime,bool,bool,bool,QString,QString,QString)),
                this, SLOT(OnCodeEditDoneSave(int,int,int,QString,QString,QString,QDateTime,QDateTime,bool,bool,bool,QString,QString,QString)));
        connect(this, SIGNAL(__OnAdminInfoCodes(QString,QString)), _pFrmCodeEdit, SLOT(OnAdminInfoCodes(QString,QString)));
    }
}

void CFrmAdminInfo::addCodeByRow(int row)
{
    checkAndCreateCodeEditForm();

    // Get line values
    CLockSet::Iterator itor;
    _pState = 0;
    int nRow = 0;

    _pState = createNewLockState();
    if(!_pworkingSet) {
        _pworkingSet = new CLockSet();
    }

    _pFrmCodeEdit->setValues(_pState->getID(), _pState->getLockNum(), _pState->getCode1().c_str(), _pState->getCode2().c_str(), _pState->getDescription().c_str(), _pState->getStartTime(), _pState->getEndTime(), false, false, _pState->getAskQuestions(), _pState->getQuestion1(), _pState->getQuestion2(), _pState->getQuestion3());
    _pFrmCodeEdit->setEditingRow(-1);
    _pFrmCodeEdit->show();
}

void CFrmAdminInfo::editCodeByRow(int row)
{
    checkAndCreateCodeEditForm();

    // Get line values
    CLockSet::Iterator itor;
    _pState = 0;
    int nRow = 0;

    qDebug() << "before loop iterator cellClicked";

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        if(nRow == row) {
            // itor is our man!
            _pState = itor.value();
            break;
        }
        nRow++;
    }

    if(_pState) {
        qDebug() << "   pState Found One";
        _pFrmCodeEdit->setValues(_pState->getID(), _pState->getLockNum(), _pState->getCode1().c_str(), _pState->getCode2().c_str(), _pState->getDescription().c_str(),
                                 _pState->getStartTime(), _pState->getEndTime(), _pState->getFingerprint1(), _pState->getFingerprint2(), _pState->getAskQuestions(), _pState->getQuestion1(), _pState->getQuestion2(), _pState->getQuestion3());
        _pFrmCodeEdit->setEditingRow(row);
        _pFrmCodeEdit->show();
    }
    else {
        qDebug() << "   pState NOT Found";

        _pState = createNewLockState();
        _pworkingSet->addToSet(_pState);

        _pFrmCodeEdit->setValues(_pState->getID(), _pState->getLockNum(), _pState->getCode1().c_str(), _pState->getCode2().c_str(), _pState->getDescription().c_str(),
                                 _pState->getStartTime(), _pState->getEndTime(), false, false, _pState->getAskQuestions(), _pState->getQuestion1(), _pState->getQuestion2(), _pState->getQuestion3());
        _pFrmCodeEdit->setEditingRow(-1);
        _pFrmCodeEdit->show();
    }
}

void CFrmAdminInfo::deleteCodeByRow(int row)
{
    checkAndCreateCodeEditForm();

    // Get line values
    CLockSet::Iterator itor;
    _pState = 0;
    int nRow = 0;

    qDebug() << "before loop iterator cellClicked";

    for(itor = _pworkingSet->begin(); itor != _pworkingSet->end(); itor++)
    {
        if(nRow == row) {
            // itor is our man!
            _pState = itor.value();
            break;
        }
        nRow++;
    }

    if(_pState)
    {
        _pState->setMarkForDeletion();

        QString printDirectory = "/home/pi/run/prints/";
        printDirectory += _pState->getCode1().c_str();

        qDebug() << "CFrmAdminInfo::deleteCodeByRow(), printDirectory: " << printDirectory;

        if( QDir(printDirectory).exists() )
            std::system( ("sudo rm -rf " + printDirectory.toStdString()).c_str());
        emit __OnUpdateCodeState(_pState);
    }
}

void CFrmAdminInfo::purgeCodes()
{
    // Removes all codes
    qDebug() << "CFrmAdminInfo::purgeCodes()";

    // Get line values
    CLockSet::Iterator itor;
    _pState = 0;
    int nRow = 0;

    if(!_pworkingSet) {
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

    QString printDirectory = "/home/pi/run/prints/*";

    qDebug() << "CFrmAdminInfo::purgeCodes(), printDirectory: " << printDirectory;
    std::system( ("sudo rm -rf " + printDirectory.toStdString()).c_str());
}

void CFrmAdminInfo::on_tblCodesList_cellClicked(int row, int column)
{
    qDebug() << "CFrmAdminInfo::on_tblCodesList_cellClicked. Column:" << QVariant(column).toString()
             << " Row:" << QVariant(row).toString();
    //
    _nRowSelected = row;
    qDebug() << "Row Selected:" << QVariant(_nRowSelected).toString();
}

void CFrmAdminInfo::on_btnRebootSystem_clicked()
{
    int nRC = QMessageBox::warning(this, tr("Verify System Reboot"),
                                   tr("System is about to be restarted.\nDo you want to restart?"),
                                   QMessageBox::Yes, QMessageBox::Cancel);
    if(nRC == QMessageBox::Yes) {
        sync();
        std::system("sudo shutdown -r now");
    }
}

void CFrmAdminInfo::on_btnPurgeCodes_clicked() { qDebug() <<
                                                             "CFrmAdminInfo::on_btnPurgeCodes_clicked()"; on_btnReadCodes_clicked(); int nRC
            = QMessageBox::warning(this, tr("Verify Remove All Codes"), tr("All access
                                                                           codes will be removed from the system\nDo you want to continue?"),
                                                                                                                                          QMessageBox::Yes, QMessageBox::Cancel); if(nRC == QMessageBox::Yes) { qDebug()
            << "CFrmAdminInfo::on_btnPurgeCodes_clicked()"; purgeCodes(); usleep(50000);
            on_btnReadCodes_clicked(); nRC = QMessageBox::warning(this, tr("Code Removal
                                                                           Success"), tr("Code Removal is successful!!\nPlease give the codes list a
                                                                           moment to update."), QMessageBox::Ok); } }
