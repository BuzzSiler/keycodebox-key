#include "kcbapplication.h"
#include <QString>
#include <QDebug>

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
                selection = "Take";
                break;

            case ACCESS_RETURN:
                selection = "Return";
                break;

            case ACCESS_NONE:
            default:
                selection = "None";
                break;
        }

        clearAccessSelection();

        qDebug() << "Getting Access Selection" << selection;

        return selection;
    }

    void Application::clearAccessSelection()
    {
        m_selection = ACCESS_NONE;
    }

    bool Application::isTakeSelection()
    {
        return m_selection == ACCESS_TAKE;
    }

    bool Application::isReturnSelection()
    {
        return m_selection == ACCESS_RETURN;
    }

}
