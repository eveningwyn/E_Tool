﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcpipserver.h"
#include "tcpipclient.h"
#include <QMutex>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void server_ReadData(QString IP,int Port,QString readMsg);
    void server_clientConnect(QString IP,int Port);
    void server_clientDisconnected(QString IP,int Port);
    void client_readData(int clientID,QString IP,int Port,QString msg);
    void client_clientDisConnect(int clientID,QString IP,int Port);

private slots:
    void on_pushButton_creat_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_send_clicked();

    void on_comboBox_server_client_currentIndexChanged(int index);

    void on_pushButton_loadFile_clicked();

    void on_checkBox_saveLog_clicked();

    void on_actionAbout_triggered();

    void on_pushButton_timer_clicked();

private:
    Ui::MainWindow *ui;
    TcpIpServer *server;
    TcpIpClient *client;
    void showInformation(uint index, QString msg);
    QMutex show_mutex;
    void checkMsg(QString &msg, int &sleep_time);
    QString msgFileName;
    bool client_disconn;
    QString logFileName;
    void saveLog(QString strMsg);
    QString timerFileName;
    QString timer_iniFileName;
    void check_timerMsg(QString sendMsg);
    enum SHOW_MSG
    {
        SHOW_SENDER,
        SHOW_RECEIVE,
        SHOW_NULL,
    };

protected:
    virtual void timerEvent(QTimerEvent *event);

};

#endif // MAINWINDOW_H
