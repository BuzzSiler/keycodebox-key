#include "codeexporter.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QJsonObject>
#include <QPair>
#include <QJsonDocument>

#include "kcbcommon.h"


CodeExporter::CodeExporter(CodeImportExportUtil::CODEFORMAT format, QString& root_path, CLockSet& lock_set, CodeImportExportUtil::CODESECURITY security)
    :
    m_format(format),
    m_root_path(root_path),
    m_lock_set(lock_set),
    m_security(security)
{
}

CodeExporter::~CodeExporter()
{
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
        case CodeImportExportUtil::XML_FORMAT:
            result = ExportAsXml(output_filename);
            break;

        case CodeImportExportUtil::JSON_FORMAT:
            result = ExportAsJson(output_filename);
            break;

        case CodeImportExportUtil::CSV_FORMAT:
            result = ExportAsCsv(output_filename);
            break;

        case CodeImportExportUtil::SQL_FORMAT:
            // Not implemented
        case CodeImportExportUtil::INVALID_FORMAT:
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
    stream.writeAttribute("security", QString("%1").arg(CodeImportExportUtil::SecurityToString(m_security)));
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
            "security": "clear|encrypted|not specified",
            "codes":
            [
                {
                    "locknums": "1,6,11,16", 
                    "code1": "AwJtIxh81Dgb", 
                    "code2": "AwJt3HI=", 
                    "username": "", 
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
                    "username": "", 
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
        code.insert(QString("username"), QJsonValue(pState->getDescription()));
        code.insert(QString("question1"), QJsonValue(pState->getQuestion1()));
        code.insert(QString("question2"), QJsonValue(pState->getQuestion2()));
        code.insert(QString("question3"), QJsonValue(pState->getQuestion3()));
        code.insert(QString("starttime"), QJsonValue(pState->getStartTime().toString(DATETIME_FORMAT)));
        code.insert(QString("endtime"), QJsonValue(pState->getEndTime().toString(DATETIME_FORMAT)));
        code.insert(QString("access_type"), QJsonValue(pState->getAccessType()));

        codeListing.push_back(QJsonValue(code));
    }

    QJsonObject obj;
    obj.insert("security", CodeImportExportUtil::SecurityToString(m_security));
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

    file.write(QString("locknums,code1,code2,username,question1,question2,question3,starttime,endtime,access_type\n").toUtf8()); 

    CLockSet::Iterator itor;
    CLockState *pState;
    for (itor = m_lock_set.begin(); itor != m_lock_set.end(); itor++)
    {
        pState = itor.value();

        QString group1 = QString("\"%1\",%2,%3,\"%4\"").arg(pState->getLockNums()).arg(pState->getCode1()).arg(pState->getCode2()).arg(pState->getDescription());
        QString group2 = QString("\"%1\",\"%2\",\"%3\"").arg(pState->getQuestion1()).arg(pState->getQuestion2()).arg(pState->getQuestion3());
        QString group3 = QString("%1,%2,%3").arg(pState->getStartTime().toString(DATETIME_FORMAT)).arg(pState->getEndTime().toString(DATETIME_FORMAT)).arg(pState->getAccessType());
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