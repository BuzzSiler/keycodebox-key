#ifndef CODEIMPORTEXPORTUTIL_H
#define CODEIMPORTEXPORTUTIL_H

#include <QString>

namespace CodeImportExportUtil
{
    typedef enum { XML_FORMAT, JSON_FORMAT, CSV_FORMAT, SQL_FORMAT, INVALID_FORMAT = -1 } CODEFORMAT;
    typedef enum { CLEAR_SECURITY, ENCRYPTED_SECURITY, NOTSPECIFIED_SECURITY, INVALID_SECURITY = -1 } CODESECURITY;

    CODEFORMAT StringToFormat(const QString& string);
    QString FormatToString(const CODEFORMAT& format);

    CODESECURITY StringToSecurity(const QString& string);
    QString SecurityToString(const CODESECURITY& security);

}
#endif