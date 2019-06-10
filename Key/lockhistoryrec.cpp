#include <QString>
#include <QDebug>

#include "lockhistoryrec.h"

CLockHistoryRec::CLockHistoryRec(CLockState *parent) : CLockState(parent)
{

}


void CLockHistoryRec::setFromLockState(CLockState &newLockState)
{
    _ids = newLockState.getID();            // Record id // integer primary key unique, (if -1 then this is a new record)
    _sequence = newLockState.getSequence();    // Sequence // text,
    _sequence_order = newLockState.getSequenceOrder();
    _lock_nums = newLockState.getLockNums();
    _description = newLockState.getDescription();
    _code1 = newLockState.getCode1();
    _code2 = newLockState.getCode2();
    _access_selection = "None";
    _starttime = newLockState.getStartTime();
    _endtime = newLockState.getEndTime();
    _status = newLockState.getStatus();
    _access_count = newLockState.getAccessCount();
    _retry_count = newLockState.getRetryCount();
    _max_access = newLockState.getMaxAccess();
    _max_retry = newLockState.getMaxRetry();
    _bModified = newLockState.isModified();
    _bMarkForDeletion = newLockState.isMarkedForDeletion();

    _access_time = QDateTime::currentDateTime();
    _adminNotificationSent = false;
    _userNotificationEmail = "";
    _userNotificationSent = false;

    _question1 = newLockState.getQuestion1();
    _question2 = newLockState.getQuestion2();
    _question3 = newLockState.getQuestion3();
}

