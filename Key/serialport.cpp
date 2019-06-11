#include "serialport.h"

#include <QSerialPort>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include "kcbcommon.h"

SerialPort::SerialPort(const QString& portName, int baudRate, int writeTimeout, int readTimeout) :
    QObject(nullptr),
    m_writeTimeout(writeTimeout),
    m_readTimeout(readTimeout),
    m_port(*new QSerialPort(portName))
{
    m_port.setBaudRate(baudRate);

    if (!m_port.open(QIODevice::ReadWrite)) 
    {
        KCB_CRITICAL_TRACE(QString("Can't open %1, error code %2").arg(portName).arg(m_port.error()));
    }
}

SerialPort::~SerialPort()
{
}

int SerialPort::WriteData(const QByteArray &request)
{
    // KCB_DEBUG_TRACE("WriteData:" << qPrintable(request.toHex()));

    qint16 num_bytes = m_port.write(request);
    if (m_port.waitForBytesWritten(m_writeTimeout)) 
    {
        // KCB_DEBUG_TRACE("WriteData:" << num_bytes << "written to serial port");
    }
    else
    {
        KCB_CRITICAL_TRACE("Serial port timeout");
    }
    
    return (int) num_bytes;
}

int SerialPort::ReadData(QByteArray& response)
{
    // read response
    if (m_port.waitForReadyRead(m_readTimeout)) 
    {
        response = m_port.readAll();
        while (m_port.waitForReadyRead(m_readTimeout))
        {
            response += m_port.readAll();
        }

        // KCB_DEBUG_TRACE("ReadData:" << response.toHex());
    }
    else
    {
        KCB_CRITICAL_TRACE("Serial port timeout");
    }
    
    // KCB_DEBUG_TRACE("ReadData size: " << response.size());
    return response.size();
}