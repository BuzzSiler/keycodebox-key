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


    ui->edValue->setCursorPosition(ui->edValue->text().length()-1);
}

DlgNumberPad::~DlgNumberPad()
{
    delete ui;
}

void DlgNumberPad::setValue(QString value)
{
    m_value = value;
    ui->edValue->setText(m_value);
    m_only_once = false;
}

void DlgNumberPad::getValue(QString& value)
{
    value = m_value;
    setValue("");
}

void DlgNumberPad::on_pbBack_clicked()
{
    m_value.truncate(m_value.length()-1);
    setValue(m_value);
}

void DlgNumberPad::on_pbClear_clicked()
{
    setValue("");
}

void DlgNumberPad::on_pbCancel_clicked()
{
    emit NotifyRejected();
}

void DlgNumberPad::on_pbEnter_clicked()
{
    qDebug() << "on_pbEnter_clicked";
    if (!m_only_once)
    {
        m_only_once = true;
        emit NotifyAccepted();
    }
}

void DlgNumberPad::buttonClicked(int index)
{
    m_value += m_buttons[index]->text();
    setValue(m_value);
}
