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

void MainWindow::Callback_Heartbeat()
{
    HeartbeatResponse message;
    UART::SendMessage(&message);
    targetIsConnected = true;
    targetConnectedRecently = true;
    this->ui->connectedStatusLabel->setText("Connected");
    ui->connectedStatusLabel->setStyleSheet("color: green;");
}

void MainWindow::on_actionIDCD_Editor_triggered()
{
    if(!this->editor){
        this->editor = new IDCD_Editor();
    }
    this->editor->show();
}
