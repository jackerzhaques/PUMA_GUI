#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->uart = UART::getInstance();
    uart->hook(this, MessageHandler);

    this->settings = new SerialSettingsWindow();
    this->settings->autoConnect();

    QTimer *heartbeatTimeoutTimer = new QTimer(this);
    connect(heartbeatTimeoutTimer, SIGNAL(timeout()), this, SLOT(heartbeatTimeout()));
    heartbeatTimeoutTimer->start(5500);

    this->positionStreamTimer = new QTimer(this);
    connect(positionStreamTimer, SIGNAL(timeout()), this, SLOT(streamPositions()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    delete uart;
    delete settings;
    delete editor;
}

void MainWindow::MessageHandler(void* context, Message *m)
{
    MainWindow* obj = static_cast<MainWindow*>(context);
    obj->ProcessMessage(m);
}

void MainWindow::ProcessMessage(Message *m)
{
    if(m->ID == Heartbeat().id){
        this->Callback_Heartbeat();
    }
    else if(m->ID == Get_Encoder_Status_Response().id){
        this->Callback_EncoderStatus(m);
    }
    else if(m->ID == Get_Control_Status_Response().id){
        this->Callback_ControlStatus(m);
    }
}

void MainWindow::heartbeatTimeout()
{
    if(targetConnectedRecently){
        targetConnectedRecently = false;
    }
    else{
        targetIsConnected = false;
        ui->connectedStatusLabel->setText("Disconnected");
        ui->connectedStatusLabel->setStyleSheet("color: red;");
    }
}

void MainWindow::streamPositions()
{
    static float lastX = 0, lastY = 0, lastZ = 0, lastTheta = 0;
    float x, y, z, theta;

    x = static_cast<float>(ui->xPosInput->value());
    y = static_cast<float>(ui->yPosInput->value());
    z = static_cast<float>(ui->zPosInput->value());
    theta = static_cast<float>(ui->thetaPosInput->value());

    if(fabs(x - lastX) >= 0.01f){ //OKAY because we're just checking for change
        lastX = x;
        this->SetArmPosition(x, 1);
    }
    else{
        //Do nothing
    }

    if(fabs(y - lastY) >= 0.01f){
        lastY = y;
        this->SetArmPosition(y, 2);
    }
    else{
        //Do nothing
    }

    if(fabs(z - lastZ) >= 0.01f){
        lastZ = z;
        this->SetArmPosition(z, 3);
    }
    else{
        //Do nothing
    }

    if(fabs(theta - lastTheta) >= 0.01f){
        lastTheta = theta;
        this->SetArmPosition(theta, 4);
    }

}

void MainWindow::Callback_Heartbeat()
{
    Heartbeat_Response message;
    UART::SendMessage(&message);
    targetIsConnected = true;
    targetConnectedRecently = true;
    this->ui->connectedStatusLabel->setText("Connected");
    ui->connectedStatusLabel->setStyleSheet("color: green;");
}

void MainWindow::Callback_EncoderStatus(Message *m)
{
    Get_Encoder_Status_Response *status = dynamic_cast<Get_Encoder_Status_Response*>(m);

    if(status){
        QString text = "";
        int i = 0;
        int count = 0;
        while(status->data.Joint > 0){
            bool faulted = status->data.Joint & 0x01;

            if(faulted){
                if(count == 0){
                    text += "Encoder %1 faulted";
                }
                else{
                    text += ", Encoder %1 faulted";
                }

                text = text.arg(i);
                count++;
            }

            status->data.Joint = status->data.Joint >> 1;
            i++;
        }

        if(text.isEmpty()){
            ui->encoderStatusLabel->setText("All Encoders Connected");
            ui->encoderStatusLabel->setStyleSheet("color: green;");
        }
        else{
            ui->encoderStatusLabel->setText(text);
            ui->encoderStatusLabel->setStyleSheet("color: red;");
        }
    }
}

void MainWindow::Callback_ControlStatus(Message *m)
{
    Get_Control_Status_Response *status = dynamic_cast<Get_Control_Status_Response*>(m);

    QString statusText = "";

    if(status->data.status == 0){
        statusText = "N/A";
    }
    else if(status->data.status == 1){
        statusText = "Initializing";
    }
    else if(status->data.status == 2){
        statusText = "Homing";
    }
    else if(status->data.status == 3){
        statusText = "Control Loop Engaged";
    }
    else{
        statusText = "Unknown State";
    }

    ui->controlStatusLabel->setText(statusText);
}

void MainWindow::Callback_JointAngle(Message *m)
{
    Get_Joint_Angle_Response *angle = dynamic_cast<Get_Joint_Angle_Response*>(m);

    float val = UART::byteToFloats(angle->data.angleBytes);
    QString text = QString::number(static_cast<double>(val));

    switch(angle->data.Joint){
        case 1:
            ui->joint1Status->setText(text);
            break;
        case 2:
            ui->joint2Status->setText(text);
            break;
        case 3:
            ui->joint3Status->setText(text);
            break;
        case 4:
            ui->joint4Status->setText(text);
            break;
        case 5:
            ui->joint5Status->setText(text);
            break;
        case 6:
            ui->joint6Status->setText(text);
            break;
        defualt:
            //Do nothing
            break;
    }
}

void MainWindow::SendJointAngle(float angle, uint8_t joint)
{
    Set_Joint_Angle m;
    m.data.floatDegrees = UART::floatToBytes(angle);
    m.data.Joint = joint;
    UART::SendMessage(&m);
}

void MainWindow::SetArmPosition(float pos, uint8_t Dimension)
{
    Set_Position m;
    m.data.floatPos = UART::floatToBytes(pos);
    m.data.Dimension = Dimension;
    UART::SendMessage(&m);
}

void MainWindow::on_actionIDCD_Editor_triggered()
{
    if(!this->editor){
        this->editor = new IDCD_Editor();
    }
    this->editor->show();
}

void MainWindow::on_joint1SendButton_released()
{
    SendJointAngle(static_cast<float>(ui->joint1Input->value()), 1);
}

void MainWindow::on_joint2SendButton_released()
{
    SendJointAngle(static_cast<float>(ui->joint2Input->value()), 2);
}

void MainWindow::on_joint3SendButton_released()
{
    SendJointAngle(static_cast<float>(ui->joint3Input->value()), 3);
}

void MainWindow::on_joint4SendButton_released()
{
    SendJointAngle(static_cast<float>(ui->joint4Input->value()), 4);
}

void MainWindow::on_joint5SendButton_released()
{
    SendJointAngle(static_cast<float>(ui->joint5Input->value()), 5);
}

void MainWindow::on_joint6SendButton_released()
{
    SendJointAngle(static_cast<float>(ui->joint6Input->value()), 6);
}

void MainWindow::on_xPosSlider_valueChanged(int value)
{
    double doubleVal = static_cast<double>(value) / 1000;
    //Scale to input values
    doubleVal = (doubleVal * 20) - 10;

    ui->xPosInput->setValue(doubleVal);
}

void MainWindow::on_yPosSlider_valueChanged(int value)
{
    double doubleVal = static_cast<double>(value) / 1000;
    //Scale to input values
    doubleVal = (doubleVal * 20) - 10;

    ui->yPosInput->setValue(doubleVal);
}

void MainWindow::on_zPosSlider_valueChanged(int value)
{
    double doubleVal = static_cast<double>(value) / 1000;
    //Scale to input values
    doubleVal = (doubleVal * 20) - 10;

    ui->zPosInput->setValue(doubleVal);
}

void MainWindow::on_thetaPosSlider_valueChanged(int value)
{
    double doubleVal = static_cast<double>(value) / 1000;
    //Scale to input values
    doubleVal = (doubleVal * 180) - 90;

    ui->thetaPosInput->setValue(doubleVal);
}

void MainWindow::on_posStreamButton_toggled(bool checked)
{
    if(checked){
        int period = 1000 / ui->posStreamInput->value();
        this->positionStreamTimer->start(period);
    }
    else{
        this->positionStreamTimer->stop();
    }
}

void MainWindow::on_autoHomeButton_released()
{
    Auto_Home m;
    UART::SendMessage(&m);
}

void MainWindow::on_manualHomeButton_released()
{
    Manual_Home m;
    UART::SendMessage(&m);
}

void MainWindow::on_xSendButton_released()
{
    SetArmPosition(static_cast<float>(ui->xPosInput->value()), 1);
}

void MainWindow::on_ySendButton_released()
{
    SetArmPosition(static_cast<float>(ui->yPosInput->value()), 2);
}

void MainWindow::on_zSendButton_released()
{
    SetArmPosition(static_cast<float>(ui->zPosInput->value()), 3);
}

void MainWindow::on_thetaSendButton_released()
{
    SetArmPosition(static_cast<float>(ui->thetaPosInput->value()), 4);
}
