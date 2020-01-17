#include "keycodeboxsettings.h"

#include <unistd.h>

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

#include "kcbcommon.h"
#include "encryption.h"
#include "kcbutils.h"

static QString const KCB_BRANDING_IMAGE("kcbbranding.jpg");
static QString const CONFIG_FILE = QString("config.json");
static QString const SETTINGS_FULL_PATH = QString("%0/%1").arg(KCB_STORAGE_PATH).arg(CONFIG_FILE);

QJsonObject KeyCodeBoxSettings::m_json_obj = QJsonObject();
QString KeyCodeBoxSettings::m_filename = SETTINGS_FULL_PATH;
kcb::CABINET_COLLECTION KeyCodeBoxSettings::m_cabinet_info = kcb::CABINET_COLLECTION(0);

// static QString const DEFAULT_KCB_SETTINGS = QString(
//     "{"
//     "\"enable_fleetwave\":false,"
//     "\"cabinets\":[],\"lock_selection\":1,"
//     "\"display\": false,"
//     "\"internetTime\": false"
//     "}");

/*
{
    "adminPassword": "AwImzUdRgG4qc+E=",
    "cabinets": [
        {
            "addr": "b105",
            "max_locks": 32,
            "model": "KCB32",
            "num_locks": 16,
            "start": 1,
            "stop": 16,
            "sw_version": "04.00"
        }
    ],
    "display": true,
    "displayShowHideButton": false,
    "displayTakeReturnButtons": false,
    "enableFleetwave": true,
    "hwDiscoveryOnStartup": true,
    "internetTime": true,
    "lockSelection": 1,
    "powerDownTimeout": 0,
    "smtp": {
        "password": "keycodebox",
        "port": 465,
        "server": "smtpout.secureserver.net",
        "type": 1,
        "username": "kcb@keycodebox.com"
    },
    "staticIp": {
        "address": "0.0.0.0",
        "broadcast": "0.0.0.0",
        "dns": "0.0.0.0",
        "enabled": false,
        "gateway": "0.0.0.0",
        "mask": "0.0.0.0"
    },
    "vnc": {
        "enable": false,
        "password": "AwKkFbs=",
        "port": ""
    }
}
*/

void KeyCodeBoxSettings::createDefault()
{
    if (kcb::Utils::fileExists( m_filename ))
    {
        return;
    }

    QJsonObject json;

    json.insert("adminPassword", CEncryption::encryptString("CH3V1N"));
    json.insert("cabinets", QJsonValue());
    json.insert("display", false);
    json.insert("displayShowHideButton", false);
    json.insert("displayTakeReturnButtons", false);
    json.insert("hwDiscoveryOnStartup", true);
    json.insert("internetTime", true);
    json.insert("lockSelection", 1);
    json.insert("powerDownTimeout", 0);
    json.insert("showCardId", false);

    QJsonObject autocode;
    autocode.insert("enabled", false);
    autocode.insert("password", CEncryption::encryptString("keycodebox"));
    autocode.insert("key", "");
    autocode.insert("committed", false);
    autocode.insert("email", false);
    autocode.insert("nextGenDateTime", "");
    json.insert("autocode", QJsonValue(autocode));

    QJsonObject ip_settings;
    ip_settings.insert("address", "0.0.0.0");
    ip_settings.insert("broadcast", "0.0.0.0");
    ip_settings.insert("dns", "0.0.0.0");
    ip_settings.insert("enabled", false);
    ip_settings.insert("gateway", "0.0.0.0");
    ip_settings.insert("mask", "0.0.0.0");
    json.insert("staticIp", QJsonValue(ip_settings));

    QJsonObject vnc_settings;
    vnc_settings.insert("enable", false);
    vnc_settings.insert("password", CEncryption::encryptString(""));
    vnc_settings.insert("port", "");
    json.insert("vnc", QJsonValue(vnc_settings));

    kcb::Utils::JsonToFile(m_filename, json);
}

void KeyCodeBoxSettings::JsonFromFile()
{
    createDefault();
    m_json_obj = kcb::Utils::JsonFromFile(m_filename);
}

void KeyCodeBoxSettings::JsonToFile()
{
    kcb::Utils::JsonToFile(m_filename, m_json_obj);
}

void KeyCodeBoxSettings::setBoolValue(QString const & key, bool value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();
    kcb::Utils::setBoolValue(m_json_obj, key, value);
    JsonToFile();
    // KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::getBoolValue(QString const & key, bool default_value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();
    int result = kcb::Utils::getBoolValue(m_json_obj, key, default_value);
    // KCB_DEBUG_EXIT;
    return result;
}

void KeyCodeBoxSettings::setStringValue(QString const & key, QString const & value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();
    kcb::Utils::setStringValue(m_json_obj, key, value);
    JsonToFile();
    // KCB_DEBUG_EXIT;
}

QString KeyCodeBoxSettings::getStringValue(QString const & key, QString const & default_value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();
    QString result = kcb::Utils::getStringValue(m_json_obj, key, default_value);
    // KCB_DEBUG_EXIT;
    return result;
}

