#include <QUdpSocket>
#include <QDateTime>
#include <QFile>

#include "testwindow.h"
#include "ui_testwindow.h"
#include "settings_manager.h"

TestWindow::TestWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TestWindow)
{
    ui->setupUi(this);
    connect(ui->fakeLogButton, &QPushButton::clicked, this, &TestWindow::onFakeLogButtonClicked);

    foo();
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
        //TODO: Variety
        );
    QByteArray msg = line.toUtf8();
    sendTestLog(msg);
}

void TestWindow::foo()
{
    QFile file("test_logs.txt");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open test_log file: " << file.errorString();
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            sendTestLog(line.toUtf8());
        }
    }

    file.close();
}
