#ifndef QTXMODEM_H
#define QTXMODEM_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class QtXmodem; }
QT_END_NAMESPACE
#define WAIT_REPLY              1000

class QtXmodem : public QMainWindow
{
    Q_OBJECT

public:
    QtXmodem(QWidget *parent = nullptr);
    ~QtXmodem();

private slots:
    void on_Port_comboBox_currentTextChanged(const QString &arg1);

    void on_SelectFile_pushButton_clicked();

    void on_DownloadXMODEMRX_pushButton_clicked();

    void on_Power_pushButton_clicked();

    void on_Data_pushButton_clicked();

    void on_DownloadTu_uP_pushButton_clicked();

    void on_CheckDownload_pushButton_clicked();

    void on_SendFlashCommand_pushButton_clicked();

    void on_GetSensorInfoCommand_pushButton_clicked();

private:
    Ui::QtXmodem *ui;

    int serial_tx( QByteArray hex_line);
    int serial_rx(  int do_echo);
    void download_binary(void);
    void create_buf_and_tx(char *data);
    void hash_file();

    QSerialPort serial;
    int serial_started;
    QString filename,bin_filename;
    int file_size;

    QByteArray blob;
    QString hashStr;

};
#endif // QTXMODEM_H
