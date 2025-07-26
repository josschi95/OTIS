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

// Generate a fake log to ingest
void TestWindow::onFakeLogButtonClicked()
{
    QTimeZone tz = SettingsManager::instance().currentTimeZone();
    QString line = QString("%1 host snort: Alert: Something happened").arg(
        QDateTime::currentDateTime(tz).toString(Qt::ISODate)
        //TODO: Variety in source/severity/message
        );
    QByteArray msg = line.toUtf8();

    QUdpSocket* udpSocket = new QUdpSocket(this);
    QHostAddress address("127.0.0.1");
    quint16 port = 5140;

    qint64 bytesSent = udpSocket->writeDatagram(msg, address, port);
    if (bytesSent == -1) {
        qDebug() << "Failed to send UDP datagram:" << udpSocket->errorString();
    } else {
        qDebug() << "Sent UDP datagram, bytes:" << bytesSent;
    }
}
