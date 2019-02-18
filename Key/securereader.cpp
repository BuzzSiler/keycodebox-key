#include "securereader.h"

#include <QProcess>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>

#include "kcbcommon.h"
#include "kcbsystem.h"
#include "kcbutils.h"

SecureReader::SecureReader(QObject *parent) :
    CardReader(parent)
{
    kcb::Utils::ConstructorMsg(this);
}

SecureReader::~SecureReader()
{
    kcb::Utils::DestructorMsg(this);
}

void SecureReader::Connect(QThread& thread)
{
    connect(&thread, &QThread::started, this, &CardReader::Scan);
}

void SecureReader::DoScan()
{
    // QString stdOut;
    // QString stdErr;
    // int status;
    connect(&m_proc, &QProcess::readyReadStandardOutput, this, &CardReader::ParseStdOut);

    while (1)
    {
        m_proc.start(QString("python3"), QStringList() << "/home/pi/kcb-config/scripts/hudson.py" << "2>&1");
        (void) m_proc.waitForFinished();

        // KCB_DEBUG_TRACE("Starting hudson.py");
        // kcb::ExecuteCommand(QString("python3"), QStringList() << "/home/pi/kcb-config/scripts/hudson.py" << "2>&1", stdOut, stdErr, status);
        // KCB_DEBUG_TRACE("Ending hudson.py" << stdOut << stdErr << status);
    }
    KCB_DEBUG_EXIT;
}

void SecureReader::DoParse()
{
    KCB_DEBUG_ENTRY;
    QByteArray output;
    output = m_proc.readAllStandardOutput();

    KCB_DEBUG_TRACE("Card Reader Output: " << output);

    bool not_none = !output.contains("None");
    bool not_empty = !output.trimmed().isEmpty();

    // The hudson script will return the card number, None, or nothing (i.e., linefeed)
    if (not_none && not_empty)
    {
        emit DigitsReceived(output.trimmed());
    }
    KCB_DEBUG_EXIT;
}

QString SecureReader::DoConvert(QString value)
{
    bool isOk;
    qulonglong dec_value = value.toInt(&isOk, 10);
    if (isOk)
    {
        return QString::number(dec_value);
    }
    else
    {
        return "COULD NOT CONVERT";
    }

}
