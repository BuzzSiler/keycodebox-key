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
        if (datetime > next_gen)
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

    int SecsToNextGen(const QDateTime& datetime)
    {
        QDateTime next_gen = KeyCodeBoxSettings::GetAutoCodeNextGenDateTime();
        return datetime.secsTo(next_gen);
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

    void CreateAndStoreSecureKey(const AutoCodeParams& params, const QString& password)
    {
        AutoCodeSettings acs = KeyCodeBoxSettings::GetAutoCodeSettings();
        acs.key = AutoCodeGenerator::SecureKeyFromParams(params, password);
        KeyCodeBoxSettings::setAutoCodeSettings(acs);
    }

}

//------------------------- Tests -------------------------------

static void TestSecureKeyToFromParams()
{
    KCB_DEBUG_TRACE("TestSecureKeyToFromParams");
    AutoCodeParams in_params;
    in_params.code_mode = 2;
    in_params.code_mode = 7;
    in_params.numcodes = 64;
    in_params.period = 2;
    in_params.startofday = 12;
    in_params.id = "keycodebox-xxxxxxxx";
    QString password("12345");
    QByteArray key = AutoCodeGenerator::SecureKeyFromParams(in_params, password);
    AutoCodeParams out_params = AutoCodeGenerator::ParamsFromSecureKey(key, password);

    DisplayParams(in_params);
    DisplayParams(out_params);
}

static void TestNextGen(const AutoCodeParams& params, const QString&  text, const QDateTime& datetime = QDateTime::currentDateTime())
{
    AutoCodeGenerator acg(params);
    QStringList codes = acg.Generate(datetime);

    KCB_DEBUG_TRACE("next gen(" << text <<")" << acg.NextGenDateTime());
}

static void TestNextGenDateTime()
{
    KCB_DEBUG_TRACE("TestNextGenDateTime");
    AutoCodeParams params;
    params.code_mode = 1;
    params.codelength = 4;
    params.numcodes = 8;
    params.startofday = 1;
    params.id = "keycodebox-xxxxxxxx";

    params.period = 1; // hourly
    params.units = 6;  // 6 hours
    TestNextGen(params, "Hourly, 6 hours");
    params.period = 1; // hourly
    params.units = 8;  // 8 hours
    TestNextGen(params, "Hourly, 8 hours");
    params.period = 1; // hourly
    params.units = 12;  // 12 hours
    TestNextGen(params, "Hourly, 12 hours");

    params.period = 2; // daily
    params.units = 1;  // 1 day
    TestNextGen(params, "Daily, 1 day");
    params.period = 2; // daily
    params.units = 2;  // 2 day
    TestNextGen(params, "Daily, 2 day");
    params.period = 2; // daily
    params.units = 3;  // 3 day
    TestNextGen(params, "Daily, 3 day");

    params.period = 3; // weekly
    params.units = 1;  // 1 day
    TestNextGen(params, "Weekly, 1 week");
    params.period = 3; // weekly
    params.units = 2;  // 2 day
    TestNextGen(params, "Weekly, 2 weeks");
    params.period = 3; // weekly
    params.units = 3;  // 3 day
    TestNextGen(params, "Weekly, 3 weeks");
    params.period = 3; // weekly
    params.units = 5;  // 5 day
    TestNextGen(params, "Weekly, 5 weeks");
    params.period = 3; // weekly
    params.units = 6;  // 6 day
    TestNextGen(params, "Weekly, 6 weeks");

    params.period = 4; // monthly
    params.units = 1;  // 1 month
    TestNextGen(params, "Monthly, 1 month");
    params.period = 4; // monthly
    params.units = 2;  // 2 months
    TestNextGen(params, "Monthly, 2 months");
    params.period = 4; // monthly
    params.units = 3;  // 3 month
    TestNextGen(params, "Monthly, 3 months");
    params.period = 4; // monthly
    params.units = 4;  // 4 months
    TestNextGen(params, "Monthly, 4 months");
    params.period = 4; // monthly
    params.units = 6;  // 6 months
    TestNextGen(params, "Monthly, 6 months");

    params.period = 5; // yearly
    params.units = 1;  // 1 year
    TestNextGen(params, "Yearly, 1 year");
}

static void TestSecsNextGen(const AutoCodeParams& params, const QString&  text, const QDateTime& datetime = QDateTime::currentDateTime())
{
    AutoCodeGenerator acg(params);
    QStringList codes = acg.Generate(datetime);
    QDateTime dt = acg.NextGenDateTime();
    KeyCodeBoxSettings::SetAutoCodeNextGenDateTime(dt);

    // KCB_DEBUG_TRACE("next gen(" << text <<")" << acg.NextGenDateTime());
    // KCB_DEBUG_TRACE("secs to next gen" << AutoCodeGeneratorStatic::SecsToNextGen(datetime));
}


static void TestSecsToNextGenDateTime()
{
    KCB_DEBUG_TRACE("TestSecsToNextGenDateTime");
    AutoCodeParams params;
    params.code_mode = 1;
    params.codelength = 4;
    params.numcodes = 8;
    params.startofday = 1;
    params.id = "keycodebox-xxxxxxxx";

    params.period = 1; // hourly
    params.units = 6;  // 6 hours
    TestSecsNextGen(params, "Hourly, 6 hours, midnight", QDateTime(QDate(2019, 1, 1), QTime(0, 0, 0)));
    TestSecsNextGen(params, "Hourly, 6 hours, 3 am", QDateTime(QDate(2019, 1, 1), QTime(3, 0, 0)));
    TestSecsNextGen(params, "Hourly, 6 hours, 5 am", QDateTime(QDate(2019, 1, 1), QTime(5, 0, 0)));
    TestSecsNextGen(params, "Hourly, 6 hours, 7 am", QDateTime(QDate(2019, 1, 1), QTime(7, 0, 0)));
    TestSecsNextGen(params, "Hourly, 6 hours, 11 am", QDateTime(QDate(2019, 1, 1), QTime(11, 0, 0)));
    TestSecsNextGen(params, "Hourly, 6 hours, 11:59:59 am", QDateTime(QDate(2019, 1, 1), QTime(11, 59, 59)));
    TestSecsNextGen(params, "Hourly, 6 hours, 12:00:01 pm", QDateTime(QDate(2019, 1, 1), QTime(12, 00, 01)));
    TestSecsNextGen(params, "Hourly, 6 hours, 23:59:00 pm", QDateTime(QDate(2019, 1, 1), QTime(23, 59, 00)));
}

void Test()
{
    KCB_DEBUG_TRACE("Start Test");
    TestSecureKeyToFromParams();
    TestNextGenDateTime();
    TestSecsToNextGenDateTime();
    KCB_DEBUG_TRACE("End Test");
}