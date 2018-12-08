#ifndef XMLCODELISTINGREADER_H
#define XMLCODELISTINGREADER_H

#include <QXmlStreamReader>
#include <QString>

class CodeListing;
class Code;
class QIODevice;

class XmlCodeListingReader
{
public:
    XmlCodeListingReader(CodeListing* codelisting);
    bool read(QIODevice *device);
    QString errorString() const;
    bool encrypted() const;
private:
    QXmlStreamReader m_reader;
    CodeListing*     m_codelisting;

    void readCodeListing();
    void readEncryptedAttribute();
    void readCode();
    void readLocks(Code *code);
    void readCode1(Code *code);
    void readCode2(Code *code);
    void readUsername(Code *code);
    void readQuestion1(Code *code);
    void readQuestion2(Code *code);
    void readQuestion3(Code *code);
    void readStartTime(Code *code);
    void readEndTime(Code *code);
    void readAccessType(Code *code);
};

#endif

//-------------------------------------------------------------------------------------------------
// EOF