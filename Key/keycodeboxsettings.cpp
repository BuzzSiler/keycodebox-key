#include "keycodeboxsettings.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

#include "kcbcommon.h"

static QString const SETTINGS_PATH = QString("/home/pi/kcb-config/settings");
static QString const KEYCODEBOX_FILE = QString("kcb.json");
static QString const SETTINGS_FULL_PATH = SETTINGS_PATH + "/" + KEYCODEBOX_FILE;

KeyCodeBoxSettings::KeyCodeBoxSettings(QObject *parent, const QString& filename) :
    QObject(parent),
    m_filename(filename)
{
    if (m_filename.isEmpty())
    {
        m_filename = SETTINGS_FULL_PATH;
    }
    else
    {
        m_filename = filename;
    }

    KCB_DEBUG_TRACE(m_filename);
}

void KeyCodeBoxSettings::JsonFromFile()
{
    QString val;
    QFile file;
    QJsonDocument doc;

    file.setFileName(m_filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    doc = QJsonDocument::fromJson(val.toUtf8());
    m_json_obj = doc.object();
}

void KeyCodeBoxSettings::JsonToFile()
{
    QString val;
    QFile file;
    QJsonDocument doc(m_json_obj);
    file.setFileName(SETTINGS_FULL_PATH);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(doc.toJson());
    file.close();
}

bool KeyCodeBoxSettings::isFleetwaveEnabled()
{
    JsonFromFile();
    return m_json_obj["enable_fleetwave"].toBool();
}

