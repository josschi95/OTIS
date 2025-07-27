#include <QUdpSocket>
#include <QDateTime>

#include "testwindow.h"
#include "ui_testwindow.h"
#include "settings_manager.h"

TestWindow::TestWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TestWindow)
{
    ui->setupUi(this);
    connect(ui->fakeLogButton, &QPushButton::clicked, this, &TestWindow::onFakeLogButtonClicked);
}

TestWindow::~TestWindow()
{
    delete ui;
}

void TestWindow::sendTestLog(QByteArray log)
{
    QUdpSocket* udpSocket = new QUdpSocket(this);
    QHostAddress address("127.0.0.1");
    quint16 port = 5140;

    qint64 bytesSent = udpSocket->writeDatagram(log, address, port);
    if (bytesSent == -1) {
        qDebug() << "Failed to send UDP datagram:" << udpSocket->errorString();
    } else {
        //qDebug() << "Sent UDP datagram, bytes:" << bytesSent;
    }
}

// Generate a fake log to ingest
void TestWindow::onFakeLogButtonClicked()
{
    foo();
    return;
    QTimeZone tz = SettingsManager::instance()->currentTimeZone();
    QString line = QString("%1 host snort: Alert: Something happened").arg(
        QDateTime::currentDateTime(tz).toString(Qt::ISODate)
        //TODO: Variety in source/severity/message
        );
    QByteArray msg = line.toUtf8();
    sendTestLog(msg);
}

void TestWindow::foo()
{
    QStringList logs = {
        "2025-07-27T13:32:15-04:00 host01 snort Attempted User Privilege Gain",
        "2025-07-27T13:35:48-04:00 host02 snort Suspicious DNS request to known malicious domain",
        "2025-07-27T13:37:59-04:00 host01 snort TCP flood detected targeting port 443",
        "2025-07-27T13:40:12-04:00 host03 snort ICMP packet with oversized payload",
        "2025-07-27T13:42:07-04:00 host02 snort Unauthorized access attempt to SMB share",

        "2025-07-27T13:33:01-04:00 plc01 PLC: Temperature threshold exceeded on sensor 2",
        "2025-07-27T13:36:15-04:00 plc02 PLC: Motor overload detected on conveyor 3",
        "2025-07-27T13:38:42-04:00 plc01 PLC: Sensor failure reported from station 7",
        "2025-07-27T13:41:03-04:00 plc03 PLC: Emergency stop activated by operator panel",
        "2025-07-27T13:43:19-04:00 plc02 PLC: Unexpected voltage drop detected in circuit A",
    };

    for (const QString& log : logs) {
        sendTestLog(log.toUtf8());
    }
}
