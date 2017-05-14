
#include <stdio.h>
#include "securityuser.h"

CSecurityUser::CSecurityUser(QObject *parent) : QObject(parent),
    _userType("User")
{

}

int CSecurityUser::hasLevelAccess(int level, std::string sCode)
{
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
    if(sCode.length() > 0) { // Verify code

    } else {
        // Current access code must work or fail this call.
    }
}

std::string CSecurityUser::usertype()
{
    return _userType;
}


