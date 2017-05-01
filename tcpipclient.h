#ifndef TCPIPCLIENT_H
#define TCPIPCLIENT_H

/*需要在.pro文件添加QT += network*/
#include <QTcpSocket>

class TcpIpClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpIpClient(QObject *parent = 0);
    int clientID;//客户端序号，由服务器端调配
    QString prefix;
    QString suffix;


    void clientSendData(QString msg);//客户端发送数据
    bool newConnect(const QString address, quint16 port);//连接服务器
    void closeConnect();//断开连接
    bool bindClient(const QString address, quint16 port=0);//绑定客户端对象到一个固定的IP和Port

private:
    QByteArray readByteMsg;//接收的字符串信息

signals:
    void readData(int clientID,QString IP,int Port,QString msg);//客户端接收数据之后，发送信号给上层调用，根据客户端信息解析
    void clientDisConnect(int clientID,QString IP,int Port);//客户端断开连接之后，发送断开的客户端信息信号给上层调用
    void cliendErrorMsg(QString errorMsg);

private slots:
    void clientReadData();//客户端接收数据
    void DisConnect();

public slots:

};

#endif // TCPIPCLIENT_H
