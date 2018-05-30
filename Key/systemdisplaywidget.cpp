#include "systemdisplaywidget.h"
#include "ui_systemdisplaywidget.h"

#include <QProcess>

#include "kcbcommon.h"
#include "kcbsystem.h"

SystemDisplayWidget::SystemDisplayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemDisplayWidget)
{
    ui->setupUi(this);

    QString serial_number;
    kcb::GetRpiSerialNumber(serial_number);
    ui->lbltxtRpiSerialNumber->setText(serial_number);
}

SystemDisplayWidget::~SystemDisplayWidget()
{
    delete ui;
}
