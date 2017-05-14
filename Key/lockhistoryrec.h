#ifndef CLOCKHISTORYREC_H
#define CLOCKHISTORYREC_H

#include <QObject>
#include "lockstate.h"


class CLockHistoryRec : public CLockState
{
    Q_OBJECT
public:
    explicit CLockHistoryRec(CLockState *parent = 0);

protected:
    const char *faccess_time = "access_time";   // integer,
    const char *fadmin_notification_sent = "admin_notification_sent"; // integer,
    const char *fuser_notification_email = "user_notification_email"; // integer,
    const char *fuser_notification_sent = "user_notification_sent";  // integer)

private:

    QDateTime   _access_time;
    bool        _adminNotificationSent;
    std::string _userNotificationEmail;
    bool        _userNotificationSent;

public:
    QDateTime getAccessTime() { return _access_time; }
    void setAccessTime(QDateTime dt) { _access_time = dt; }
    bool getAdminNotificationSent() { return _adminNotificationSent; }
    void setAdminNotificationSent(bool bSent) { _adminNotificationSent = bSent; }
    std::string getUserNotificationEmail() { return _userNotificationEmail; }
    void setUserNotificationEmail(std::string email) { _userNotificationEmail = email; }
    bool getUserNotificationSent() { return _userNotificationSent; }
    void setUserNotificationSent(bool bSent) { _userNotificationSent = bSent; }

    std::string getDescription() { return CLockState::getDescription(); }   // text,
    void setDescription(std::string desc) { CLockState::setDescription(desc); }

    std::string getQuestion1() { return CLockState::getQuestion1(); }
    std::string getQuestion2() { return CLockState::getQuestion2(); }
    std::string getQuestion3() { return CLockState::getQuestion3(); }

    virtual QJsonObject &jsonRecord(QJsonObject &json);
    virtual QString jsonRecordAsString();

    virtual bool setFromJsonObject(QJsonObject jsonObj);
    virtual bool setFromJsonString(std::string strJson);

    bool setFromLockState(CLockState &lockState);


signals:

public slots:

};



#endif // CLOCKHISTORYREC_H
