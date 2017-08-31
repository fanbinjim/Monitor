#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QProxyStyle>
#include <QGraphicsWidget>
#include <QMessageBox>
#include <QtGamepad/QtGamepad>



class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
        const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
            s.transpose();
            s.rwidth() = 90; // 设置每个tabBar中item的大小
            s.rheight() = 44;
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel) {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
                QRect allRect = tab->rect;

                if (tab->state & QStyle::State_Selected) {
                    painter->save();
                    painter->setPen(0x89cfff);
                    painter->setBrush(QBrush(0x89cfff));
                    painter->drawRect(allRect.adjusted(6, 6, -6, -6));
                    painter->restore();
                }
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);
                if (tab->state & QStyle::State_Selected) {
                    painter->setPen(0xf8fcff);
                }
                else {
                    painter->setPen(0x5d5d5d);
                }

                painter->drawText(allRect, tab->text, option);
                return;
            }
        }

        if (element == CE_TabBarTab) {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
};






MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setTabPosition(QTabWidget::West);
    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);

    ui->rx_ascii->setChecked(true);
    ui->rx_hex->setChecked(false);
    ui->tx_ascii->setChecked(true);
    ui->tx_hex->setChecked(false);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->portNameBox->addItem(info.portName());
    }

    initGamepad();

}

MainWindow::~MainWindow()
{
    delete ui;
}

char hexBase[16] = {'0','1','2','3',
                     '4','5','6','7',
                     '8','9','A','B',
                     'C','D','E','F'};

QString intoh(int n)
{
    QString str;
    //str.append("0x");
    str.append(hexBase[n/16]);
    str.append(hexBase[n%16]);
    return str;
}
int htoint(QString str)
{
    int result = 0;
    for(int i = 0; i < str.length(); i ++)
    {
        result *= 16;

        if(str[i] >= '0' && str[i] <= '9')
            result += str[i].toLatin1() - '0';
        else if(str[i] >= 'a' && str[i] <= 'f')
            result += str[i].toLatin1() - 'a';
        else if(str[i] >= 'A' && str[i] <= 'F')
            result += str[i].toLatin1() - 'A' + 10;
    }
    if(result > 255)
        result = 255;
    return result;
}

int dtoint(QString str)
{
    int result = 0;
    for(int i = 0; i < str.length(); i ++)
    {
        result *= 10;
        if(str[i] >= '0' && str[i] <= '9')
            result += str[i].toLatin1() - '0';
        else
            break;
    }
    if(result > 255)
        result = 255;
    return result;
}

int btoint(QString str)
{
    int result = 0;
    for(int i = 0; i < str.length(); i ++)
    {
        result *= 2;
        if(str[i] >= '0' && str[i] <= '1')
            result += str[i].toLatin1() - '0';
    }
    if(result >= 255)
        result = 255;
    return result;
}

void MainWindow::showRxData(QByteArray rxData)
{
    if(ui->tab->isVisible() && ui->isShowRxData->isChecked())
    {
        ui->showData->moveCursor(QTextCursor::End);

        if(ui->rx_ascii->isChecked())
        {
            if(ui->isAutoReturnLine->isChecked())
                ui->showData->appendPlainText(QString(rxData));
            else
                ui->showData->insertPlainText(QString(rxData));
        }
        else if(ui->rx_hex->isChecked())
        {
            QString str;
            for(int i = 0; i < rxData.size(); i++)
            {
                str.append(intoh(rxData[i]));
                str.append(" ");
            }

            if(ui->isAutoReturnLine->isChecked())
                ui->showData->appendPlainText(str);
            else
                ui->showData->insertPlainText(str);
        }

        ui->showData->moveCursor(QTextCursor::End);
    }
}


