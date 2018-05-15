#include "systemdisplaywidget.h"
#include "ui_systemdisplaywidget.h"

SystemDisplayWidget::SystemDisplayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemDisplayWidget)
{
    ui->setupUi(this);
}

SystemDisplayWidget::~SystemDisplayWidget()
{
    delete ui;
}
