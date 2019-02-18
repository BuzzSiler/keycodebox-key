#include "fleetwave.h"
#include "kcbcommon.h"
#include "kcbsystem.h"
#include "kcbutils.h"

#include <QProcess>

namespace fleetwave
{
    static QString const CHEVIN_SCRIPT = QString("/home/pi/kcb-config/scripts/chevin.py");
    static QString const SETTINGS_PATH = QString("/home/pi/kcb-config/settings");
    static QString const CHEVIN_FILE = QString("chevin.json");
    static QString const SETTINGS_FULL_PATH = SETTINGS_PATH + "/" + CHEVIN_FILE;

    static QString const USED_CODE_FLEETWAVE_KEYPAD_PROMPT = QString("<%1>").arg(QObject::tr("Please Enter Code Using Keypad"));
    static QString const USER_CODE_FLEETWAVE_HIDCARD_PROMPT = QString("<%1>").arg(QObject::tr("Please Present Your Card"));
    static QString const HIDCARD = QString("hidcard");
    static QString const KEYPAD = QString("keypad");


    QJsonObject FleetwaveSettings::m_json_obj = QJsonObject();
    QString FleetwaveSettings::m_filename = SETTINGS_FULL_PATH;

    static bool isValidLock(QString lock)
    {
        bool result;

        int lockNumTest = lock.toInt(&result, 10);
        KCB_DEBUG_TRACE(lockNumTest);
        if (result)
        {
            if (lockNumTest >= 1 && lockNumTest <= 32)
            {
                return true;
            }
        }

        return false;
    }

    FLEETWAVE_RETURN_TYPE SendTakeRequest(QString code, QString& lockNum)
    {
        KCB_DEBUG_TRACE("Requesting code" << code);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("take");
        arguments << QString("request");
        arguments << QString("%1").arg(code);
        QString stdOut;
        QString stdErr;
        int status;
        bool result;

        kcb::ExecuteCommand(program, arguments, stdOut, stdErr, status);
        if (stdOut == "failed" || status == QProcess::CrashExit)
        {
            KCB_DEBUG_TRACE("Fleetwave failed");
            return FLEETWAVE_FAILED;
        }

        if (stdOut == "error")
        {
            KCB_DEBUG_TRACE("Fleetwave error");
            return FLEETWAVE_ERROR;
        }

        result = isValidLock(stdOut);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << stdOut);
            return FLEETWAVE_FAILED;
        }

        lockNum = stdOut;
        KCB_DEBUG_TRACE("Received lock" << lockNum);
        
        return FLEETWAVE_OK;
    }

    FLEETWAVE_RETURN_TYPE SendTakeComplete(QString code, QString lockNum)
    {
        KCB_DEBUG_TRACE("Completing code" << code << "Lock" << lockNum);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("take");
        arguments << QString("complete");
        arguments << QString("%1").arg(lockNum);
        QString stdOut;
        QString stdErr;
        int status;
        bool result;

        kcb::ExecuteCommand(program, arguments, stdOut, stdErr, status);
        if (stdOut == "failed" || status == QProcess::CrashExit)
        {
            return FLEETWAVE_FAILED;
        }

        if (stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        result = isValidLock(stdOut);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << stdOut);
            return FLEETWAVE_FAILED;
        }

        KCB_DEBUG_TRACE("Received lock" << stdOut);

        return lockNum == stdOut ? FLEETWAVE_OK : FLEETWAVE_ERROR;
    }

    FLEETWAVE_RETURN_TYPE SendReturnRequest(QString code, QString& lockNum, QString& question1, QString& question2, QString& question3)
    {
        KCB_DEBUG_TRACE("Requesting code" << code);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("return");
        arguments << QString("request");
        arguments << QString("%1").arg(code);
        QString stdOut;
        QString stdErr;
        int status;
        bool result;

        kcb::ExecuteCommand(program, arguments, stdOut, stdErr, status);
        if (stdOut == "failed" || status == QProcess::CrashExit)
        {
            return FLEETWAVE_FAILED;
        }

        if (stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        QStringList response = stdOut.split(',');
        if (response.count() != 4)
        {
            KCB_DEBUG_TRACE("Incorrect number of response parameters: " << response.count() << stdOut);
            return FLEETWAVE_FAILED;
        }

        result = isValidLock(response[0]);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << stdOut);
            return FLEETWAVE_FAILED;
        }

        lockNum = response[0];
        question1 = response[1];
        question2 = response[2];
        question3 = response[3];

        KCB_DEBUG_TRACE("Received lock" << lockNum << "Questions" << question1 << question2 << question3);
        return FLEETWAVE_OK;
    }

    FLEETWAVE_RETURN_TYPE SendReturnComplete(QString lockNum, QString answer1, QString answer2, QString answer3)
    {
        KCB_DEBUG_TRACE("Completing lock" << lockNum << answer1 << answer2 << answer3);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("return");
        arguments << QString("complete");

        arguments << QString("%1,%2,%3,%4").arg(lockNum).arg(answer1).arg(answer2).arg(answer3);
        QString stdOut;
        QString stdErr;
        int status;
        bool result;

        kcb::ExecuteCommand(program, arguments, stdOut, stdErr, status);
        if (stdOut == "failed" || status == QProcess::CrashExit)
        {
            return FLEETWAVE_FAILED;
        }

        if (stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        result = isValidLock(stdOut);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << stdOut);
            return FLEETWAVE_FAILED;
        }

        KCB_DEBUG_TRACE("Received lock" << stdOut);

        return lockNum == stdOut ? FLEETWAVE_OK : FLEETWAVE_ERROR;
    }

    QString FleetwaveSettings::getPrompt()
    {
        FLEETWAVE_INPUT result = getInput();
        if (result == FLEETWAVE_INPUT::KEYPAD)
        {
            return USED_CODE_FLEETWAVE_KEYPAD_PROMPT;
        }
        else if (result == FLEETWAVE_INPUT::HIDCARD)
        {
            return USER_CODE_FLEETWAVE_HIDCARD_PROMPT;
        }

        return "INVALID";
    }

    FLEETWAVE_INPUT FleetwaveSettings::getInput()
    {
        KCB_DEBUG_ENTRY;
        FLEETWAVE_INPUT result = FLEETWAVE_INPUT::NONE;
        kcb::Utils::JsonFromFile(m_filename, m_json_obj);
        QString value = m_json_obj["input"].toString();
        if (value.toLower() == HIDCARD)
        {
            result = FLEETWAVE_INPUT::HIDCARD;
        }
        else if (value.toLower() == KEYPAD)
        {
            result = FLEETWAVE_INPUT::KEYPAD;
        }
        KCB_DEBUG_EXIT;
        return result;
    }

    bool FleetwaveSettings::isSecure()
    {
        KCB_DEBUG_ENTRY;
        bool result = false;
        kcb::Utils::JsonFromFile(m_filename, m_json_obj);
        QJsonValue value = m_json_obj["secure"];
        if ( value != QJsonValue::Undefined )
        {
            result = value.toBool(false);
        }        
        KCB_DEBUG_EXIT;
        return result;
    }

}
