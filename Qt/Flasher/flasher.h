#ifndef FLASHER_H
#define FLASHER_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class Flasher; }
QT_END_NAMESPACE
#define WAIT_REPLY              2000

class Flasher : public QMainWindow
{
    Q_OBJECT

public:
    Flasher(QWidget *parent = nullptr);
    ~Flasher();

private slots:
    void on_SelectFile_pushButton_clicked();

    void on_DownloadXMODEMRX_pushButton_clicked();

    void on_Power_pushButton_clicked();

    void on_SendFlashCommand_pushButton_clicked();

    void on_Port_comboBox_currentTextChanged(const QString &arg1);

    void on_GetSensorInfoCommand_pushButton_clicked();

    void on_DownloadTuSensor_pushButton_clicked();

    void on_ConcentratorVersion_pushButton_clicked();

private:
    Ui::Flasher *ui;

    int serial_tx( QByteArray hex_line);
    int serial_rx(  int do_echo);
    void download_binary(void);
    void create_buf_and_tx(char *data);
    void hash_file();

    QSerialPort serial;
    QByteArray serial_reply;

    int serial_started;
    QString filename,bin_filename;
    int file_size;

    QByteArray blob;
    QString hashStr;

    const char *serial_packet;
};
#endif // FLASHER_H
