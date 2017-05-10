#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRegExp>
#include <QDateTime>
#include "language.h"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QThread>

#define TIMER_TIME_OUT 500

#define PRO_VERSION "V1.01"
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,NULL,QString(tr("\nVersion: %1\n\nBuilt on 2017-05-010\n")).arg(PRO_VERSION));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_delete->setDisabled(true);
    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    QRegExp regExpNetPort("((6553[0-5])|[655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{3}|[1-9][0-9]{2}|[1-9][0-9]|[0-9])");

    ui->lineEdit_serverIP->setValidator(new QRegExpValidator(regExpIP,this));
    ui->lineEdit_serverPort->setValidator(new QRegExpValidator(regExpNetPort,this));
    ui->lineEdit_serverIP_send->setValidator(new QRegExpValidator(regExpIP,this));
    ui->lineEdit_serverPort_send->setValidator(new QRegExpValidator(regExpNetPort,this));

    msgFileName = "";
    logFileName = "";
    timerFileName = "";
    client_disconn = true;;

    initDoneTimer = new QTimer(this);
    writeFileTimer = new QTimer(this);
    startTestTimer = new QTimer(this);
    sortCompleteTimer = new QTimer(this);
    connect(initDoneTimer,&QTimer::timeout,this,&MainWindow::initDoneTimerTimeout);
    connect(writeFileTimer,&QTimer::timeout,this,&MainWindow::writeFileTimerTimeout);
    connect(startTestTimer,&QTimer::timeout,this,&MainWindow::startTestTimerTimeout);
    connect(sortCompleteTimer,&QTimer::timeout,this,&MainWindow::sortCompleteTimerTimeout);
    startTimer(1000);
}

MainWindow::~MainWindow()
{
    if(!ui->pushButton_creat->isEnabled())
    {
        on_pushButton_delete_clicked();
    }
    delete ui;
}

void MainWindow::on_pushButton_creat_clicked()
{
    QString server_IP = ui->lineEdit_serverIP->text();
    QString server_Port = ui->lineEdit_serverPort->text();
    QString prefix = ui->comboBox_prefix->currentText();
    QString suffix = ui->comboBox_suffix->currentText();
    suffix.replace("\\r","\r");
    suffix.replace("\\n","\n");
    if(""==server_IP || ""==server_Port)
    {
        showInformation(2,tr("IP地址或者端口号不能为空!"));
        return;
    }
    if(0==ui->comboBox_server_client->currentIndex())//服务器
    {
        server = new TcpIpServer(this);
        connect(server,&TcpIpServer::serverReadData,this,&MainWindow::server_ReadData);
        connect(server,&TcpIpServer::clientConnect,this,&MainWindow::server_clientConnect);
        connect(server,&TcpIpServer::clientDisconnected,this,&MainWindow::server_clientDisconnected);

        if(!server->stratListen(server_IP,(quint16)server_Port.toInt()))
        {
            showInformation(2,QString("%1 %2服务器创建失败!").arg(server_IP).arg(server_Port));
            return;
        }
        showInformation(2,QString("%1 %2服务器创建成功!").arg(server_IP).arg(server_Port));
        server->set_prefix_suffix(prefix,suffix);
    }
    else
    {
        if(1==ui->comboBox_server_client->currentIndex())//客户端
        {
            client = new TcpIpClient(this);
            connect(client,&TcpIpClient::readData,this,&MainWindow::client_readData);
            connect(client,&TcpIpClient::clientDisConnect,this,&MainWindow::client_clientDisConnect);

            if(!client->newConnect(server_IP, (quint16)server_Port.toInt()))
            {
                client_disconn = true;
                showInformation(2,QString("连接服务器%1 %2失败!").arg(server_IP).arg(server_Port));
                return;
            }
            client_disconn = false;
            showInformation(2,QString("连接服务器%1 %2成功!").arg(server_IP).arg(server_Port));
            client->prefix = prefix;
            client->suffix = suffix;
        }
    }
    ui->pushButton_creat->setDisabled(true);
    ui->comboBox_server_client->setDisabled(true);
    ui->pushButton_delete->setDisabled(false);
    ui->lineEdit_serverIP->setDisabled(true);
    ui->lineEdit_serverPort->setDisabled(true);
    ui->comboBox_prefix->setDisabled(true);
    ui->comboBox_suffix->setDisabled(true);
}

