#-------------------------------------------------
#
# Project created by QtCreator 2016-07-27T10:04:34
#
#-------------------------------------------------

QT       += core gui serialport sql network

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Alpha
target.path = /home/pi/kcb-config/bin/Alpha
INSTALLS += target
TEMPLATE = app

INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/include/libusb-1.0
INCLUDEPATH += /usr/include/arm-linux-gnueabihf/qt5
INCLUDEPATH += /usr/include/libxml2

LIBS += -L/usr/lib/qt5
LIBS += -L/usr/lib/arm-linux-gnueabihf/qt5
LIBS += -lhidapi-hidraw 
LIBS += -lsqlite3
LIBS += -lfprint
LIBS += -lxml2

SOURCES += main.cpp\
    keycodeboxmain.cpp \
    lockcontroller.cpp \
    usbdrivecontroller.cpp \
    usbcontroller.cpp \
    fingerprintreader.cpp \
    magtekcardreader.cpp \
    usbhwkeypad.cpp \
    securityadmin.cpp \
    securitycontroller.cpp \
    securityuser.cpp \
    frmusercode.cpp \
    lcdgraphicscontroller.cpp \
    systemcontroller.cpp \
    modelsecurity.cpp \
    tblcodes.cpp \
    tbladmin.cpp \
    tblcodehistory.cpp \
    encryption.cpp \
    systemcontrollerthread.cpp \
    frmadmininfo.cpp \
    frmadminpassword.cpp \
    lockset.cpp \
    lockstate.cpp \
    menucheckbox.cpp \
    menupushbutton.cpp \
    clickablelabel.cpp \
    lockhistoryset.cpp \
    lockhistoryrec.cpp \
    reportcontroller.cpp \
    smtp/emailaddress.cpp \
    smtp/mimeattachment.cpp \
    smtp/mimecontentformatter.cpp \
    smtp/mimefile.cpp \
    smtp/mimehtml.cpp \
    smtp/mimeinlinefile.cpp \
    smtp/mimemessage.cpp \
    smtp/mimemultipart.cpp \
    smtp/mimepart.cpp \
    smtp/mimetext.cpp \
    smtp/quotedprintable.cpp \
    smtp/smtpclient.cpp \
    dlgfingerprint.cpp \
    dlgfingerprintverify.cpp \
    dlgeditquestions.cpp \
    dlgquestions.cpp \
    clickablegraphicsitem.cpp \
    clickablelineedit.cpp \
    hidreader.cpp \
    simplecrypt.cpp \
    usbprovider.cpp \
    serialport.cpp \
    selectlockswidget.cpp \
    frmselectlocks.cpp \
    frmcodeeditmulti.cpp \
    lockcabinetwidget.cpp \
    kcbkeyboardwidget.cpp \
    kcbkeyboarddialog.cpp \
    kcbutils.cpp \
    checkablestringlistmodel.cpp \
    reportcontrolwidget.cpp \
    dlgdownloadreports.cpp \
    kcbsystem.cpp \
    kcbapplication.cpp \
    omnikey5427ckreader.cpp \
    keycodeboxsettings.cpp \
    frmnetworksettings.cpp \
    cardreader.cpp \
    adminrec.cpp \
    codeexporter.cpp \
    codeelement.cpp \
    codelistingelement.cpp \
    xmlcodelistingreader.cpp \
    codeimportexportutil.cpp \
    codeimporter.cpp \
    logcategory.cpp \
    logger.cpp \
    cabinetrowdelegate.cpp \
    autocodegenwidget.cpp \
    autocodegenerator.cpp \
    autocodegenstatic.cpp \
    kcbstartup.cpp \
    cabinet_type.cpp \

HEADERS  += keycodeboxmain.h \
    usbdrivecontroller.h \
    usbcontroller.h \
    fingerprintreader.h \
    lockcontroller.h \
    magtekcardreader.h \
    usbhwkeypad.h \
    securityadmin.h \
    securitycontroller.h \
    securityuser.h \
    frmusercode.h \
    lcdgraphicscontroller.h \
    systemcontroller.h \
    modelsecurity.h \
    tblcodes.h \
    tbladmin.h \
    tblcodehistory.h \
    encryption.h \
    tbladmin.h \
    systemcontrollerthread.h \
    frmadmininfo.h \
    frmadminpassword.h \
    lockset.h \
    lockstate.h \
    menucheckbox.h \
    menupushbutton.h \
    clickablelabel.h \
    lockhistoryset.h \
    lockhistoryrec.h \
    reportcontroller.h \
    smtp/emailaddress.h \
    smtp/mimeattachment.h \
    smtp/mimecontentformatter.h \
    smtp/mimefile.h \
    smtp/mimehtml.h \
    smtp/mimeinlinefile.h \
    smtp/mimemessage.h \
    smtp/mimemultipart.h \
    smtp/mimepart.h \
    smtp/mimetext.h \
    smtp/quotedprintable.h \
    smtp/smtpclient.h \
    smtp/smtpexports.h \
    dlgfingerprint.h \
    dlgfingerprintverify.h \
    dlgeditquestions.h \
    dlgquestions.h \
    clickablegraphicsitem.h \
    reportcontroller.h \
    clickablelineedit.h \
    hidreader.h \
    simplecrypt.h \
    usbprovider.h \
    serialport.h \
    selectlockswidget.h \
    frmselectlocks.h \
    frmcodeeditmulti.h \
    lockcabinetwidget.h \
    kcbcommon.h \
    kcbkeyboardwidget.h \
    kcbkeyboarddialog.h \
    kcbutils.h \
    checkablestringlistmodel.h \
    reportcontrolwidget.h \
    dlgdownloadreports.h \
    kcbsystem.h \
    kcbapplication.h \
    omnikey5427ckreader.h \
    keycodeboxsettings.h \
    frmnetworksettings.h \
    cardreader.h \
    adminrec.h \
    codeexporter.h \
    codeelement.h \
    codelistingelement.h \
    xmlcodelistingreader.h \
    codeimportexportutil.h \
    codeimporter.h \
    logcategory.h \
    logger.h \
    cabinetrowdelegate.h \
    autocodegenwidget.h \
    autocodegenerator.h \
    autocodegenstatic.h \
    kcbstartup.h \
    cabinet_type.h \

FORMS    += mainwindow.ui \
    frmusercode.ui \
    frmadmininfo.ui \
    frmadminpassword.ui \
    dlgfingerprint.ui\
    dlgfingerprintverify.ui \
    dlgeditquestions.ui \
    dlgquestions.ui \
    selectlockswidget.ui \
    frmselectlocks.ui \
    frmcodeeditmulti.ui \
    lockcabinetwidget.ui \
    kcbkeyboardwidget.ui \
    kcbkeyboarddialog.ui \
    reportcontrolwidget.ui \
    dlgdownloadreports.ui \
    frmnetworksettings.ui \
    autocodegenwidget.ui
    




CONFIG += c++11
TRANSLATIONS = trans_fr.ts trans_sp.ts
