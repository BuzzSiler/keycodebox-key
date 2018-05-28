#include "kcbapplication.h"

namespace kcb
{
    ACCESS_SELECTION Application::m_selection = ACCESS_NONE;

    void Application::setAccessSelection(ACCESS_SELECTION selection)
    {
        m_selection = selection;
    }

    ACCESS_SELECTION Application::getAccessSelection()
    {
        ACCESS_SELECTION temp;

        temp = m_selection;
        clearAccessSelection();

        return temp;
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
