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
    ui->Info_frame->setEnabled(false);
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
    if ( serial_started == 0 )
        return "1";
    serial.flush();
    serial.write(hex_line);
    if(serial.waitForReadyRead(WAIT_REPLY))
    {
        serial_reply = serial.readAll();
    }
    else
    {
        serial.flush();
        serial_reply = "";
    }
    qApp->processEvents();
    return serial_reply;
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
        ui->Info_frame->setEnabled(false);
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
            ui->Info_frame->setEnabled(false);
            ui->Power_pushButton->setEnabled(false);
        }
        else
        {
            ui->Comm_label->setPixmap(greenled);
            serial_started = 1;
            ui->statusbar->showMessage("Serial port "+arg1+" opened");
            serial.setReadBufferSize (1024);
            ui->Power_pushButton->setEnabled(true);
            ui->Info_frame->setEnabled(true);
        }
    }
    else
    {
        ui->Comm_label->setPixmap(redled);
        ui->statusbar->showMessage(arg1+" : "+serial.errorString());
        ui->data_frame->setEnabled(false);
        ui->Info_frame->setEnabled(false);
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


void DataCollector::on_Scan_pushButton_clicked()
{
    QByteArray reply;
    QByteArray Command;
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    toggle = 0;
    if ( ui->Scan_pushButton->text() == "Stop")
    {
        ui->Scan_pushButton->setText("Scan");
        killTimer(timer0Id);
        ui->statusbar->showMessage("Stopped");
        ui->SCAN_label->setPixmap(greenled);

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
        ui->SCAN_label->setPixmap(redled);
        cmd_counter1 = cmd_counter2 = cmd_counter3 = cmd_counter4 = 0;
        Command = "<S>";
        if ( (reply = serial_tx(Command)) != "1" )
        {
            qDebug()<< "Received";
        }
        ui->statusbar->showMessage("Running Scan");
    }

}


void DataCollector::on_GetSensorInfoCommand_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;

   cmd = "<J "+ui->SensorInfoLine_comboBox->currentText()+" "+ui->SensorInfoSensor_comboBox->currentText()+">";
   serial.flush();
   serial_tx(cmd.toUtf8());
   ui->label_SensorInfo->setText(serial_reply);
}


void DataCollector::on_ConcentratorVersion_pushButton_clicked()
{
    QString cmd;
    QByteArray Command;
    cmd = "<v>";
    serial.flush();
    qDebug()<< cmd;
    serial_tx(cmd.toUtf8());
    ui->label_CONCENTRATORVERSION->setText(serial_reply);
}


