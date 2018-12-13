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


void KeyCodeBoxSettings::JsonFromFile()
{
    QString val;
    QFile file;
    QJsonDocument doc;

    KCB_DEBUG_ENTRY;
    file.setFileName(m_filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    //KCB_DEBUG_TRACE(val);
    file.close();
    doc = QJsonDocument::fromJson(val.toUtf8());
    m_json_obj = doc.object();
    KCB_DEBUG_EXIT;
}

void KeyCodeBoxSettings::JsonToFile()
{
    QString val;
    QFile file;
    KCB_DEBUG_ENTRY;
    QJsonDocument doc(m_json_obj);
    file.setFileName(SETTINGS_FULL_PATH);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(doc.toJson());
    file.close();
    KCB_DEBUG_EXIT;
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



