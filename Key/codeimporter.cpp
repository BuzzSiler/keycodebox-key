#include "codeimporter.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>


#include "codeimportexportutil.h"
#include "kcbcommon.h"
#include "xmlcodelistingreader.h"
#include "codeelement.h"


CodeImporter::CodeImporter(CodeImportExportUtil::CODEFORMAT format, const QString& root_path, CodeImportExportUtil::CODESECURITY security)
    :
    m_format(format),
    m_root_path(root_path),
    m_security(security)
{
}

CodeImporter::~CodeImporter()
{
}

bool CodeImporter::import(CodeListing& codeListing)
{
    KCB_DEBUG_ENTRY;
    bool result = false;

    QFile file(m_root_path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KCB_DEBUG_TRACE("Cannot read file" << file.errorString());
        return result;
    }

    switch (m_format)
    {
        case CodeImportExportUtil::XML_FORMAT:
            result = ImportAsXml(file, codeListing);
            break;

        case CodeImportExportUtil::JSON_FORMAT:
            result = ImportAsJson(file, codeListing);
            break;

        case CodeImportExportUtil::CSV_FORMAT:
            result = ImportAsCsv(file, codeListing);
            break;

        case CodeImportExportUtil::SQL_FORMAT:
            // Not implemented
        case CodeImportExportUtil::INVALID_FORMAT:
        default:
            KCB_DEBUG_TRACE("Invalid format requested");
            break;
    }

    file.close();
    KCB_DEBUG_EXIT;
    return result;
}

bool CodeImporter::ImportAsXml(QFile& file, CodeListing& codeListing)
{
    XmlCodeListingReader xmlReader(codeListing);
    if (!xmlReader.read(file))
    {
        KCB_DEBUG_TRACE("Parse error in file " << xmlReader.errorString());
        return false;
    }

    return true;
}

bool CodeImporter::ImportAsJson(QFile& file, CodeListing& codeListing)
{
    KCB_DEBUG_ENTRY;

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        KCB_DEBUG_TRACE("Failed to parse file" << jsonError.errorString());
        return false;
    }

    if (doc.isNull() || doc.isEmpty())
    {
        KCB_DEBUG_TRACE("Null or Empty document");
        return false;
    }

    if (!doc.isObject())
    {
        KCB_DEBUG_TRACE("No object found");
        return false;
    }

    QJsonObject root_obj = doc.object();
    if (root_obj.isEmpty())
    {
        KCB_DEBUG_TRACE("Root object has no object");
        return false;
    }

    CodeImportExportUtil::CODESECURITY security = CodeImportExportUtil::StringToSecurity(root_obj["security"].toString());
    codeListing.setEncrypted(security == CodeImportExportUtil::ENCRYPTED_SECURITY);

    QJsonArray codes = root_obj["codes"].toArray();

    KCB_DEBUG_TRACE("found" << codes.count() << "codes");

    foreach (const QJsonValue & value, codes) 
    {
        QJsonObject code_obj = value.toObject();
        if (code_obj.isEmpty())
        {
            KCB_DEBUG_TRACE("found empty code object ... continuing");
            continue;
        }

        Code *code = new Code();

        code->setLocks(code_obj["locknums"].toString());
        code->setCode1(code_obj["code1"].toString());
        code->setCode2(code_obj["code2"].toString());
        code->setUsername(code_obj["username"].toString());
        code->setQuestion1(code_obj["question1"].toString());
        code->setQuestion2(code_obj["question2"].toString());
        code->setQuestion3(code_obj["question3"].toString());
        code->setStartTime(code_obj["starttime"].toString());
        code->setEndTime(code_obj["endtime"].toString());
        code->setAccessType(code_obj["access_type"].toInt());

        codeListing.addCode(code);
    }

    KCB_DEBUG_EXIT;
    return true;
}

