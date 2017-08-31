#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include <QThread>

#include <QtGamepad>

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
    QGamepad *gamepad;

    QTimer *gamepadTimer;

    void initGamepad(void);

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

    void on_sendData_clicked();

    void on_clearTxData_clicked();

    void on_isSendGamepad_clicked();

    void sendGamepad(void);

    void on_sendGamePad_clicked();

    void on_intervalGamepad_editingFinished();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
