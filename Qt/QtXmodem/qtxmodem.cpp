#include "qtxmodem.h"
#include "ui_qtxmodem.h"
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
#include <QCryptographicHash>

QtXmodem::QtXmodem(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QtXmodem)
{
    ui->setupUi(this);
}

QtXmodem::~QtXmodem()
{
    delete ui;
}

int QtXmodem::serial_tx( QByteArray hex_line)
{
QPixmap redled (":/ledred.png");
QPixmap greenled(":/ledgreen.png");
QByteArray reply;

    if ( serial_started == 0 )
        return 1;
    serial.write(hex_line);
    return 1;
}

int QtXmodem::serial_rx( int do_echo)
{
QPixmap redled (":/ledred.png");
QPixmap greenled(":/ledgreen.png");
QByteArray reply;

    if(serial.waitForReadyRead(WAIT_REPLY))
    {
        reply = serial.readAll();
        const char *data = reply.data();
        if ( do_echo )
            qDebug()<< reply;
        return data[0];
    }
    qDebug()<< "RX timeout";
    serial.flush();

    return 0x41;
}


void QtXmodem::on_Port_comboBox_currentTextChanged(const QString &arg1)
{
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    serial.close();
    if ( arg1 == "Invalid")
    {
        ui->Comm_label->setPixmap(redled);
        ui->Fileframe->setEnabled(false);
        ui->statusbar->showMessage("Serial port closed");
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
            ui->Fileframe->setEnabled(false);
        }
        else
        {
            ui->Comm_label->setPixmap(greenled);
            serial_started = 1;
            ui->statusbar->showMessage("Serial port "+arg1+" opened");
            serial.setReadBufferSize (1024);
            ui->Fileframe->setEnabled(true);
        }
    }
    else
    {
        ui->Comm_label->setPixmap(redled);
        ui->statusbar->showMessage(arg1+" : "+serial.errorString());
        ui->Fileframe->setEnabled(false);
    }
}

void QtXmodem:: create_buf_and_tx(char    *data)
{
#define SLEEP_HERE   10
    QByteArray ba4(QByteArray::fromRawData(data, 132));
    serial.flush();
    serial_tx(ba4);
}

void QtXmodem::download_binary(void)
{
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");
    char    data[132];
    QByteArray reply;
    int i,retry=10,rx_data;
    int s_unit;
    int index=0;
    int block_number;
    int csum;

    ui->statusbar->showMessage("Downloading "+bin_filename);
    block_number = 1;
    ui->Flashing_label->setPixmap(redled);
    s_unit = file_size/100;
    ui->download_progressBar->setValue(0);

    index=0;
    while ( index < file_size)
    {
        retry=10;
        data[0] = 0x01;
        data[1] = block_number;
        data[2] = 255 - block_number;
        block_number++;
        if ( block_number == 0 )
            block_number = 1;
        csum = 0;
        for(i=0;i<128;i++,index++)
        {
            if ( index < file_size )
                data[i+3] = blob[index];
            else
                data[i+3] = 0;
            csum += data[i+3];
        }
        data[131] = csum;

        serial.flush();
        create_buf_and_tx(data);
        serial.flush();
        while ( (rx_data = serial_rx(0)) != 0x06 )
        {
            ui->statusbar->showMessage("Retry");
            qDebug()<<"Retry on Ack, block_number "<< block_number<<" data "<<rx_data;
            serial.flush();
            create_buf_and_tx(data);
            retry--;
            if ( retry == 0 )
            {
                ui->statusbar->showMessage(bin_filename+" aborted download");
                qDebug()<<bin_filename<<" aborted download";
                return;
            }
        }
        ui->download_progressBar->setValue(index/s_unit);
    }
    data[0] = 0x04;
    QByteArray ba1(QByteArray::fromRawData(data, 1));
    serial_tx(ba1);
    ui->statusbar->showMessage(bin_filename+" downloaded");
    ui->download_progressBar->setValue(100);
    qDebug()<<bin_filename<<" downloaded";
    ui->Flashing_label->setPixmap(greenled);
}

void QtXmodem::hash_file()
{
        QFileInfo fi(filename);
        QString base = fi.completeBaseName() + "." +fi.completeSuffix();

        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        file.close();

        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(data);
        hashStr = hash.result().toHex();
        qDebug() << hashStr;
}

