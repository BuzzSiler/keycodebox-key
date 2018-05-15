#include "autogeneratecontrolwidget.h"
#include "ui_autogeneratecontrolwidget.h"

AutoGenerateControlWidget::AutoGenerateControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoGenerateControlWidget)
{
    ui->setupUi(this);
}

AutoGenerateControlWidget::~AutoGenerateControlWidget()
{
    delete ui;
}
