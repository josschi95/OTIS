#include <QSettings>
#include <QDebug>

#include "syslog_receiver.h"


SyslogReceiver::SyslogReceiver(QObject *parent) : QObject(parent)
{
    createSocket();
}

void SyslogReceiver::createSocket()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    int port = settings.value("port", 5140).toInt();

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, port);
    connect(udpSocket, &QUdpSocket::readyRead, this, &SyslogReceiver::processPendingDatagrams);
}

void SyslogReceiver::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        qDebug() << "SyslogReceiver processing datagram";
        QByteArray datagram;
        datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));
        udpSocket->readDatagram(datagram.data(), datagram.size());
        emit logReceived(QString::fromUtf8(datagram));
    }
}
