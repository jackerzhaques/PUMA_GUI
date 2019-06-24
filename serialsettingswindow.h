#ifndef SERIALSETTINGSWINDOW_H
#define SERIALSETTINGSWINDOW_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <uart.h>

namespace Ui {
class SerialSettingsWindow;
}

class SerialSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SerialSettingsWindow(QWidget *parent = nullptr);
    ~SerialSettingsWindow();

    void autoConnect();

private slots:
    void on_RefreshButton_released();

    void on_ConnectButton_released();

private:
    Ui::SerialSettingsWindow *ui;
    UART *uart = nullptr;

    QSerialPort::StopBits GetStopBits(bool &Ok);
    QSerialPort::Parity GetParity(bool &Ok);
    QSerialPort::DataBits GetDataBits(bool &Ok);
    QSerialPort::BaudRate GetBaudRate();
    QString GetPortName(bool &Ok);
};

#endif // SERIALSETTINGSWINDOW_H
