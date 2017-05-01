#ifndef TCPIPSERVER_H
#define TCPIPSERVER_H

/*需要在.pro文件添加QT += network*/
#include <QTcpServer>
#include "tcpipclient.h"

class TcpIpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpIpServer(QObject *parent = 0);
    bool stratListen(QString address,quint16 port); //监听连接
    void closeServerListen();//关闭监听
    void deleteServer();//删除服务器对象
    void sendData(quint16 port,QString sendMsg);    //发送数据
    int getClientID(QString IP,int Port);//获取分配给client的ID
    QString prefix;
    QString suffix;
    void set_prefix_suffix(QString prefix,QString suffix);

private:
    QList<TcpIpClient *> clientSocketList;  //保存客户端对象集合
    QList<int> clientSocketID;              //保存客户端对象ID集合

protected:
    void incomingConnection(qintptr socketDescriptor);  //虚函数，有tcp请求时会触发

signals:
    void serverReadData(QString IP,int Port,QString readMsg);   //读取数据信号
    void clientConnect(QString IP,int Port);                    //发送已连接的客户端IP地址和Port信息
    void clientDisconnected(QString IP,int Port);               //发送已断开连接的客户端IP地址和Port信息
    void errorMessage(QString errorMsg);                        //错误信号

private slots:
    void serverReadMsg(int clientID,QString IP,int Port,QString readMsg);   //读取数据，并发射信号给上层调用
    void disConnect(int clientID,QString IP,int Port);                      //获得已断开的客户端信息，并发送信号给上层调用

public slots:

};

#endif // TCPIPSERVER_H
