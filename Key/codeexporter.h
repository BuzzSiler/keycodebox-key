#ifndef CODEEXPORTER_H
#define CODEEXPORTER_H

#include <QString>
#include "lockset.h"


class CodeExporter
{
    public:
        enum { XML_FORMAT, JSON_FORMAT, CSV_FORMAT, SQL_FORMAT, INVALID_FORMAT };

        static int StringToFormat(const QString& format);
        static QString FormatToString(const int& format);


        explicit CodeExporter(int format, QString& root_path, CLockSet& lock_set, bool clear_or_encrypted);
        ~CodeExporter();

        void setFormat(int format);
        bool Export(QString filename="codeexport");

    protected:


    private:
        int m_format;
        QString m_root_path;
        CLockSet& m_lock_set;
        bool m_clear_or_encrypted;

        bool ExportAsXml(QString filename);
        bool ExportAsJson(QString filename);
        bool ExportAsCsv(QString filename);
        bool ExportAsSql(QString filename);
};

#endif