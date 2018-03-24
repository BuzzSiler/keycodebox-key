#ifndef CLOCKSTATE_H
#define CLOCKSTATE_H

#include "stdint-gcc.h"
#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>
#include <QStringList>


/**
 * @brief The CLockState class
 * This holds essentially an in memory representation of the tblcodes + tblhistory record.
 * May be in table or new.
 */
class CLockState : public QObject
{
    Q_OBJECT


public:
    explicit CLockState(QObject *parent = 0);

    virtual int getID() { return _ids; }// integer primary key unique,
    virtual void setID(int id) { _ids = id; }
    virtual QString getSequence() { return _sequence; }    // text,
    virtual void setSequence(QString seq) { _sequence = seq; }
    virtual int getSequenceOrder() { return _sequence_order; } // integer,
    virtual void setSequenceOrder(int seqOrder) { _sequence_order = seqOrder; }
    virtual int getLockNum() { return _lock_num; }    // integer,
    virtual void setLockNum(int nNewLock) { _lock_num = nNewLock; }
    virtual QString getLockNums() { return _lock_nums; }    // text,
    virtual void setLockNums(QString locks) { _lock_nums = locks; }
    virtual QString getDescription() { return _description; }   // text,
    virtual void setDescription(QString desc) { _description = desc; }
    virtual QString getCode1() { return _code1; }  // text,
    virtual void setCode1(QString code) { _code1 = code; }
    virtual QString getCode2() { return _code2; }  // text,
    virtual void setCode2(QString code) { _code2 = code; }
    virtual QDateTime getStartTime() { return _starttime; }   // DATETIME,
    virtual void setStartTime(QDateTime dt) { _starttime = dt; }
    virtual QDateTime getEndTime() { return _endtime; } // DATETIME,
    virtual void setEndTime(QDateTime dt) { _endtime = dt; }
    virtual QString getStatus() { return _status; } // text,
    virtual void setStatus(QString stat) { _status = stat; }
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
    virtual bool isMarkedForReset() { return _bMarkForReset; }

    virtual void setFingerprint1() { _bFingerprint1 = true; };
    virtual void clearFingerprint1() { _bFingerprint1 = false; };
    virtual bool getFingerprint1() { return _bFingerprint1; };

    virtual void setFingerprint2() { _bFingerprint2 = true; };
    virtual void clearFingerprint2() { _bFingerprint2 = false; };
    virtual bool getFingerprint2() { return _bFingerprint2; };

    virtual void setAskQuestions(bool askQuestions) { _bAskQuestions = askQuestions; }
    virtual bool getAskQuestions() { return _bAskQuestions; }

    virtual void setQuestion1(QString question) { _question1 = question; }
    virtual QString getQuestion1() { return _question1; }
    
    virtual void setQuestion2(QString question) { _question2 = question; }
    virtual QString getQuestion2() { return _question2; }
    
    virtual void setQuestion3(QString question) { _question3 = question; }
    virtual QString getQuestion3() { return _question3; }

    virtual void setAccessType(int access_type) { _access_type = access_type; }
    virtual int getAccessType() { return _access_type; }
    
    virtual void setNew() { _bIsNew = true; }
    virtual void clearIsNew() { _bIsNew = false; }
    virtual void setModified() { _bModified = true; }
    virtual void clearModified() { _bModified = false; }
    virtual void setMarkForDeletion() { _bMarkForDeletion = true; }
    virtual void clearMarkForDeletion() { _bMarkForDeletion = false; }

    virtual void setMarkForReset() { _bMarkForReset = true; }
    virtual void clearMarkForReset() { _bMarkForReset = false; }
    
    virtual QJsonObject &jsonRecord(QJsonObject &json);
    virtual QString jsonRecordAsString();

    virtual bool setFromJsonObject(QJsonObject jsonObj);
    virtual bool setFromJsonString(QString strJson);

    virtual bool isActive() { return _access_count < _max_access; }
    virtual int getRemainingUses() { return _max_access - _access_count; }

    void show();

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
    const char *faccess_type = "access_type";

    int             _ids;            // Record id // integer primary key unique, (if -1 then this is a new record)
    QString      _sequence;    // Sequence // text,
    int          _sequence_order;  // integer,
    uint16_t     _lock_num;    // integer,
    QString      _lock_nums;
    QString      _description;    // text,
    QString      _code1;  // text,
    QString      _code2;  // text,
    QDateTime    _starttime;   // DATETIME,
    QDateTime    _endtime; // DATETIME,
    QString      _status;  // text,
    int             _access_count;   // integer,
    int             _retry_count; // integer,
    int             _max_access; // integer,
    int             _max_retry;  // integer)

    bool            _bIsNew = false;
    bool            _bModified = false;
    bool            _bMarkForDeletion = false;
    bool            _bMarkForReset = false;
    bool            _bFingerprint1;
    bool            _bFingerprint2;

    bool            _bAskQuestions;
    QString     _question1;
    QString     _question2;
    QString     _question3;
    int             _access_type;


};

#endif // CLOCKSTATE_H
