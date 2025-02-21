#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class DataCollector; }
QT_END_NAMESPACE

#define WAIT_REPLY              100
#define NUM_DSC             4
#define FIRST_WSENSOR       1
#define LAST_WSENSOR        8
#define TEMP_SENSOR         9

class DataCollector : public QMainWindow
{
    Q_OBJECT

public:
    DataCollector(QWidget *parent = nullptr);
    ~DataCollector();

private slots:
    void on_Port_comboBox_currentTextChanged(const QString &arg1);

    void on_Power_pushButton_clicked();

    void on_Scan_pushButton_clicked();

private:
    Ui::DataCollector *ui;

    QSerialPort serial;
    QByteArray serial_reply;
    int serial_started;

    QByteArray serial_tx( QByteArray hex_line);
    int timer0Id;
    int timerint;
protected:
    void timerEvent(QTimerEvent *event);
};
#endif // DATACOLLECTOR_H
