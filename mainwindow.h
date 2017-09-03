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
    QTimer *gamepadPaintTimer;

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
    void paintGamepad(void);

    void on_intervalGamepad_editingFinished();

    void connectStatusChanged();
    void connectDevice(int deviceId);
    void disconnectDevice(int deviceId);
    void axisLeftXShow(double value);
    void axisLeftYShow(double value);
    void axisRightXShow(double value);
    void axisRightYShow(double value);
    void buttonAShow(bool value);
    void buttonBShow(bool value);
    void buttonXShow(bool value);
    void buttonYShow(bool value);
    void buttonL1Show(bool value);
    void buttonR1Show(bool value);
    void buttonL2Show(double value);
    void buttonR2Show(double value);
    void buttonSelectShow(bool value);
    void buttonStartShow(bool value);
    void buttonL3Show(bool value);
    void buttonR3Show(bool value);
    void buttonUpShow(bool value);
    void buttonDownShow(bool value);
    void buttonLeftShow(bool value);
    void buttonRightShow(bool value);

    void on_pb_sendGamePadData_clicked();

private:
    // gamepad
    bool needRepaintAxis = false;


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
