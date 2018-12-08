#include "codeexporter.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QJsonObject>
#include <QPair>
#include <QJsonDocument>

#include "kcbcommon.h"


int CodeExporter::StringToFormat(const QString& string)
{
    QString string_lower = string.toLower();

    if (string_lower == "xml")
    {
        return XML_FORMAT;
    }
    else if (string_lower == "json")
    {
        return JSON_FORMAT;
    }
    else if (string_lower == "csv")
    {
        return CSV_FORMAT;
    }
    else if (string_lower == "sql")
    {
        return SQL_FORMAT;
    }
    else
    {
        KCB_DEBUG_TRACE("Invalid format requested" << string);
        return INVALID_FORMAT;
    }
}

QString CodeExporter::FormatToString(const int& format)
{
    QString string = "";

    switch (format)
    {
        case XML_FORMAT:
            string = "xml";
            break;

        case JSON_FORMAT:
            string = "json";
            break;

        case CSV_FORMAT:
            string = "csv";
            break;

        case SQL_FORMAT:
            string = "sql";
            break;

        case INVALID_FORMAT:
        default:
            KCB_DEBUG_TRACE("Invalid format requested");
            break;            
    }

    return string;
}


CodeExporter::CodeExporter(int format, QString& root_path, CLockSet& lock_set, bool clear_or_encrypted)
    :
    m_format(format),
    m_root_path(root_path),
    m_lock_set(lock_set),
    m_clear_or_encrypted(clear_or_encrypted)
{
}

CodeExporter::~CodeExporter()
{
}

void CodeExporter::setFormat(int format)
{
    m_format = format;
}

bool CodeExporter::Export(QString filename)
{
    KCB_DEBUG_ENTRY;
    bool result = false;

    QString datetime = QDateTime::currentDateTime().toString(REPORT_FILE_FORMAT);
    QString ext = FormatToString(m_format);
    QString output_filename = QString("%1/%2_%3.%4").arg(m_root_path).arg(filename).arg(datetime).arg(ext);

    switch (m_format)
    {
        case XML_FORMAT:
            result = ExportAsXml(output_filename);
            break;

        case JSON_FORMAT:
            result = ExportAsJson(output_filename);
            break;

        case CSV_FORMAT:
            result = ExportAsCsv(output_filename);
            break;

        case SQL_FORMAT:
            // Not implemented
        case INVALID_FORMAT:
        default:
            KCB_DEBUG_TRACE("Invalid format requested");
    }
    KCB_DEBUG_EXIT;
    return result;
}

