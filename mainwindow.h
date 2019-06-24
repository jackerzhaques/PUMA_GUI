#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

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

private slots:
    void on_actionIDCD_Editor_triggered();

private:
    Ui::MainWindow *ui;
    UART* uart                      = nullptr;
    SerialSettingsWindow *settings  = nullptr;
    IDCD_Editor *editor             = nullptr;
    bool targetIsConnected          = false;
    bool targetConnectedRecently    = false;

    //Message callback
    void Callback_Heartbeat();
};

#endif // MAINWINDOW_H
