#include <QDebug>

#include "syslog_receiver.h"
#include "settings_manager.h"


SyslogReceiver::SyslogReceiver(QObject *parent) : QObject(parent)
{
    createSocket();
    connect(SettingsManager::instance(), &SettingsManager::syslogPortChanged, this, &SyslogReceiver::createSocket);
}

void SyslogReceiver::createSocket()
{
    int port = SettingsManager::instance()->getPort();

    if (udpSocket) {
        //qDebug() << "Tearing down old socket";
        udpSocket->close();
        udpSocket->deleteLater();
        udpSocket = nullptr;
    }

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, port);
    connect(udpSocket, &QUdpSocket::readyRead, this, &SyslogReceiver::processPendingDatagrams);
}

void SyslogReceiver::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        //qDebug() << "SyslogReceiver processing datagram";
        QByteArray datagram;
        datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));
        udpSocket->readDatagram(datagram.data(), datagram.size());
        emit logReceived(QString::fromUtf8(datagram));
    }
}
