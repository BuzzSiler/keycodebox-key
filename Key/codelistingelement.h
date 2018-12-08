#ifndef CODELISTINGELEMENT_H
#define CODELISTINGELEMENT_H

#include <QList>

class Code;

class CodeListing
{
    public:
        typedef QList<Code*>::Iterator Iterator;

        void addCode(Code* code);
        void print();
        void setEncrypted(bool value) {m_encrypted = value;}
        bool encrypted() {return m_encrypted;}

        Iterator begin() { return m_codes.begin(); }
        Iterator end() { return m_codes.end(); }
        
    private:
        QList<Code*> m_codes;
        bool m_encrypted;
};

#endif

//-------------------------------------------------------------------------------------------------
// EOF