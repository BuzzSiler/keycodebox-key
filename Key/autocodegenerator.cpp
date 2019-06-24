#include "autocodegenerator.h"

#include <random>

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>

#include "kcbcommon.h"
#include "kcbutils.h"
#include "encryption.h"


void DisplayParams(const AutoCodeParams& params)
{
    Q_UNUSED(params);
    KCB_DEBUG_TRACE("params");
    KCB_DEBUG_TRACE("\tcode_mode" << params.code_mode);
    KCB_DEBUG_TRACE("\tnumcodes" << params.numcodes);
    KCB_DEBUG_TRACE("\tcodelength" << params.codelength);
    KCB_DEBUG_TRACE("\tperiod" << params.period);
    KCB_DEBUG_TRACE("\tunits" << params.units);
    KCB_DEBUG_TRACE("\tstartofday" << params.startofday);
    KCB_DEBUG_TRACE("\tid" << params.id);
}

void DisplayJson(const QJsonObject& json)
{
    Q_UNUSED(json);
    KCB_DEBUG_TRACE("json" << kcb::JsonToString(json));
}

AutoCodeGenerator::AutoCodeGenerator(const AutoCodeParams& params) :
    key_(QByteArray()),
    params_(params),
    next_gen_(QDateTime())
{
}

AutoCodeGenerator::AutoCodeGenerator(const QString& key) :
    key_(key),
    params_(AutoCodeParams())
{
}

AutoCodeGenerator::~AutoCodeGenerator()
{
}

static int CalcYearlyGenerations(const QDateTime& epoch, const QDateTime& now)
{
    int yearly_gens = now.date().year() - epoch.date().year() + 1;
    return yearly_gens;
}

static int CalcMonthlyGenerations(const QDateTime& epoch, const QDateTime& now, int months_per_gen)
{
    int monthly_gens = (now.date().month() - epoch.date().month() + 1) / months_per_gen;
    return monthly_gens;
}

static int CalcWeeklyGenerations(const QDateTime& epoch, const QDateTime& now, int weeks_per_gen)
{
    int secs_to_now = epoch.secsTo(now);
    int weeks_to_now = secs_to_now / SECS_IN_WEEK;
    int weekly_gens = (weeks_to_now + 1) / weeks_per_gen;
    return weekly_gens;
}

static int CalcDailyGenerations(const QDateTime& epoch, const QDateTime& now, int days_per_gen)
{
    int secs_to_now = epoch.secsTo(now);
    int days_to_now = secs_to_now / SECS_IN_DAY;
    int daily_gens = (days_to_now + 1) / days_per_gen;
    return daily_gens;
}

static int CalcHourlyGenerations(const QDateTime& epoch, const QDateTime& now, int shifts_per_day)
{
    int secs_to_now = epoch.secsTo(now);
    int days_to_now = secs_to_now / SECS_IN_DAY;
    int shift_gens = days_to_now * shifts_per_day;
    int hours_per_shift = HOURS_IN_DAY / shifts_per_day;
    int shifts_today = (now.time().hour() / hours_per_shift) + 1;
    shift_gens += shifts_today;
    return shift_gens;
}

bool AutoCodeGenerator::IsHourlyGeneration()
{
    return params_.period == 1;
}

bool AutoCodeGenerator::IsDailyGeneration()
{
    return params_.period == 2;
}

bool AutoCodeGenerator::IsWeeklyGeneration()
{
    return params_.period == 3;
}

bool AutoCodeGenerator::IsMonthlyGeneration()
{
    return params_.period == 4;
}

bool AutoCodeGenerator::IsYearlyGeneration()
{
    return params_.period == 5;
}

QDateTime AutoCodeGenerator::ResolveToCurrentHour(const QDateTime& datetime)
{
    return QDateTime(
        datetime.date(), 
        QTime(
            datetime.time().hour(), 0, 0
        )
    );
}

QDateTime AutoCodeGenerator::ResolveToStartOfDay(const QDateTime& datetime)
{
    return QDateTime(
        datetime.date(), 
        QTime(
            params_.startofday - 1, 0, 0
        )
    );
}

QDateTime AutoCodeGenerator::CalcEpoch(const QDateTime& datetime)
{
    return QDateTime(
        QDate(
            datetime.date().year(), 1, 1
        ), 
        QTime(
            params_.startofday - 1, 0, 0
        )
    );
}