void MainWindow::on_connect_clicked()
{


    if(serialPort.isOpen())
    {
        serialPort.close();
        serialThread->exit();
        ui->connect->setChecked(false);
        ui->statusBar->showMessage("关闭串口", 2000);
    }
    else
    {
        qint32 baudRate = ui->baudRateBox->currentText().toInt();

        serialPort.setBaudRate(baudRate);
        serialPort.setDataBits(QSerialPort::Data8);
        serialPort.setStopBits(QSerialPort::OneStop);
        serialPort.setPortName(ui->portNameBox->currentText());

        if(serialPort.open(QSerialPort::ReadWrite))
        {
            serialPort.close();

            serialThread = new SerialThread(&serialPort);
            serialThread->start();

            connect(serialThread, SIGNAL(gotSomeBytes(QByteArray)), this, SLOT(showRxData(QByteArray)));



            ui->connect->setChecked(true);
            ui->statusBar->showMessage("打开串口成功");

            // 打开监听线程

        }
        else
        {
            ui->connect->setChecked(false);
            ui->statusBar->showMessage("打开串口失败");
        }

    }


}

void MainWindow::on_rx_ascii_clicked()
{
    if(ui->rx_ascii->isChecked())
    {
        ui->rx_hex->setChecked(false);
    }
    else
    {
        ui->rx_hex->setChecked(true);
    }
}

void MainWindow::on_rx_hex_clicked()
{
    if(ui->rx_hex->isChecked())
    {
        ui->rx_ascii->setChecked(false);
    }
    else
    {
        ui->rx_ascii->setChecked(true);
    }
}

void MainWindow::on_clearButton_clicked()
{
    ui->showData->clear();
}

void MainWindow::on_tx_ascii_clicked()
{
    if(ui->tx_ascii->isChecked())
    {
        ui->tx_hex->setChecked(false);
    }
    else
    {
        ui->tx_hex->setChecked(true);
    }
}

void MainWindow::on_tx_hex_clicked()
{
    if(ui->tx_hex->isChecked())
    {
        ui->tx_ascii->setChecked(false);
    }
    else
    {
        ui->tx_ascii->setChecked(true);
    }
}

void MainWindow::on_sendData_clicked()
{
    QByteArray tx_data_array;

    if(ui->tab->isVisible())
    {
        QStringList strList;
        if(ui->tx_ascii->isChecked())
        {
            tx_data_array = ui->tx_data->toPlainText().toUtf8();
        }
        else if(ui->tx_hex->isChecked())
        {
            tx_data_array.clear();

            strList = ui->tx_data->toPlainText().split(" ");
            uint8_t number = 0;
            foreach( QString str, strList)
            {
                number = htoint(str);
                tx_data_array.append(number);
            }
        }
        else if(ui->tx_decimal->isChecked())
        {
            tx_data_array.clear();
            strList = ui->tx_data->toPlainText().split(" ");
            uint8_t number = 0;
            foreach( QString str, strList)
            {
                number = dtoint(str);
                tx_data_array.append(number);
            }
        }
        else if(ui->tx_binary->isChecked())
        {
            tx_data_array.clear();
            strList = ui->tx_data->toPlainText().split(" ");
            uint8_t number = 0;
            foreach (QString str, strList) {
                number = btoint(str);
                tx_data_array.append(number);
            }
        }
    }
    if(ui->isShowTxData->isChecked())
    {
        ui->showData->appendPlainText(ui->tx_data->toPlainText());
    }
    serialPort.write(tx_data_array);
}









void MainWindow::on_clearTxData_clicked()
{
    ui->tx_data->clear();
}



void MainWindow::on_isSendGamepad_clicked()
{
    if(ui->connect->isChecked())
    {

            gamepadTimer = new QTimer(this);
            connect(gamepadTimer, SIGNAL(timeout()), this, SLOT(sendGamepad()));
            gamepadTimer->start(20);

    }
    else
    {
        ui->statusBar->showMessage("请先打开串口");
    }

}

void MainWindow::sendGamepad()
{
    static qint64 count = 0;
    if(ui->connect->isChecked())
    {
        qDebug()<<count ++;
    }
    else
    {
        ui->sendGamePad->setChecked(false);
        gamepadTimer->stop();
    }

}

void MainWindow::on_sendGamePad_clicked()
{
    if(ui->connect->isChecked())
    {
        if(ui->sendGamePad->isChecked())
        {
            gamepadTimer = new QTimer(this);
            connect(gamepadTimer, SIGNAL(timeout()), this, SLOT(sendGamepad()));
            gamepadTimer->start(ui->intervalGamepad->value());
        }
        else
        {
            gamepadTimer->stop();
        }
    }
    else
    {
        ui->statusBar->showMessage("请先打开串口");
    }
}

