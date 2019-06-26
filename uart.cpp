#include "uart.h"
#include <QDebug>

#include <QByteArray>
#include <QList>
#include <QStringList>

UART::UART(QObject *parent) : QObject(parent)
{
    this->serialPort = new QSerialPort(this);

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(ReadSerialData()));

    if(!instance){
        instance = this;
    }
}

UART *UART::getInstance()
{
    if(!instance){
        instance = new UART();
    }

    return instance;
}

void UART::SetStopBits(QSerialPort::StopBits stopBits)
{
    serialPort->setStopBits(stopBits);
}

void UART::SetParity(QSerialPort::Parity parity)
{
    serialPort->setParity(parity);
}

void UART::SetDataBits(QSerialPort::DataBits dataBits)
{
    serialPort->setDataBits(dataBits);
}

void UART::SetBaudRate(QSerialPort::BaudRate baudRate)
{
    serialPort->setBaudRate(baudRate);
}

void UART::SetPortName(QString portName)
{
    serialPort->setPortName(portName);
}

void UART::SetFlowControl(QSerialPort::FlowControl flowControl)
{
    serialPort->setFlowControl(flowControl);
}

bool UART::isOpen()
{
    return serialPort->isOpen();
}

bool UART::Open()
{
    bool connectSuccessfull = false;

    connectSuccessfull = this->serialPort->open(QIODevice::ReadWrite);

    return connectSuccessfull;
}

void UART::Close()
{
    this->serialPort->close();
}

QList<QSerialPortInfo> UART::GetDevices()
{
    return QSerialPortInfo::availablePorts();
}

void UART::SendBytes(QByteArray bytes)
{
    this->serialPort->write(bytes);
}

void UART::hook(void* context, callback_func functionHook)
{
    this->hooks.append(functionHook);
    this->contexts.append(context);
}

void UART::SendMessage(Message *m)
{
    if(!instance){
        instance = new UART();
    }

    //Convert message to ascii
    QString data;
    data = QString("%1%2%3").arg(MESSAGE_START_BYTE).arg(m->ID).arg(MESSAGE_ID_END_BYTE);
    for(int i = 0; i < m->DLC; i++){
        data += QString::number(m->pData[i]);

        if(i < m->DLC - 1){
            data += MESSAGE_FRAMING_BYTE;
        }
    }

    data += MESSAGE_END_BYTE;

    instance->serialPort->write(data.toLocal8Bit());
}

uint32_t UART::floatToBytes(float p)
{
    uint32_t result = 0;

    memcpy(&result, &p, sizeof(p));

    return result;
}

float UART::byteToFloats(uint32_t b)
{
    float result = 0;

    memcpy(&result, &b, sizeof(b));

    return result;
}

void UART::ReadSerialData()
{
    while(this->serialPort->bytesAvailable()){
        QByteArray bytes = this->serialPort->readAll();
        this->AddBytesToBuffer(bytes);
    }
}

void UART::AddBytesToBuffer(QByteArray bytes)
{
    this->IncomingBytes.append(bytes);
    this->ProcessBuffer();
}

//Moves the first start byte to the start of the buffer
//Removes the first corrupted message found
void UART::CleanBuffer()
{
    if(IncomingBytes.size() > 0){
        //Remove any bytes up to a start byte
        while(!IncomingBytes.startsWith(MESSAGE_START_BYTE)){
            IncomingBytes.remove(0, 1);
        }

        //Grab a substring up to the first end byte
        QByteArray sub = IncomingBytes.split(MESSAGE_END_BYTE)[0];

        if(sub.contains(MESSAGE_START_BYTE) && sub.contains(MESSAGE_END_BYTE) && !sub.contains(MESSAGE_ID_END_BYTE)){
            //First message is corrupted, remove it
            int EndByteIdx = sub.size();
            IncomingBytes.remove(0, EndByteIdx);
        }
    }

}

void UART::ProcessBuffer()
{

    while(MessageAvailable()){
        //Remove any corrupted messages;
        CleanBuffer();

        qDebug() << this->IncomingBytes;
        GetNextMessage();   //This function automatically emits the found message
    }
}

void UART::GetNextMessage()
{
    //Find the end index of the current message
    int EndByteIdx = 0;
    for(int i = 0; i < IncomingBytes.size(); i++){
        if(IncomingBytes[i] == MESSAGE_END_BYTE){
            EndByteIdx = i;
            break;
        }
    }

    //Grab the current message's data
    QByteArray messageData = IncomingBytes.left(EndByteIdx + 1);

    //Process the message
    if(messageData.contains(MESSAGE_END_BYTE)){
        bool IDok = false, dataOk[MAX_MESSAGE_DATA_LENGTH] = {false};
        uint16_t ID = 0;
        uint8_t data[MAX_MESSAGE_DATA_LENGTH] = {0};
        uint8_t DLC = 0;

        //Clear the start byte and stop byte
        messageData.remove(0, 1);
        messageData.remove(messageData.size() - 1, 1);

        //Grab the ID
        QString IDData = messageData.split(MESSAGE_ID_END_BYTE)[0];
        ID = static_cast<uint16_t>(IDData.toUInt(&IDok));

        //Grab the data
        QString dataString = messageData.split(MESSAGE_ID_END_BYTE)[1];
        QStringList dataStringList = dataString.split(MESSAGE_FRAMING_BYTE);
        if(dataStringList[0] != ""){
            for(int j = 0; j < dataStringList.size(); j++){
                data[j] = static_cast<uint8_t>(dataStringList[j].toUInt(&dataOk[j]));
                DLC++;
            }
        }

        //Check the results
        if(!IDok){
            qDebug() << "Message Parser: ID invalid - " << messageData;
        }
        else{
            //Do nothing
        }

        for(int j = 0; j < DLC; j++){
            if(!dataOk[j]){
                qDebug() << "Message Parser: Data at index " << j << " is invalid - " << messageData;
            }
            else{
                //Do nothing
            }
        }

        //Remove data from buffer
        IncomingBytes.remove(0, messageData.size() + 2);    //Add back the start and stop byte

        //Form the message
        Message *m = new Message();
        m->ID = ID;
        m->DLC = DLC;
        m->pData = new uint8_t[DLC];
        for(int i = 0; i < DLC; i++){
            m->pData[i] = data[i];
        }

        emitMessage(m);
    }
}

bool UART::MessageAvailable()
{
    if(IncomingBytes.contains(MESSAGE_END_BYTE)){
        return true;
    }
    else{
        return false;
    }
}

void UART::emitMessage(Message *m)
{
    for(int i = 0; i < contexts.size(); i++){
        hooks[i](contexts[i], m);
    }
}
