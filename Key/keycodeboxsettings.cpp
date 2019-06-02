#include "keycodeboxsettings.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

#include "kcbcommon.h"
#include "kcbsystem.h"
#include "encryption.h"

static QString const SETTINGS_PATH = QString("/home/pi/kcb-config/settings");
static QString const KEYCODEBOX_FILE = QString("kcb.json");
static QString const SETTINGS_FULL_PATH = SETTINGS_PATH + "/" + KEYCODEBOX_FILE;

QJsonObject KeyCodeBoxSettings::m_json_obj = QJsonObject();
QString KeyCodeBoxSettings::m_filename = SETTINGS_FULL_PATH;
CABINET_VECTOR KeyCodeBoxSettings::m_cabinet_info = CABINET_VECTOR(0);

static QString const DEFAULT_KCB_SETTINGS = QString("{\"enable_fleetwave\":false,\"cabinets\":[],\"code_selection\":1,\"display\": false}");


void KeyCodeBoxSettings::createDefault()
{
    // If the kcb.json file does not exist, then create one with a default cabinet
    if (!QFile::exists(m_filename))
    {
        QFile file(m_filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out << DEFAULT_KCB_SETTINGS;
            file.close();
        }
    }
}

void KeyCodeBoxSettings::JsonFromFile()
{
    QString val;
    QFile file;
    QJsonDocument doc;

    // KCB_DEBUG_ENTRY;
    
    createDefault();

    file.setFileName(m_filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    //KCB_DEBUG_TRACE(val);
    file.close();
    doc = QJsonDocument::fromJson(val.toUtf8());
    m_json_obj = doc.object();

    // KCB_DEBUG_TRACE(doc.toJson(QJsonDocument::Compact));
    // KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::JsonToFile()
{
    QString val;
    QFile file;
    // KCB_DEBUG_ENTRY;
    QJsonDocument doc(m_json_obj);

    //KCB_DEBUG_TRACE(doc.toJson(QJsonDocument::Compact));

    file.setFileName(SETTINGS_FULL_PATH);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(doc.toJson());
    file.close();
    // KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::isFleetwaveEnabled()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();
    bool value = m_json_obj["enable_fleetwave"].toBool();
    KCB_DEBUG_EXIT;
    return value;
}

CABINET_VECTOR KeyCodeBoxSettings::getCabinetsInfo()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();

    QJsonArray cabArray = m_json_obj["cabinets"].toArray();

    //KCB_DEBUG_TRACE("cabArray count" << cabArray.count());

    #define NULL_CABINET_INFO { "", 0, -1, -1, "", 0}
    m_cabinet_info.clear();

    foreach (auto elem, cabArray)
    {
        QJsonObject obj = elem.toObject();

        CABINET_INFO cab_info = { obj["model"].toString(), 
                                  obj["num_locks"].toInt(), 
                                  obj["start"].toInt(), 
                                  obj["stop"].toInt(),
                                  obj["sw_version"].toString(),
                                  obj["addr"].toString() };
        //KCB_DEBUG_TRACE(cab_info.model << cab_info.num_locks << cab_info.start << cab_info.stop << cab_info.sw_version << cab_info.addr);

        m_cabinet_info.append(cab_info);
    }

    std::sort(m_cabinet_info.begin(), m_cabinet_info.end());

    // foreach (auto entry, m_cabinet_info)
    // {
    //     KCB_DEBUG_TRACE(entry.model << entry.num_locks << entry.start << entry.stop << entry.sw_version << entry.addr);
    // }

    KCB_DEBUG_EXIT;
    return m_cabinet_info;
}

int KeyCodeBoxSettings::getNumCabinets()
{
    KCB_DEBUG_ENTRY;
    getCabinetsInfo();
    KCB_DEBUG_EXIT;
    return m_cabinet_info.count();
}

int KeyCodeBoxSettings::getLocksPerCabinet(int cab_index)
{
    KCB_DEBUG_ENTRY;
    getCabinetsInfo();
    KCB_DEBUG_EXIT;
    return m_cabinet_info[cab_index].num_locks;
}

int KeyCodeBoxSettings::getTotalLocks()
{
    int total = 0;
    getCabinetsInfo();
    if (m_cabinet_info.count() > 0)
    {
        foreach (const auto cab, m_cabinet_info)
        {
            total += cab.num_locks;
        }
    }

    return total;
}

void KeyCodeBoxSettings::ClearCabinetConfig()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();

    if (m_json_obj.contains("cabinets"))
    {
        m_json_obj.remove("cabinets");
    }

    JsonToFile();

    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::AddCabinet(CABINET_INFO const &cab)
{
    KCB_DEBUG_ENTRY;
    QJsonObject cabinet;
    cabinet.insert(QString("model"), QJsonValue(cab.model));
    cabinet.insert(QString("num_locks"), QJsonValue(cab.num_locks));
    cabinet.insert(QString("start"), QJsonValue(cab.start));
    cabinet.insert(QString("stop"), QJsonValue(cab.stop));
    cabinet.insert(QString("sw_version"), QJsonValue(cab.sw_version));
    cabinet.insert(QString("addr"), QJsonValue(cab.addr));

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

    KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::isDisplaySet()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();
    bool value = m_json_obj["display"].toBool();
    KCB_DEBUG_EXIT;
    return value;
}

void KeyCodeBoxSettings::setDisplay()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();
    if (m_json_obj.contains("display"))
    {
        m_json_obj["display"] = QJsonValue(true).toBool();
    }
    else
    {
        m_json_obj.insert(QString("display"), QJsonValue(true));
    }
    JsonToFile();
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::setNetworkingSettings(NETWORK_SETTINGS const & settings)
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    QJsonObject static_ip;

    if (m_json_obj.contains("static_ip"))
    {
        static_ip = m_json_obj["static_ip"].toObject();

        m_json_obj.remove("static_ip");

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

    m_json_obj.insert(QString("static_ip"), QJsonValue(static_ip));

    JsonToFile();
    // KCB_DEBUG_EXIT;
}

NETWORK_SETTINGS KeyCodeBoxSettings::getNetworkingSettings()
{
    // KCB_DEBUG_ENTRY;
    JsonFromFile();

    NETWORK_SETTINGS ns { "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0" };

    if (m_json_obj.contains("static_ip"))
    {
        QJsonObject temp = m_json_obj["static_ip"].toObject();

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
    if (m_json_obj.contains("static_ip"))
    {
        static_ip = m_json_obj["static_ip"].toObject();
        m_json_obj.remove("static_ip");
        static_ip["enabled"] = true;
    }    
    else
    {
        static_ip.insert(QString("enabled"), QJsonValue(true));
    }

    m_json_obj.insert(QString("static_ip"), QJsonValue(static_ip));

    JsonToFile();
}

void KeyCodeBoxSettings::ClearEnableStaticAddressing()
{
    JsonFromFile();

    QJsonObject static_ip;
    if (m_json_obj.contains("static_ip"))
    {
        static_ip = m_json_obj["static_ip"].toObject();
        m_json_obj.remove("static_ip");
        static_ip["enabled"] = false;
    }    
    else
    {
        static_ip.insert(QString("enabled"), QJsonValue(true));
    }

    m_json_obj.insert(QString("static_ip"), QJsonValue(static_ip));

    JsonToFile();
}

void KeyCodeBoxSettings::EnableStaticAddressing()
{
    NETWORK_SETTINGS ns = getNetworkingSettings();
    SetEnableStaticAddressing();
    setNetworkingSettings(ns);
}

void KeyCodeBoxSettings::DisableStaticAddressing()
{
    NETWORK_SETTINGS ns = getNetworkingSettings();
    ClearEnableStaticAddressing();
    setNetworkingSettings(ns);
}

bool KeyCodeBoxSettings::StaticAddressingEnabled()
{
    JsonFromFile();

    bool result = false;
    
    if (m_json_obj.contains("static_ip"))
    {
        auto temp = m_json_obj["static_ip"].toObject();
        result = temp["enabled"].toBool();
    }

    return result;
}

QString KeyCodeBoxSettings::GetVncPort()
{
    QString result("");
    // Get the VNC port from the configuration
    // presently, vnc port is stored in two places:
    //   the admin table in the database
    //   a text file in kcb-config/config/vnc_creds.txt
    // This needs to be fixed so that it is only in a single place
    //   - preferrably kcb.json
    if (QFile::exists("/home/pi/kcb-config/config/vnc_creds.txt"))
    {
        QFile vnc_file("/home/pi/kcb-config/config/vnc_creds.txt");

        if (vnc_file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString text = vnc_file.readAll();

            // |<port> keycodebox|
            QStringList parts = text.split(" ");

            result = parts[0].right(parts[0].length() - 1);
        }
    }

    KCB_DEBUG_TRACE("result" << result);
    return result;
}

void KeyCodeBoxSettings::SetVncCredentials(QString port, QString password)
{
    KCB_DEBUG_TRACE(port << password);

    std::system("rm /home/pi/kcb-config/config/vnc_creds.txt");
    QString command = QString("echo '|%1 %2|' > /home/pi/kcb-config/config/vnc_creds.txt").arg(port).arg(password);
    KCB_DEBUG_TRACE(command);
    std::system(command.toStdString().c_str());
}

SelectionType KeyCodeBoxSettings::GetLockSelectionType()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();

    SelectionType result = SelectionType::SINGLE;
    
    if (m_json_obj.contains("code_selection"))
    {
        result = static_cast<SelectionType>(m_json_obj["code_selection"].toInt());
    }
    else
    {
        KCB_DEBUG_TRACE("settings selection to single because object not found");
        SetLockSelectionType(SelectionType::SINGLE);
    }

    KCB_DEBUG_EXIT;
    return result;
}

void KeyCodeBoxSettings::SetLockSelectionType(SelectionType value)
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();

    if (m_json_obj.contains("lock_selection"))
    {
        m_json_obj["lock_selection"] = static_cast<int>(value);
    }
    else
    {
        m_json_obj.insert(QString("lock_selection"), QJsonValue(static_cast<int>(value)));
    }

    JsonToFile();
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::SetLockSelectionSingle()
{
    KCB_DEBUG_ENTRY;
    SetLockSelectionType(SelectionType::SINGLE);
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::SetLockSelectionMulti()
{
    KCB_DEBUG_ENTRY;
    SetLockSelectionType(SelectionType::MULTI);
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::SetLockSelectionDisabled()
{
    KCB_DEBUG_ENTRY;
    SetLockSelectionType(SelectionType::DISABLED);
    KCB_DEBUG_EXIT;
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

void KeyCodeBoxSettings::setAutoCodeSettings(AutoCodeSettings settings)
{
    KCB_DEBUG_ENTRY;
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
    KCB_DEBUG_EXIT;
}

AutoCodeSettings KeyCodeBoxSettings::GetAutoCodeSettings()
{
    KCB_DEBUG_ENTRY;
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

    KCB_DEBUG_EXIT;
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
    KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.enabled = state;
    setAutoCodeSettings(acs);

    KCB_DEBUG_EXIT;
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
    KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();

    KCB_DEBUG_EXIT;
    return acs.enabled;
}

bool KeyCodeBoxSettings::IsAutoCodeCommitted()
{
    KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();

    KCB_DEBUG_EXIT;
    return acs.committed;
}

void KeyCodeBoxSettings::SetAutoCodeNextGenDateTime(const QDateTime& datetime)
{
    KCB_DEBUG_ENTRY;

    AutoCodeSettings acs = GetAutoCodeSettings();
    acs.nextgendatetime = datetime.toString(DATETIME_FORMAT);
    setAutoCodeSettings(acs);

    KCB_DEBUG_EXIT;
}

QDateTime KeyCodeBoxSettings::GetAutoCodeNextGenDateTime()
{
    AutoCodeSettings acs = GetAutoCodeSettings();

    return QDateTime::fromString(acs.nextgendatetime, DATETIME_FORMAT);
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

bool KeyCodeBoxSettings::GetInternetTimeSetting()
{
    KCB_DEBUG_ENTRY;
    JsonFromFile();

    bool result = false;
    
    if (m_json_obj.contains("internetTime"))
    {
        result = m_json_obj["internetTime"].toBool();
    }
    else
    {
        SetInternetTimeSetting(false);
    }

    return result;
}

void KeyCodeBoxSettings::SetInternetTimeSetting(bool value)
{
    JsonFromFile();

    if (m_json_obj.contains("internetTime"))
    {
        m_json_obj["internetTime"] = QJsonValue(value);
    }
    else
    {
        m_json_obj.insert(QString("internetTime"), QJsonValue(value));
    }

    JsonToFile();
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
