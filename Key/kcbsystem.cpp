#include "kcbsystem.h"

#include <QProcess>
#include "kcbcommon.h"

namespace kcb
{
    void ExecuteCommand(QString program, QStringList arguments, QString& stdOut, QString& stdErr, int& status)
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
        status = proc.exitStatus();
    }

    void GetRpiSerialNumber(QString& serial_number)
    {
        QString program = "cat";
        QStringList arguments;
        arguments << "/proc/cpuinfo";
        QString stdOut;
        QString stdErr;
        int status;

        serial_number = "Unknown";

        ExecuteCommand(program, arguments, stdOut, stdErr, status);

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
        int status;

        ExecuteCommand(program, arguments, stdOut, stdErr, status);

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


}