int AutoCodeGenerator::GenTimesSinceEpoch(const QDateTime& datetime)
{
    QDateTime epoch_start = CalcEpoch(datetime);

    if (IsHourlyGeneration())
    {
        QDateTime now = ResolveToCurrentHour(datetime);
        return CalcHourlyGenerations(epoch_start, now, GetNumShifts());
    }
    else if (IsDailyGeneration())
    {
        QDateTime now = ResolveToStartOfDay(datetime);
        return CalcDailyGenerations(epoch_start, now, params_.units);
    }
    else if (IsWeeklyGeneration())
    {
        QDateTime now = ResolveToStartOfDay(datetime);
        return CalcWeeklyGenerations(epoch_start, now, params_.units);
    }
    else if (IsMonthlyGeneration())
    {
        QDateTime now = ResolveToStartOfDay(datetime);
        return CalcMonthlyGenerations(epoch_start, now, params_.units);
    }
    else if (IsYearlyGeneration())
    {
        QDateTime now = ResolveToStartOfDay(datetime);
        return CalcYearlyGenerations(epoch_start, now);
    }

    return 0;
}

QString AutoCodeGenerator::CreateCode(int value)
{
    int length = QString::number(value).length();
    if (length > params_.codelength)
    {
        return QString::number(value).right(params_.codelength);
    }
    else
    {
        return QString("%1").arg(value, params_.codelength, 10, QChar('0'));
    }
}

QStringList AutoCodeGenerator::CalcCodeBlock()
{
    QVector<QString> codes;
    codes.clear();
    while (codes.count() < params_.numcodes)
    {
        int value = rand_();
        QString code = CreateCode(value);
        if (codes.contains(code))
        {
            continue;
        }
        else
        {
            codes.append(code);
        }
    }

    return QStringList(codes.toList());
}

void AutoCodeGenerator::InitCodeSequence()
{
    bool valid;

    QJsonObject json = ParamsToJson(params_);
    QString json_str = kcb::JsonToString(json);
    QByteArray hash = QCryptographicHash::hash(json_str.toUtf8(), QCryptographicHash::Sha256);

    QByteArray upper_bytes = hash.toHex().left(8);
    quint32 rndSeed = upper_bytes.toUInt(&valid, 16);
    if (!valid)
    {
        KCB_DEBUG_TRACE("FAILURE: invalid seed conversion");
    }

    rand_.seed(rndSeed);
}

void AutoCodeGenerator::CalcCodeHistory(const QDateTime& now)
{
    int gens_since_epoch = GenTimesSinceEpoch(now);

    InitCodeSequence();

    for (int ii = 0; ii < gens_since_epoch; ++ii)
    {
        (void) CalcCodeBlock();
    }
}

QStringList AutoCodeGenerator::CalcCurrentCodeBlock()
{
    return CalcCodeBlock();
}

int AutoCodeGenerator::GetNumShifts()
{
    return 24 / params_.units;
}

QList<int> AutoCodeGenerator::CalculateShifts()
{
    if (IsHourlyGeneration())
    {
        QList<int> shifts;
        int shift_hour = params_.startofday - 1;
        int num_shifts = GetNumShifts();
        
        for (int ii = 0; ii < num_shifts; ++ii)
        {
            shift_hour += params_.units;
            shifts.append(shift_hour);
        }

        return shifts;
    }
    return QList<int>();
}

void AutoCodeGenerator::CalcNextGenDateTime(const QDateTime& datetime)
{
    if (IsHourlyGeneration())
    {
        QList<int> shifts_end = CalculateShifts();
        foreach (const auto& shift_end, shifts_end)
        {
            if (datetime.time().hour() < shift_end)
            {
                bool next_shift_is_next_day = ((shift_end > 0) && ((shift_end % 24) == 0));
                next_gen_ = QDateTime(datetime.date(), QTime(shift_end % 24, 0, 0));
                if (next_shift_is_next_day)
                {
                    next_gen_ = next_gen_.addDays(1);
                }
                break;
            }
        }
    }
    else
    {
        int secs_in_period = SecsInPeriod(datetime);
        QDateTime next_date = datetime.addSecs(secs_in_period);
        next_gen_ = QDateTime(next_date.date(), QTime(params_.startofday - 1, 0, 0));
    }
}

QStringList AutoCodeGenerator::Generate(const QDateTime& datetime)
{
    CalcNextGenDateTime(datetime);
    CalcCodeHistory(datetime);
    return CalcCurrentCodeBlock();
}

QDateTime AutoCodeGenerator::NextGenDateTime()
{
    return next_gen_;
}

