
#include <stdio.h>
#include "securityuser.h"

CSecurityUser::CSecurityUser(QObject *parent) : QObject(parent),
    _userType("User")
{

}

int CSecurityUser::hasLevelAccess(int level, std::string sCode)
{
    Q_UNUSED(level);
    if(sCode.length() > 0) { // Verify code
        // Check time of day
        return 1;       // Always for test
    } else {
        // Current access code must work or fail this call.
        return 0;
    }
}

bool CSecurityUser::hasSpecialAccess(std::string sAccess, std::string sCode)
{
    Q_UNUSED(sAccess);
    if(sCode.length() > 0) { // Verify code

    } else {
        // Current access code must work or fail this call.
    }
    return false;
}

std::string CSecurityUser::usertype()
{
    return _userType;
}