void MainWindow::showInformation(uint index, QString msg)
{
    show_mutex.lock();
    QString time = QDateTime::currentDateTime().toString("yyyyMMdd_hh:mm:ss_zzz");
    QString senderStr;
    if(0==index)
    {
        senderStr = "Send to";
    }
    else
    {
        if(1==index)
        {
            senderStr = "Receive from";
        }
        else
        {
            if(2==index)
            {
                senderStr = "";
            }
        }
    }
    QString msgStr = QString("%1 %2:%3\n").arg(time).arg(senderStr).arg(msg);
    if(!ui->checkBox_pauseShow->isChecked())
    {
        ui->textBrowser_show_msg->moveCursor(QTextCursor::End);
        ui->textBrowser_show_msg->insertPlainText(msgStr);
        ui->textBrowser_show_msg->moveCursor(QTextCursor::End);
    }
    if(ui->checkBox_saveLog->isChecked())
    {
        saveLog(msgStr);
    }
    show_mutex.unlock();
}

void MainWindow::on_pushButton_clear_clicked()
{
    if(QMessageBox::Yes==QMessageBox::warning(this,NULL,tr("是否清空显示区域内容?"),QMessageBox::Yes|QMessageBox::No))
    {
        ui->textBrowser_show_msg->clear();
    }
}

void MainWindow::on_pushButton_delete_clicked()
{
    if(0==ui->comboBox_server_client->currentIndex())//服务器
    {
        server->close();
        server->deleteServer();
        QThread::msleep(500);
        delete server;
        showInformation(2,QString("服务器已删除!"));
    }
    else
    {
        if(1==ui->comboBox_server_client->currentIndex())//客户端
        {
            if(false==client_disconn)
            {
                client->closeConnect();
                QThread::msleep(500);
                delete client;
            }
            showInformation(2,QString("客户端已删除!"));
        }
    }
    ui->pushButton_creat->setDisabled(false);
    ui->comboBox_server_client->setDisabled(false);
    ui->pushButton_delete->setDisabled(true);
    ui->lineEdit_serverIP->setDisabled(false);
    ui->lineEdit_serverPort->setDisabled(false);
    ui->comboBox_prefix->setDisabled(false);
    ui->comboBox_suffix->setDisabled(false);
}

void MainWindow::server_ReadData(QString IP, int Port, QString readMsg)
{
    showInformation(1,QString("%1 %2:%3").arg(IP).arg(Port).arg(readMsg));
    readMsg.replace(server->prefix,"");
    readMsg.replace(server->suffix,"");
    int sleep_time;
    checkMsg(readMsg,sleep_time);
    if(!readMsg.isEmpty())
    {
        QThread::msleep(sleep_time);
        readMsg = QString("%1%2%3").arg(server->prefix).arg(readMsg).arg(server->suffix);
        server->sendData((quint16) Port,readMsg);
        showInformation(0,QString("%1 %2:%3").arg(IP).arg(Port).arg(readMsg));
    }
}

