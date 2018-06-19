#ifndef CLOCKHISTORYREC_H
#define CLOCKHISTORYREC_H

#include <QObject>
#include <QString>
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
        QString     _userNotificationEmail;
        bool        _userNotificationSent;
        QString     _lockNums;
        QString     _access_selection;

    public:
        QDateTime getAccessTime() { return _access_time; }
        void setAccessTime(QDateTime dt) { _access_time = dt; }
        bool getAdminNotificationSent() { return _adminNotificationSent; }
        void setAdminNotificationSent(bool bSent) { _adminNotificationSent = bSent; }
        QString getUserNotificationEmail() { return _userNotificationEmail; }
        void setUserNotificationEmail(QString email) { _userNotificationEmail = email; }
        bool getUserNotificationSent() { return _userNotificationSent; }
        void setUserNotificationSent(bool bSent) { _userNotificationSent = bSent; }

        QString getDescription() { return CLockState::getDescription(); }   // text,
        void setDescription(QString desc) { CLockState::setDescription(desc); }

        QString getQuestion1() { return CLockState::getQuestion1(); }
        QString getQuestion2() { return CLockState::getQuestion2(); }
        QString getQuestion3() { return CLockState::getQuestion3(); }

        virtual QJsonObject &jsonRecord(QJsonObject &json);
        virtual QString jsonRecordAsString();

        virtual bool setFromJsonObject(QJsonObject jsonObj);
        virtual bool setFromJsonString(QString strJson);

        void setFromLockState(CLockState &lockState);
        void setLockNums(QString locknums) { _lockNums = locknums; };
        QString getLockNums() { return _lockNums; };

        void setAccessSelection(QString accessSelection) { _access_selection = accessSelection; }
        QString getAccessSelection() { return _access_selection; }


};

#endif // CLOCKHISTORYREC_H
