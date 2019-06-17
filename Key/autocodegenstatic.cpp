#include "autocodegenstatic.h"
#include "kcbcommon.h"
#include "keycodeboxsettings.h"
#include "autocodegenerator.h"


namespace AutoCodeGeneratorStatic
{
    static bool AutoCodeCache = false;
    static AutoCodeGenerator::CodeMap AutoCodeMapCache{};


    bool IsNextGenDateTime(const QDateTime& datetime)
    {
        QDateTime next_gen = KeyCodeBoxSettings::GetAutoCodeNextGenDateTime();
        qint64 msecs = datetime.msecsTo(next_gen);
        if (msecs <= 0)
        {
            return true;
        }
        return false;
    }

    AutoCodeGenerator::CodeMap CreateCodeMapAndStoreNextGenDateTime(const AutoCodeParams& params)
    {
        QPair<AutoCodeGenerator::CodeMap, QDateTime> result = AutoCodeGenerator::CreateCodeMap(params);
        KeyCodeBoxSettings::SetAutoCodeNextGenDateTime(result.second);
        return result.first;
    }

    AutoCodeGenerator::CodeMap GetCurrentCode2Codes()
    {
        QByteArray key = KeyCodeBoxSettings::GetAutoCodeKey();
        QString password = KeyCodeBoxSettings::GetAutoCodePassword();
        AutoCodeParams acp = AutoCodeGenerator::ParamsFromSecureKey(key, password);

        if (static_cast<CodeMode>(acp.code_mode) == CodeMode::CODE_MODE_2)
        {
            if (!AutoCodeCache)
            {
                QPair<AutoCodeGenerator::CodeMap, QDateTime> result = AutoCodeGenerator::CreateCodeMap(acp);
                AutoCodeMapCache = result.first;
                AutoCodeCache = true;
            }
            return AutoCodeMapCache;
        }

        return AutoCodeGenerator::CodeMap();
    }

    int MsecsToNextGen(const QDateTime& datetime)
    {
        QDateTime next_gen = KeyCodeBoxSettings::GetAutoCodeNextGenDateTime();
        return datetime.msecsTo(next_gen);
    }

    AutoCodeGenerator::CodeMap GenerateCodeMap()
    {
        QByteArray key = KeyCodeBoxSettings::GetAutoCodeKey();
        QString password = KeyCodeBoxSettings::GetAutoCodePassword();
        AutoCodeParams params = AutoCodeGenerator::ParamsFromSecureKey(key, password);
        AutoCodeGenerator::CodeMap result = AutoCodeGeneratorStatic::CreateCodeMapAndStoreNextGenDateTime(params);
        return result;
    }

    bool IsCode1Mode()
    {
        AutoCodeSettings acs = KeyCodeBoxSettings::GetAutoCodeSettings();
        if (!acs.enabled)
        {
            return false;
        }

        QJsonObject json = AutoCodeGenerator::JsonFromSecureKey(acs.key.toUtf8(), acs.password);
        AutoCodeParams params = AutoCodeGenerator::JsonToParams(json);

        return acs.enabled && static_cast<CodeMode>(params.code_mode) == CodeMode::CODE_MODE_1;
    }

    bool IsCode2Mode()
    {
        AutoCodeSettings acs = KeyCodeBoxSettings::GetAutoCodeSettings();
        if (!acs.enabled)
        {
            return false;
        }
        
        QJsonObject json = AutoCodeGenerator::JsonFromSecureKey(acs.key.toUtf8(), acs.password);
        AutoCodeParams params = AutoCodeGenerator::JsonToParams(json);

        return acs.enabled && static_cast<CodeMode>(params.code_mode) == CodeMode::CODE_MODE_2;
    }

    bool IsEnabled()
    {
        return KeyCodeBoxSettings::IsAutoCodeEnabled();
    }

    bool IsCommitted()
    {
        return KeyCodeBoxSettings::IsAutoCodeCommitted();
    }

    void CreateAndStoreSecureKey(const AutoCodeParams& params, const QString& password)
    {
        AutoCodeSettings acs = KeyCodeBoxSettings::GetAutoCodeSettings();
        acs.key = AutoCodeGenerator::SecureKeyFromParams(params, password);
        KeyCodeBoxSettings::setAutoCodeSettings(acs);
    }

}

