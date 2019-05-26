#ifndef AUTOCODEGENERATOR_H
#define AUTOCODEGENERATOR_H

#include <QMap>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QStringList>
#include <QJsonObject>

struct AutoCodeSettings
{
    AutoCodeSettings() :
        enabled(false),
        committed(false),
        email(false),
        password(""),
        key(""),
        nextgendatetime("") {}

    bool enabled;
    bool committed;
    bool email;
    QString password;
    QString key;
    QString nextgendatetime;
};

struct AutoCodeParams
{
    AutoCodeParams() :
        code_mode(1),
        startofday(1),
        codelength(4),
        period(1),
        units(1),
        numcodes(0),
        id("") {}

    int code_mode;
    int startofday;
    int codelength;
    int period;
    int units;
    int numcodes;
    QString id;
};

enum class CodeMode { CODE_MODE_1 = 1, CODE_MODE_2 = 2 };


void DisplayParams(const AutoCodeParams& params);

class AutoCodeGenerator
{
    public:
        typedef QMap<QString,QString> CodeMap;

        static QPair<AutoCodeGenerator::CodeMap, QDateTime> CreateCodeMap(const AutoCodeParams& params);
        static AutoCodeParams JsonToParams(const QJsonObject& json);
        static QJsonObject ParamsToJson(const AutoCodeParams& params);
        static QByteArray SecureKeyFromJson(const QJsonObject& json, const QString& password);
        static QJsonObject JsonFromSecureKey(const QByteArray& key, const QString& password);
        static QByteArray SecureKeyFromParams(const AutoCodeParams& params, const QString& password);
        static AutoCodeParams ParamsFromSecureKey(const QByteArray& key, const QString& password);
        static quint64 PasswordToEncryptionKey(const QString& password);

        AutoCodeGenerator(const AutoCodeParams& params);
        AutoCodeGenerator(const QString& key);
        ~AutoCodeGenerator();

        QStringList Generate(const QDateTime& datetime = QDateTime::currentDateTime());
        QDateTime NextGenDateTime();

    private:
        AutoCodeGenerator();

        QString key_;
        AutoCodeParams params_;
        QDateTime next_gen_;
        std::linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647> rand_;


        int GenTimesSinceEpoch(const QDateTime& datetime);
        void InitCodeSequence();
        QString CreateCode(int value);
        QStringList CalcCodeBlock();
        void CalcCodeHistory(const QDateTime& now);
        QStringList CalcCurrentCodeBlock();
        int SecsInPeriod(const QDateTime& datetime);

        bool IsHourlyGeneration();
        bool IsDailyGeneration();
        bool IsWeeklyGeneration();
        bool IsMonthlyGeneration();
        bool IsYearlyGeneration();

        int GetNumShifts();
        QList<int> CalculateShifts();
        QDateTime ResolveToCurrentHour(const QDateTime& datetime);
        QDateTime ResolveToStartOfDay(const QDateTime& datetime);
        QDateTime CalcEpoch(const QDateTime& datetime);
        void CalcNextGenDateTime(const QDateTime& datetime);

};

#endif