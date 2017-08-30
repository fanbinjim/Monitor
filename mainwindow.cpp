#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QProxyStyle>
#include <QGraphicsWidget>
#include <QMessageBox>




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
