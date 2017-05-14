#ifndef CSERIALCONTROLLER_H
#define CSERIALCONTROLLER_H

#include <map>
#include <QtCore>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/serial_port_base.hpp>
#include <hidapi/hidapi.h>

using namespace boost;

/**
 * @brief The CUSBController class
 *   Determines USB attached hardware.
 *   Determines the type of device attached to each port.
 *     - signals admin-setup request for input if serial ports undetermined.
 */
class CUSBController : public QObject
{
    Q_OBJECT

    typedef struct STPORT {
        asio::io_service    _io;
        asio::serial_port   *_pport;
    } stPort, *pstPort;

    std::map<std::string, pstPort>   mapPorts;   // Map a system port name to the port.
    typedef std::map<std::string, pstPort>::iterator tyitorMap;


    void initController();

    void ExtractCommandOutput(FILE *pF, std::string &rtnStr);
public:
    CUSBController();
    ~CUSBController();

    int CountDevicePorts(QString deviceType, QString filterString);
    std::string getDevicePortString(QString deviceType, QString filterString, QString findDeviceString);
    std::string getDevicePortString(QString deviceType, QString filterString, QString findDeviceString, int nFindPos);

    asio::serial_port* getPortForDevice( std::string portName,
            std::string sDevice,
            asio::serial_port_base::baud_rate baud=asio::serial_port_base::baud_rate(19200),
            asio::serial_port_base::flow_control flow = asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none),
            asio::serial_port_base::parity parity = asio::serial_port_base::parity(asio::serial_port_base::parity::none),
            asio::serial_port_base::stop_bits stops = asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one),
            asio::serial_port_base::character_size charSize = asio::serial_port_base::character_size(8) );

signals:
    void __SerialPortChanged();
    void __USBDiskChanged();

public slots:

private:
    asio::serial_port *findPortByPortName(std::string portName);
    asio::serial_port *createPort(std::string portName, std::string sDevice,
            asio::serial_port_base::baud_rate baud,
            asio::serial_port_base::flow_control flow, asio::serial_port_base::parity parity,
            asio::serial_port_base::stop_bits stops, asio::serial_port_base::character_size charSize);

};


#endif // CSERIALCONTROLLER_H
