#include "securityadmin.h"

CSecurityAdmin::CSecurityAdmin(QObject *parent) : CSecurityUser(parent),
    _userType("Admin")
{

}

int CSecurityAdmin::hasLevelAccess(int level, std::string sCode)
{
    return 1;
}

bool CSecurityAdmin::hasSpecialAccess(std::string sAccess, std::string sCode)
{
    return true;
}

std::string CSecurityAdmin::usertype()
{
    return _userType;
}

