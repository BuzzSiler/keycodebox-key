#ifndef CSECURITYADMIN_H
#define CSECURITYADMIN_H

#include <QObject>
#include "securityuser.h"

class CSecurityAdmin : public CSecurityUser
{
    Q_OBJECT
private:
    std::string _userType;

public:
    explicit CSecurityAdmin(QObject *parent = 0);

    int hasLevelAccess(int level, std::string sCode="");
    bool hasSpecialAccess(std::string sAccess, std::string sCode="");
    virtual std::string usertype();

signals:

public slots:

};


#endif // CSECURITYADMIN_H
