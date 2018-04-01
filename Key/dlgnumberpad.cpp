#include "dlgnumberpad.h"
#include "ui_dlgnumberpad.h"
#include <QDebug>
#include <QPushButton>
#include <QSignalMapper>
#include <QRegularExpression>

DlgNumberPad::DlgNumberPad(QWidget *parent) :
    QDialog(parent),
    m_mapper(* new QSignalMapper(this)),
    m_only_once(false),
    ui(new Ui::DlgNumberPad)
{
    ui->setupUi(this);

    setWindowState(Qt::WindowFullScreen);

    // Get a static list of the lock buttons on the widget
    m_buttons = this->findChildren<QPushButton *>(QRegularExpression("pb\\d"), Qt::FindChildrenRecursively);

    qDebug() << "buttons size:" << m_buttons.length();
    // Map the click signal from the buttons to lockSelected slot via a mapper
    for (int ii = 0; ii < m_buttons.length(); ++ii)
    {
        QPushButton *btn = m_buttons[ii];
        connect(btn, SIGNAL(clicked()), &m_mapper, SLOT(map()));
        m_mapper.setMapping(btn, ii);
    }
    connect(&m_mapper, SIGNAL(mapped(int)), this, SLOT(buttonClicked(int)));

    ui->pbEnter->setDisabled(true);
    ui->pbCancel->setEnabled(true);
    ui->edValue->setCursorPosition(ui->edValue->text().length()-1);
}

DlgNumberPad::~DlgNumberPad()
{
    delete ui;
}

void DlgNumberPad::setValue(const QString value)
{
    m_value = value;
    ui->edValue->setText(value);
    ui->pbEnter->setDisabled(true);
}

void DlgNumberPad::getValue(QString& value)
{
    value = ui->edValue->text();
}

void DlgNumberPad::on_pbBack_clicked()
{
    QString value = ui->edValue->text();
    value.truncate(value.length()-1);
    ui->edValue->setText(value);
    ui->pbEnter->setEnabled(value != "" && value != m_value);
}

void DlgNumberPad::on_pbClear_clicked()
{
    ui->edValue->setText("");
    ui->pbEnter->setEnabled(ui->edValue->text() != "" && ui->edValue->text() != m_value);
}

void DlgNumberPad::on_pbCancel_clicked()
{
    emit NotifyRejected();
    m_value = "";
}

void DlgNumberPad::on_pbEnter_clicked()
{
    qDebug() << "on_pbEnter_clicked";
    emit NotifyAccepted();
    m_value = "";
}

void DlgNumberPad::buttonClicked(int index)
{
    QString value = ui->edValue->text();
    value += m_buttons[index]->text();
    ui->edValue->setText(value);
    ui->pbEnter->setEnabled(ui->edValue->text() != "" && ui->edValue->text() != m_value);
}
