#ifndef CLOCKREC_H
#define CLOCKREC_H

#include <QObject>

class CLockRec : public QObject
{
    Q_OBJECT
public:
    explicit CLockRec(QObject *parent = 0);

private:
    int             ids; // integer primary key unique,
    std::string     sequence;    // text,\
    int             sequence_order;  // integer,
    int             door;    // integer,
    std::string     description;    // text,
    std::string     code1;  // text,
    std::string     code2;  // text,\
    datetime        starttime;   // DATETIME,
    datetime        endtime; // DATETIME,
    std::string     status;  // text,
    int             access_count;   // integer,\
    int             retry_count; // integer,
    int             max_access; // integer,
    int             max_retry;  // integer
    bool            fingerprint1;
    bool            fingerprint2;
    bool            ask_questions;
    std::string     question1;
    std::string     question2;
    std::string     question3;

public:
    int getID() { return ids; }// integer primary key unique,
    void setID(int id) { ids = id; }
    int getSequence() { return sequence; }    // text,\
    void setSequence(std::string seq) { sequence = seq; }
    int getSequenceOrder() { return sequence_order; } // integer,
    void setSequenceOrder(int seqOrder) { sequence_order = seqOrder; }
    int getDoorNum() { return door; }    // integer,
    void setDoorNum(int nNewDoor) { door = nNewDoor; }
    std::string getDescription() { return description; }   // text,
    void setDescription(std::string desc) { description = desc; }
    std::string getCode1() { return code1; }  // text,
    void setCode1(std::string code) { code1 = code; }
    std::string getCode2() { return code2; }  // text,\
    void setCode2(std::string code) { code2 = code; }
    QDateTime getStartTime() { return starttime; }   // DATETIME,
    void setStartTime(QDateTime dt) { starttime = dt; }
    QDateTime getEndTime() { return endtime; } // DATETIME,
    void setEndTime(QDateTime dt) { endtime = date; }
    std::string getStatus() { return status; } // text,
    void setStatus(std::string stat) { status = stat; }
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

    void setQuestion1(std::string question) { question1 = question; }
    void getQuestion1() { return question1; }
    void setQuestion2(std::string question) { question2 = question; }
    void getQuestion2() { return question2; }
    void setQuestion3(std::string question) { question3 = question; }
    void getQuestion3() { return question3; }
    
signals:

public slots:
};

#endif // CLOCKREC_H
