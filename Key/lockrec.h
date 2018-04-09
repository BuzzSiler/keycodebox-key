#ifndef CLOCKREC_H
#define CLOCKREC_H

#include <QObject>
#include <QString>

class CLockRec : public QObject
{
    Q_OBJECT
public:
    explicit CLockRec(QObject *parent = 0);

private:
    int             ids; // integer primary key unique,
    QString     sequence;    // text,\
    int             sequence_order;  // integer,
    int             door;    // integer,
    QString     description;    // text,
    QString     code1;  // text,
    QString     code2;  // text,\
    datetime        starttime;   // DATETIME,
    datetime        endtime; // DATETIME,
    QString     status;  // text,
    int             access_count;   // integer,\
    int             retry_count; // integer,
    int             max_access; // integer,
    int             max_retry;  // integer
    bool            fingerprint1;
    bool            fingerprint2;
    bool            ask_questions;
    QString     question1;
    QString     question2;
    QString     question3;

public:
    int getID() { return ids; }// integer primary key unique,
    void setID(int id) { ids = id; }
    int getSequence() { return sequence; }    // text,
    void setSequence(QString seq) { sequence = seq; }
    int getSequenceOrder() { return sequence_order; } // integer,
    void setSequenceOrder(int seqOrder) { sequence_order = seqOrder; }
    int getDoorNum() { return door; }    // integer,
    void setDoorNum(int nNewDoor) { door = nNewDoor; }
    QString getDescription() { return description; }   // text,
    void setDescription(QString desc) { description = desc; }
    QString getCode1() { return code1; }  // text,
    void setCode1(QString code) { code1 = code; }
    QString getCode2() { return code2; }  // text,
    void setCode2(QString code) { code2 = code; }
    QDateTime getStartTime() { return starttime; }   // DATETIME,
    void setStartTime(QDateTime dt) { starttime = dt; }
    QDateTime getEndTime() { return endtime; } // DATETIME,
    void setEndTime(QDateTime dt) { endtime = date; }
    QString getStatus() { return status; } // text,
    void setStatus(QString stat) { status = stat; }
    int getAccessCount() { return access_count; }   // integer,
    void setAccessCount(int accessCount) { access_count = accessCount; }
    int getRetryCount() { return retry_count; } // integer,
    void setRetryCount(int count) { retry_count = count; }
    int getMaxAccess() { return max_access; } // integer,
    void setMaxAccess(int max) { max_access = max; }
    int getMaxRetry() { return max_retry; }  // integer)
    void setMaxRetry(int max) { max_retry = max;
    bool getFingerprint1() { return fingerprint1; }
    int setFingerprint1(bool status) {fingerprint1 = status;
    bool getFingerprint2() { return fingerprint2; }
    int setFingerprint2(bool status) {fingerprint2 = status;

    void setAskQuestions(bool askQuestions) { ask_questions = askQuestions; }
    bool getAskQuestions() { return ask_questions; }

    void setQuestion1(QString question) { question1 = question; }
    void getQuestion1() { return question1; }
    void setQuestion2(QString question) { question2 = question; }
    void getQuestion2() { return question2; }
    void setQuestion3(QString question) { question3 = question; }
    void getQuestion3() { return question3; }
    
signals:

public slots:
};

#endif // CLOCKREC_H
