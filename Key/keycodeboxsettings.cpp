#include "keycodeboxsettings.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

#include "kcbcommon.h"

static QString const SETTINGS_PATH = QString("/home/pi/kcb-config/settings");
static QString const KEYCODEBOX_FILE = QString("kcb.json");
static QString const SETTINGS_FULL_PATH = SETTINGS_PATH + "/" + KEYCODEBOX_FILE;

QJsonObject KeyCodeBoxSettings::m_json_obj = QJsonObject();
QString KeyCodeBoxSettings::m_filename = SETTINGS_FULL_PATH;
CABINET_VECTOR KeyCodeBoxSettings::m_cabinet_info = CABINET_VECTOR(0);

static QString const DEFAULT_KCB_SETTINGS = QString("{\"enable_fleetwave\":false,\"cabinets\":[{\"model\":\"KB32\",\"num_locks\":32,\"start\":1,\"stop\":32}]}");

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

    //KCB_DEBUG_TRACE(doc.toJson(QJsonDocument::Compact));
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

    #define NULL_CABINET_INFO { "", 0, -1, -1 }
    #define RANGE_TO_INDEX(s, p) (s >= 1 && p <= 32 ? 0 : s >= 32 && p <= 64 ? 1 : s >= 65 && p <= 86 ? 2 : -1)

    QJsonArray cabArray = m_json_obj["cabinets"].toArray();

    KCB_DEBUG_TRACE("cabArray count" << cabArray.count());

    m_cabinet_info.clear();
    m_cabinet_info.fill(NULL_CABINET_INFO, cabArray.size());

    foreach (auto elem, cabArray)
    {
        QJsonObject obj = elem.toObject();
        CABINET_INFO cab_info = { obj["model"].toString(), 
                                  obj["num_locks"].toInt(), 
                                  obj["start"].toInt(), 
                                  obj["stop"].toInt() };
        KCB_DEBUG_TRACE(cab_info.model << cab_info.num_locks << cab_info.start << cab_info.stop);

        m_cabinet_info[RANGE_TO_INDEX(cab_info.start, cab_info.stop)] = cab_info;
    }

    foreach (auto entry, m_cabinet_info)
    {
        KCB_DEBUG_TRACE(entry.model << entry.num_locks << entry.start << entry.stop);
    }

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
    KCB_DEBUG_ENTRY;
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
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::ClearEnableStaticAddressing()
{
    KCB_DEBUG_ENTRY;
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
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::EnableStaticAddressing()
{
    KCB_DEBUG_ENTRY;
    NETWORK_SETTINGS ns = getNetworkingSettings();
    SetEnableStaticAddressing();
    setNetworkingSettings(ns);
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::DisableStaticAddressing()
{
    KCB_DEBUG_ENTRY;
    NETWORK_SETTINGS ns = getNetworkingSettings();
    ClearEnableStaticAddressing();
    setNetworkingSettings(ns);
    KCB_DEBUG_EXIT;
}

bool KeyCodeBoxSettings::StaticAddressingEnabled()
{
    KCB_DEBUG_ENTRY;
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