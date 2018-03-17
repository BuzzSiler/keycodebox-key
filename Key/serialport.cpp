#include "serialport.h"

#include <QSerialPort>
#include <QDebug>
#include <QString>
#include <QByteArray>

SerialPort::SerialPort(const QString& portName, int baudRate, int writeTimeout, int readTimeout) :
    QObject(nullptr),
    m_writeTimeout(writeTimeout),
    m_readTimeout(readTimeout),
    m_port(*new QSerialPort(portName))
{
    m_port.setBaudRate(baudRate);

    if (!m_port.open(QIODevice::ReadWrite)) 
    {
        qCritical() << QString("Can't open %1, error code %2").arg(portName).arg(m_port.error());
    }        
}

SerialPort::~SerialPort()
{
}

int SerialPort::WriteData(const QByteArray &request)
{
    // write request
    qDebug() << "WriteData:" << qPrintable(request.toHex());

    qint16 num_bytes = m_port.write(request);
    if (m_port.waitForBytesWritten(m_writeTimeout)) 
    {
        qDebug() << "WriteData:" << num_bytes << "written to serial port";
    }
    else
    {
        qCritical() << "Timeout occurred while writing to serial port";
    }        
    
    return (int) num_bytes;
}

int SerialPort::ReadData(QByteArray& response)
{
    // read response
    if (m_port.waitForReadyRead(m_readTimeout)) 
    {
        response = m_port.readAll();
        qDebug() << "ReadData size: " << response.size();
        while (m_port.waitForReadyRead(m_readTimeout))
        {
            response += m_port.readAll();
            qDebug() << "ReadData size: " << response.size();
        }

        qDebug() << "ReadData:" << response.toHex();
    }
    else
    {
        qCritical() << "Timeout occurred while reading from serial port";
    }
    
    qDebug() << "ReadData size: " << response.size();
    return response.size();
}