void KeyCodeBoxSettings::setIntValue(QString const & key, int value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();
    kcb::Utils::setIntValue(m_json_obj, key, value);
    JsonToFile();
    // KCB_DEBUG_EXIT;
}

int KeyCodeBoxSettings::getIntValue(QString const & key, int default_value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();
    int result = kcb::Utils::getIntValue(m_json_obj, key, default_value);
    // KCB_DEBUG_EXIT;
    return result;
}

void KeyCodeBoxSettings::CreateDefaultConfigJson()
{
    QJsonObject json;

    json.insert("adminPassword", CEncryption::encryptString("keycodebox"));
    json.insert("cabinets", QJsonValue());
    json.insert("display", false);
    json.insert("displayShowHideButton", false);
    json.insert("displayTakeReturnButtons", false);
    json.insert("enableFleetwave", true);
    json.insert("hwDiscoveryOnStartup", true);
    json.insert("internetTime", true);
    json.insert("lockSelection", 1);
    json.insert("powerDownTimeout", 0);
    json.insert("showCardId", false);
    QJsonObject ip_settings;
    ip_settings.insert("address", "0.0.0.0");
    ip_settings.insert("broadcast", "0.0.0.0");
    ip_settings.insert("dns", "0.0.0.0");
    ip_settings.insert("enabled", false);
    ip_settings.insert("gateway", "0.0.0.0");
    ip_settings.insert("mask", "0.0.0.0");
    json.insert("staticIp", QJsonValue(ip_settings));
    QJsonObject vnc_settings;
    vnc_settings.insert("enable", false);
    vnc_settings.insert("password", CEncryption::encryptString(""));
    vnc_settings.insert("port", "");
    json.insert("vnc", QJsonValue(vnc_settings));

    kcb::Utils::JsonToFile(m_filename, json);
}

void KeyCodeBoxSettings::CreateDefaults(QString const& config_path)
{
    CreateDefaultConfigJson();
    std::system(QString("mkdir %0/images").arg(config_path).toStdString().c_str());
    std::system(QString("mkdir %0/database").arg(config_path).toStdString().c_str());
}

bool KeyCodeBoxSettings::ValidateStorage(QString const& storage_path, QString const& config_path)
{
    KCB_DEBUG_TRACE("storage path" << storage_path);
    KCB_DEBUG_TRACE("config path" << config_path);

    if (!kcb::IsStorageMounted(storage_path))
    {
        KCB_DEBUG_TRACE("storage not mounted");
        return false;
    }

    if (!kcb::DirExists(config_path))
    {
        std::system(QString("mkdir %0").arg(config_path).toStdString().c_str());
    }

    if (!kcb::DirExists(config_path))
    {
        KCB_DEBUG_TRACE("unable to create configuration path");
        return false;
    }

    return true;
}

bool KeyCodeBoxSettings::ValidateConfiguration(QString const& config_path, QString const& config_file)
{
    KCB_DEBUG_TRACE("configuration path" << config_path);
    KCB_DEBUG_TRACE("configuration file" << config_file);

    m_filename = QString("%0/%1").arg(config_path).arg(config_file);

    if (!kcb::FileExists(m_filename))
    {
        CreateDefaults(config_path);
    }

    bool config_file_exists = kcb::FileExists(m_filename);

    // We will want to validate that the default set of directories exist under 'config' as well
    //   - images
    //   - database
    //   - ???

    if (!config_file_exists)
    {   
        m_filename = "";
    }

    return config_file_exists;
}

bool KeyCodeBoxSettings::Validate()
{
    if (!ValidateStorage("/mnt/storage", "/mnt/storage/config"))
    {
        KCB_DEBUG_TRACE("Invalid storage");
        return false;
    }

    if (!ValidateConfiguration("/mnt/storage/config", CONFIG_FILE))
    {
        KCB_DEBUG_TRACE("Invalid configuration");
        return false;
    }

    KCB_DEBUG_TRACE("settings validated");
    return true;
}

QString KeyCodeBoxSettings::GetBrandingImagePath()
{
    QString path(KCB_SYSTEM_IMAGE_PATH);
    auto setting = m_json_obj["brandingImage"].toString();
    if (setting == "override")
    {
        path = KCB_IMAGE_PATH;
    }

    return path;
}

QString KeyCodeBoxSettings::GetUserImagePath()
{
    return KCB_IMAGE_PATH;
}

QString KeyCodeBoxSettings::GetSystemImagePath()
{
    return KCB_SYSTEM_IMAGE_PATH;
}

uint16_t KeyCodeBoxSettings::validateNumLocks(int value)
{
    // The minimum number of locks per cabinet is 8
    // Note: It has been observed that the number of locks settings was corrupted to be -8
    // 8 was the correct value.  On the off chance there is a corrupt force a negative number
    // we'll try to just remove the sign bit by negating if less than zero.
    // Ultimately, the min check will force any large number to be MAX or less.  While that may
    // cause start/stop to be incorrect, it should eliminate catestrophic errors upstream.
    uint16_t num_locks = MIN_NUM_LOCKS_PER_CABINET;
    if (value < 0)
    {
        value = -value;
        KCB_WARNING_TRACE("Invalid valid number of locks detected, compensating");
    }
    num_locks = static_cast<uint16_t>(value);
    return qMin(num_locks, static_cast<uint16_t>(MAX_NUM_LOCKS_PER_CABINET));
}

