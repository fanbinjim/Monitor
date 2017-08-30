#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QByteArray>
#include <QtSerialPort>

class SerialThread : public QThread
{
    Q_OBJECT
public:
    SerialThread(QSerialPort *serialPort);
    QSerialPort *serialport;
protected:
    void run();
signals:
    void gotSomeBytes(QByteArray bytes);
};

#endif // SERIALTHREAD_H
