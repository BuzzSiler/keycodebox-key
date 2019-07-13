#include "kcbsystem.h"

#include <unistd.h>

#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFileInfo>
#include <QImage>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QScreen>
#include <QApplication>
#include <QWidget>
#include <QList>
#include <QNetworkInterface>
#include <QPair>

#include "kcbcommon.h"
#include "keycodeboxsettings.h"
#include "kcbutils.h"

namespace kcb
{
    static int const DISPLAY_OFF_7_INCH = 1;
    static int const DISPLAY_OFF_10_INCH = 0;
    static int const DISPLAY_ON_7_INCH = 0;
    static int const DISPLAY_ON_10_INCH = 1;

    static int const WAVESHARE_10INCH_WIDTH = 848;
    static int const WAVESHARE_10INCH_HEIGHT = 480;
    static int const OFFICIAL_7INCH_WIDTH = 800;
    static int const OFFICIAL_7INCH_HEIGHT = 480;

    typedef enum {DISP_POWER_OFF, DISP_POWER_ON} DISP_POWER_STATE;
    static DISP_POWER_STATE display_power_state = DISP_POWER_ON;

    static QString const NEW_APP_COPY_COMMAND = "sudo cp %1 %2";
    static QString const APP_DIR = "/home/pi/kcb-config/bin";
    static QString const NEW_APP_TARGET = "%1/%2_NEW";

    static QString const KCBCONFIG_SETTINGS_PATH("/home/pi/kcb-config/settings");
    static QString const KCBCONFIG_WAVESHARE_DISPLAY_CONFIG_FILE("config.waveshare.txt");
    static QString const KCBCONFIG_OFFICIAL_DISPLAY_CONFIG_FILE("config.official.txt");
    static QString const BOOT_CONFIG_FILE("/boot/config.txt");

    typedef enum { HOST_ADDRESS, BCAST_ADDRESS, NETWORK_MASK, GATEWAY_ADDRESS, DNS_ADDRESS, MAC_ADDRESS } NETWORK_INFO_TYPE;

    QString const IP_COMPONENTS_REGEXP = "^inet addr:(.*)  Bcast:(.*)  Mask:(.*)$";
    QString const MAC_ADDRESS_REGEXP = "^.*HWaddr (.*)$";
    #define INET_ADDR_TEXT (1)
    #define MAC_ADDR_TEXT (0)

    static QString const BASE_DHCPCD_SETTINGS("hostname\n"
                                "persistent\n"
                                "option rapid_commit\n"
                                "option domain_name_servers, domain_name, domain_search, host_name\n"
                                "option classless_static_routes\n"
                                "option ntp_servers\n"
                                "require dhcp_server_identifier\n"
                                "slaac private\n"
                                "nohook lookup-hostname\n\n");


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

