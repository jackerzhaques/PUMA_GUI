#ifndef UART_H
#define UART_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "message.h"
#include "idcd.h"

typedef void (*callback_func)(void* context, Message *m);

class UART : public QObject
{
    Q_OBJECT
public:
    explicit UART(QObject *parent = nullptr);
    static UART* getInstance();
    void SetStopBits(QSerialPort::StopBits stopBits);
    void SetParity(QSerialPort::Parity parity);
    void SetDataBits(QSerialPort::DataBits dataBits);
    void SetBaudRate(QSerialPort::BaudRate baudRate);
    void SetPortName(QString portName);
    void SetFlowControl(QSerialPort::FlowControl flowControl);

    bool isOpen();
    bool Open();
    void Close();
    QList<QSerialPortInfo> GetDevices();

    void SendBytes(QByteArray bytes);

    void hook(void* context, callback_func functionHook);

    static void SendMessage(Message *m);
signals:

public slots:

private slots:
    void ReadSerialData();

private:
    QSerialPort *serialPort = nullptr;
    QByteArray IncomingBytes;

    QList<void (*)(void*,Message*)> hooks;
    QList<void*> contexts;

    void AddBytesToBuffer(QByteArray bytes);
    void CleanBuffer();
    void ProcessBuffer();
    void GetNextMessage();
    bool MessageAvailable();
    void emitMessage(Message *m);
};

static UART *instance = nullptr;

#endif // UART_H
