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
    //connect(ui->fakeLogButton, &QPushButton::clicked, this, &TestWindow::onFakeLogButtonClicked);

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

void TestWindow::on_user_successfulLoginButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<134>1 %1 workstation-01 winlogon 4624 LOGIN_SUCCESS - User 'john.doe' successfully logged in from 10.1.2.33").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_user_failedLoginButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<138>1 %1 workstation-01 winlogon 4625 LOGIN_FAILURE - Failed login attempt for user 'admin' from 10.1.2.33").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_user_firewallDisabledButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<137>1 %1 workstation-01 endpoint-guard 3010 FIREWALL_DISABLED - Windows Defender Firewall disabled by user 'tech1'").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_hmi_operatorLoginButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<134>1 %1 hmi-panel-a hmi-runtime 3102 LOGIN_SUCCESS - Operator 'operator1' authenticated locally on HMI").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_hmi_pumpStartButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<133>1 %1 hmi-panel-a hmi-control 4100 PUMP_START - Pump P-101 started by operator 'operator1'").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_hmi_pumpStopButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<133>1 %1 hmi-panel-a hmi-control 4101 PUMP_STOP - Pump P-101 stopped by operator 'operator1'").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_hmi_unauthAccessButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<138>1 %1 hmi-panel-a hmi-auth 3201 ACCESS_DENIED - User 'tech1' attempted to access restricted screen 'System Config'").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_scada_plcStartButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<134>1 %1 scada plc-control 2010 PLC_START - PLC 'plc01' switched to RUN mode by engineer 'admin1'").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_scada_plcStopButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<134>1 %1 scada plc-control 2011 PLC_STOP - PLC 'plc01' switched to STOP mode by engineer 'admin1'").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_scada_periodicReadingButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<134>1 %1 scada sensor-monitor 6001 SENSOR_READING - Read value 78.5°C from 'ValveTemp' on plc01").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_scada_sensorOfflineButton_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<137>1 %1 scada sensor-monitor 7102 SENSOR_OFFLINE - Lost communication with sensor 'vib-17' (Modbus ID 17)").arg(timestamp);
    sendTestLog(log.toUtf8());
}


void TestWindow::on_tempSensorSlider_valueChanged(int value)
{
    ui->tempSensorValueLabel->setText(QString("%1°C").arg(value));
}


void TestWindow::on_tempSensorSlider_sliderReleased()
{
    static const int tempAlertLevel = 90;
    const int temp = ui->tempSensorSlider->value();
    if (temp > tempAlertLevel) {
        auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        //TODO: Priority should be higher
        QString log = QString("<134>1 %1 scada sensor-monitor 6001 SENSOR_READING - Read value %2°C from 'ValveTemp' on plc01").arg(timestamp, temp);
        sendTestLog(log.toUtf8());
    }
}


// Will need to create a timer with a duration of 60s. Start/Stop with enabled, at timeout, reset and report if sensor is offline, else report normal status
void TestWindow::on_vibrationSensorOnlineCheckbox_checkStateChanged(const Qt::CheckState &arg1)
{
    //TODO
}


void TestWindow::on_vibrationSensorEnabledCheckbox_checkStateChanged(const Qt::CheckState &arg1)
{
    //TODO
}








void TestWindow::on_user_failedLoginButton_2_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<138>1 %1 workstation-02 winlogon 4625 LOGIN_FAILURE - Failed login attempt for user 'admin' from 10.1.2.33").arg(timestamp);
    sendTestLog(log.toUtf8());
}

