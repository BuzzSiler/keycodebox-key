#include "kcbsystem.h"

#include <unistd.h>

#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFileInfo>
#include "kcbcommon.h"

namespace kcb
{
    static const QString NEW_APP_COPY_COMMAND = "sudo cp %1 %2";
    static const QString APP_DIR = "/home/pi/kcb-config/bin";
    static const QString NEW_APP_TARGET = "%1/%2_NEW";

    typedef enum { HOST_ADDRESS, BCAST_ADDRESS, NETWORK_MASK, MAC_ADDRESS } NETWORK_INFO_TYPE;


    void ExecuteCommand(QString program, QStringList arguments, QString& stdOut, QString& stdErr, int& status)
    {
        QProcess proc;

        stdOut = "";
        stdErr = "";

        //KCB_DEBUG_TRACE(program << arguments);

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

    static QString GetNetworkInfo(NETWORK_INFO_TYPE type)
    {
        /*
        eth0      Link encap:Ethernet  HWaddr b8:27:eb:1e:67:9a
                inet addr:192.168.1.144  Bcast:192.168.1.255  Mask:255.255.255.0
                inet6 addr: fe80::18a0:bf0:1832:49e6/64 Scope:Link
                UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
                RX packets:1136880 errors:0 dropped:8 overruns:0 frame:0
                TX packets:1141978 errors:0 dropped:0 overruns:0 carrier:0
                collisions:0 txqueuelen:1000
                RX bytes:112912688 (107.6 MiB)  TX bytes:332918432 (317.4 MiB)
        */    
        QString stdOut;
        QString stdErr;
        int status;

        ExecuteCommand(QString("ifconfig"), QStringList() << QString("eth0"), stdOut, stdErr, status);

        if (!stdOut.isEmpty())
        {
            QStringList strList = stdOut.split("\n");
            
            switch (type)
            {
                case HOST_ADDRESS:
                case BCAST_ADDRESS:
                case NETWORK_MASK:
                {
                    QRegularExpression regex("^inet addr:(.*)  Bcast:(.*)  Mask:(.*)$");
                    // Inverted greediness allows .* to be minimal
                    regex.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
                    QRegularExpressionMatch match = regex.match(strList[1].trimmed());
                    if (match.hasMatch()) 
                    {
                        int index = 0;
                        switch (type)
                        {
                            case BCAST_ADDRESS:
                                index = 2;
                                break;

                            case NETWORK_MASK:
                                index = 3;
                                break;

                            case HOST_ADDRESS:
                            default:
                                index = 1;
                                break;

                        }

                        return match.captured(index).trimmed();
                    }
                }
                break;

                case MAC_ADDRESS:
                {
                    QRegularExpression regex("^.*HWaddr (.*)$");
                    // Inverted greediness allows .* to be minimal
                    regex.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
                    QRegularExpressionMatch match = regex.match(strList[0].trimmed());
                    if (match.hasMatch()) 
                    {
                        return match.captured(1).trimmed();
                    }
                }
                break;
                    
                default:
                    return QString("");
                    break;

            };
        }

        return QString("");
    }

    QString GetHostAddress()
    {
        QString result = GetNetworkInfo(HOST_ADDRESS);
        return result;
    }

    QString GetBcastAddress()
    {
        QString result = GetNetworkInfo(BCAST_ADDRESS);
        return result;
    }

    QString GetMacAddress()
    {
        QString result = GetNetworkInfo(MAC_ADDRESS);
        return result;
    }

    QString GetNetworkMask()
    {
        QString result = GetNetworkInfo(NETWORK_MASK);
        KCB_DEBUG_TRACE("network mask" << result);
        return result;
    }

    QString GetGatewayAddress()
    {
        /*
            Command:
                ip route show

            Outputs:
                default via 192.168.1.1 dev eth0  metric 202
                192.168.1.0/24 dev eth0  proto kernel  scope link  src 192.168.1.144  metric 202
                192.168.63.0/24 dev usb0  proto kernel  scope link  src 192.168.63.100  metric 204
        */
        QString stdOut;
        QString stdErr;
        int status;
        ExecuteCommand(QString("ip"), QStringList() << QString("route") << QString("show"), stdOut, stdErr, status);

        QString result("");
        if (!stdOut.isEmpty())
        {
            /* Take the 3rd field of the first line */

            QStringList lines = stdOut.split('\n');

            if (lines.count() >= 1)
            {
                QString line = lines[0];

                if (line.contains("default via"))
                {
                    QStringList fields = line.split(' ');
                    
                    if (fields.count() >= 3)
                    {
                        result = fields[2];
                    }
                }
            }
        }

        return result;
    }

    bool FPingAddress(QString address)
    {
        int exitCode = QProcess::execute("fping", QStringList() << QString("-r 0 -t 50") << address);

        return (exitCode == 0) ? true : false;
    }

    bool UpdateAppFile(QString filename_fq)
    {
        // filename_fq is the fully qualified filename
        // we want to preserve the filename because it has version information
        QFileInfo fi(filename_fq);
        QString filename = fi.fileName();
        KCB_DEBUG_TRACE(filename);
        QRegularExpression validator("^alpha-v\\d+\\.\\d+\\.\\d+$");
        QRegularExpressionMatch match = validator.match(filename);
        bool result = match.hasMatch();
        if (result)
        {
            QString target = NEW_APP_TARGET.arg(APP_DIR).arg(filename);
            KCB_DEBUG_TRACE("App target" << target);
            QString cmd = NEW_APP_COPY_COMMAND.arg(filename_fq).arg(target);
            KCB_DEBUG_TRACE("CMD" << cmd);
            std::system(cmd.toStdString().c_str());
        }

        return result;
    }

    void UnmountUsb(QString path)
    {
        std::system(QString("umount %1").arg(path).toStdString().c_str());
    }

    void Reboot()
    {
        sync();
        std::system("sudo shutdown -r now");
    }

}