void MainWindow::on_intervalGamepad_editingFinished()
{
    if(ui->sendGamePad->isChecked())
    {
        gamepadTimer->setInterval(ui->intervalGamepad->value());
    }
}


void MainWindow::initGamepad()
{
    connect(QGamepadManager::instance(), SIGNAL(connectedGamepadsChanged()), this, SLOT(connectStatusChanged()));
    connect(QGamepadManager::instance(), SIGNAL(gamepadConnected(int)), this, SLOT(connectDevice(int)));
    connect(QGamepadManager::instance(), SIGNAL(gamepadDisconnected(int)), this, SLOT(disconnectDevice(int)));
    connect(QGamepadManager::instance(), SIGNAL(gamepadAxisEvent(int,QGamepadManager::GamepadAxis,double)), this, SLOT(axisDataChange(int,QGamepadManager::GamepadAxis,double)));
    connect(QGamepadManager::instance(), SIGNAL(gamepadButtonPressEvent(int,QGamepadManager::GamepadButton,double)), this, SLOT(buttonPress(int,QGamepadManager::GamepadButton,double)));
    connect(QGamepadManager::instance(), SIGNAL(gamepadButtonReleaseEvent(int,QGamepadManager::GamepadButton)), this, SLOT(buttonRelease(int,QGamepadManager::GamepadButton)));


//    connect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected, this,
//        [](int deviceId) { qDebug() << "gamepad connected:" << deviceId; });


//    connect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected, this,
//        [](int deviceId) { qDebug() << "gamepad disconnected:" << deviceId; });


//    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this,
//        [](int deviceId, QGamepadManager::GamepadButton button, double value) { qDebug() << "button press event:" << deviceId << button << value; });


//    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this,
//        [](int deviceId, QGamepadManager::GamepadButton button) { qDebug() << "button release event:" << deviceId << button; });


//    connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent, this,
//        [](int deviceId, QGamepadManager::GamepadAxis axis, double value) {
//        qDebug() << "axis event:" << deviceId << axis << value;


//    });


    connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, this,
        [](int deviceId, QGamepadManager::GamepadButton button) { qDebug() << "button configured:" << deviceId << button; });


    connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured, this,
        [](int deviceId, QGamepadManager::GamepadAxis axis) { qDebug() << "axis configured:" << deviceId << axis; });


    connect(QGamepadManager::instance(), &QGamepadManager::configurationCanceled, this,
        [](int deviceId) { qDebug() << "configuration canceled:" << deviceId; });
}


void MainWindow::connectStatusChanged()
{
    ui->gamepadText->append("hello");
}

void MainWindow::connectDevice(int deviceId)
{
    QString str = "gamepad connected:";
    str.append(QString::number(deviceId));
    ui->gamepadText->append(str);
}

void MainWindow::disconnectDevice(int deviceId)
{
    QString str = "gamepad disconnected:";
    str.append(QString::number(deviceId));
    ui->gamepadText->append(str);
}

void MainWindow::axisDataChange(int deviceId, QGamepadManager::GamepadAxis axis, double value)
{
    QString str = "axis data change: ";
    str.append("deviceId:");
    str.append(QString::number(deviceId));
    str.append(" axis:");
    str.append(QString::number(axis));
    str.append(" value:");
    str.append(QString::number(value));
    ui->gamepadText->append(str);

    if(axis == QGamepadManager::GamepadAxis::AxisLeftY)
    {
        ui->verticalSlider->setValue(value*100);
    }

}

void MainWindow::buttonPress(int deviceId, QGamepadManager::GamepadButton button, double value)
{
    QString str = "button press: ";
    str.append("deviceId:");
    str.append(QString::number(deviceId));
    str.append(" button");
    str.append(QString::number(button));
    str.append(" value:");
    str.append(QString::number(value));
    ui->gamepadText->append(str);
}

void MainWindow::buttonRelease(int deviceId, QGamepadManager::GamepadButton button)
{
    QString str = "button release: ";
    str.append("deviceId:");
    str.append(QString::number(deviceId));
    str.append(" button");
    str.append(QString::number(button));
    ui->gamepadText->append(str);
}