uint16_t KeyCodeBoxSettings::validateMaxLocks(uint16_t max_locks)
{
    // Max locks can only be maximum for a bank or maximum for a cabinet
    // Choose 32 if there an error
    if (max_locks == MAX_NUM_LOCKS_PER_BANK || max_locks == MAX_NUM_LOCKS_PER_CABINET)
    {
        return max_locks;
    }

    return MAX_NUM_LOCKS_PER_BANK;
}

uint16_t KeyCodeBoxSettings::validateTotalLocks(uint16_t total_num_locks, uint16_t max_locks)
{
    return qMin(total_num_locks, max_locks);
}


QPair<uint16_t, uint16_t> KeyCodeBoxSettings::validateStartStop(int start, int stop, uint16_t num_locks, uint16_t total_locks)
{
    // start and stop must be positive
    if (start < 0)
    {
        start = -start;
        KCB_WARNING_TRACE("Invalid valid lock start detected, compensating");
    }
    if (stop < 0)
    {
        stop = -stop;
        KCB_WARNING_TRACE("Invalid valid lock stop detected, compensating");
    }
    // start/stop are no longer negative but they may be too large
    uint16_t start_value = qMin(total_locks, static_cast<uint16_t>(start));
    uint16_t stop_value = qMin(total_locks, static_cast<uint16_t>(stop));
    bool invalid_start_stop = (stop_value - start_value + 1) != num_locks;
    if (invalid_start_stop)
    {
        KCB_WARNING_TRACE("Invalid valid lock start/stop detected, compensating" << stop_value << start_value << num_locks);
        start_value = 1;
        stop_value = num_locks;
    }

    return QPair<uint16_t, uint16_t>(start_value, stop_value);
}

kcb::CABINET_COLLECTION KeyCodeBoxSettings::getCabinetsInfo()
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    QJsonArray cabArray = m_json_obj["cabinets"].toArray();

    // KCB_DEBUG_TRACE("cabArray count" << cabArray.count());

    m_cabinet_info.clear();
    uint16_t total_num_locks = 0;

    foreach (auto elem, cabArray)
    {
        QJsonObject obj = elem.toObject();

        uint16_t num_locks = validateNumLocks(obj["num_locks"].toInt());
        uint16_t max_locks = validateMaxLocks(obj["max_locks"].toInt());

        total_num_locks += num_locks;
        total_num_locks = validateTotalLocks(total_num_locks, MAX_NUM_LOCKS_PER_CABINET);
        auto start_stop = validateStartStop(obj["start"].toInt(), obj["stop"].toInt(), num_locks, total_num_locks);

        kcb::CABINET_INFO cab_info = { obj["model"].toString(),
                                  num_locks,
                                  start_stop.first,
                                  start_stop.second,
                                  obj["sw_version"].toString(),
                                  obj["addr"].toString(),
                                  max_locks };
        // KCB_DEBUG_TRACE(cab_info.model << cab_info.num_locks << cab_info.start << cab_info.stop << cab_info.sw_version << cab_info.addr << cab_info.max_locks);

        m_cabinet_info.append(cab_info);
    }

    std::sort(m_cabinet_info.begin(), m_cabinet_info.end());

    // KCB_DEBUG_EXIT;
    return m_cabinet_info;
}

int KeyCodeBoxSettings::getNumCabinets()
{
    // KCB_DEBUG_ENTRY;
    getCabinetsInfo();
    // KCB_DEBUG_EXIT;
    return m_cabinet_info.count();
}

int KeyCodeBoxSettings::getLocksPerCabinet(int cab_index)
{
    // KCB_DEBUG_ENTRY;
    getCabinetsInfo();
    // KCB_DEBUG_EXIT;
    return m_cabinet_info[cab_index].num_locks;
}

int KeyCodeBoxSettings::getTotalLocks()
{
    // KCB_DEBUG_ENTRY;
    int total = 0;
    getCabinetsInfo();
    // KCB_DEBUG_TRACE("cab count" << m_cabinet_info.count());
    if (m_cabinet_info.count() > 0)
    {
        foreach (const auto cab, m_cabinet_info)
        {
            total += cab.num_locks;
            // KCB_DEBUG_TRACE("lock count" << total);
        }
    }

    // KCB_DEBUG_EXIT;
    return total;
}

int KeyCodeBoxSettings::getMaxLocks(int cab_index)
{
    // KCB_DEBUG_ENTRY;
    getCabinetsInfo();
    // KCB_DEBUG_EXIT;
    return m_cabinet_info[cab_index].max_locks;
}

