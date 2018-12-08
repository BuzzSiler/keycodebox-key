#ifndef CODEELEMENT_H
#define CODEELEMENT_H

#include <QString>

class Code
{
    public:
        QString locks() const {return m_locks;}
        void setLocks(const QString& value) {m_locks = value;}
        QString code1() const {return m_code1;}
        void setCode1(const QString& value) {m_code1 = value;}
        QString code2() const {return m_code2;}
        void setCode2(const QString& value) {m_code2 = value;}
        QString username() const {return m_username;}
        void setUsername(const QString& value) {m_username = value;}
        QString question1() const {return m_question1;}
        void setQuestion1(const QString& value) {m_question1 = value;}
        QString question2() const {return m_question2;}
        void setQuestion2(const QString& value) {m_question2 = value;}
        QString question3() const {return m_question3;}
        void setQuestion3(const QString& value) {m_question3 = value;}
        QString starttime() const {return m_starttime;}
        void setStartTime(const QString& value) {m_starttime = value;}
        QString endtime() const {return m_endtime;}        
        void setEndTime(const QString& value) {m_endtime = value;}
        int accesstype() const {return m_accesstype;}
        void setAccessType(const int& value) {m_accesstype = value;}
        
        void print() const;
        
    private:
        QString m_locks;
        QString m_code1;
        QString m_code2;
        QString m_username;
        QString m_question1;
        QString m_question2;
        QString m_question3;
        QString m_starttime;
        QString m_endtime;
        int m_accesstype;
};

#endif

//-------------------------------------------------------------------------------------------------
// EOF