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
    ui->tab_5->setBackgroundRole(QPalette::ColorRole::Dark);
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
            if(gamepad->isConnected())
            {
                gamepadTimer = new QTimer(this);
                connect(gamepadTimer, SIGNAL(timeout()), this, SLOT(sendGamepad()));
                gamepadTimer->start(ui->intervalGamepad->value());
            }
            else
            {
                ui->statusBar->showMessage("没有可用的手柄");
                ui->sendGamePad->setChecked(false);
            }
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

    if(!gamepad)
    {
        gamepad = new QGamepad(deviceId);

        connect(gamepad,SIGNAL(axisLeftXChanged(double)), this, SLOT(axisLeftXShow(double)));
        connect(gamepad,SIGNAL(axisLeftYChanged(double)), this, SLOT(axisLeftYShow(double)));
        connect(gamepad,SIGNAL(axisRightXChanged(double)), this, SLOT(axisRightXShow(double)));
        connect(gamepad,SIGNAL(axisRightYChanged(double)), this, SLOT(axisRightYShow(double)));
        connect(gamepad,SIGNAL(buttonUpChanged(bool)), this, SLOT(buttonUpShow(bool)));
        connect(gamepad,SIGNAL(buttonDownChanged(bool)), this, SLOT(buttonDownShow(bool)));
        connect(gamepad,SIGNAL(buttonLeftChanged(bool)), this, SLOT(buttonLeftShow(bool)));
        connect(gamepad,SIGNAL(buttonRightChanged(bool)), this, SLOT(buttonRightShow(bool)));
        connect(gamepad, SIGNAL(buttonL1Changed(bool)), this, SLOT(buttonL1Show(bool)));
        connect(gamepad, SIGNAL(buttonL2Changed(double)), this, SLOT(buttonL2Show(double)));
    }
}

void MainWindow::disconnectDevice(int deviceId)
{
    QString str = "gamepad disconnected:";
    str.append(QString::number(deviceId));
    ui->gamepadText->append(str);
    if(gamepad)
        delete gamepad;
}

void MainWindow::axisLeftXShow(double value)
{
    ui->le_l_axis_x->setText(QString::number(value));
}

void MainWindow::axisLeftYShow(double value)
{
    ui->le_l_axis_y->setText(QString::number(value));
}

void MainWindow::axisRightXShow(double value)
{
    ui->le_r_axis_x->setText(QString::number(value));
}

void MainWindow::axisRightYShow(double value)
{
    ui->le_r_axis_y->setText(QString::number(value));
}

void MainWindow::buttonL1Show(bool value)
{
    ui->pb_l1->setChecked(value);
}

void MainWindow::buttonL2Show(double value)
{
    ui->vs_l2->setValue(value * 100);
}

void MainWindow::buttonUpShow(bool value)
{
    ui->pb_up->setChecked(value);
}

void MainWindow::buttonDownShow(bool value)
{
    ui->pb_down->setChecked(value);
}

void MainWindow::buttonLeftShow(bool value)
{
    ui->pb_left->setChecked(value);
}

void MainWindow::buttonRightShow(bool value)
{
    ui->pb_right->setChecked(value);
}
