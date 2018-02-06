#ifndef CTBLCODES_H
#define CTBLCODES_H

#include <iostream>
#include <cstdlib>
#include <QtSql>
#include <QtDebug>
#include "lockstate.h"
#include "lockset.h"

/**
 * @brief The CTblCodes class
 * @table "codes" fields
 *      int ids; // integer primary key unique,
 *      std::string sequence;    // text,\
 *      int sequence_order;  // integer,
 *      int door;    // integer,
 *      std::string  description;    // text,
 *      std::string  code1;  // text,
 *      std::string  code2;  // text,\
 *      datetime     starttime;   // DATETIME,
 *      datetime     endtime; // DATETIME,
 *      bool fingerprint1;
 *      bool fingerprint2;
 *      std::string  status;  // text,
 *      int  access_count;   // integer,\
 *      int  retry_count; // integer,
 *      int  max_access; // integer,
 *      int  max_retry;  // integer)
 */
class CTblCodes
{
public:

    // Constants
    const std::string TABLENAME = "codes";

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

    const char *ffingerprint1 = "fingerprint1";
    const char *ffingerprint2 = "fingerprint2";

    const char *fstatus = "status";  // text,
    const char *faccess_count = "access_count";   // integer,
    const char *fretry_count = "retry_count"; // integer,
    const char *fmax_access = "max_access"; // integer,
    const char *fmax_retry = "max_retry";  // integer)

    const char *fmodified = "modified";
    const char *fmarktodelete = "delete";

    const char *flockboxstate = "lockbox_state";

    const char *faccess_type = "access_type";

    // Attributes
    int _lastIDS = -1;

    // Methods

    CTblCodes(QSqlDatabase *db) {
        std::cout << "CTblCodes constructor\n";
        setDatabase(db);
        // make sure the table is created if it does not exist.
        initialize();
    }

    void setDatabase(QSqlDatabase *db) {
        _pDB = db;
    }

    void setLastCodeOne(QString code);

    int getLastSuccessfulIDS() { return _lastIDS; }
    int checkCodeOne(std::string code, bool &bSecondCodeRequired, bool &bFingerprintRequired, int &nDoorNum );
    int checkCodeTwo(std::string code, bool &bFingerprintRequired, bool &bQuestionsRequired, std::string &codeOne, int &nDoorNum, bool &bAskQuestions, QString &question1, QString &question2, QString &question3);

    int addLockCodeClear(int locknum, std::string code1, std::string code2="",
                         QDateTime starttime=QDateTime(QDate(1990,01,01), QTime(0,0,0)),
                         QDateTime endtime=QDateTime(QDate(1990,01,01), QTime(0,0,0)),
                         bool fingerprint1=false, bool fingerprint2=false,
                         bool askQuestions=false, std::string question1="", std::string question2="", std::string question3="",
                         std::string status="",std::string desc="", std::string sequence="", int sequenceNum=0,
                         int maxAccess=0, int maxRetry=0, int accessType=0, int accessCount=0);
    int addLockCode(int locknum, std::string code1, std::string code2="",
                    QDateTime starttime=QDateTime(QDate(1990,01,01), QTime(0,0,0)),
                    QDateTime endtime=QDateTime(QDate(1990,01,01), QTime(0,0,0)),
                    bool fingerprint1=false, bool fingerprint2=false,
                    bool askQuestions=false, std::string question1="", std::string question2="", std::string question3="",
                    std::string status="",std::string desc="", std::string sequence="", int sequenceNum=0,
                    int maxAccess=0, int maxRetry=0, int accessType=0, int accessCount=0);
    void addJSONCodes(const CLockState *prec);
    void addJSONCodes(const CLockSet *pcodeSet);
    void addJSONCodes(std::iostream iofile);
    void addJSONCodes(QString jsonCodes);
    void currentTimeFormat(std::string format, std::string strBuffer, int nExpectedLength);

    bool updateCode(CLockState *prec);
    bool updateCodeSet(CLockSet &codeSet);
    bool updateCodes(QJsonObject &jsonObj);

    bool updateRecord(CLockState &rec);
    bool updateLockboxState(int fids, bool lockstate);

    bool updateAskQuestions(int fids, bool askQuestions);
    bool updateQuestion1(int fids, QString question);
    bool updateQuestion2(int fids, QString question);
    bool updateQuestion3(int fids, QString question);

    bool deleteCode(CLockState &rec);
    bool deleteCode(QString locknum, QString code1, QString code2,
                    QDateTime starttime, QDateTime endtime);

    void selectCodeSet(int &nLockNum, QDateTime start, QDateTime end, CLockSet **pLockSet);
    void selectCodeSet(int ids, CLockSet **pLockSet);

private:
    QSqlDatabase *_pDB;
    QString _sCodeOne;
    QString _sCodeTwo;

    QString _DATENONE = QDateTime(QDate(1990,1,1), QTime(0,0,0)).toString("yyyy-MM-dd HH:mm:ss");

    void createTable();
    bool tableExists();
    bool columnExists(QString column);
    void createColumn(QString column, QString fieldType);

    void initialize();

    bool readTestDefault();
    bool createTestDefault();

    bool isWhiteSpace(const QString &str);
    bool isExpired(int access_type, int access_count, int max_access);
    bool incrementAccessCount(int ids);
};

#endif // CTBLCODES_H
