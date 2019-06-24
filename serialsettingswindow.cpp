#include "serialsettingswindow.h"
#include "ui_serialsettingswindow.h"

#include <QDebug>

SerialSettingsWindow::SerialSettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialSettingsWindow)
{
    ui->setupUi(this);

    this->uart = UART::getInstance();

    this->on_RefreshButton_released();
}

SerialSettingsWindow::~SerialSettingsWindow()
{
    delete ui;
}

void SerialSettingsWindow::autoConnect()
{
    this->on_ConnectButton_released();
}

QString SerialSettingsWindow::GetPortName(bool &Ok)
{
    QString PortName = this->ui->ComPort->currentText();

    if(PortName.isEmpty()){
        Ok = false;
    }

    return PortName;
}

QSerialPort::BaudRate SerialSettingsWindow::GetBaudRate()
{
    int Index = this->ui->BaudRate->currentIndex();
    QSerialPort::BaudRate baud;

    switch(Index){
    case 0:
        baud = QSerialPort::Baud1200;
        break;
    case 1:
        baud = QSerialPort::Baud2400;
        break;
    case 2:
        baud = QSerialPort::Baud4800;
        break;
    case 3:
        baud = QSerialPort::Baud9600;
        break;
    case 4:
        baud = QSerialPort::Baud19200;
        break;
    case 5:
        baud = QSerialPort::Baud38400;
        break;
    case 6:
        baud = QSerialPort::Baud57600;
        break;
    case 7:
        baud = QSerialPort::Baud115200;
        break;
    default:
        //Invalid baud rate
        baud = QSerialPort::Baud115200;
        break;

    }

    return baud;
}

QSerialPort::DataBits SerialSettingsWindow::GetDataBits(bool &Ok)
{
    bool ConversionGood = false;
    QSerialPort::DataBits databits;
    uint32_t DataBitsAsInt = this->ui->DataBits->text().toUInt(&ConversionGood);
    switch(DataBitsAsInt){
        case 5:
            databits = QSerialPort::Data5;
            break;
        case 6:
            databits = QSerialPort::Data6;
            break;
        case 7:
            databits = QSerialPort::Data7;
            break;
        case 8:
            databits = QSerialPort::Data8;
            break;
        default:
            //Default to 8 bits on error
            databits = QSerialPort::Data8;
            ConversionGood = false;
            break;
    }

    Ok = Ok & ConversionGood;
    return databits;
}

QSerialPort::Parity SerialSettingsWindow::GetParity(bool &Ok)
{
    bool ConversionGood = true;
    QSerialPort::Parity parity;
    int ParityComboIndex = this->ui->Parity->currentIndex();
    switch(ParityComboIndex){
        case 0:
        //none
            parity = QSerialPort::NoParity;
            break;
        case 1:
        //Odd
            parity = QSerialPort::OddParity;
            break;
        case 2:
        //Even
            parity = QSerialPort::EvenParity;
            break;
        case 3:
        //Mark
            parity = QSerialPort::MarkParity;
            break;
        case 4:
            parity = QSerialPort::SpaceParity;
            //Space
            break;
        default:
            //Default to no parity on error
            parity = QSerialPort::NoParity;
            ConversionGood = false;
            break;
    }

    Ok = Ok & ConversionGood;
    return parity;
}

QSerialPort::StopBits SerialSettingsWindow::GetStopBits(bool &Ok)
{
    bool ConversionGood = false;
    QSerialPort::StopBits stopbits;
    uint32_t StopBitsInt = this->ui->StopBits->text().toUInt(&ConversionGood);
    switch(StopBitsInt){
        case 1:
        //none
            stopbits = QSerialPort::OneStop;
            break;
        case 2:
        //Even
            stopbits = QSerialPort::TwoStop;
            break;
        default:
            //Default to one stop on error
            stopbits = QSerialPort::OneStop;
            ConversionGood = false;
            break;
    }

    Ok = Ok & ConversionGood;
    return stopbits;
}

void SerialSettingsWindow::on_ConnectButton_released()
{
    if(this->uart->isOpen()){
        this->uart->Close();
        this->ui->ConnectButton->setText("Connect");
        this->ui->StatusLabel->setText("Disconnected successfully");
    }
    else{
        bool Ok = true;
        QString PortName = this->GetPortName(Ok);
        QSerialPort::BaudRate BaudRate = this->GetBaudRate();
        QSerialPort::DataBits bits = this->GetDataBits(Ok);
        QSerialPort::Parity parity = this->GetParity(Ok);
        QSerialPort::StopBits stopbits = this->GetStopBits(Ok);
        QSerialPort::FlowControl flowcontrol = QSerialPort::NoFlowControl;
        if(Ok){
            this->uart->SetPortName(PortName);
            this->uart->SetBaudRate(BaudRate);
            this->uart->SetDataBits(bits);
            this->uart->SetParity(parity);
            this->uart->SetStopBits(stopbits);
            this->uart->SetFlowControl(flowcontrol);

            bool ConnectSuccessfull = this->uart->Open();

            if(ConnectSuccessfull){
                this->ui->ConnectButton->setText("Disconnect");
                this->ui->StatusLabel->setText("Connected to " + PortName);
            }
            else{
                this->ui->StatusLabel->setText("Failed to connect to " + PortName);
            }
        }
        else{
            qDebug() << "Failed to convert serial settings.";
            this->ui->StatusLabel->setText("Invalid COM Settings");
        }
    }
}

void SerialSettingsWindow::on_RefreshButton_released()
{
    this->ui->ComPort->clear();

    //Refresh the list of valid com ports
    for(QSerialPortInfo PortInfo : this->uart->GetDevices()){
        this->ui->ComPort->addItem(PortInfo.portName());
    }
}
