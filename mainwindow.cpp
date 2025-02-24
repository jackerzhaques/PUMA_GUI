#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
#include <cmath>

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
    else if(m->ID == Get_Joint_Current_Response().id){
        //Not implemented yet
    }
    else if(m->ID == Get_Position_Response().id){
        //Not implemented yet
    }
    else if(m->ID == Get_Joint_Angle_Response().id){
        this->Callback_JointAngle(m);
    }
    else if(m->ID == Home_Response().id){
        //Not implemented yet
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
    Get_Joint_Angle_Response::Get_Joint_Angle_Response_data *angle
            = reinterpret_cast<Get_Joint_Angle_Response::Get_Joint_Angle_Response_data*>(m->pData);

    float val = UART::byteToFloats(angle->angleBytes);
    QString text = QString::number(static_cast<double>(val));

    if(angle->Joint > 0 && angle->Joint <= 6){
        jointAngles[angle->Joint - 1] =
                static_cast<double>(val) * M_PI/180;
        this->updatePosition();
    }

    switch(angle->Joint){
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
        default:
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

void MainWindow::updatePosition()
{
    const static double
            L1 = 13,
            L2 = 2.5,
            L3 = 8.0,
            L4 = 2.5,
            L5 = 8.0,
            L6 = 3.0;

    //Calculate forward kinematics
    double x = 0, y = 0, z = 0, theta = 0;

    //J1
    x = 0;
    y = 0;
    z = L1;

    //J1 offset
    //z = z
    double xy = L2;
    x = x + cos(jointAngles[0]) * xy;
    y = y + sin(jointAngles[0]) * xy;

    //J2
    xy = cos(jointAngles[1]) * L3;
    z = z + (sin(jointAngles[1]) * L3);
    x = x + cos(jointAngles[0] + M_PI/2) * xy;
    y = y + sin(jointAngles[0] + M_PI/2) * xy;

    //J2 offset
    //z = z
    xy = L4;
    x = x + (xy * cos(jointAngles[0]));
    y = y + (xy * sin(jointAngles[0]));

    //J3
    xy = cos(jointAngles[2] + jointAngles[2]) * L5;
    z = z + (sin(jointAngles[2] + jointAngles[1]) * L5);
    x = x + (cos(jointAngles[0] + M_PI/2) * xy);
    y = y + (sin(jointAngles[0] + M_PI/2) * xy);

    //J4
    xy = cos(jointAngles[3] + jointAngles[2] + jointAngles[1]) * L6;
    z = z + (sin(jointAngles[3] + jointAngles[2] + jointAngles[1]) * L6);
    x = x + (cos(jointAngles[0] + M_PI/2) * xy);
    y = y + (sin(jointAngles[0] + M_PI/2) * xy);

    ui->RealXLabel->setText(QString::number(x));
    ui->RealYLabel->setText(QString::number(y));
    ui->RealZLabel->setText(QString::number(z));
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

void MainWindow::on_actionSerial_Settings_triggered()
{
    if(!this->settings){
        this->settings = new SerialSettingsWindow();
    }

    this->settings->show();
}
