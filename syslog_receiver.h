#pragma once

#include <QObject>
#include <QUdpSocket>


// Creates a UDP Socket and listens for incoming logs, then passes them off to LogParser
class SyslogReceiver : public QObject
{
    Q_OBJECT

public:
    explicit SyslogReceiver(QObject *parent = nullptr);
    void createSocket();

signals:
    void logReceived(const QString &line);

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket = nullptr;
};
