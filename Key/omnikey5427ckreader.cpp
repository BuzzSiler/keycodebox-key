#include "omnikey5427ckreader.h"

#include <QThread>
#include <QDebug>
#include "cardreader.h"
#include "kcbcommon.h"
#include "kcbutils.h"

Omnikey5427CKReader::Omnikey5427CKReader(QThread* const thread, CardReader* const reader, QObject *parent) :
    QObject(parent),
    m_thread(*thread),
    m_cardReader(*reader)
{
    kcb::Utils::ConstructorMsg(this);
}

void Omnikey5427CKReader::Start()
{
    KCB_DEBUG_ENTRY;

    // Start the card reader thread to detect card insertion and securely read the card number

    connect(&m_cardReader, SIGNAL(DigitsReceived(QString)), this, SLOT(OnDigitsReceived(QString)));

    KCB_DEBUG_TRACE("Starting card reader");
    m_cardReader.Connect(m_thread);
    m_cardReader.moveToThread(&m_thread);    
    m_thread.start();

    KCB_DEBUG_EXIT;
}

void Omnikey5427CKReader::OnDigitsReceived(QString digits)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE(digits);

    QString value = m_cardReader.Convert(digits);

    emit __onHIDSwipeCodes(value,"");
    KCB_DEBUG_EXIT;
}
