#pragma once

#include <QObject>
#include <QUdpSocket>


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
    QUdpSocket *udpSocket;
};