void KeyCodeBoxSettings::ClearCabinetConfig()
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    if (m_json_obj.contains("cabinets"))
    {
        m_json_obj.remove("cabinets");
    }

    JsonToFile();

    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::AddCabinet(kcb::CABINET_INFO const &cab)
{
    // KCB_DEBUG_ENTRY;
    QJsonObject cabinet;

    bool invalid_start_stop = (cab.stop - cab.start + 1) != cab.num_locks;
    bool invalid_num_locks = cab.num_locks > MAX_NUM_LOCKS_PER_CABINET;
    uint16_t start = cab.start;
    uint16_t stop = cab.stop;
    uint16_t num_locks = cab.num_locks;
    uint16_t max_locks = cab.max_locks;
    if (invalid_start_stop || invalid_num_locks)
    {
        start = 1;
        stop = static_cast<uint16_t>(MAX_NUM_LOCKS_PER_BANK);
        num_locks = static_cast<uint16_t>(MAX_NUM_LOCKS_PER_BANK);
        max_locks = static_cast<uint16_t>(MAX_NUM_LOCKS_PER_BANK);
    }

    // KCB_DEBUG_TRACE(cab.model << cab.num_locks << cab.start << cab.stop << cab.sw_version << cab.addr << cab.max_locks);

    cabinet.insert(QString("model"), QJsonValue(cab.model));
    cabinet.insert(QString("num_locks"), QJsonValue(num_locks));
    cabinet.insert(QString("start"), QJsonValue(start));
    cabinet.insert(QString("stop"), QJsonValue(stop));
    cabinet.insert(QString("sw_version"), QJsonValue(cab.sw_version));
    cabinet.insert(QString("addr"), QJsonValue(cab.addr));
    cabinet.insert(QString("max_locks"), QJsonValue(max_locks));

    QJsonArray cabinets;
    if (m_json_obj.contains("cabinets"))
    {
        cabinets = m_json_obj["cabinets"].toArray();
        m_json_obj.remove("cabinets");

        cabinets.push_back(QJsonObject(cabinet));
    }    
    else
    {
        cabinets.push_back(cabinet);
    }

    m_json_obj.insert(QString("cabinets"), QJsonValue(cabinets));

    JsonToFile();

    // KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::isDisplaySet()
{
    // KCB_DEBUG_ENTRY;
    bool value = getBoolValue("display", false);
    // KCB_DEBUG_EXIT;
    return value;
}

void KeyCodeBoxSettings::setDisplay()
{
    // KCB_DEBUG_ENTRY;
    setBoolValue("display", true);
    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::setNetworkingSettings(NetworkSettings const & settings)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    QJsonObject static_ip;

    if (m_json_obj.contains("staticIp"))
    {
        static_ip = m_json_obj["staticIp"].toObject();

        m_json_obj.remove("staticIp");

        static_ip["address"] = settings.address;
        static_ip["mask"] = settings.mask;
        static_ip["broadcast"] = settings.broadcast;
        static_ip["gateway"] = settings.gateway;
        static_ip["dns"] = settings.dns;
    }
    else
    {
        static_ip.insert(QString("address"), QJsonValue(settings.address));
        static_ip.insert(QString("mask"), QJsonValue(settings.mask));
        static_ip.insert(QString("broadcast"), QJsonValue(settings.broadcast));
        static_ip.insert(QString("gateway"), QJsonValue(settings.gateway));
        static_ip.insert(QString("dns"), QJsonValue(settings.dns));
    }

    m_json_obj.insert(QString("staticIp"), QJsonValue(static_ip));

    JsonToFile();
    // KCB_DEBUG_EXIT;
}

NetworkSettings KeyCodeBoxSettings::getNetworkingSettings()
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    NetworkSettings ns { "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0" };

    if (m_json_obj.contains("staticIp"))
    {
        QJsonObject temp = m_json_obj["staticIp"].toObject();

        ns.address = temp["address"].toString();
        ns.mask = temp["mask"].toString();
        ns.broadcast = temp["broadcast"].toString();
        ns.gateway = temp["gateway"].toString();
        ns.dns = temp["dns"].toString();
    }

    // KCB_DEBUG_EXIT;
    return ns;
}

void KeyCodeBoxSettings::SetEnableStaticAddressing()
{
    JsonFromFile();

    QJsonObject static_ip;
    if (m_json_obj.contains("staticIp"))
    {
        static_ip = m_json_obj["staticIp"].toObject();
        m_json_obj.remove("staticIp");
        static_ip["enabled"] = true;
    }    
    else
    {
        static_ip.insert(QString("enabled"), QJsonValue(true));
    }

    m_json_obj.insert(QString("staticIp"), QJsonValue(static_ip));

    JsonToFile();
}

void KeyCodeBoxSettings::ClearEnableStaticAddressing()
{
    JsonFromFile();

    QJsonObject static_ip;
    if (m_json_obj.contains("staticIp"))
    {
        static_ip = m_json_obj["staticIp"].toObject();
        m_json_obj.remove("staticIp");
        static_ip["enabled"] = false;
    }    
    else
    {
        static_ip.insert(QString("enabled"), QJsonValue(true));
    }

    m_json_obj.insert(QString("staticIp"), QJsonValue(static_ip));

    JsonToFile();
}

