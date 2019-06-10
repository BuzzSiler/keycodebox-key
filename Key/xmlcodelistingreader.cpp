#include "xmlcodelistingreader.h"
#include "codelistingelement.h"
#include "codeelement.h"
#include "kcbcommon.h"
#include "codeimportexportutil.h"

XmlCodeListingReader::XmlCodeListingReader(CodeListing& codelisting)
    :
    m_codelisting(codelisting)
{
}

bool XmlCodeListingReader::read(QIODevice& device)
{
    // KCB_DEBUG_ENTRY;
    m_reader.setDevice(&device);

    if (m_reader.readNextStartElement()) 
    {
        if (m_reader.name() == "codeListing")
        {
            readEncryptedAttribute();
            readCodeListing();
        }
        else
        {
            m_reader.raiseError(QObject::tr("Not a codelisting file"));
        }
    }
    // KCB_DEBUG_EXIT;
    return !m_reader.error();
}

QString XmlCodeListingReader::errorString() const
{
    return m_reader.errorString();
}

void XmlCodeListingReader::readCodeListing()
{
    // KCB_DEBUG_ENTRY;
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "code")
        {
            readCode();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readEncryptedAttribute()
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.name() == "codeListing" && m_reader.attributes().hasAttribute("security"));

    QString security = "not specified";
    if (m_reader.attributes().hasAttribute("security"))
    {
        security = m_reader.attributes().value("security").toString();
    }
    bool encrypted = CodeImportExportUtil::StringToSecurity(security) == CodeImportExportUtil::ENCRYPTED_SECURITY ? true : false;
    m_codelisting.setEncrypted(encrypted);
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readCode()
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "code");

    Code* code = new Code;

    while (m_reader.readNextStartElement()) 
    {
        if (m_reader.name() == "locks")
        {
            readLocks(code);
        }
        else if (m_reader.name() == "code1")
        {            
            readCode1(code);
        }
        else if (m_reader.name() == "code2")
        {            
            readCode2(code);
        }
        else if (m_reader.name() == "username")
        {            
            readUsername(code);
        }
        else if (m_reader.name() == "question1")
        {            
            readQuestion1(code);
        }
        else if (m_reader.name() == "question2")
        {            
            readQuestion2(code);
        }
        else if (m_reader.name() == "question3")
        {            
            readQuestion3(code);
        }
        else if (m_reader.name() == "starttime" || m_reader.name() == "startDT")
        {            
            readStartTime(code);
        }
        else if (m_reader.name() == "endtime" || m_reader.name() == "endDT")
        {            
            readEndTime(code);
        }
        else if (m_reader.name() == "access_type")
        {            
            readAccessType(code);
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }

    m_codelisting.addCode(code);
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readLocks(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "locks");

    QString value = m_reader.readElementText();
    code->setLocks(value);
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readCode1(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "code1");

    QString value = m_reader.readElementText();
    code->setCode1(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readCode2(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "code2");

    QString value = m_reader.readElementText();
    code->setCode2(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readUsername(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "username");

    QString value = m_reader.readElementText();
    code->setUsername(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readQuestion1(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "question1");

    QString value = m_reader.readElementText();
    code->setQuestion1(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readQuestion2(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "question2");

    QString value = m_reader.readElementText();
    code->setQuestion2(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readQuestion3(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "question3");

    QString value = m_reader.readElementText();
    code->setQuestion3(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readStartTime(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && (m_reader.name() == "starttime" || m_reader.name() == "startDT"));

    QString value = m_reader.readElementText();
    code->setStartTime(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readEndTime(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && (m_reader.name() == "endtime" || m_reader.name() == "endDT"));

    QString value = m_reader.readElementText();
    code->setEndTime(value);    
    // KCB_DEBUG_EXIT;
}

void XmlCodeListingReader::readAccessType(Code *code)
{
    // KCB_DEBUG_ENTRY;
    Q_ASSERT(m_reader.isStartElement() && m_reader.name() == "access_type");

    QString value = m_reader.readElementText();
    code->setAccessType(value.toInt());    
    // KCB_DEBUG_EXIT;
}

//-------------------------------------------------------------------------------------------------
// EOF
