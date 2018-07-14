#include "scanner.h"

#include <QProcess>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>

#include "kcbcommon.h"

Scanner::Scanner(QObject *parent) :
    QObject(parent),
    m_proc(* new QProcess())
{

}

void Scanner::Connect(QThread& thread)
{
    connect(&thread, &QThread::started, this, &Scanner::Scan);
}

void Scanner::Scan()
{
    connect(&m_proc, &QProcess::readyReadStandardOutput, this, &Scanner::ParseStdOut);

    while (1)
    {
        m_proc.start(QString("pcsc_scan"), QStringList() << "-n");
        (void) m_proc.waitForFinished();
    }
}

void Scanner::ParseStdOut()
{
    static bool is_omnikey5427_reader = false;
    static bool is_card_inserted_removed = false;
    static bool is_valid_atr = false;

    QByteArray output;
    output = m_proc.readAllStandardOutput();

    /*
     * This text is presented when the card is inserted

Wed Jul 11 20:56:25 2018
Reader 0: HID OMNIKEY 5427 CK (010100534841353633015B4923175730) 00 00
  Card state: Card inserted,
  ATR: 3B 8F 80 01 80 4F 0C A0 00 00 03 06 0A 00 1F 00 00 00 00 7D

    * This text is presented when the card is removed

Wed Jul 11 20:56:26 2018
Reader 0: HID OMNIKEY 5427 CK (010100534841353633015B4923175730) 00 00
  Card state: Card removed,

     */

    /* Note: The pcsc_scan utility inserts ansi color codes.  They must be removed
     * before we can compare text.  The easist way is to convert the string to
     * hex, remove the escape sequences and convert back to text
     */
    QString after("");
    QString hexoutput = output.toHex();
    QRegularExpression regex("1b5b.*6d");
    // Inverted greediness allows .* to be minimal
    regex.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    hexoutput.replace(regex, after);
    QString stripped_output = QString(QByteArray::fromHex(hexoutput.toUtf8()));

    /* Process the state variables
        - Detect the monikey5427 reader
        - Detect the card state (inserted/removed)
        - Validate the ATR
     */
    if (!is_omnikey5427_reader)
    {
        is_omnikey5427_reader = stripped_output.contains("HID OMNIKEY 5427 CK");
    }

    if (is_card_inserted_removed)
    {
        is_card_inserted_removed = !stripped_output.contains("Card state: Card removed");
    }
    else
    {
        is_card_inserted_removed = stripped_output.contains("Card state: Card inserted");
    }

    if (!is_valid_atr)
    {
        is_valid_atr = stripped_output.contains("ATR: 3B 8F 80 01 80 4F 0C A0 00 00 03 06 0A 00 1F 00 00 00 00 7D");
    }

    if (is_omnikey5427_reader)
    {
        KCB_DEBUG_TRACE("OMNIKEY5427 Reader Detected");
    }

    QString card_state = is_card_inserted_removed ? "Card Inserted" : "Card Removed";
    KCB_DEBUG_TRACE(card_state);

    QString atr_state = is_valid_atr ? "Valid ATR" : "Invalid ATR";
    KCB_DEBUG_TRACE(atr_state);

    if (is_omnikey5427_reader && is_card_inserted_removed && is_valid_atr)
    {
        KCB_DEBUG_TRACE("Everything is good.  Go get the UID");

        QProcess proc;
        proc.start(QString("opensc-tool"), QStringList() << "-s" << "ffca000000");
        (void) proc.waitForStarted();
        (void) proc.waitForFinished();

        QString stdOut = proc.readAllStandardOutput();

        if (stdOut.contains("Received (SW1=0x90, SW2=0x00"))
        {
            QStringList lines = stdOut.split('\n');
            QStringList chars = lines[2].split(' ');
            chars.removeLast();
            QString digits = chars.join("");

            emit DigitsReceived(digits);
        }

        is_omnikey5427_reader = false;
        is_valid_atr = false;
    }
}
