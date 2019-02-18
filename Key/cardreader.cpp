#include "cardreader.h"

#include <QProcess>
#include <QThread>

#include "kcbutils.h"

CardReader::CardReader(QObject *parent) :
    QObject(parent),
    m_proc(* new QProcess())
{
    kcb::Utils::ConstructorMsg(this);
}

CardReader::~CardReader()
{
    kcb::Utils::DestructorMsg(this);
}

void CardReader::Connect(QThread& thread)
{
    Q_UNUSED(thread);
}

void CardReader::ParseStdOut()
{
    DoParse();
}

void CardReader::Scan()
{
    DoScan();
}

QString CardReader::Convert(QString value)
{
    return DoConvert(value);
}