bool CodeImporter::ImportAsCsv(QFile& file, CodeListing& codeListing)
{
    KCB_DEBUG_ENTRY;
    QStringList header;
    bool first_line = true;

    QRegularExpression regex("(?:,\"|^\")(\"\"|[\\w\\W]*?)(?=\",|\"$)|(?:,(?!\")|^(?!\"))([^,]*?)(?=$|,)|(\\r\\n|\\n)");
    QRegularExpressionMatch match;
    QTextStream in(&file);
    int num_fields = 0;
    QString line;

    while (!in.atEnd()) 
    {
        line = in.readLine();
        KCB_DEBUG_TRACE("line:" << line);
        QRegularExpressionMatchIterator iter = regex.globalMatch(line, 0);
        line.clear();

        if (first_line)
        {
            first_line = false;

            while (iter.hasNext())
            {
                QRegularExpressionMatch match = iter.next();

                header.append(match.captured(regex.captureCount() - 1));

            }
            KCB_DEBUG_TRACE("header:" << header);
            num_fields = header.count();
        }
        else
        {
            QStringList record;
            while (iter.hasNext())
            {
                QRegularExpressionMatch match = iter.next();

                // Note: For the above regular expression, 
                //    - if a field is within quotes, the field data will be at capture index 1
                //    - if a field is not within quotes, the field data will be at capture index 2
                // The logic to be applied is:
                //    1. check if capture index 2 is empty, if not empty use
                //    2. if capture index 2 is empty, check if capture index 1 is empty
                //    3. if capture index 1 is empty, then the field is empty
                // 
                // Example:
                //      line: "1,1111,2222,ABCD,,,,1990-01-01 00:00:00,1990-01-01 00:00:00,0"
                //          capture ( 0 ) "1"
                //          capture ( 1 ) ""
                //          capture ( 2 ) "1"
                //
                //      line: ""1,2,3,4",1111,2222,ABCD,"this is question1","this is question2","this is question3",1990-01-01 00:00:00,1990-01-01 00:00:00,0"
                //          capture ( 0 ) ""1,2,3,4"
                //          capture ( 1 ) "1,2,3,4"
                //          capture ( 2 ) ""

                QString field = match.captured(1);
                if (field.isEmpty())
                {
                    field = match.captured(2);
                }                    
                KCB_DEBUG_TRACE("field" << field << "(" << field.isEmpty() << ")");
                record.append(field);
            }
            KCB_DEBUG_TRACE("record" << record);

            if (record.count() != num_fields)
            {
                KCB_DEBUG_TRACE("field mismatch");
                continue;
            }

            Code *code = new Code();
            int index = 0;
            QString value;

            foreach (auto entry, header)
            {
                index = header.indexOf(entry);                
                if (index < 0)
                {                    
                    KCB_DEBUG_TRACE("Invalid index");
                    continue;
                }

                value = record[index];

                KCB_DEBUG_TRACE("Entry" << entry << "Index" << index << "Value" << value);

                if (entry == "locknums")
                {
                    code->setLocks(value);
                }
                else if (entry == "code1")
                {
                    code->setCode1(value);
                }
                else if (entry == "code2")
                {
                    code->setCode2(value);
                }
                else if (entry == "description")
                {
                    code->setUsername(value);
                }
                else if (entry == "question1")
                {
                    code->setQuestion1(value);
                }
                else if (entry == "question2")
                {
                    code->setQuestion2(value);
                }
                else if (entry == "question3")
                {
                    code->setQuestion3(value);
                }
                else if (entry == "startime")
                {
                    code->setStartTime(value);
                }
                else if (entry == "endtime")
                {
                    code->setEndTime(value);
                }
                else if (entry == "access_type")
                {
                    code->setAccessType(value.toInt());
                }
            }

            codeListing.addCode(code);
        }
    }

    // CSV files do not have a facility for storing the code security, use the security
    // provided when the importer was created, i.e., m_security.
    codeListing.setEncrypted(m_security == CodeImportExportUtil::ENCRYPTED_SECURITY);
    
    KCB_DEBUG_EXIT;
    return true;
}

//-------------------------------------------------------------------------------------------------
// EOF