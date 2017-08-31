#include "serialthread.h"


SerialThread::SerialThread(QSerialPort *serial)
{
    serialport = serial;
    if(!serial->open(QSerialPort::ReadWrite))
    {
        this->exit();
    }
}

void SerialThread::run()
{
    QByteArray rxData;
    qint64 rx_data_size_old = 0;

    while(true)
    {
        qint64 rx_data_size = serialport->size();
        if((rx_data_size_old == rx_data_size) && (rx_data_size_old > 0))
        {
            rxData = serialport->readAll();

            emit this->gotSomeBytes(rxData);
        }
        rx_data_size_old = rx_data_size;
        msleep(10);
    }
}