#ifdef Q_OS_WIN
QString dirPath= "c:/MembraneLog_3.2";
#else
QString dirPath= "/Devel/MembraneLog";
#endif
void DataCollector::store_sensor_data( int dsc , QByteArray reply)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate currentDate = currentDateTime.date();
    QTime currentTime = currentDateTime.time();
    QDir top_directory(dirPath);
    int  concentrator_counter = 1;
    QByteArray q_concentrator_counter;
    q_concentrator_counter.setNum(concentrator_counter);
    QString TopDir_dayPath = dirPath+"/"+currentDate.toString("yyMMdd")+"_CON"+q_concentrator_counter+"_iCON";
    int type,sensor,scalefactor,readout,temp_micro,cmd_counter,total_readout,calibration,tempPT1000;

    if ( !top_directory.exists())
    {
        qDebug()<< dirPath << " created";
        top_directory.mkpath(dirPath);
    }

    QDir top_directory_folderpath(TopDir_dayPath);
    if ( !top_directory_folderpath.exists())
    {
        qDebug()<< TopDir_dayPath << " created";
        top_directory_folderpath.mkpath(TopDir_dayPath);
    }

    QByteArray q_dsc;
    q_dsc.setNum(dsc);
    filename = TopDir_dayPath+"/"+currentDate.toString("yyMMdd")+"_CON"+q_concentrator_counter+"_DSC"+q_dsc+".csv";

    QFile file(filename);

    if ( ! file.exists())
    {
        qDebug()<< filename << " : File not present, created";
        CsvFile.setFileName(filename);
        CsvFile.open(QIODevice::Append | QIODevice::Text);
        CsvFileStream.setDevice(&CsvFile);
        CsvFileStream << "################################################################\n";
        CsvFileStream << "Concentrator version,v1.1-241212\n";
        CsvFileStream << "Sensors      version,v1.1-241212\n";
        CsvFileStream << "-,-\n";
        CsvFileStream << "-,-\n";
        CsvFileStream << "################################################################\n";
        CsvFileStream << "Time stamp,Sequence number,Concentrator,DSC,Sensor,Scale,Readout,Total Readout,Total Noise,Temp Micro,Temp PT1000,DAC,Active,Status\n";
    }
    else
    {
        CsvFile.setFileName(filename);
        CsvFile.open(QIODevice::Append | QIODevice::Text);
        CsvFileStream.setDevice(&CsvFile);
    }
    /*
     * type sens  scale  val  temp
     * "01   08    00   0824  0021"
    */
    calibration = 0;
    tempPT1000  = 0;
    switch(dsc)
    {
    case 1  :    cmd_counter = cmd_counter1; cmd_counter1++;break;
    case 2  :    cmd_counter = cmd_counter2; cmd_counter2++;break;
    case 3  :    cmd_counter = cmd_counter3; cmd_counter3++;break;
    case 4  :    cmd_counter = cmd_counter4; cmd_counter4++;break;
    }

    const char* DataAsString = reply.constData();
    QString timestamp = currentDate.toString("dd/MM/yy")+" "+currentTime.toString("hh:mm:ss");
    sscanf(DataAsString,"%d %d %d %04x %04x",
           &type,&sensor,&scalefactor,&readout,&temp_micro);
    if ( type == 1 )
    {
        total_readout = readout * (scalefactor+1);
        //qDebug() << type << "," << sensor << "," << scalefactor << "," << readout << "," << total_readout << "," << temp_micro;
        if ( ui->CSVDebugEnable_checkBox->isChecked() == true )
            qDebug() << timestamp << "," << cmd_counter << "," << concentrator_counter << "," << dsc << "," << sensor << "," << scalefactor+1 << "," << readout << "," << total_readout << "," << calibration << "," << temp_micro << "," << tempPT1000 << ",Y,A";
        CsvFileStream << timestamp << "," << cmd_counter << "," << concentrator_counter << "," << dsc << "," << sensor << "," << scalefactor+1 << "," << readout << "," << total_readout << "," << calibration << "," << temp_micro << "," << tempPT1000 << ",Y,A\n";
    }
    CsvFile.close();
}

void DataCollector::timerEvent(QTimerEvent *event)
{
    QByteArray reply;
    QByteArray Command;
    QByteArray qline;
    QByteArray qsensor;
    int dsc,sensor;
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    if ( event->timerId() == timer0Id )
    {
        int sensor_debug = ui->QSENSOR_DEBUG_comboBox->currentText().toInt();// 0 if All
        int line_debug = ui->QLINE_DEBUG_comboBox->currentText().toInt(); // 0 if All

        if ( toggle )
            ui->SCAN_label->setPixmap(redled);
        else
            ui->SCAN_label->setPixmap(greenled);
        toggle++;
        toggle &= 1;

        for(dsc=1;dsc<NUM_DSC+1;dsc++)
        {
            for(sensor=FIRST_WSENSOR;sensor<LAST_WSENSOR+2;sensor++)
            {
                qline.setNum(dsc);
                qsensor.setNum(sensor);
                Command = "<A "+qline+" "+qsensor+">";
                if ( (reply = serial_tx(Command)) != "" )
                {
                    if ( ui->DebugEnable_checkBox->isChecked() == true)
                    {
                        if (( line_debug == 0) && ( sensor_debug == 0))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                        if (( line_debug == dsc) && ( sensor_debug == 0))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                        if (( line_debug == 0) && ( sensor_debug == sensor))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                        if (( line_debug == dsc) && ( sensor_debug == sensor))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                    }
                    store_sensor_data(dsc,reply);
                }
            }
        }
    }
}
