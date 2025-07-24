#pragma once

#include <QObject>
#include <QUdpSocket>

// Binds to UDP Port 5140 and listens for syslog files
class SyslogReceiver : public QObject
{
    Q_OBJECT

public:
    explicit SyslogReceiver(QObject *parent = nullptr);

signals:
    void logReceived(const QString &line);

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket;
};
