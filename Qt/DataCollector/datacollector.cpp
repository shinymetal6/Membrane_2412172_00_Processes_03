#include "datacollector.h"
#include "ui_datacollector.h"
#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QThread>

DataCollector::DataCollector(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DataCollector)
{
    ui->setupUi(this);
    ui->data_frame->setEnabled(false);
    ui->Power_pushButton->setEnabled(false);
    ui->Scan_pushButton->setText("Scan");
    timer0Id = 0;

}

DataCollector::~DataCollector()
{
    delete ui;
}

QByteArray DataCollector::serial_tx( QByteArray hex_line)
{
QPixmap redled (":/ledred.png");
QPixmap greenled(":/ledgreen.png");
QByteArray reply;
    if ( serial_started == 0 )
        return "1";
    serial.flush();
    serial.write(hex_line);
    if(serial.waitForReadyRead(WAIT_REPLY))
    {
        reply = serial.readAll();
    }
    else
    {
        serial.flush();
        reply = "";
    }
    qApp->processEvents();

    return reply;
}

void DataCollector::on_Port_comboBox_currentTextChanged(const QString &arg1)
{
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    serial.close();
    if ( arg1 == "Invalid")
    {
        ui->Comm_label->setPixmap(redled);
        ui->data_frame->setEnabled(false);
        ui->statusbar->showMessage("Serial port closed");
        ui->Power_pushButton->setEnabled(false);
        return;
    }
    serial_started = 0;
    serial.setPortName(arg1);
    if(serial.open(QIODevice::ReadWrite))
    {
        if(!serial.setBaudRate(QSerialPort::Baud115200))
        {
            ui->Comm_label->setPixmap(redled);
            ui->statusbar->showMessage(arg1+" : "+serial.errorString());
            ui->data_frame->setEnabled(false);
            ui->Power_pushButton->setEnabled(false);
        }
        else
        {
            ui->Comm_label->setPixmap(greenled);
            serial_started = 1;
            ui->statusbar->showMessage("Serial port "+arg1+" opened");
            serial.setReadBufferSize (1024);
            ui->Power_pushButton->setEnabled(true);
        }
    }
    else
    {
        ui->Comm_label->setPixmap(redled);
        ui->statusbar->showMessage(arg1+" : "+serial.errorString());
        ui->data_frame->setEnabled(false);
    }
}


void DataCollector::on_Power_pushButton_clicked()
{
    QByteArray reply;
    QString cmd;
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    if ( ui->Power_pushButton->text() == "Power ON" )
    {
        cmd = "<P>";
        ui->Power_pushButton->setText("Power OFF");
        ui->data_frame->setEnabled(true);
    }
    else
    {
        ui->Power_pushButton->setText("Power ON");
        cmd = "<O>";
        ui->data_frame->setEnabled(false);
    }

    serial.flush();
    qDebug()<< cmd;
    serial_tx(cmd.toUtf8());
}

void DataCollector::timerEvent(QTimerEvent *event)
{
    QByteArray reply;
    QByteArray Command;
    QByteArray qline;
    QByteArray qsensor;
    int dsc,sensor;

    if ( event->timerId() == timer0Id )
    {
        for(dsc=1;dsc<NUM_DSC+1;dsc++)
        {
            for(sensor=FIRST_WSENSOR;sensor<LAST_WSENSOR+2;sensor++)
            {
                qline.setNum(dsc);
                qsensor.setNum(sensor);
                Command = "<A "+qline+" "+qsensor+">";
                if ( (reply = serial_tx(Command)) != "" )
                {
                    int sensor_debug = ui->QSENSOR_DEBUG_comboBox->currentText().toInt();// 0 if All
                    int line_debug = ui->QLINE_DEBUG_comboBox->currentText().toInt(); // 0 if All
                    if (( dsc == line_debug) && ( sensor == sensor_debug))
                        qDebug()<< qline << " " << qsensor << " " << reply;
                }
            }
        }
    }
}

void DataCollector::on_Scan_pushButton_clicked()
{
    QByteArray reply;
    QByteArray Command;

    if ( ui->Scan_pushButton->text() == "Stop")
    {
        ui->Scan_pushButton->setText("Scan");
        killTimer(timer0Id);
        ui->statusbar->showMessage("Stopped");
        Command = "<H>";
        if ( (reply = serial_tx(Command)) != "1" )
        {
            qDebug()<< "Received";
        }
    }
    else
    {
        ui->Scan_pushButton->setText("Stop");
        timer0Id = startTimer(1000);
        Command = "<S>";
        if ( (reply = serial_tx(Command)) != "1" )
        {
            qDebug()<< "Received";
        }
        ui->statusbar->showMessage("Running Scan");
    }

}

