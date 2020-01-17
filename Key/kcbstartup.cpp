#include "kcbstartup.h"

#include <QObject>
#include <QMessageBox>

#include "kcbcommon.h"
#include "kcbsystem.h"
#include "logger.h"
#include "keycodeboxsettings.h"

namespace kcb
{
    static QString const KCB_CONFIG_FILE = QString("%0/%1").arg(KCB_STORAGE_PATH, "config.json");

    static void warnConfigurationFileFailure()
    {
        QMessageBox msgbox;

        KCB_CRITICAL_TRACE("Operating configuration not found");

        msgbox.setWindowTitle(QObject::tr("Invalid Operating Configuration"));
        msgbox.setText(QObject::tr("The operating configuration could not be found.\n"
                                   "Please ensure the storage device is attached.\n\n"
                                   "Select 'shutdown' to shutdown the system and inspect.\n"
                                   "Select 'reboot' to retry system startup."));
        msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::Discard);
        msgbox.setButtonText(QMessageBox::Yes, QObject::tr("Shutdown"));
        msgbox.setButtonText(QMessageBox::Discard, QObject::tr("Reboot"));
        
        int result = msgbox.exec();
        bool shutdown = result == QMessageBox::Yes;
        bool reboot = result == QMessageBox::Discard;

        if (shutdown)
        {
            //std::Shutdown();
            KCB_DEBUG_TRACE("Shutting down the system");
        }
        else if (reboot)
        {
            //std::Reboot();
            KCB_DEBUG_TRACE("Rebooting the system");
        }
    }

    bool Startup()
    {
        if (!KeyCodeBoxSettings::Validate())
        {
            // how to get rid of -|x in dialog?
            // maybe make this a fullscreen dialog, since it is catastrophic?
            warnConfigurationFileFailure();
            return false;
        }

        //kcb::BackupDatabase();

        if (!KeyCodeBoxSettings::isDisplaySet())
        {
            //kcb::SetupDisplay();
            kcb::updateDisplayConfig();
            KeyCodeBoxSettings::setDisplay();
            KCB_DEBUG_TRACE("Display Config Change - Rebooting");
            Reboot();
        }

        return true;
    }

}
