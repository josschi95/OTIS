#include <QUdpSocket>
#include <QDateTime>
#include <QTimer>
#include <QFile>

#include "testwindow.h"
#include "ui_testwindow.h"
#include "database_manager.h"


TestWindow::TestWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TestWindow)
{
    ui->setupUi(this);

    logTimer = new QTimer(this);
    connect(logTimer, &QTimer::timeout, this, &TestWindow::onTimerTrigger);

    if (DatabaseManager::instance().logCount() == 0) { // TESTING
        // Submit some filler logs if the db is empty
        qDebug() << "Adding filler logs";
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

    if (DatabaseManager::instance().ruleCount() == 0) { // TESTING

        auto failedLoginsRule = std::make_shared<Rule>();
        failedLoginsRule->name = "Multiple Failed Logins";
        failedLoginsRule->alertSeverity = Severity::Warning;
        failedLoginsRule->msgid = "LOGIN_FAILURE";
        failedLoginsRule->msgidOp = StringComparison::ExactMatch;
        failedLoginsRule->perHost = true;
        failedLoginsRule->thresholdCount = 3;
        failedLoginsRule->timeWindow = QTime().fromString("00:01:00");
        failedLoginsRule->triggerCondition = ComparisonOperator::gte;

        auto noLogRule = std::make_shared<Rule>();
        noLogRule->name = "No Incoming Logs";
        noLogRule->alertSeverity = Severity::Error;
        noLogRule->thresholdCount = 1;
        noLogRule->timeWindow = QTime().fromString("00:01:00");
        noLogRule->triggerCondition = ComparisonOperator::lt;

        DatabaseManager::instance().saveRule(failedLoginsRule);
        DatabaseManager::instance().saveRule(noLogRule);
    }

    //QDateTime& ts, Severity se, const QString& so, const QString& rn
    if (DatabaseManager::instance().alertCount() == 0) { // TESTING
        auto alert1 = std::make_shared<Alert>(
            QDateTime::currentDateTime(),
            Severity::Alert,
            "source_name",
            "rule_name"
        );
        DatabaseManager::instance().saveAlert(alert1);
    }
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


// this one is just used for testing that the per-host setting for rules is working
void TestWindow::on_user_failedLoginButton_2_clicked()
{
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString log = QString("<138>1 %1 workstation-02 winlogon 4625 LOGIN_FAILURE - Failed login attempt for user 'admin' from 10.1.2.33").arg(timestamp);
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


void TestWindow::on_vibrationSensorEnabledCheckbox_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::CheckState::Checked) {
        logTimer->setInterval(60 * 1000); // 60 seconds
        logTimer->start();
    } else if (arg1 == Qt::CheckState::Unchecked) {
        logTimer->stop();
    }
}


void TestWindow::onTimerTrigger()
{
    if (ui->vibrationSensorOnlineCheckbox->isChecked()) {
        auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        QString structuredData = R"([sensor@32473 id="vib-002" type="vibration" unit="mm/s" value="1.2" status="normal" threshold="5.0"])";
        QString log = QString("<134>1 %1 sensor-gateway-01 vibration-sensor 4521 ID47 %2 Vibration levels within normal operating range.").arg(timestamp, structuredData);
        sendTestLog(log.toUtf8());
    }
    logTimer->start();
}
