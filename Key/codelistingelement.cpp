#include "codelistingelement.h"
#include "codeelement.h"
#include "kcbcommon.h"

void CodeListing::addCode(Code* code)
{
    m_codes.append(code);
}

void CodeListing::print()
{
    // KCB_DEBUG_TRACE("Encrypted" << m_encrypted);
    foreach(Code* code, m_codes)
    {
        code->print();
    }
}
