#include "securityadmin.h"

CSecurityAdmin::CSecurityAdmin(QObject *parent) : CSecurityUser(parent),
    _userType("Admin")
{

}

int CSecurityAdmin::hasLevelAccess(int level, std::string sCode)
{
    Q_UNUSED(level);
    Q_UNUSED(sCode);
    return 1;
}

bool CSecurityAdmin::hasSpecialAccess(std::string sAccess, std::string sCode)
{
    Q_UNUSED(sAccess);
    Q_UNUSED(sCode);
    return true;
}

std::string CSecurityAdmin::usertype()
{
    return _userType;
}

