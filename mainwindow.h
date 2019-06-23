#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "serialsettingswindow.h"
#include "uart.h"
#include "idcd.h"
#include "message.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static void MessageHandler(void* context, Message *m);
    void ProcessMessage(Message *m);

public slots:
    void heartbeatTimeout();

private:
    Ui::MainWindow *ui;
    SerialSettingsWindow *settings  = nullptr;
    bool targetIsConnected          = false;
    bool targetConnectedRecently    = false;

    //Message callback
    void Callback_Heartbeat();
};

#endif // MAINWINDOW_H