        if (status == QProcess::CrashExit)
        {
            KCB_DEBUG_TRACE(QString("%1 crashed").arg(program));
        }
    }

    QString GetRpiSerialNumber()
    {
        QString program = "cat";
        QStringList arguments;
        arguments << "/proc/cpuinfo";
        QString stdOut;
        QString stdErr;
        int status;

        ExecuteCommand(program, arguments, stdOut, stdErr, status);

        /*
            $ cat /proc/cpuinfo
            processor       : 0
            model name      : ARMv7 Processor rev 4 (v7l)
            BogoMIPS        : 38.40
            Features        : half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt vfpd32 lpae evtstrm crc32
            CPU implementer : 0x41
            CPU architecture: 7
            CPU variant     : 0x0
            CPU part        : 0xd03
            CPU revision    : 4

            processor       : 1
            model name      : ARMv7 Processor rev 4 (v7l)
            BogoMIPS        : 38.40
            Features        : half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt vfpd32 lpae evtstrm crc32
            CPU implementer : 0x41
            CPU architecture: 7
            CPU variant     : 0x0
            CPU part        : 0xd03
            CPU revision    : 4

            processor       : 2
            model name      : ARMv7 Processor rev 4 (v7l)
            BogoMIPS        : 38.40
            Features        : half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt vfpd32 lpae evtstrm crc32
            CPU implementer : 0x41
            CPU architecture: 7
            CPU variant     : 0x0
            CPU part        : 0xd03
            CPU revision    : 4

            processor       : 3
            model name      : ARMv7 Processor rev 4 (v7l)
            BogoMIPS        : 38.40
            Features        : half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt vfpd32 lpae evtstrm crc32
            CPU implementer : 0x41
            CPU architecture: 7
            CPU variant     : 0x0
            CPU part        : 0xd03
            CPU revision    : 4

            Hardware        : BCM2709
            Revision        : a22082
        >>>>Serial          : 00000000fb1e679a<<<<
         */

        if (!stdOut.isEmpty())
        {
            QStringList strList = stdOut.split("\n");
            foreach (auto entry, strList)
            {
                if (entry.contains("Serial"))
                {
                    // KCB_DEBUGEXIT;
                    QString value = entry.split(":")[1].trimmed();
                    return value.remove( QRegExp("^[0]*") );
                    //return entry.split(":")[1].right(8).trimmed();
                }
            }
        }

        // KCB_DEBUGEXIT;
        return QString("");
    }

    static QString MatchRegularExpression(QString text, int index, QString regexp)
    {
        QRegularExpression regex(regexp);
        // Inverted greediness allows .* to be minimal
        regex.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
        QRegularExpressionMatch match = regex.match(text.trimmed());
        if (match.hasMatch()) 
        {                        
            return match.captured(index).trimmed();
        }

        return QString("");
    }

    static int TypeToIndex(NETWORK_INFO_TYPE type)
    {
        if (type == BCAST_ADDRESS)
        {
            return 2;
        }
        else if (type == NETWORK_MASK)
        {
            return 3;
        }
        else // HOST_ADDRESS || MAC_ADDRESS
        {
            return 1;
        }
    }

    static QString ParseNetworkComponent(NETWORK_INFO_TYPE type, QString text)
    {
        QString regexp = IP_COMPONENTS_REGEXP;
        if (type == MAC_ADDRESS)
        {
            regexp = MAC_ADDRESS_REGEXP;
        }
        int index = TypeToIndex(type);
        return MatchRegularExpression(text, index, regexp);
    }

    static QString ParseNetworkInfo(NETWORK_INFO_TYPE type)
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
            QString text = strList[INET_ADDR_TEXT];
            if (type == MAC_ADDRESS)
            {
                text = strList[MAC_ADDR_TEXT];
            }

            return ParseNetworkComponent(type, text);
        }

        return QString("");
    }

    static QString ParseGatewayAddress()
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

    static QString ParseDnsAddress()
    {
        /*
            # Generated by resolvconf
            nameserver 192.168.1.1
            nameserver 192.168.2.1
            nameserver 192.168.3.1
            nameserver 192.168.5.1
        */
        
        QString stdOut;
        QString stdErr;
        int status;

        // KCB_DEBUG_ENTRY;
        ExecuteCommand(QString("cat"), 
                       QStringList() << 
                       QString("/etc/resolv.conf") << 
                       QString("|") << 
                       QString("grep") << 
                       QString("nameserver"), stdOut, stdErr, status);

        QString result("");
        if (!stdOut.isEmpty())
        {
            QStringList strList = stdOut.split("\n");
            foreach (auto str, strList)
            {
                if (str.contains("nameserver"))
                {
                    QString address = str.trimmed().split(" ")[1];

                    if (result.isEmpty())
                    {
                        result = address;
                    }
                    else
                    {
                        result += " " + address;
                    }
                }
            }
        }

        // KCB_DEBUG_EXIT;
        return result;
    }

    static QString GetStaticNetworkInfo(NETWORK_INFO_TYPE type)
    {
        QString result("");
        NETWORK_SETTINGS ns = KeyCodeBoxSettings::getNetworkingSettings();

        if (type == HOST_ADDRESS)
        {
            result = ns.address;
        }
        else if (type == BCAST_ADDRESS)
        {
            result = ns.broadcast;
        }
        else if (type == NETWORK_MASK)
        {
            result = ns.mask;
        }
        else if (type == GATEWAY_ADDRESS)
        {
            result = ns.gateway;
        }
        else if (type == DNS_ADDRESS)
        {
            result = ns.dns;
        }
        else if (type == MAC_ADDRESS)
        {
            result = ParseNetworkInfo(type);
        }
        else
        {
            result = "0.0.0.0";
        }

        return result;
    }

    static QString GetDynamicNetworkInfo(NETWORK_INFO_TYPE type)
    {
        QString result("");

        if (type == GATEWAY_ADDRESS)
        {
            result = ParseGatewayAddress();
        }
        else if (type == DNS_ADDRESS)
        {
            result = ParseDnsAddress();
        }
        else
        {
            result = ParseNetworkInfo(type);
        }

        return result;
    }

    static QString GetNetworkInfo(NETWORK_INFO_TYPE type)
    {
        if (KeyCodeBoxSettings::StaticAddressingEnabled())
        {
            return GetStaticNetworkInfo(type);
        }
        else
        {
            return GetDynamicNetworkInfo(type);
        }
    }

    void SetHostAddress(QString const &value)
    {
        NETWORK_SETTINGS ns = KeyCodeBoxSettings::getNetworkingSettings();
        ns.address = value;
        KeyCodeBoxSettings::setNetworkingSettings(ns);
    }

    QString GetHostAddress()
    {
        QString result = GetNetworkInfo(HOST_ADDRESS);
        return result;
    }

    void SetNetworkMask(QString const &value)
    {
        NETWORK_SETTINGS ns = KeyCodeBoxSettings::getNetworkingSettings();
        ns.mask = value;
        KeyCodeBoxSettings::setNetworkingSettings(ns);
    }

    QString GetNetworkMask()
    {
        QString result = GetNetworkInfo(NETWORK_MASK);
        return result;
    }

    void SetBcastAddress(QString const &value)
    {
        NETWORK_SETTINGS ns = KeyCodeBoxSettings::getNetworkingSettings();
        ns.broadcast = value;
        KeyCodeBoxSettings::setNetworkingSettings(ns);
    }

    QString GetBcastAddress()
    {
        QString result = GetNetworkInfo(BCAST_ADDRESS);
        return result;
    }

    void SetGatewayAddress(QString const &value)
    {
        NETWORK_SETTINGS ns = KeyCodeBoxSettings::getNetworkingSettings();
        ns.gateway = value;
        KeyCodeBoxSettings::setNetworkingSettings(ns);
    }

    QString GetGatewayAddress()
    {
        QString result = GetNetworkInfo(GATEWAY_ADDRESS);
        return result;
    }

    void SetDnsAddress(QString const & value)
    {
        NETWORK_SETTINGS ns = KeyCodeBoxSettings::getNetworkingSettings();
        ns.dns = value;
        KeyCodeBoxSettings::setNetworkingSettings(ns);
    }

    QString GetDnsAddress()
    {
        QString result = GetNetworkInfo(DNS_ADDRESS);
        return result;
    }

    QString GetMacAddress()
    {
        QString result = GetNetworkInfo(MAC_ADDRESS);
        return result;
    }

    void EnableStaticAddressing()
    {
        KeyCodeBoxSettings::EnableStaticAddressing();
    }

    void DisableStaticAddressing()
    {
        KeyCodeBoxSettings::DisableStaticAddressing();
    }

    bool StaticAddressingEnabled()
    {
        return KeyCodeBoxSettings::StaticAddressingEnabled();
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
        // KCB_DEBUG_TRACE(filename);
        QRegularExpression validator("^alpha-v\\d+\\.\\d+\\.\\d+$");
        QRegularExpressionMatch match = validator.match(filename);
        bool result = match.hasMatch();
        if (result)
        {
            QString target = NEW_APP_TARGET.arg(APP_DIR).arg(filename);
            // KCB_DEBUG_TRACE("App target" << target);
            QString cmd = NEW_APP_COPY_COMMAND.arg(filename_fq).arg(target);
            // KCB_DEBUG_TRACE("CMD" << cmd);
            std::system(cmd.toStdString().c_str());
        }

        return result;
    }

    void UnmountUsb(QString path)
    {
        // QDir::toNativeSeparators() is supposed to escape spaces in paths but it doesn't seem to
        // Using replace instead
        path = path.replace(" ", "\\ ");
        // Note: -l is needed to unmount while there may be components attached to the drive
        // Per umount help,
        //      -l, --lazy : detach the filesystem now, clean up things later
        std::system(QString("umount -l %1").arg(path).toStdString().c_str());
    }

    void Reboot()
    {
        sync();
        std::system("sudo shutdown -r now");
    }

    void TakeAndStorePicture(QString filename)
    {
        if (HasCamera())
        {
            if (filename.isEmpty())
            {
                filename = QString("temp_%1.jpg").arg(QDate::currentDate().toString("yyyy-MM-dd"));
                // KCB_DEBUG_TRACE("Taking/Storing image:" << filename);
            }

            QString path = QString("%1/%2").arg(KCB_IMAGE_PATH).arg(filename);
			if (QFile::exists(path))
			{
				KCB_DEBUG_TRACE(path << "exists" << "no picture taken");
				return;
			}
			
            // Issue the command to take a picture: raspistill
            // raspistill -n -w 320 -h 240 -q 50 -o <path to image files> 
            // -n no preview
			// -t timeout 1 second
			//     Note: the help says timeout is in milliseconds, but timeing tests
			//           show that it is seconds.  The default timeout is 5 seconds.
            // -w image width
            // -h image height
            // -q jpeg quality (0 .. 100)
            // -o <path to image files>
            //  & Run in background
            std::system(QString("raspistill -n -t 1 -w 160 -h 120 -q 50 -o %1").arg(path).toStdString().c_str());
        }
    }

    bool HasCamera()
    {
        // Issuing: vcgencmd get_camera
        // Outputs: supported=x detected=y, where x and y will be either 0 or 1
        // Typically: 
        //    supported=0 detected=0 means the camera is not connected or connected and not configured
        //    supported=1 detected=1 means the camera is connected and configured
        // There should not be any case where the camera is conected but not configured or not connected and configured;
        // however, such occurrences will be logged

        QString stdOut;
        QString stdErr;
        int status;

        ExecuteCommand(QString("vcgencmd"), QStringList() << QString("get_camera"), stdOut, stdErr, status);

        if (status == QProcess::CrashExit)
        {
            return false;
        }

        if (!stdOut.isEmpty())
        {
            QString trimmed = stdOut.trimmed();
            // bool equals = trimmed == "supported=1 detected=1";
            // KCB_DEBUG_TRACE(trimmed << equals);
            if (trimmed == "supported=1 detected=1")
            {
                return true;
            }
        }

        return false;
    }

    QByteArray GetImageAsByteArray(QString filename, bool delete_file)
    {
        QString path;

        //KCB_DEBUG_TRACE("filename" << filename);
        if (filename.isEmpty())
        {            
            // Find a file with format: temp_<datetime>.jpg
            QDir dir(KCB_IMAGE_PATH);
            QStringList name_filters;
            name_filters << QString("temp_%1.jpg").arg(QDate::currentDate().toString("yyyy-MM-dd"));
            QFileInfoList fil = dir.entryInfoList(name_filters, QDir::NoDotAndDotDot|QDir::Files);
            fil = dir.entryInfoList(name_filters, QDir::NoDotAndDotDot|QDir::Files);
            // KCB_DEBUG_TRACE("num files" << fil.count());

            if (fil.count() > 0)
            {
                filename = fil[0].fileName();
                // KCB_DEBUG_TRACE("filename" << filename);
            }
			else
			{
				KCB_DEBUG_TRACE("failed to locate image file");
				// Consider using a default image
				return QByteArray();
			}
        }

        path = QString("%1/%2").arg(KCB_IMAGE_PATH).arg(filename);

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) 
        {
            KCB_DEBUG_TRACE("unable to open" << path);
            return QByteArray();
        }
        QByteArray ba = file.readAll();

        if (delete_file)
        {       
            if (QFile::exists(path))
            {
                QFile::remove(path);
            }
        }

        return ba;
    }

    void BackupDatabase()
    {
        // KCB_DEBUG_ENTRY;
        (void) std::system("rm /home/pi/kcb-config/database/alpha-*.db.bak");
        QString backup_command = QString("cp /home/pi/run/Alpha.db /home/pi/kcb-config/database/alpha-%1.db.bak");
        backup_command = backup_command.arg(QDateTime::currentDateTime().toString(REPORT_FILE_FORMAT));
        (void) std::system(backup_command.toStdString().c_str());
        // KCB_DEBUG_EXIT;
    }

    static void SetDisplayPower(int value7inch, int value10inch)
    {
        // KCB_DEBUG_ENTRY;

        // KCB_DEBUG_TRACE("7 inch" << value7inch << "10 inch" << value10inch);
        // Note: Below are two commands for setting the power of the displays.  Only one display is installed
        // at a time, but they use completely different commands and do not conflict with each other.
        // Issuing both commands when only on display is present has no detrimental effects.
        QString cmd7inch = QString("sudo /home/pi/kcb-config/scripts/displayonoff.sh %1").arg(value7inch);
        QString cmd10inch = QString("vcgencmd display_power %1").arg(value10inch);
        system(qPrintable(cmd7inch));
        system(qPrintable(cmd10inch));
        // KCB_DEBUG_EXIT;
    }

    void TurnOffDisplay()
    {
        // KCB_DEBUG_ENTRY;
        display_power_state = DISP_POWER_OFF;
        SetDisplayPower(DISPLAY_OFF_7_INCH, DISPLAY_OFF_10_INCH);
        // KCB_DEBUG_EXIT;
    }

    void TurnOnDisplay()
    {
        // KCB_DEBUG_ENTRY;
        display_power_state = DISP_POWER_ON;
        SetDisplayPower(DISPLAY_ON_7_INCH, DISPLAY_ON_10_INCH);
        // KCB_DEBUG_EXIT;
    }

    bool isDisplayPowerOn()
    {
        return display_power_state == DISP_POWER_ON;
    }

    void GetScreenDimensions(int&  width, int& height)
    {
        QScreen* screen = QApplication::primaryScreen();
        // KCB_DEBUG_TRACE("available app screen" << screen->availableGeometry());
        // QScreen* gui_screen = QGuiApplication::primaryScreen();
        // KCB_DEBUG_TRACE("available gui screen" << gui_screen->availableGeometry());

        width = screen->availableGeometry().right();
        height = screen->availableGeometry().bottom();

        // KCB_DEBUG_TRACE("Screen Dimensions:" << width << "x" << height);
    }

    void GetAvailableGeometry(QRect& rect)
    {
        QScreen* screen = QApplication::primaryScreen();
        rect = screen->availableGeometry();
        // KCB_DEBUG_TRACE("Available Geometry:" << rect);
    }

    void SetWindowParams(QWidget* widget)
    {
        int width;
        int height;
        GetScreenDimensions(width, height);        
        QRect ag;
        GetAvailableGeometry(ag);

        widget->setGeometry(ag.x(), ag.y(), ag.width(), ag.height());
        widget->setMinimumSize(width, height);
        widget->setMaximumSize(width, height);
        widget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        widget->showFullScreen();
    }

    bool IsHdmiConnected()
    {
        QString stdOut;
        QString stdErr;
        int status;
        bool is_connected = false;

        ExecuteCommand(QString("tvservice"), QStringList() << QString("-s"), stdOut, stdErr, status);

        if (status == QProcess::CrashExit)
        {
            return false;
        }

        if (!stdOut.isEmpty())
        {
            // HDMI cable disconnected
            // ~ $ tvservice -s
            // state 0x120009 [HDMI DMT (87) RGB full 16:9], 848x480 @ 60.00Hz, progressive

            // HDMI cable connected
            // ~ $ tvservice -s
            // state 0x12000a [HDMI DMT (87) RGB full 16:9], 848x480 @ 60.00Hz, progressive

            QString trimmed = stdOut.trimmed();
            QStringList splits = trimmed.split(" ");
            // KCB_DEBUG_TRACE("hdmi splits" << splits);
            QString tvservice_state = splits[1];


            is_connected = tvservice_state == QString("0x12000a");
        }

        return is_connected;
    }

    void SetupDisplay()
    {
        if (KeyCodeBoxSettings::isDisplaySet())
        {
            return;
        }

        QString config_file;
        if (IsHdmiConnected())
        {
            config_file = KCBCONFIG_WAVESHARE_DISPLAY_CONFIG_FILE;
        }
        else
        {
            config_file = KCBCONFIG_OFFICIAL_DISPLAY_CONFIG_FILE;
        }

        QString command = QString("sudo cp %1/%2 %3").arg(KCBCONFIG_SETTINGS_PATH).arg(config_file).arg(BOOT_CONFIG_FILE);
        // KCB_DEBUG_TRACE(command);
        int status = std::system(command.toStdString().c_str());
        if (status != 0)
        {
            KCB_DEBUG_TRACE("system command failure:" << status);
        }

        KeyCodeBoxSettings::setDisplay();
        KCB_DEBUG_TRACE("Rebooting");
        Reboot();
    }

    void RestartNetworkInterface()
    {
        std::system(QString("ip link set eth0 down && ip link set eth0 up").toStdString().c_str());
    }

    bool isVncConnectionActive()
    {
        /* 
        This command:
            ss sport = :<port>

        returns the following:
            Netid  State      Recv-Q Send-Q   Local Address:Port       Peer Address:Port   
            tcp    ESTAB      0      281      192.168.1.146:5901      192.168.1.123:30310

        when split on newline, this will yield two entrys in the string list when VNC is active
        and one entry in the string list when VNC is not active.
        */

        QString stdOut;
        QString stdErr;
        int status;

        QString vncPort = KeyCodeBoxSettings::GetVncPort();

        ExecuteCommand(QString("ss"), QStringList() << QString("sport = :%1").arg(vncPort), stdOut, stdErr, status);

        if (status == QProcess::CrashExit)
        {
            return false;
        }

        QStringList strList;

        if (!stdOut.isEmpty())
        {
            strList = stdOut.trimmed().split("\n");
            
            // KCB_DEBUG_TRACE(strList);

        }

        return strList.count() > 1;
    }

    static void SetDhcpcdConfFile(QString content)
    {
        if (QFile::exists("/etc/dhcpcd.conf"))
        {
            QFile::remove("/etcdhcpcd.conf");
        }

        QFile dhcpcdconf("/etc/dhcpcd.conf");
        if (!dhcpcdconf.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            KCB_DEBUG_TRACE("Unable to create /etc/dhcpcd.conf");
            return;
        }

        QTextStream out(&dhcpcdconf);
        out << content;
    }

    void AddStaticAddressing(QString values)
    {
        SetDhcpcdConfFile(QString("%1%2").arg(BASE_DHCPCD_SETTINGS).arg(values));
    }

    void RemoveStaticAddressing()
    {
        SetDhcpcdConfFile(BASE_DHCPCD_SETTINGS);
    }

    QString IpAddrSubnetMaskToCidr(QString ip_addr, QString subnet_mask)
    {
        // Convert the ip address and subnet mask, e.g., 192.168.1.1, 255.255.255.0, to CIDR format, 192.168.1.1/24

        if (subnet_mask.isEmpty() || !subnet_mask.contains(".") || ip_addr.isEmpty() || !ip_addr.contains("."))
        {
            KCB_DEBUG_TRACE("Invalid ip address and/or subnet mask");
            return ip_addr;
        }

        QStringList mask_split = subnet_mask.split(".");
        unsigned int mask_int = 0;
        for (int ii = 3; ii >= 0; --ii)
        {            
            mask_int |= mask_split[ii].toInt() << (ii * 8);
        }

        // Count the bits set in the number
        unsigned int num_bits = countSetBits(mask_int);

        return QString("%1/%2").arg(ip_addr).arg(QString::number(num_bits));
    }

    static QList<QNetworkInterface> GetQualifiedInterfaces()
    {        
        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

        QList<QNetworkInterface> qualified_ifaces;

        for (int i=0; i < ifaces.size(); i++)
        {
            unsigned int flags = ifaces[i].flags();
            bool isLoopback = (bool)(flags & QNetworkInterface::IsLoopBack);
            bool isP2P = (bool)(flags & QNetworkInterface::IsPointToPoint);
            bool isRunning = (bool)(flags & QNetworkInterface::IsRunning);
            QString name = ifaces[i].name();

            bool ignore_usb_networks = name.contains("usb");
            bool ignore_non_running = !isRunning;
            bool ignore_non_loopback_or_virtual = !ifaces[i].isValid() || isLoopback || isP2P;

            if ( ignore_usb_networks || ignore_non_running || ignore_non_loopback_or_virtual)
            {
                continue;
            }

            qualified_ifaces.append(ifaces[i]);
        }

        return qualified_ifaces;
    }

    static QList< QPair<QString, unsigned int> > GetPossibleMatches(QList<QNetworkInterface> qualified_ifaces)
    {
        QList< QPair<QString, unsigned int> > possibleMatches;

        foreach (auto iface, qualified_ifaces)
        {
            QList<QNetworkAddressEntry> addresses = iface.addressEntries();

            foreach (auto addrEntry, addresses)
            {
                QHostAddress addr = addrEntry.ip();

                bool ignore_local_host = addr == QHostAddress::LocalHost;
                bool ignore_nonipv4_address = !addr.toIPv4Address();
                bool ignore_null_address = addr.toString().isEmpty();
                
                if ( ignore_local_host || ignore_nonipv4_address || ignore_null_address)
                {
                    continue;
                }

                possibleMatches.append(QPair<QString, unsigned int>(addr.toString(), iface.flags()));
            }
        }

        return possibleMatches;
    }

    void GetIpAddressAndStatus(QString &ip_address, bool &can_ping, bool &can_multicast)
    {
        QList<QNetworkInterface> qualified_ifaces = GetQualifiedInterfaces();
        QList< QPair<QString, unsigned int> > possibleMatches = GetPossibleMatches(qualified_ifaces);

        bool is_match = possibleMatches.length() == 1;
        bool interface_isup = false;
        can_multicast = false;
        can_ping = false;
        ip_address = "";

        if (is_match)
        {    
            QPair<QString, unsigned int>ip_display(possibleMatches[0]);
            interface_isup = ip_display.second & QNetworkInterface::IsUp;
            can_multicast = ip_display.second & QNetworkInterface::CanMulticast;
            if (interface_isup)
            {
                QString gateway = kcb::GetGatewayAddress();
                can_ping = kcb::FPingAddress(gateway);
                ip_address = ip_display.first;
            }
        }
    }

    QString GetSystemId()
    {
        return QString("keycodebox-%1").arg(GetRpiSerialNumber());
    }

    void EnableInternetTime()
    {
        // The following code works with init.d ntp service
        std::system("sudo /etc/init.d/ntp stop");
        std::system("sudo ntpd -s");
        std::system("sudo /etc/init.d/ntp start");
        KCB_DEBUG_TRACE("Internet time enabled");

        // The following code works with systemd via timedatectl
        // See this thread and kcb-config/scripts/kcb-ntpsetup.sh
        // info on systemd configuration.
        // When the image is updated to remove init.d and use only systemd
        // this code can be incorporated.
        // std::system("sudo timedatectl set-ntp true");
    }

    void DisableInternetTime()
    {
        std::system("sudo /etc/init.d/ntp stop");
        // std::system("sudo timedatectl set-ntp false");
        KCB_DEBUG_TRACE("Internet time disabled");
    }

    void SetDateTime(const QDateTime& datetime)
    {
        QString sysDate("sudo date " + datetime.toString("MMddhhmmyyyy.ss"));
        KCB_DEBUG_TRACE("Date/Time Update" << sysDate);
        std::system(sysDate.toStdString().c_str());
        std::system("sudo hwclock --systohc");
        KCB_DEBUG_TRACE("Date/Time committed to RTC");
    }

    void SetTimeZone(const QString& timezone)
    {
        QString unlink = QString("sudo unlink /etc/localtime");
        QString link = QString("sudo ln -s /usr/share/zoneinfo/%1 %2").arg(timezone).arg(QString(" /etc/localtime"));
        std::system(unlink.toStdString().c_str());
        std::system(link.toStdString().c_str());
        KCB_DEBUG_TRACE("Timezone set");
    }
}
