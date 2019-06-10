#include "omnikey5427ckreader.h"

#include <QThread>
#include <QDebug>
#include "cardreader.h"
#include "kcbcommon.h"

Omnikey5427CKReader::Omnikey5427CKReader(QObject *parent) :
    QObject(parent),
    m_cardReaderThread(*new QThread()),
    m_cardReader(*new CardReader())
{

}

void Omnikey5427CKReader::Start()
{
    // KCB_DEBUG_ENTRY;

    // Start the card reader thread to detect card insertion and securely read the card number

    connect(&m_cardReader, SIGNAL(DigitsReceived(QString)), this, SLOT(OnDigitsReceived(QString)));

    // KCB_DEBUG_TRACE("Starting card reader");
    m_cardReader.Connect(m_cardReaderThread);
    m_cardReader.moveToThread(&m_cardReaderThread);    
    m_cardReaderThread.start();

    // KCB_DEBUG_EXIT;
}

void Omnikey5427CKReader::OnDigitsReceived(QString digits)
{
    // KCB_DEBUG_ENTRY;

    // KCB_DEBUG_TRACE(digits);
    bool isOk;
    qulonglong value = digits.toInt(&isOk, 10);

    emit __onHIDSwipeCodes(QString::number(value),"");
    // KCB_DEBUG_EXIT;
}
