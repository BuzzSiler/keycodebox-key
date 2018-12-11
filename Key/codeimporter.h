#ifndef CODEIMPORTER_H
#define CODEIMPORTER_H

#include <QString>
#include <QFile>

#include "codeimportexportutil.h"
#include "codelistingelement.h"

class CodeImporter
{
    public:
        explicit CodeImporter(CodeImportExportUtil::CODEFORMAT format, const QString& root_path, CodeImportExportUtil::CODESECURITY security);
        ~CodeImporter();

        bool import(CodeListing& codeListing);

    private:
        CodeImportExportUtil::CODEFORMAT m_format;
        QString m_root_path;
        CodeImportExportUtil::CODESECURITY m_security;

        bool ImportAsXml(QFile& file, CodeListing& codeListing);
        bool ImportAsJson(QFile& file, CodeListing& codeListing);
        bool ImportAsCsv(QFile& file, CodeListing& codeListing);
};

#endif