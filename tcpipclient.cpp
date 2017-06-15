#include "tcpipclient.h"
#include <QHostAddress>

TcpIpClient::TcpIpClient(QObject *parent) :
    QTcpSocket(parent)
{
    clientID = -1;
    readByteMsg = "";
//    prefix = PREFIX;
//    suffix = SUFFIX;

    connect(this,&TcpIpClient::readyRead,this,&TcpIpClient::clientReadData);
    connect(this,&TcpIpClient::disconnected,this,&TcpIpClient::DisConnect);//关闭连接时，发送断开连接信号
//    connect(this,&TcpIpClient::disconnected,this,&TcpIpClient::deleteLater);//关闭连接时，对象自动删除
}

void TcpIpClient::clientSendData(QString msg)
{
    QByteArray sendByte = msg.toLatin1();
    this->write(sendByte);
}

void TcpIpClient::clientReadData()
{
    if(prefix.isEmpty() && suffix.isEmpty())
    {//如果无前缀无后缀，直接读取返回
        readByteMsg = this->readAll();
        emit readData(this->clientID,this->peerAddress().toString(),this->peerPort(),QString(readByteMsg));
        readByteMsg = "";
        return;
    }
    QByteArray pre = prefix.toLatin1();     //获得前缀
    QByteArray suf = suffix.toLatin1();     //获得后缀
    QByteArray byteBuf;     //接收数据缓冲区

    byteBuf = this->readAll();
    readByteMsg.append(byteBuf);
    if(1024<=readByteMsg.length())
    {
        emit readData(this->clientID,this->peerAddress().toString(),this->peerPort(),QString(readByteMsg));
        readByteMsg = "";
        return;
    }
    /*判断是否接收完毕*/
    if(!prefix.isEmpty() && suffix.isEmpty())
    {   //如果有前缀无后缀
        if(!readByteMsg.contains(pre))
        {
            return;
        }
    }
    else
        if(prefix.isEmpty() && !suffix.isEmpty())
        {   //如果无前缀有后缀
            if(!readByteMsg.contains(suf))
            {
                return;
            }
        }
        else
            if(!prefix.isEmpty() && !suffix.isEmpty())
            {   //如果有前缀有后缀
                if(!readByteMsg.contains(pre) && !readByteMsg.contains(suf))
                {
                    return;
                }
            }

    emit readData(this->clientID,this->peerAddress().toString(),this->peerPort(),QString(readByteMsg));
    readByteMsg = "";
}

void TcpIpClient::DisConnect()
{
    //断开连接时，发送断开信号
    emit clientDisConnect(this->clientID,this->peerAddress().toString(),this->peerPort());
}

bool TcpIpClient::newConnect(const QString address, quint16 port)
{
//    this->abort();//关闭已有连接
    this->connectToHost(QHostAddress(address),port);
    if (!this->waitForConnected(3000))
    {
        emit cliendErrorMsg(this->errorString()+"\n");
        return false;
    }
    return true;
}

void TcpIpClient::closeConnect()
{
    this->disconnectFromHost();
//    if(!(this->state() == QAbstractSocket::UnconnectedState || this->waitForDisconnected(1000)))
//        emit cliendErrorMsg(this->errorString()+"\n");
}

bool TcpIpClient::bindClient(const QString address, quint16 port)
{
    QHostAddress addr(address);
    if(this->bind(addr, port, DefaultForPlatform))
        return true;

    return false;
}
