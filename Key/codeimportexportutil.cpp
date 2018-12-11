#include "codeimportexportutil.h"

#include "kcbcommon.h"

namespace CodeImportExportUtil
{

    CODEFORMAT StringToFormat(const QString& string)
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

    QString FormatToString(const CODEFORMAT& format)
    {
        QString string = "invalid";

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

    CODESECURITY StringToSecurity(const QString& string)
    {
        QString string_lower = string.toLower();

        if (string_lower == "clear")
        {
            return CLEAR_SECURITY;
        }
        else if (string_lower == "encrypted")
        {
            return ENCRYPTED_SECURITY;
        }
        else if (string_lower == "not specified")
        {
            return NOTSPECIFIED_SECURITY;
        }
        else
        {
            KCB_DEBUG_TRACE("Invalid security requested" << string);
            return INVALID_SECURITY;
        }
    }

    QString SecurityToString(const CODESECURITY& security)
    {
        QString string = "invalid";

        switch (security)
        {
            case CLEAR_SECURITY:
                string = "clear";
                break;
                
            case ENCRYPTED_SECURITY:
                string = "encrypted";
                break;
                
            case NOTSPECIFIED_SECURITY:
                string = "not specified";
                break;
                
            case INVALID_SECURITY:
            default:
                KCB_DEBUG_TRACE("Invalid security requested");
                break;
        }

        return string;
    }
}

//-------------------------------------------------------------------------------------------------
// EOF
