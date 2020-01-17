#include "fleetwave.h"
#include "kcbcommon.h"
#include "kcbsystem.h"

namespace fleetwave
{
    static const QString CHEVIN_SCRIPT = QString(KCB_SCRIPTS_PATH+"/chevin.py");

    static bool isValidLock(QString lock)
    {
        bool result;

        int lockNumTest = lock.toInt(&result, 10);
        // KCB_DEBUG_TRACE(lockNumTest);
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
        // KCB_DEBUG_TRACE("Requesting code" << code);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("take");
        arguments << QString("request");
        arguments << QString("%1").arg(code);
        bool result;

        auto status = kcb::ExecuteCommand(program, arguments);
        if (status.stdOut == "failed" || status.exitCode < 0)
        {
            return FLEETWAVE_FAILED;
        }

        // if (stdOut == "error")
        if (status.stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        result = isValidLock(status.stdOut);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << status.stdOut);
            return FLEETWAVE_FAILED;
        }

        lockNum = status.stdOut;
        // KCB_DEBUG_TRACE("Received lock" << lockNum);
        
        return FLEETWAVE_OK;
    }

    FLEETWAVE_RETURN_TYPE SendTakeComplete(QString code, QString lockNum)
    {
        Q_UNUSED(code);
        // KCB_DEBUG_TRACE("Completing code" << code << "Lock" << lockNum);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("take");
        arguments << QString("complete");
        arguments << QString("%1").arg(lockNum);
        bool result;

        auto status = kcb::ExecuteCommand(program, arguments);
        if (status.stdOut == "failed" || status.exitCode < 0)
        {
            return FLEETWAVE_FAILED;
        }

        if (status.stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        result = isValidLock(status.stdOut);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" <<status. stdOut);
            return FLEETWAVE_FAILED;
        }

        // KCB_DEBUG_TRACE("Received lock" << status.stdOut);

        return lockNum == status.stdOut ? FLEETWAVE_OK : FLEETWAVE_ERROR;
    }

    FLEETWAVE_RETURN_TYPE SendReturnRequest(QString code, QString& lockNum, QString& question1, QString& question2, QString& question3)
    {
        // KCB_DEBUG_TRACE("Requesting code" << code);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("return");
        arguments << QString("request");
        arguments << QString("%1").arg(code);
        bool result;

        auto status = kcb::ExecuteCommand(program, arguments);
        if (status.stdOut == "failed" || status.exitCode < 0)
        {
            return FLEETWAVE_FAILED;
        }

        if (status.stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        QStringList response = status.stdOut.split(',');
        if (response.count() != 4)
        {
            KCB_DEBUG_TRACE("Incorrect number of response parameters: " << response.count() << status.stdOut);
            return FLEETWAVE_FAILED;
        }

        result = isValidLock(response[0]);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << status.stdOut);
            return FLEETWAVE_FAILED;
        }

        lockNum = response[0];
        question1 = response[1];
        question2 = response[2];
        question3 = response[3];

        // KCB_DEBUG_TRACE("Received lock" << lockNum << "Questions" << question1 << question2 << question3);
        return FLEETWAVE_OK;
    }

    FLEETWAVE_RETURN_TYPE SendReturnComplete(QString lockNum, QString answer1, QString answer2, QString answer3)
    {
        // KCB_DEBUG_TRACE("Completing lock" << lockNum << answer1 << answer2 << answer3);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("return");
        arguments << QString("complete");

        arguments << QString("%1,%2,%3,%4").arg(lockNum).arg(answer1).arg(answer2).arg(answer3);
        bool result;

        auto status = kcb::ExecuteCommand(program, arguments);
        if (status.stdOut == "failed" || status.exitCode < 0)
        {
            return FLEETWAVE_FAILED;
        }

        if (status.stdOut == "error")
        {
            return FLEETWAVE_ERROR;
        }

        result = isValidLock(status.stdOut);
        if (!result)
        {
            KCB_DEBUG_TRACE("Invalid lock" << status.stdOut);
            return FLEETWAVE_FAILED;
        }

        // KCB_DEBUG_TRACE("Received lock" << status.stdOut);

        return lockNum == status.stdOut ? FLEETWAVE_OK : FLEETWAVE_ERROR;
    }

}
