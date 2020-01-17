#include "cardreader.h"

#include <QProcess>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>

#include "kcbcommon.h"

CardReader::CardReader(QObject *parent) :
    QObject(parent),
    m_proc(* new QProcess())
{

}

void CardReader::Connect(QThread& thread)
{
    connect(&thread, &QThread::started, this, &CardReader::Scan);
}

void CardReader::Scan()
{
    connect(&m_proc, &QProcess::readyReadStandardOutput, this, &CardReader::ParseStdOut);

    while (1)
    {
        m_proc.start(QString("python3"), QStringList() << KCB_SCRIPTS_PATH+"/hudson.py" << "2>&1");
        (void) m_proc.waitForFinished();
    }
}

void CardReader::ParseStdOut()
{
    QByteArray output;
    output = m_proc.readAllStandardOutput();

    // KCB_DEBUG_TRACE("Card Reader Output: " << output);

    bool not_none = !output.contains("None");
    bool not_empty = !output.trimmed().isEmpty();

    // The hudson script will return the card number, None, or nothing (i.e., linefeed)
    if (not_none && not_empty)
    {
        emit DigitsReceived(output.trimmed());
    }
}
