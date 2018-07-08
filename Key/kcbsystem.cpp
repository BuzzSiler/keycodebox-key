#include "kcbsystem.h"

#include <QProcess>
#include "kcbcommon.h"

static const QString CHEVIN_SCRIPT = QString("/home/pi/kcb-config/scripts/chevin.py");

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

namespace kcb
{
    void ExecuteCommand(QString program, QStringList arguments, QString& stdOut, QString& stdErr)
    {
        QProcess proc;

        stdOut = "";
        stdErr = "";

        KCB_DEBUG_TRACE(program << arguments);

        proc.start(program, arguments);
        (void) proc.waitForStarted();
        (void) proc.waitForFinished();

        stdOut = proc.readAllStandardOutput();
        stdErr = proc.readAllStandardError();
    }

    void GetRpiSerialNumber(QString& serial_number)
    {
        QString program = "cat";
        QStringList arguments;
        arguments << "/proc/cpuinfo";
        QString stdOut;
        QString stdErr;

        serial_number = "Unknown";

        ExecuteCommand(program, arguments, stdOut, stdErr);

        /*
            pi@raspberrypi:~$ cat /proc/cpuinfo
            Processor       : ARMv6-compatible processor rev 7 (v6l)
            BogoMIPS        : 697.95
            Features        : swp half thumb fastmult vfp edsp java tls
            CPU implementer : 0x41
            CPU architecture: 7
            CPU variant     : 0x0
            CPU part        : 0xb76
            CPU revision    : 7

            Hardware        : BCM2708
            Revision        : 1000002
            Serial          : 000000000000000d
         */

        if (!stdOut.isEmpty())
        {
            QStringList strList = stdOut.split("\n");
            foreach (auto entry, strList)
            {
                if (entry.contains("Serial"))
                {
                    serial_number = entry.split(":")[1].trimmed();
                    break;
                }
            }
        }
    }

    void SetVNCCredentials(QString vnc_port, QString vnc_password)
    {
        // echo '|<vncport> <vncpassword>|' > /home/pi/kcb-config/config/vnc_creds.txt

        KCB_DEBUG_TRACE(vnc_port << vnc_password);

        QString program = QString("echo '|%1 %2|' > /home/pi/kcb-config/config/vnc_creds.txt").arg(vnc_port).arg(vnc_password);
        QStringList arguments;
//        arguments << QString("'|%1 %2|'").arg(vnc_port).arg(vnc_password);
//        arguments << QString(">");
//        arguments << QString("/home/pi/kcb-config/config/vnc_creds.txt");
        QString stdOut;
        QString stdErr;

        ExecuteCommand(program, arguments, stdOut, stdErr);

        qDebug() << stdOut;
        qDebug() << stdErr;

        // FILE *pF;
        // std::string sOutput = "";
        // QString createCmd = "echo '|";
        // createCmd += QString::number(vncport);
        // createCmd += " ";
        // createCmd += vncpassword;
        // createCmd +="|' > /home/pi/run/vnc_creds.txt";

        // pF = popen(createCmd.toStdString().c_str(), "r");
        // if(!pF)
        // {
        //     qDebug() << "failed to create vnc file";
        // }

        // ExtractCommandOutput(pF, sOutput);
        // fclose(pF);
    }

    bool SendChevinTakeRequest(QString code, QString& lockNum)
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
        bool result;

        ExecuteCommand(program, arguments, stdOut, stdErr);
        if (stdOut == "failed")
        {
            return false;
        }

        result = isValidLock(stdOut);
        if (!result)
        {
            return result;
        }

        lockNum = stdOut;
        KCB_DEBUG_TRACE("Received lock" << lockNum);
        
        return true;
    }

    bool SendChevinTakeComplete(QString code, QString lockNum)
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
        bool result;

        ExecuteCommand(program, arguments, stdOut, stdErr);
        if (stdOut == "failed")
        {
            return false;
        }

        result = isValidLock(stdOut);
        if (!result)
        {
            return result;
        }

        KCB_DEBUG_TRACE("Received lock" << stdOut);

        return lockNum == stdOut;
    }

    bool SendChevinReturnRequest(QString code, QString& lockNum, QString& question1, QString& question2, QString& question3)
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
        bool result;

        ExecuteCommand(program, arguments, stdOut, stdErr);
        if (stdOut == "failed")
        {
            return false;
        }

        QStringList response = stdOut.split(',');        
        if (response.count() != 4)
        {
            KCB_DEBUG_TRACE("Incorrect number of response parameters: " << response.count() << stdOut);
            return false;
        }

        result = isValidLock(response[0]);
        if (!result)
        {
            return result;
        }

        lockNum = response[0];
        question1 = response[1];
        question2 = response[2];
        question3 = response[3];

        KCB_DEBUG_TRACE("Received lock" << lockNum << "Questions" << question1 << question2 << question3);
        return true;
    }

    bool SendChevinReturnComplete(QString lockNum, QString answer1, QString answer2, QString answer3)
    {
        KCB_DEBUG_TRACE("Completing lock" << lockNum << answer1 << answer2 << answer3);

        QString program("python");
        QStringList arguments;
        arguments << QString(CHEVIN_SCRIPT);
        arguments << QString("return");
        arguments << QString("complete");

        arguments << QString("%1,%2,%3,%4").arg(lockNum).arg(answer1).arg(answer2).arg(answer3);
        //arguments << QString("\"%1\"").arg(answer1);
        //arguments << QString("\"%1\"").arg(answer2);
        //arguments << QString("\"%1\"").arg(answer3);
        QString stdOut;
        QString stdErr;
        bool result;

        ExecuteCommand(program, arguments, stdOut, stdErr);
        if (stdOut == "failed")
        {
            return false;
        }

        result = isValidLock(stdOut);
        if (!result)
        {
            return result;
        }

        KCB_DEBUG_TRACE("Received lock" << stdOut);

        return lockNum == stdOut;
    }



}
