#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTimer>

#include "serialsettingswindow.h"
#include "uart.h"
#include "idcd.h"
#include "message.h"
#include "idcd_editor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent* event);

    static void MessageHandler(void* context, Message *m);
    void ProcessMessage(Message *m);

public slots:
    void heartbeatTimeout();
    void streamPositions();

private slots:
    void on_actionIDCD_Editor_triggered();

    void on_joint1SendButton_released();

    void on_joint2SendButton_released();

    void on_joint3SendButton_released();

    void on_joint4SendButton_released();

    void on_joint5SendButton_released();

    void on_joint6SendButton_released();

    void on_xPosSlider_valueChanged(int value);

    void on_yPosSlider_valueChanged(int value);

    void on_zPosSlider_valueChanged(int value);

    void on_thetaPosSlider_valueChanged(int value);

    void on_posStreamButton_toggled(bool checked);

    void on_autoHomeButton_released();

    void on_manualHomeButton_released();

    void on_xSendButton_released();

    void on_ySendButton_released();

    void on_zSendButton_released();

    void on_thetaSendButton_released();

    void on_actionSerial_Settings_triggered();

private:
    Ui::MainWindow *ui;
    UART* uart                      = nullptr;
    SerialSettingsWindow *settings  = nullptr;
    IDCD_Editor *editor             = nullptr;
    bool targetIsConnected          = false;
    bool targetConnectedRecently    = false;
    QTimer *positionStreamTimer     = nullptr;

    //Message callback
    void Callback_Heartbeat();
    void Callback_EncoderStatus(Message *m);
    void Callback_ControlStatus(Message *m);
    void Callback_JointAngle(Message *m);

    //Message send helper functions
    void SendJointAngle(float angle, uint8_t joint);
    void SetArmPosition(float pos, uint8_t Dimension);
};

#endif // MAINWINDOW_H
