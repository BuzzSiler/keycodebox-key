#ifndef CLOCKSTATE_H
#define CLOCKSTATE_H

#include "stdint-gcc.h"
#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <string>

/**
 * @brief The CLockState class
 * This holds essentially an in memory representation of the tblcodes + tblhistory record.
 * May be in table or new.
 */
class CLockState : public QObject
{
    Q_OBJECT

protected:
    const char *datetimeFormat = "yyyy-MM-dd HH:mm:ss";
    const char *timeFormatShort = "HH:mm";
    const char *timeFormat = "HH:mm:ss";

    const char *fids = "ids";            // Record id // integer primary key unique, (if -1 then this is a new record)
    const char *fsequence = "sequence";    // Sequence // text,
    const char *fsequence_order = "sequence_order";  // integer,
    const char *flocknum = "locknum";    // integer,
    const char *fdescription = "description";    // text,
    const char *fcode1 = "code1";  // text,
    const char *fcode2 = "code2";  // text,
    const char *fstarttime = "starttime";   // DATETIME,
    const char *fendtime = "endtime"; // DATETIME,
    const char *fstatus = "status";  // text,
    const char *faccess_count = "access_count";   // integer,
    const char *fretry_count = "retry_count"; // integer,
    const char *fmax_access = "max_access"; // integer,
    const char *fmax_retry = "max_retry";  // integer)

    const char *fmodified = "modified";
    const char *fmarktodelete = "delete";

    const char *ffingerprint1 = "fingerprint1";
    const char *ffingerprint2 = "fingerprint2";

    const char *fask_questions = "ask_questions";
    const char *fquestion1 = "question1";
    const char *fquestion2 = "question2";
    const char *fquestion3 = "question3";

    const char *flockboxstate = "lockbox_state";
public:
    explicit CLockState(QObject *parent = 0);
    CLockState(const CLockState&newLockState) {
        _ids = newLockState._ids;            // Record id // integer primary key unique, (if -1 then this is a new record)
        _sequence = newLockState._sequence;    // Sequence // text,
        _sequence_order = newLockState._sequence_order;  // integer,
        _lock_num = newLockState._lock_num;    // integer,
        _description = newLockState._description;    // text,
        _code1 = newLockState._code1;  // text,
        _code2 = newLockState._code2;  // text,
        _starttime = newLockState._starttime;   // DATETIME,
        _endtime = newLockState._endtime; // DATETIME,
        _status = newLockState._status;  // text,
        _access_count = newLockState._access_count;   // integer,
        _retry_count = newLockState._retry_count; // integer,
        _max_access = newLockState._max_access; // integer,
        _max_retry = newLockState._max_retry;  // integer)
        _bModified = newLockState._bModified;
        _bMarkForDeletion = newLockState._bMarkForDeletion;
	_bFingerprint1 = newLockState._bFingerprint1;
	_bFingerprint2 = newLockState._bFingerprint2;
	_bAskQuestions = newLockState._bAskQuestions; //bool
	_question1 = newLockState._question1; //text
    	_question2 = newLockState._question2; //text
    	_question3 = newLockState._question3; //text
	_bLockboxState = newLockState._bLockboxState;
    }

protected:
    int             _ids;            // Record id // integer primary key unique, (if -1 then this is a new record)
    std::string     _sequence;    // Sequence // text,
    int             _sequence_order;  // integer,
    uint16_t        _lock_num;    // integer,
    std::string     _description;    // text,
    std::string     _code1;  // text,
    std::string     _code2;  // text,
    QDateTime       _starttime;   // DATETIME,
    QDateTime       _endtime; // DATETIME,
    std::string     _status;  // text,
    int             _access_count;   // integer,
    int             _retry_count; // integer,
    int             _max_access; // integer,
    int             _max_retry;  // integer)

    bool            _bIsNew = false;
    bool            _bModified = false;
    bool            _bMarkForDeletion = false;
    bool            _bFingerprint1;
    bool            _bFingerprint2;

