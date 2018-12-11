#ifndef CODEEXPORTER_H
#define CODEEXPORTER_H

#include <QString>
#include "lockset.h"

#include "codeimportexportutil.h"

class CodeExporter
{
    public:
        explicit CodeExporter(CodeImportExportUtil::CODEFORMAT format, QString& root_path, CLockSet& lock_set, CodeImportExportUtil::CODESECURITY security);
        ~CodeExporter();

        bool Export(QString filename="codeexport");

    protected:


    private:
        CodeImportExportUtil::CODEFORMAT m_format;
        QString m_root_path;
        CLockSet& m_lock_set;
        CodeImportExportUtil::CODESECURITY m_security;

        bool ExportAsXml(QString filename);
        bool ExportAsJson(QString filename);
        bool ExportAsCsv(QString filename);
};

#endif