bool CodeExporter::ExportAsXml(QString filename)
{
    KCB_DEBUG_ENTRY;
    bool result = false;

    QFile file(filename);
    result = file.open( QIODevice::WriteOnly | QIODevice::Text );
    if (!result)
    {
        KCB_CRITICAL_TRACE("Failed to open" << filename << "Error:" << file.errorString());
        return result;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("codeListing");
    stream.writeAttribute("encrypted", QString("%1").arg(m_clear_or_encrypted ? "Yes" : "No"));
    stream.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");

    CLockSet::Iterator itor;
    CLockState *pState;

    for (itor = m_lock_set.begin(); itor != m_lock_set.end(); itor++)
    {
        pState = itor.value();
        if (pState)
        {
            stream.writeStartElement("code");
            stream.writeTextElement("locks", QString("%1").arg(pState->getLockNums()));
            stream.writeTextElement("code1", QString("%1").arg(pState->getCode1()));
            stream.writeTextElement("code2", QString("%1").arg(pState->getCode2()));
            stream.writeTextElement("username", QString("%1").arg(pState->getDescription()));
            stream.writeTextElement("question1", QString("%1").arg(pState->getQuestion1()));
            stream.writeTextElement("question2", QString("%1").arg(pState->getQuestion2()));
            stream.writeTextElement("question3", QString("%1").arg(pState->getQuestion3()));
            stream.writeTextElement("starttime", QString("%1").arg(pState->getStartTime().toString(DATETIME_FORMAT)));
            stream.writeTextElement("endtime", QString("%1").arg(pState->getEndTime().toString(DATETIME_FORMAT)));
            stream.writeTextElement("access_type", QString("%1").arg(pState->getAccessType()));
            stream.writeEndElement();
        }
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();

    KCB_DEBUG_EXIT;
    return result;
}

bool CodeExporter::ExportAsJson(QString filename)
{
    KCB_DEBUG_ENTRY;
    bool result = false;

    QFile file(filename);
    file.open( QIODevice::WriteOnly | QIODevice::Text );

    /*
        {
            "encrypted": true|false,
            "codes":
            [
                {
                    "locknums": "1,6,11,16", 
                    "code1": "AwJtIxh81Dgb", 
                    "code2": "AwJt3HI=", 
                    "description": "", 
                    "question1": "", 
                    "question2": "", 
                    "question3": "", 
                    "starttime": "2018-12-01 18:03:08",
                    "endtime": "2018-12-01 18:03:08",
                    "access_type": 0
                },
                ...
                {
                    "locknums": "1,6,11,16", 
                    "code1": "AwJtIxh81Dgb", 
                    "code2": "AwJt3HI=", 
                    "description": "", 
                    "question1": "", 
                    "question2": "", 
                    "question3": "", 
                    "starttime": "2018-12-01 18:03:08",
                    "endtime": "2018-12-01 18:03:08",
                    "access_type": 0
                }
            ]
        }
    */

    QJsonArray codeListing;

    CLockSet::Iterator itor;
    CLockState *pState;

    for (itor = m_lock_set.begin(); itor != m_lock_set.end(); itor++)
    {
        pState = itor.value();
        auto code = QJsonObject();
        code.insert(QString("locknums"), QJsonValue(pState->getLockNums()));
        code.insert(QString("code1"), QJsonValue(pState->getCode1()));
        code.insert(QString("code2"), QJsonValue(pState->getCode2()));
        code.insert(QString("description"), QJsonValue(pState->getDescription()));
        code.insert(QString("question1"), QJsonValue(pState->getQuestion1()));
        code.insert(QString("question2"), QJsonValue(pState->getQuestion2()));
        code.insert(QString("question3"), QJsonValue(pState->getQuestion3()));
        code.insert(QString("starttime"), QJsonValue(pState->getStartTime().toString(DATETIME_FORMAT)));
        code.insert(QString("endtime"), QJsonValue(pState->getEndTime().toString(DATETIME_FORMAT)));
        code.insert(QString("access_type"), QJsonValue(pState->getAccessType()));

        codeListing.push_back(QJsonValue(code));
    }

    QJsonObject obj;
    obj.insert("encrypted", !m_clear_or_encrypted);
    obj.insert("codes", codeListing);
    QByteArray ba = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    file.write(ba);
    file.close();
    result = true;

    KCB_DEBUG_EXIT;
    return result;
}

bool CodeExporter::ExportAsCsv(QString filename)
{
    KCB_DEBUG_ENTRY;
    bool result = false;

    QFile file(filename);
    file.open( QIODevice::WriteOnly | QIODevice::Text );

    file.write(QString("locknums,code1,code2,description,question1,question2,question3,starttime,endtime,access_type\n").toUtf8()); 

    CLockSet::Iterator itor;
    CLockState *pState;
    for (itor = m_lock_set.begin(); itor != m_lock_set.end(); itor++)
    {
        pState = itor.value();

        QString group1 = QString("\"%1\",\"%2\",\"%3\",\"%4\"").arg(pState->getLockNums()).arg(pState->getCode1()).arg(pState->getCode2()).arg(pState->getDescription());
        QString group2 = QString("\"%1\",\"%2\",\"%3\",%4").arg(pState->getQuestion1()).arg(pState->getQuestion2()).arg(pState->getQuestion3());
        QString group3 = QString("\"%1\",\"%2\",%3").arg(pState->getStartTime().toString(DATETIME_FORMAT)).arg(pState->getEndTime().toString(DATETIME_FORMAT)).arg(pState->getAccessType());
        int num_bytes = file.write(QString("%1,%2,%3\n").arg(group1).arg(group2).arg(group3).toUtf8());
        KCB_DEBUG_TRACE("text size" << group1.length() + group2.length() + group3.length() << "wrote" << num_bytes);
    }

    file.close();
    result = true;

    KCB_DEBUG_EXIT;
    return result;
}

//-------------------------------------------------------------------------------------------------
// EOF