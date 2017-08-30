#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include <QThread>

#include "serialthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QSerialPort serialPort;
    SerialThread *serialThread;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connect_clicked();
    void showRxData(QByteArray rxData);

    void on_rx_ascii_clicked();

    void on_rx_hex_clicked();

    void on_clearButton_clicked();

    void on_tx_ascii_clicked();

    void on_tx_hex_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
