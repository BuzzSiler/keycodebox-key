#ifndef CSECURITYUSER_H
#define CSECURITYUSER_H

#include <QObject>
#include <stdio.h>

class CSecurityUser : public QObject
{
    Q_OBJECT
private:
    std::string _userType;

public:
    explicit CSecurityUser(QObject *parent = 0);

    virtual int hasLevelAccess(int level, std::string sCode="");
    virtual bool hasSpecialAccess(std::string sAccess, std::string sCode="");
    virtual std::string usertype();

signals:

public slots:

};

#endif // CSECURITYUSER_H
