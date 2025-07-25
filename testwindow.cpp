#include <QUdpSocket>

#include "testwindow.h"
#include "ui_testwindow.h"


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
    qDebug() << "Button pressed";
    QUdpSocket* udpSocket = new QUdpSocket(this);
    QByteArray msg = "<13>Jul 23 20:30:00 host snort: Alert: Something happened";
    QHostAddress address("127.0.0.1");
    quint16 port = 5140;

    qint64 bytesSent = udpSocket->writeDatagram(msg, address, port);
    if (bytesSent == -1) {
        qDebug() << "Failed to send UDP datagram:" << udpSocket->errorString();
    } else {
        qDebug() << "Sent UDP datagram, bytes:" << bytesSent;
    }
}
