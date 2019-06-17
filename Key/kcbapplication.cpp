#include "kcbapplication.h"

#include <QString>
#include <QDebug>
#include <QObject>

#include "kcbcommon.h"

static QString last_transaction_username;
static QString last_transaction_locks;
static QString last_transaction_datetime;

namespace kcb
{
    Application::ACCESS_SELECTION Application::m_selection = ACCESS_NONE;

    void Application::setTakeAccessSelection()
    {
        m_selection = ACCESS_TAKE;
    }

    void Application::setReturnAccessSelection()
    {
        m_selection = ACCESS_RETURN;
    }

    QString Application::getAccessSelection()
    {
        QString selection = "";

        switch (m_selection)
        {
            case ACCESS_TAKE:
                selection = QObject::tr("Take");
                break;

            case ACCESS_RETURN:
                selection = QObject::tr("Return");
                break;

            case ACCESS_NONE:
            default:
                selection = QObject::tr("None");
                break;
        }

        clearAccessSelection();

        // KCB_DEBUG_TRACE("Getting Access Selection" << selection);

        return selection;
    }

    void Application::clearAccessSelection()
    {
        // KCB_DEBUG_ENTRY;
        m_selection = ACCESS_NONE;
        // KCB_DEBUG_EXIT;
    }

    bool Application::isTakeSelection()
    {
        // KCB_DEBUG_ENTRY;
        bool result = m_selection == ACCESS_TAKE;
        // KCB_DEBUG_EXIT;
        return result;
    }

    bool Application::isReturnSelection()
    {
        // KCB_DEBUG_ENTRY;
        bool result = m_selection == ACCESS_RETURN;
        // KCB_DEBUG_EXIT;
        return result;
    }

    void Application::SetLastTransaction(QString username, QString locks, QString datetime)
    {
        if (username.isEmpty())
        {
            username = "unknown";
        }
        last_transaction_username = username;
        last_transaction_locks = locks;
        last_transaction_datetime = datetime;
    }

    QString Application::GetLastTransactionAsString()
    {
        return QString("%1_%2_%3").arg(last_transaction_username).arg(last_transaction_locks).arg(last_transaction_datetime);
    }

}