void KeyCodeBoxSettings::EnableStaticAddressing()
{
    NetworkSettings ns = getNetworkingSettings();
    SetEnableStaticAddressing();
    setNetworkingSettings(ns);
}

void KeyCodeBoxSettings::DisableStaticAddressing()
{
    NetworkSettings ns = getNetworkingSettings();
    ClearEnableStaticAddressing();
    setNetworkingSettings(ns);
}

bool KeyCodeBoxSettings::StaticAddressingEnabled()
{
    JsonFromFile();

    bool result = false;
    
    if (m_json_obj.contains("staticIp"))
    {
        auto temp = m_json_obj["staticIp"].toObject();
        result = temp["enabled"].toBool();
    }

    return result;
}

QString KeyCodeBoxSettings::GetVncPort()
{
    VncSettings vs = getVncSettings();
    return vs.port;
}

void KeyCodeBoxSettings::setVncSettings(VncSettings const & settings)
{
    JsonFromFile();

    QJsonObject vnc_obj;

    QString enc_password = CEncryption::encryptString(settings.password);

    if (m_json_obj.contains("vnc"))
    {
        vnc_obj = m_json_obj["vnc"].toObject();

        m_json_obj.remove("vnc");

        vnc_obj["port"] = settings.port;
        vnc_obj["password"] = enc_password;
        vnc_obj["enable"] = settings.enable;
    }
    else
    {
        vnc_obj.insert(QString("port"), QJsonValue(settings.port));
        vnc_obj.insert(QString("password"), QJsonValue(enc_password));
        vnc_obj.insert(QString("enable"), QJsonValue(settings.enable));
    }

    m_json_obj.insert(QString("vnc"), QJsonValue(vnc_obj));

    JsonToFile();
}

VncSettings KeyCodeBoxSettings::getVncSettings()
{
    JsonFromFile();

    VncSettings vs("", "", false);

    if (m_json_obj.contains("vnc"))
    {
        QJsonObject temp = m_json_obj["vnc"].toObject();

        vs.port = temp["port"].toString();
        vs.password = CEncryption::decryptString(temp["password"].toString());
        vs.enable = temp["enable"].toBool();
    }
    else
    {
        setVncSettings(vs);
    }
    

    return vs;
}

void KeyCodeBoxSettings::setSmtpSettings(SmtpSettings const & settings)
{
    JsonFromFile();

    QJsonObject smtp_obj;

    if (m_json_obj.contains("smtp"))
    {
        smtp_obj = m_json_obj["smtp"].toObject();

        m_json_obj.remove("smtp");

        smtp_obj["server"] = settings.server;
        smtp_obj["port"] = settings.port;
        smtp_obj["type"] = settings.type;
        smtp_obj["username"] = settings.username;
        smtp_obj["password"] = settings.password;
    }
    else
    {
        smtp_obj.insert(QString("server"), QJsonValue(settings.server));
        smtp_obj.insert(QString("port"), QJsonValue(settings.port));
        smtp_obj.insert(QString("type"), QJsonValue(settings.type));
        smtp_obj.insert(QString("username"), QJsonValue(settings.username));
        smtp_obj.insert(QString("password"), QJsonValue(settings.password));
    }

    m_json_obj.insert(QString("smtp"), QJsonValue(smtp_obj));

    JsonToFile();

}

SmtpSettings KeyCodeBoxSettings::getSmtpSettings()
{
    JsonFromFile();

    SmtpSettings ss("smtpout.secureserver.net", 465, 1, "kcb@keycodebox.com", "keycodebox");

    if (m_json_obj.contains("smtp"))
    {
        QJsonObject temp = m_json_obj["smtp"].toObject();

        ss.server = temp["server"].toString();
        ss.port = temp["port"].toInt();
        ss.type = temp["type"].toInt();
        ss.username = temp["username"].toString();
        ss.password = temp["password"].toString();
    }
    else
    {
        setSmtpSettings(ss);
    }
    
    return ss;
}

SelectionType KeyCodeBoxSettings::GetLockSelectionType()
{
    // KCB_DEBUG_ENTRY;
    int result = getIntValue("lockSelection", static_cast<int>(SelectionType::SINGLE));
    // KCB_DEBUG_EXIT;
    return static_cast<SelectionType>(result);
}

