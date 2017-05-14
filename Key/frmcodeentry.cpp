#include "frmcodeentry.h"
#include "ui_frmcodeentry.h"

CFrmCodeEntry::CFrmCodeEntry(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CFrmCodeEntry)
{
    ui->setupUi(this);
}

CFrmCodeEntry::~CFrmCodeEntry()
{
    delete ui;
}