void QtXmodem::on_SelectFile_pushButton_clicked()
{
    QString filters = "BIN/WAV/IHEX files (*.bin , *.wav , *.hex)";

    filename = QFileDialog::getOpenFileName(this, tr("Open bin/wav/hex File"), "/Devel/Stm32_16.1_A_os_2024.10-rc/Membrane-2412171-00-WSensor_01/Debug",filters);
    hash_file();
    ui->label_BINFILE->setText(filename);
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        qDebug()<<"File not found";
    else
    {
        const QFileInfo info(filename);
        const QString ffname(info.fileName());
        bin_filename = ffname;
        qDebug()<<ffname;
        file_size = file.size();
        blob = file.readAll();
        file.close();
    }
}

void QtXmodem::on_DownloadXMODEMRX_pushButton_clicked()
{
    QString cmd;
    int retry=10 , serial_ret;
    ui->download_progressBar->setValue(0);

    serial.flush();
//    cmd = "<h " + QString::number(file_size) + " " + bin_filename + " " + hashStr + " >";
    cmd = "<h " + QString::number(file_size) + " " + bin_filename + " >";
    qDebug()<< cmd;

    serial_tx(cmd.toUtf8());
    qDebug()<< "Awaiting Poll";
    while ( (serial_ret = serial_rx(0)) != 0x15 )
    {
        ui->statusbar->showMessage("Retry");
        qDebug()<<"Retry on 0x15 with "<< serial_ret;
        retry--;
        if ( retry == 0 )
        {
            ui->statusbar->showMessage(bin_filename+" aborted download");
            qDebug()<<bin_filename<<" aborted download";
            return;
        }
    }
    qDebug()<< "Poll received, download enabled";
    download_binary();
}


void QtXmodem::on_Power_pushButton_clicked()
{
    QByteArray reply;
    QString cmd;
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    if ( ui->Power_pushButton->text() == "Power ON" )
    {
        cmd = "<P>";
        ui->Power_pushButton->setText("Power OFF");
        ui->Download_frame->setEnabled(true);
    }
    else
    {
        ui->Power_pushButton->setText("Power ON");
        ui->Download_frame->setEnabled(false);
        cmd = "<O>";
    }

    serial.flush();
    qDebug()<< cmd;
    serial_tx(cmd.toUtf8());
    serial_rx(1);
}

void QtXmodem::on_Data_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;

    ui->FlashLine_comboBox->currentText();

   cmd = "<A "+ui->DataLine_comboBox->currentText()+" "+ui->DataSensor_comboBox->currentText()+">";
   serial.flush();
   qDebug()<< cmd;
   serial_tx(cmd.toUtf8());
   serial_rx(0);
}

void QtXmodem::on_DownloadTu_uP_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;

    ui->FlashLine_comboBox->currentText();

    cmd = "<F "+ui->FlashLine_comboBox->currentText()+" "+ui->FlashSensor_comboBox->currentText()+" >";
    serial.flush();
    qDebug()<< cmd;
    serial_tx(cmd.toUtf8());
    serial_rx(1);
}


void QtXmodem::on_CheckDownload_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;

    ui->FlashLine_comboBox->currentText();

   cmd = "<C "+ui->FlashLine_comboBox->currentText()+" "+ui->FlashSensor_comboBox->currentText()+">";
   serial.flush();
   qDebug()<< cmd;
   serial_tx(cmd.toUtf8());
   serial_rx(1);
}


void QtXmodem::on_SendFlashCommand_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;

    ui->FlashLine_comboBox->currentText();

   cmd = "<W "+ui->FlashLine_comboBox->currentText()+" "+ui->FlashSensor_comboBox->currentText()+">";
   serial.flush();
   qDebug()<< cmd;
   serial_tx(cmd.toUtf8());
   serial_rx(1);
}

void QtXmodem::on_GetSensorInfoCommand_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;

    ui->FlashLine_comboBox->currentText();

   cmd = "<J "+ui->FlashLine_comboBox->currentText()+" "+ui->FlashSensor_comboBox->currentText()+">";
   serial.flush();
   qDebug()<< cmd;
   serial_tx(cmd.toUtf8());
   serial_rx(1);
}