void KeyCodeBoxSettings::SetLockSelectionType(SelectionType value)
{
    // KCB_DEBUG_ENTRY;
    setIntValue("lockSelection", static_cast<int>(value));
    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::SetLockSelectionSingle()
{
    // KCB_DEBUG_ENTRY;
    SetLockSelectionType(SelectionType::SINGLE);
    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::SetLockSelectionMulti()
{
    // KCB_DEBUG_ENTRY;
    SetLockSelectionType(SelectionType::MULTI);
    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::SetLockSelectionDisabled()
{
    // KCB_DEBUG_ENTRY;
    SetLockSelectionType(SelectionType::DISABLED);
    // KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::IsLockSelectionSingle()
{
    return SelectionType::SINGLE == GetLockSelectionType();
}

bool KeyCodeBoxSettings::IsLockSelectionMulti()
{
    return SelectionType::MULTI == GetLockSelectionType();
}

bool KeyCodeBoxSettings::IsLockSelectionEnabled()
{
    return SelectionType::DISABLED != GetLockSelectionType();
}

bool KeyCodeBoxSettings::GetInternetTimeSetting()
{
    return getBoolValue("internetTime", true);
}

void KeyCodeBoxSettings::SetInternetTimeSetting(bool value)
{
    setBoolValue("internetTime", value);
}

bool KeyCodeBoxSettings::IsInternetTimeEnabled()
{
    return GetInternetTimeSetting();
}

void KeyCodeBoxSettings::EnableInternetTime()
{
    SetInternetTimeSetting(true);
}

void KeyCodeBoxSettings::DisableInternetTime()
{
    SetInternetTimeSetting(false);
}

void KeyCodeBoxSettings::setAutoCodeSettings(AutoCodeSettings settings)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    QJsonObject autocode;

    if (m_json_obj.contains("autocode"))
    {
        autocode = m_json_obj["autocode"].toObject();
        m_json_obj.remove("autocode");

        autocode["enabled"] = settings.enabled;
        autocode["password"] = CEncryption::encryptString(settings.password);
        autocode["key"] = settings.key;
        autocode["committed"] = settings.committed;
        autocode["email"] = settings.email;
        autocode["next_gen_datetime"] = settings.nextgendatetime;
    }
    else
    {
        autocode.insert(QString("enabled"), QJsonValue(settings.enabled));
        autocode.insert(QString("password"), QJsonValue(CEncryption::encryptString(settings.password)));
        autocode.insert(QString("key"), QJsonValue(settings.key));
        autocode.insert(QString("committed"), QJsonValue(settings.committed));
        autocode.insert(QString("email"), QJsonValue(settings.email));
        autocode.insert(QString("next_gen_datetime"), QJsonValue(settings.nextgendatetime));
    }

    m_json_obj.insert(QString("autocode"), QJsonValue(autocode));
    
    JsonToFile();
    // KCB_DEBUG_EXIT;
}

AutoCodeSettings KeyCodeBoxSettings::GetAutoCodeSettings()
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    AutoCodeSettings acs;

    if (m_json_obj.contains("autocode"))
    {
        QJsonObject temp = m_json_obj["autocode"].toObject();

        acs.enabled = temp["enabled"].toBool();
        acs.password = CEncryption::decryptString(temp["password"].toString());
        acs.email = temp["email"].toBool();
        acs.key = temp["key"].toString();
        acs.committed = temp["committed"].toBool();
        acs.nextgendatetime = temp["next_gen_datetime"].toString();
    }

    // KCB_DEBUG_EXIT;
    return acs;
}

void KeyCodeBoxSettings::SetAutoCodeDefaults()
{
    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.enabled = false;
    acs.password = "";
    acs.email = false;
    acs.key = "";
    acs.committed = false;
    acs.nextgendatetime = "";
    setAutoCodeSettings(acs);
}

void KeyCodeBoxSettings::SetAutoCodeEnableState(bool state)
{
    // KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.enabled = state;
    setAutoCodeSettings(acs);

    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::EnableAutoCode()
{
    SetAutoCodeEnableState(true);
    SetLockSelectionSingle();
}

void KeyCodeBoxSettings::DisableAutoCode()
{
    SetAutoCodeEnableState(false);
    SetAutoCodeDefaults();
}

bool KeyCodeBoxSettings::IsAutoCodeEnabled()
{
    // KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();

    // KCB_DEBUG_EXIT;
    return acs.enabled;
}

bool KeyCodeBoxSettings::IsAutoCodeCommitted()
{
    // KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();

    // KCB_DEBUG_EXIT;
    return acs.committed;
}

void KeyCodeBoxSettings::SetAutoCodeNextGenDateTime(const QDateTime& datetime)
{
    // KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.nextgendatetime = datetime.toString(DATETIME_FORMAT);
    setAutoCodeSettings(acs);

    // KCB_DEBUG_EXIT;
}

QDateTime KeyCodeBoxSettings::GetAutoCodeNextGenDateTime()
{
    AutoCodeSettings acs = GetAutoCodeSettings();

    return QDateTime::fromString(acs.nextgendatetime, DATETIME_FORMAT);
}

void KeyCodeBoxSettings::EnableAutoCodeEmail()
{
    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.email = true;
    setAutoCodeSettings(acs);
}

void KeyCodeBoxSettings::DisableAutoCodeEmail()
{
    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.email = false;
    setAutoCodeSettings(acs);
}

QByteArray KeyCodeBoxSettings::GetAutoCodeKey()
{
    AutoCodeSettings acs = GetAutoCodeSettings();

    return acs.key.toUtf8();
}

QString KeyCodeBoxSettings::GetAutoCodePassword()
{
    AutoCodeSettings acs = GetAutoCodeSettings();

    return acs.password;
}

// bool KeyCodeBoxSettings::GetInternetTimeSetting()
// {
//     // KCB_DEBUG_ENTRY;
//     JsonFromFile();

//     bool result = false;

//     if (m_json_obj.contains("internetTime"))
//     {
//         result = m_json_obj["internetTime"].toBool();
//     }
//     else
//     {
//         SetInternetTimeSetting(false);
//     }

//     return result;
// }

// void KeyCodeBoxSettings::SetInternetTimeSetting(bool value)
// {
//     JsonFromFile();

//     if (m_json_obj.contains("internetTime"))
//     {
//         m_json_obj["internetTime"] = QJsonValue(value);
//     }
//     else
//     {
//         m_json_obj.insert(QString("internetTime"), QJsonValue(value));
//     }

//     JsonToFile();
// }

// bool KeyCodeBoxSettings::IsInternetTimeEnabled()
// {
//     return GetInternetTimeSetting();
// }

// void KeyCodeBoxSettings::EnableInternetTime()
// {
//     SetInternetTimeSetting(true);
// }

// void KeyCodeBoxSettings::DisableInternetTime()
// {
//     SetInternetTimeSetting(false);
// }

bool KeyCodeBoxSettings::GetApplyAccessTypeToAllCodesSettings()
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    bool result = false;

    if (m_json_obj.contains("applyAccessTypeToAllCodes"))
    {
        result = m_json_obj["applyAccessTypeToAllCodes"].toBool();
    }
    else
    {
        SetApplyAccessTypeToAllCodesSettings(result);
    }

    // KCB_DEBUG_EXIT;
    return result;
}

void KeyCodeBoxSettings::SetApplyAccessTypeToAllCodesSettings(bool value)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    if (m_json_obj.contains("applyAccessTypeToAllCodes"))
    {
        m_json_obj["applyAccessTypeToAllCodes"] = QJsonValue(value);
    }
    else
    {
        m_json_obj.insert(QString("applyAccessTypeToAllCodes"), QJsonValue(value));
    }

    JsonToFile();
    // KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::IsApplyAccessTypeToAllCodes()
{
    return GetApplyAccessTypeToAllCodesSettings();
}

void KeyCodeBoxSettings::EnableApplyAccessTypeToAllCodes()
{
    SetApplyAccessTypeToAllCodesSettings(true);
}

void KeyCodeBoxSettings::DisableApplyAccessTypeToAllCodes()
{
    SetApplyAccessTypeToAllCodesSettings(false);
}

void KeyCodeBoxSettings::setDisplayShowHideButton(bool value)
{
    setBoolValue("displayShowHideButton", value);
}

void KeyCodeBoxSettings::setDisplayPowerDownTimeout(PowerDownTimeout timeout)
{
    setIntValue("powerDownTimeout", static_cast<int>(timeout));
}

void KeyCodeBoxSettings::setDisplayTakeReturnButtons(bool value)
{
    // KCB_DEBUG_ENTRY;
    setBoolValue("displayTakeReturnButtons", value);
    // KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::getDisplayShowHideButton()
{
    return getBoolValue("displayShowHideButton", false);
}

KeyCodeBoxSettings::PowerDownTimeout KeyCodeBoxSettings::getDisplayPowerDownTimeout()
{
    auto result = getIntValue("powerDownTimeout", static_cast<int>(PowerDownTimeout::NONE));
    return static_cast<PowerDownTimeout>(result);
}

bool KeyCodeBoxSettings::getDisplayTakeReturnButtons()
{
    return getBoolValue("displayTakeReturnButtons", false);
}

void KeyCodeBoxSettings::setAdminName(QString const & value)
{
    setStringValue("adminName", value);
}

QString KeyCodeBoxSettings::getAdminName()
{
    return getStringValue("adminName", "Invalid");
}

void KeyCodeBoxSettings::setAdminEmail(QString const & value)
{
    setStringValue("adminEmail", value);
}

QString KeyCodeBoxSettings::getAdminEmail()
{
    return getStringValue("adminEmail", "Invalid");
}

void KeyCodeBoxSettings::setAdminPhone(QString const & value)
{
    setStringValue("adminPhone", value);
}

QString KeyCodeBoxSettings::getAdminPhone()
{
    return getStringValue("adminPhone", "Invalid");
}

void KeyCodeBoxSettings::setAdminPassword(QString const & value)
{
    QString enc_value = CEncryption::encryptString(value);
    setStringValue("adminPassword", enc_value);
}

QString KeyCodeBoxSettings::getAdminPassword()
{
    QString value = getStringValue("adminPassword", "keycodebox");
    if (value == "keycodebox")
    {
        value = CEncryption::encryptString(value);
    }

    return CEncryption::decryptString(value);
}

void KeyCodeBoxSettings::setAssistPassword(QString const & value)
{
    QString enc_value = CEncryption::encryptString(value);
    setStringValue("assistPassword", enc_value);
}

QString KeyCodeBoxSettings::getAssistPassword()
{
    QString value = getStringValue("assistPassword", "KEYCODEBOX");
    if (value == "KEYCODEBOX")
    {
        value = CEncryption::encryptString(value);
    }

    return CEncryption::decryptString(value);
}

void KeyCodeBoxSettings::setEnableHwDiscoveryOnStartup(bool value)
{
    setBoolValue("hwDiscoveryOnStartup", value);
}

bool KeyCodeBoxSettings::getEnableHwDiscoveryOnStartup()
{
    return getBoolValue("hwDiscoveryOnStartup", true);
}

void KeyCodeBoxSettings::SetShowCardId(bool state)
{
    setBoolValue("showCardId", state);
}

bool KeyCodeBoxSettings::GetShowCardId()
{
    return getBoolValue("showCardId", false);
}

QString KeyCodeBoxSettings::GetBrandingImageFilename()
{
    return QString("%0/%1").arg(GetBrandingImagePath()).arg(KCB_BRANDING_IMAGE);
}

void KeyCodeBoxSettings::OverrideBrandingImage(QString const& filename)
{
    QString override_branding_image = QString("%0/%1").arg(GetUserImagePath()).arg(KCB_BRANDING_IMAGE);

    sync();
    std::system(QString("cp %0 %1").arg(filename).arg(override_branding_image).toStdString().c_str());
    sync();
}

void KeyCodeBoxSettings::RestoreDefaultBrandingImage()
{
    QString default_branding_image = QString("%0/%1").arg(GetSystemImagePath()).arg(KCB_BRANDING_IMAGE);

    sync();
    std::system(QString("cp %0 %1").arg(default_branding_image).arg(GetUserImagePath()).toStdString().c_str());
    sync();
}

void KeyCodeBoxSettings::SetHostAddress(QString const &value)
{
    NetworkSettings ns = getNetworkingSettings();
    ns.address = value;
    setNetworkingSettings(ns);
}

QString KeyCodeBoxSettings:: GetHostAddress()
{
    QString result = GetNetworkInfo(kcb::HOST_ADDRESS);
    return result;
}

void KeyCodeBoxSettings::SetNetworkMask(QString const &value)
{
    NetworkSettings ns = getNetworkingSettings();
    ns.mask = value;
    setNetworkingSettings(ns);
}

QString KeyCodeBoxSettings::GetNetworkMask()
{
    QString result = GetNetworkInfo(kcb::NETWORK_MASK);
    return result;
}

void KeyCodeBoxSettings::SetBcastAddress(QString const &value)
{
    NetworkSettings ns = getNetworkingSettings();
    ns.broadcast = value;
    setNetworkingSettings(ns);
}

QString KeyCodeBoxSettings::GetBcastAddress()
{
    QString result = GetNetworkInfo(kcb::BCAST_ADDRESS);
    return result;
}

void KeyCodeBoxSettings::SetGatewayAddress(QString const &value)
{
    NetworkSettings ns = getNetworkingSettings();
    ns.gateway = value;
    setNetworkingSettings(ns);
}

QString KeyCodeBoxSettings::GetGatewayAddress()
{
    QString result = GetNetworkInfo(kcb::GATEWAY_ADDRESS);
    return result;
}

void KeyCodeBoxSettings::SetDnsAddress(QString const & value)
{
    NetworkSettings ns = getNetworkingSettings();
    ns.dns = value;
    setNetworkingSettings(ns);
}

QString KeyCodeBoxSettings::GetDnsAddress()
{
    QString result = GetNetworkInfo(kcb::DNS_ADDRESS);
    return result;
}

QString KeyCodeBoxSettings::GetMacAddress()
{
    QString result = GetNetworkInfo(kcb::MAC_ADDRESS);
    return result;
}

QString KeyCodeBoxSettings::GetStaticNetworkInfo(kcb::NETWORK_INFO_TYPE type)
{
    QString result("");
    NetworkSettings ns = getNetworkingSettings();

    if (type == kcb::HOST_ADDRESS)
    {
        result = ns.address;
    }
    else if (type == kcb::BCAST_ADDRESS)
    {
        result = ns.broadcast;
    }
    else if (type == kcb::NETWORK_MASK)
    {
        result = ns.mask;
    }
    else if (type == kcb::GATEWAY_ADDRESS)
    {
        result = ns.gateway;
    }
    else if (type == kcb::DNS_ADDRESS)
    {
        result = ns.dns;
    }
    else if (type == kcb::MAC_ADDRESS)
    {
        result = kcb::ParseNetworkInfo(type);
    }
    else
    {
        result = "0.0.0.0";
    }

    return result;
}

QString KeyCodeBoxSettings::GetDynamicNetworkInfo(kcb::NETWORK_INFO_TYPE type)
{
    QString result("");

    if (type == kcb::GATEWAY_ADDRESS)
    {
        result = kcb::ParseGatewayAddress();
    }
    else if (type == kcb::DNS_ADDRESS)
    {
        result = kcb::ParseDnsAddress();
    }
    else
    {
        result = kcb::ParseNetworkInfo(type);
    }

    return result;
}

QString KeyCodeBoxSettings::GetNetworkInfo(kcb::NETWORK_INFO_TYPE type)
{
    if (KeyCodeBoxSettings::StaticAddressingEnabled())
    {
        return GetStaticNetworkInfo(type);
    }
    else
    {
        return GetDynamicNetworkInfo(type);
    }
}
