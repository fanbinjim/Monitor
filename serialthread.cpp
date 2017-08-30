#include "serialthread.h"


SerialThread::SerialThread(QSerialPort *serial)
{
    serialport = serial;
    serial->open(QSerialPort::ReadWrite);
}

void SerialThread::run()
{
    QByteArray rxData;
    qint64 lenOld = 0;

    while(true)
    {
        qint64 rx_data_size = serialport->size();
        if((lenOld == rx_data_size) && (lenOld > 0))
        {
            rxData = serialport->readAll();

            emit this->gotSomeBytes(rxData);
        }
        lenOld = rx_data_size;
        msleep(10);
    }
}