    bool            _bAskQuestions;
    std::string     _question1;
    std::string     _question2;
    std::string     _question3;

    bool            _bLockboxState;
public:
    virtual int getID() { return _ids; }// integer primary key unique,
    virtual void setID(int id) { _ids = id; }
    virtual std::string getSequence() { return _sequence; }    // text,
    virtual void setSequence(std::string seq) { _sequence = seq; }
    virtual int getSequenceOrder() { return _sequence_order; } // integer,
    virtual void setSequenceOrder(int seqOrder) { _sequence_order = seqOrder; }
    virtual int getLockNum() { return _lock_num; }    // integer,
    virtual void setLockNum(int nNewLock) { _lock_num = nNewLock; }
    virtual std::string getDescription() { return _description; }   // text,
    virtual void setDescription(std::string desc) { _description = desc; }
    virtual std::string getCode1() { return _code1; }  // text,
    virtual void setCode1(std::string code) { _code1 = code; }
    virtual std::string getCode2() { return _code2; }  // text,
    virtual void setCode2(std::string code) { _code2 = code; }
    virtual QDateTime getStartTime() { return _starttime; }   // DATETIME,
    virtual void setStartTime(QDateTime dt) { _starttime = dt; }
    virtual QDateTime getEndTime() { return _endtime; } // DATETIME,
    virtual void setEndTime(QDateTime dt) { _endtime = dt; }
    virtual std::string getStatus() { return _status; } // text,
    virtual void setStatus(std::string stat) { _status = stat; }
    virtual int getAccessCount() { return _access_count; }   // integer,
    virtual void setAccessCount(int accessCount) { _access_count = accessCount; }
    virtual int getRetryCount() { return _retry_count; } // integer,
    virtual void setRetryCount(int count) { _retry_count = count; }
    virtual int getMaxAccess() { return _max_access; } // integer,
    virtual void setMaxAccess(int max) { _max_access = max; }
    virtual int getMaxRetry() { return _max_retry; }  // integer)
    virtual void setMaxRetry(int max) { _max_retry = max; }

    virtual bool isNew() { return _bIsNew; }
    virtual bool isModified() { return _bModified; }
    virtual bool isMarkedForDeletion() { return _bMarkForDeletion; }

    virtual void setFingerprint1() { _bFingerprint1 = true; };
    virtual void clearFingerprint1() { _bFingerprint1 = false; };
    virtual bool getFingerprint1() { return _bFingerprint1; };

    virtual void setFingerprint2() { _bFingerprint2 = true; };
    virtual void clearFingerprint2() { _bFingerprint2 = false; };
    virtual bool getFingerprint2() { return _bFingerprint2; };

    virtual void setAskQuestions(bool askQuestions) { _bAskQuestions = askQuestions; }
    virtual bool getAskQuestions() { return _bAskQuestions; }

    virtual void setQuestion1(std::string question) { _question1 = question; }
    virtual std::string getQuestion1() { return _question1; }
    
    virtual void setQuestion2(std::string question) { _question2 = question; }
    virtual std::string getQuestion2() { return _question2; }
    
    virtual void setQuestion3(std::string question) { _question3 = question; }
    virtual std::string getQuestion3() { return _question3; }

    virtual void setLockboxState(bool bState) { _bLockboxState = bState; }
    virtual bool getLockboxState() { return _bLockboxState; }
    
    virtual void setNew() { _bIsNew = true; }
    virtual void clearIsNew() { _bIsNew = false; }
    virtual void setModified() { _bModified = true; }
    virtual void clearModified() { _bModified = false; }
    virtual void setMarkForDeletion() { _bMarkForDeletion = true; }
    virtual void clearMarkForDeletion() { _bMarkForDeletion = false; }
    
    virtual QJsonObject &jsonRecord(QJsonObject &json);
    virtual QString jsonRecordAsString();

    virtual bool setFromJsonObject(QJsonObject jsonObj);
    virtual bool setFromJsonString(std::string strJson);

    
signals:

public slots:
};




#endif // CLOCKSTATE_H