void MainWindow::checkMsg(QString &msg, int &sleep_time)
{
    QString strTemp = msg;
    QFile msgFile(msgFileName);
    if(msgFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream txtInput(&msgFile);
        QString lineStr;
        QRegExp msgRE("(.*),(.*),(.*)");
        while(!txtInput.atEnd())
        {
            lineStr = txtInput.readLine();
            if(0 <= lineStr.indexOf(msgRE))
            {
                if(msgRE.cap(1).contains(msg))
                {
                    msg = msgRE.cap(2);
                    sleep_time = msgRE.cap(3).toInt();
                    break;
                }
            }
        }
        msgFile.close();
    }
    if(strTemp == msg)
    {
        msg = "";
    }
    if(0 >= sleep_time)
    {
        sleep_time = 500;
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    QString sendStr = ui->lineEdit_input->text();
    if(sendStr.isEmpty())
    {
        return;
    }
    if(0==ui->comboBox_server_client->currentIndex())//服务器
    {
        if(!ui->pushButton_creat->isEnabled())
        {
            QString ipTemp = ui->lineEdit_serverIP_send->text();
            QString portTemp = ui->lineEdit_serverPort_send->text();
            if(!portTemp.isEmpty())
            {
                sendStr = QString("%1%2%3").arg(server->prefix).arg(sendStr).arg(server->suffix);
                server->sendData((quint16)portTemp.toInt(), sendStr);
                showInformation(0,QString("%1 %2:%3").arg(ipTemp).arg(portTemp).arg(sendStr));
            }
            else
            {
                QMessageBox::warning(this,NULL,tr("请输入一个正确的端口号!"));
            }
        }
    }
    else
    {
        if(1==ui->comboBox_server_client->currentIndex())//客户端
        {
            sendStr = QString("%1%2%3").arg(client->prefix).arg(sendStr).arg(client->suffix);
            client->clientSendData(sendStr);
            showInformation(0,QString("%1 %2:%3").arg(ui->lineEdit_serverIP->text()).arg(ui->lineEdit_serverPort->text()).arg(sendStr));
        }
    }
}

void MainWindow::on_comboBox_server_client_currentIndexChanged(int index)
{
    if(0==index)
    {
        ui->lineEdit_serverIP_send->setDisabled(false);
        ui->lineEdit_serverPort_send->setDisabled(false);
    }
    else
    {
        ui->lineEdit_serverIP_send->setDisabled(true);
        ui->lineEdit_serverPort_send->setDisabled(true);
    }
    ui->lineEdit_serverIP_send->clear();
    ui->lineEdit_serverPort_send->clear();
}

void MainWindow::on_pushButton_loadFile_clicked()
{
    msgFileName = QFileDialog::getOpenFileName(this,tr("load"),"..\\Message_list.txt");
    if(!msgFileName.isEmpty())
    {
        showInformation(2,QString("加载通讯文件%1成功!").arg(msgFileName));
    }
}

void MainWindow::server_clientConnect(QString IP, int Port)
{
    showInformation(2,QString("客户端%1 %2已连接!").arg(IP).arg(Port));
}

void MainWindow::server_clientDisconnected(QString IP, int Port)
{
    showInformation(2,QString("客户端%1 %2已断开!").arg(IP).arg(Port));
}

void MainWindow::client_readData(int clientID, QString IP, int Port, QString msg)
{
    IP = ui->lineEdit_serverIP->text();
    Port = ui->lineEdit_serverPort->text().toInt();
    showInformation(1,QString("%1 %2:%3").arg(IP).arg(Port).arg(msg));

    if(msg.contains("@Robot init done ACK"))//只用于ICT测试---------------------
    {
        if(initDoneTimer->isActive())
            initDoneTimer->stop();
    }
    if(msg.contains("@Test ready ACK"))//只用于ICT测试---------------------
    {
        if(startTestTimer->isActive())
            startTestTimer->stop();

        QThread::msleep(100);
        QString ICT_path = QString("%1:\\%2").arg("V").arg("PDM&ERP/UR/Result.txt");
        QFile file(ICT_path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out << "C4-L1N,A,ICT,J1741705,3I1101, ,ICT_V04R03,AH13012400,P, , ,0,";
            file.close();
        }
    }
    if(msg.contains("@Sort complete ACK"))//只用于ICT测试---------------------
    {
        if(sortCompleteTimer->isActive())
            sortCompleteTimer->stop();
    }

    msg.replace(client->prefix,"");
    msg.replace(client->suffix,"");
    int sleep_time;
    checkMsg(msg,sleep_time);
    if(!msg.isEmpty())
    {
        QThread::msleep(sleep_time);
        msg = QString("%1%2%3").arg(client->prefix).arg(msg).arg(client->suffix);
        client->clientSendData(msg);
        showInformation(0,QString("%1 %2:%3").arg(IP).arg(Port).arg(msg));

        if(msg.contains("@Robot init ACK"))//只用于ICT测试---------------------
        {
            if(!initDoneTimer->isActive())
                initDoneTimer->start(TIMER_TIME_OUT);
            return;
        }
        if(msg.contains("@Read SN Done ACK"))//只用于ICT测试---------------------
        {
            if(!writeFileTimer->isActive())
                writeFileTimer->start(TIMER_TIME_OUT);
            return;
        }
        if(msg.contains("@Scan done ACK"))//只用于ICT测试---------------------
        {
            if(!startTestTimer->isActive())
                startTestTimer->start(TIMER_TIME_OUT);
            if(writeFileTimer->isActive())
                writeFileTimer->stop();
            return;
        }
        if(msg.contains("@Pass done ACK") || msg.contains("@Fail done ACK"))//只用于ICT测试---------------------
        {
            if(!sortCompleteTimer->isActive())
                sortCompleteTimer->start(TIMER_TIME_OUT);
            return;
        }
    }
}

void MainWindow::client_clientDisConnect(int clientID, QString IP, int Port)
{
    showInformation(2,QString("断开服务器%1 %2连接!").arg(IP).arg(Port));
    client_disconn = true;
}

void MainWindow::on_checkBox_saveLog_clicked()
{
    if(ui->checkBox_saveLog->isChecked())
    {
        logFileName = QFileDialog::getSaveFileName(this,tr("选择存储路径"),"..\\Message_log.txt");
        if(logFileName.isEmpty())
        {
            logFileName = "";
            ui->checkBox_saveLog->setChecked(false);
            return;
        }
        showInformation(2,QString("通讯信息将保存到文件%1当中!").arg(logFileName));
    }
    else
    {
        logFileName = "";
    }
}

void MainWindow::saveLog(QString strMsg)
{
    if(!logFileName.isEmpty())
    {
        QFile file(logFileName);
        if(file.open(QFile::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            out << strMsg;
            if(!file.flush())
            {
                qWarning("log文件刷新失败!");
            }
            file.close();
        }
    }
}

void MainWindow::initDoneTimerTimeout()//只用于ICT测试---------------------
{
    QString msg = QString("%1%2%3").arg(client->prefix).arg("Robot init done").arg(client->suffix);
    client->clientSendData(QString("%1%2%3").arg(client->prefix).arg(msg).arg(client->suffix));
    showInformation(0,QString("%1 %2:%3").arg(client->peerAddress().toString()).arg(client->peerPort()).arg(msg));
}

void MainWindow::writeFileTimerTimeout()
{
    QThread::msleep(100);
    QString ICT_path = QString("%1:\\%2").arg("V").arg("PDM&ERP/UR/Request.txt");
    QFile file1(ICT_path);
    if (file1.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out1(&file1);
        out1 << "AH13012400,PASS, , ,0, ,";
        if(!file1.flush())
        {
            qWarning("Request文件刷新失败!");
        }
        file1.close();
    }
}

void MainWindow::startTestTimerTimeout()//只用于ICT测试---------------------
{
    QString msg = QString("%1%2%3").arg(client->prefix).arg("Test ready").arg(client->suffix);
    client->clientSendData(QString("%1%2%3").arg(client->prefix).arg(msg).arg(client->suffix));
    showInformation(0,QString("%1 %2:%3").arg(client->peerAddress().toString()).arg(client->peerPort()).arg(msg));

}

void MainWindow::sortCompleteTimerTimeout()//只用于ICT测试---------------------
{
    QString msg = QString("%1%2%3").arg(client->prefix).arg("Sort complete").arg(client->suffix);
    client->clientSendData(QString("%1%2%3").arg(client->prefix).arg(msg).arg(client->suffix));
    showInformation(0,QString("%1 %2:%3").arg(client->peerAddress().toString()).arg(client->peerPort()).arg(msg));
}

void MainWindow::on_pushButton_timer_clicked()
{
    timerFileName = QFileDialog::getOpenFileName(this,tr("load"),"..\\timer_list.txt");
    if(!timerFileName.isEmpty())
    {
        showInformation(2,QString("加载定时文件%1成功!").arg(timerFileName));
    }
}

void MainWindow::single_Timeout(QString id)
{
    if("1"==id)
    {
        qDebug("id1_timer");
    }
    if("2"==id)
    {
        qDebug("id2_timer");
    }
    if("3"==id)
    {
        qDebug("id3_timer");
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    qDebug()<<event->timerId();
}
