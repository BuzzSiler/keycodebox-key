#include "codeelement.h"
#include "kcbcommon.h"

void Code::print() const
{
    KCB_DEBUG_TRACE("Locks: " << m_locks.toLocal8Bit());
    KCB_DEBUG_TRACE("Code1: " << m_code1.toLocal8Bit());
    KCB_DEBUG_TRACE("Code2: " << m_code2.toLocal8Bit());
    KCB_DEBUG_TRACE("Username: " << m_username.toLocal8Bit());
    KCB_DEBUG_TRACE("Question1: " << m_question1.toLocal8Bit());
    KCB_DEBUG_TRACE("Question2: " << m_question2.toLocal8Bit());
    KCB_DEBUG_TRACE("Question3: " << m_question3.toLocal8Bit());
    KCB_DEBUG_TRACE("StartTime: " << m_starttime.toLocal8Bit());
    KCB_DEBUG_TRACE("EndTime: " << m_endtime.toLocal8Bit());
    KCB_DEBUG_TRACE("AccessType: " << m_accesstype);
}
