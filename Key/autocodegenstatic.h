#ifndef AUTOCODEGENSTATIC_H
#define AUTOCODEGENSTATIC_H

#include <QDateTime>
#include <QString>

#include "autocodegenerator.h"

namespace AutoCodeGeneratorStatic
{

    bool IsNextGenDateTime(const QDateTime& datetime);
    AutoCodeGenerator::CodeMap CreateCodeMapAndStoreNextGenDateTime(const AutoCodeParams& params);
    AutoCodeGenerator::CodeMap GetCurrentCode2Codes();
    int SecsToNextGen(const QDateTime& datetime = QDateTime::currentDateTime());
    AutoCodeGenerator::CodeMap GenerateCodeMap();
    bool IsCode1Mode();
    bool IsCode2Mode();
    bool IsEnabled();
    void CreateAndStoreSecureKey(const AutoCodeParams& params, const QString& password);

}
#endif // AUTOCODEGENSTATIC_H