int AutoCodeGenerator::SecsInPeriod(const QDateTime& datetime)
{
    int secs_in_period = 0;
    if (IsHourlyGeneration())
    {
        secs_in_period = SECS_IN_HOUR*params_.units;
    }
    else if (IsDailyGeneration())
    {
        secs_in_period = SECS_IN_DAY*params_.units;
    }
    else if (IsWeeklyGeneration())
    {
        secs_in_period = SECS_IN_WEEK*params_.units;
    }
    else if (IsMonthlyGeneration())
    {
        secs_in_period = SECS_IN_MONTH(datetime.date().daysInMonth())*params_.units;
    }
    else if (IsYearlyGeneration())
    {
        secs_in_period = SECS_IN_YEAR(datetime.date().daysInYear())*params_.units;
    }
    else
    {
        KCB_DEBUG_TRACE("Invalid period settings" << params_.period);
        return 0;
    }
    return secs_in_period;
}

quint64 AutoCodeGenerator::PasswordToEncryptionKey(const QString& password)
{
    return password.toUtf8().left(8).toULongLong();
}

QByteArray AutoCodeGenerator::SecureKeyFromJson(const QJsonObject& json, const QString& password)
{
    quint64 enckey = AutoCodeGenerator::PasswordToEncryptionKey(password);
    QString json_str = kcb::JsonToString(json);
    QByteArray cipher = CEncryption::encryptWithKey(json_str.toUtf8(), enckey);
    QByteArray hash = QCryptographicHash::hash(QByteArray::number(enckey) + cipher, QCryptographicHash::Sha256);
    QByteArray key;
    key += cipher;
    key += hash;
    return key.toBase64(QByteArray::Base64Encoding);
}

QJsonObject AutoCodeGenerator::JsonFromSecureKey(const QByteArray& key, const QString& password)
{
    QByteArray decoded_key = QByteArray::fromBase64(key);
    QByteArray hash = decoded_key.right(32);
    QByteArray cipher = decoded_key.left(decoded_key.count() - hash.count());
    quint64 enckey = AutoCodeGenerator::PasswordToEncryptionKey(password);
    bool valid_hash = QCryptographicHash::hash(QByteArray::number(enckey) + cipher, QCryptographicHash::Sha256) == hash;
    if (valid_hash)
    {
        QByteArray data = CEncryption::decryptWithKey(cipher, enckey);
        return kcb::StringToJson(QString(data));
    }
    return QJsonObject();
}

QByteArray AutoCodeGenerator::SecureKeyFromParams(const AutoCodeParams& params, const QString& password)
{
    QJsonObject json = AutoCodeGenerator::ParamsToJson(params);
    return AutoCodeGenerator::SecureKeyFromJson(json, password);
}

AutoCodeParams AutoCodeGenerator::ParamsFromSecureKey(const QByteArray& key, const QString& password)
{
    QJsonObject json = AutoCodeGenerator::JsonFromSecureKey(key, password);
    if (!json.isEmpty())
    {
        return AutoCodeGenerator::JsonToParams(json);
    }
    return AutoCodeParams();
}

QPair<AutoCodeGenerator::CodeMap, QDateTime> AutoCodeGenerator::CreateCodeMap(const AutoCodeParams& params)
{
    // KCB_DEBUG_ENTRY;
    // DisplayParams(params);
    AutoCodeGenerator acg(params);
    QStringList codes = acg.Generate();
    // KCB_DEBUG_TRACE("codes" << codes);
    QDateTime dt = acg.NextGenDateTime();

    CodeMap map;
    for (int ii = 0; ii < codes.count(); ++ii)
    {
        map[QString::number(ii + 1)] = codes[ii];
    }

    // KCB_DEBUG_EXIT;
    return QPair<AutoCodeGenerator::CodeMap, QDateTime>(map, dt);
}

QJsonObject AutoCodeGenerator::ParamsToJson(const AutoCodeParams& params)
{
    QJsonObject json_obj;

    json_obj.insert(QString("code_mode"), QJsonValue(params.code_mode));
    json_obj.insert(QString("numcodes"), QJsonValue(params.numcodes));
    json_obj.insert(QString("codelength"), QJsonValue(params.codelength));
    json_obj.insert(QString("period"), QJsonValue(params.period));
    json_obj.insert(QString("units"), QJsonValue(params.units));
    json_obj.insert(QString("startofday"), QJsonValue(params.startofday));
    json_obj.insert(QString("id"), QJsonValue(params.id));

    return json_obj;
}

AutoCodeParams AutoCodeGenerator::JsonToParams(const QJsonObject& json)
{
    AutoCodeParams params;

    if (!json.isEmpty())
    {
        params.code_mode = json["code_mode"].toInt();
        params.numcodes = json["numcodes"].toInt();
        params.codelength = json["codelength"].toInt();
        params.period = json["period"].toInt();
        params.units = json["units"].toInt();
        params.startofday = json["startofday"].toInt();
        params.id = json["id"].toString();
    }

    return params;
}