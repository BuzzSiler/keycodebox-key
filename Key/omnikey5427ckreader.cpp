#include "omnikey5427ckreader.h"

#include <QThread>
#include <QDebug>
#include "scanner.h"
#include "kcbcommon.h"

Omnikey5427CKReader::Omnikey5427CKReader(QObject *parent) :
    QObject(parent),
    m_scanThread(*new QThread()),
    m_scanner(*new Scanner())
{

}

void Omnikey5427CKReader::Start()
{
    KCB_DEBUG_ENTRY;
    connect(&m_scanner, SIGNAL(DigitsReceived(QString)), this, SLOT(OnDigitsReceived(QString)));

    m_scanner.Connect(m_scanThread);
    m_scanner.moveToThread(&m_scanThread);

    m_scanThread.start();
    KCB_DEBUG_EXIT;
}

void Omnikey5427CKReader::OnDigitsReceived(QString digits)
{
    KCB_DEBUG_ENTRY;

    KCB_DEBUG_TRACE(digits);
    bool isOk;
    qulonglong value = digits.toULongLong(&isOk, 16);

    emit __onHIDSwipeCodes(QString::number(value),"");
    KCB_DEBUG_EXIT;
}
