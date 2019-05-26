#ifndef CLOCKSTATE_H
#define CLOCKSTATE_H

//#include "stdint-gcc.h"
#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>
#include <QStringList>

#include "kcbcommon.h"

class CLockState : public QObject
{
    Q_OBJECT

    public:
        explicit CLockState(QObject *parent = 0);

        virtual int getID() { return _ids; }
        virtual void setID(int id) { _ids = id; }
        virtual QString getSequence() { return _sequence; }    
        virtual void setSequence(QString seq) { _sequence = seq; }
        virtual int getSequenceOrder() { return _sequence_order; } 
        virtual void setSequenceOrder(int seqOrder) { _sequence_order = seqOrder; }
        virtual QString getLockNums() { return _lock_nums; }    
        virtual void setLockNums(QString locks) { _lock_nums = locks; }
        virtual QString getDescription() { return _description; }   
        virtual void setDescription(QString desc) { _description = desc; }
        virtual QString getCode1() { return _code1; }  
        virtual void setCode1(QString code) { _code1 = code; }
        virtual QString getCode2() { return _code2; }  
        virtual void setCode2(QString code) { _code2 = code; }
        virtual QDateTime getStartTime() { return _starttime; }   
        virtual void setStartTime(QDateTime dt) { _starttime = dt; }
        virtual QDateTime getEndTime() { return _endtime; } 
        virtual void setEndTime(QDateTime dt) { _endtime = dt; }
        virtual QString getStatus() { return _status; } 
        virtual void setStatus(QString stat) { _status = stat; }
        virtual int getAccessCount() { return _access_count; }   
        virtual void setAccessCount(int accessCount) { _access_count = accessCount; }
        virtual int getRetryCount() { return _retry_count; } 
        virtual void setRetryCount(int count) { _retry_count = count; }
        virtual int getMaxAccess() { return _max_access; } 
        virtual void setMaxAccess(int max) { _max_access = max; }
        virtual int getMaxRetry() { return _max_retry; }  
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

        virtual void setAutoCode(bool autocode) { _autocode = autocode; }
        virtual bool getAutoCode() { return _autocode; }


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
        int         _ids;
        QString     _sequence;
        int         _sequence_order;
        QString     _lock_nums;
        QString     _description;
        QString     _code1;
        QString     _code2;
        QDateTime   _starttime;
        QDateTime   _endtime;
        QString     _status;
        int         _access_count;
        int         _retry_count;
        int         _max_access;
        int         _max_retry;

        bool        _bIsNew = false;
        bool        _bModified = false;
        bool        _bMarkForDeletion = false;
        bool        _bMarkForReset = false;
        bool        _bFingerprint1;
        bool        _bFingerprint2;

        bool        _bAskQuestions;
        QString     _question1;
        QString     _question2;
        QString     _question3;
        int         _access_type;
        bool        _autocode;
};

#endif // CLOCKSTATE_H
