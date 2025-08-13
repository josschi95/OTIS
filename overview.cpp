#include <QtCharts>
#include <QtCharts/QPieSeries>

#include "overview.h"
#include "enums.h"
#include "database_manager.h"
#include "logs.h"

Overview::Overview(QWidget *parent) : QWidget{parent} {}

void Overview::initialize()
{
    static const QStringList severities = {
        "Emergency", "Alert", "Critical", "Error",
        "Warning", "Notice", "Informational", "Debug"
    };

    if (initialized) return;
    initialized = true;

    // Probably would be an issue if there are a lot of logs coming in?
    connect(&DatabaseManager::instance(), &DatabaseManager::logInserted, this, &Overview::updateAlertTimeline);

    { // Alerts By Severity
        alertsBySeveritySeries = new QPieSeries();
        alertsBySeveritySeries->setHoleSize(0.5);
        alertsBySeveritySeries->append("Emergency", 0);
        alertsBySeveritySeries->append("Alert", 1);
        alertsBySeveritySeries->append("Critical", 2);
        alertsBySeveritySeries->append("Error", 3);
        alertsBySeveritySeries->append("Warning", 4);
        alertsBySeveritySeries->append("Notice", 5);
        alertsBySeveritySeries->append("Informational", 6);
        alertsBySeveritySeries->append("Debug", 7);


        alertsBySeveritySeries->setLabelsVisible(false);
        for (auto slice : alertsBySeveritySeries->slices()) {
            QString text = QString("%1: %2")
                            .arg(slice->label())
                            .arg(slice->value());
            slice->setLabel(text);
        }

        QChart *alertsBySeverityChart = new QChart();
        alertsBySeverityChart->addSeries(alertsBySeveritySeries);
        alertsBySeverityChart->legend()->setVisible(true);
        alertsBySeverityChart->setTitle("Alerts by Severity");
        alertsBySeverityChart->setTheme(QChart::ChartThemeDark);
        alertsBySeverityChart->legend()->setAlignment(Qt::AlignLeft);
        alertsBySeverityChart->setAnimationOptions(QChart::SeriesAnimations);
        alertsBySeverityChart->setBackgroundVisible(false);
        alertsBySeverityChart->setVisible(true);

        QChartView *alertsBySeverityChartView = new QChartView();
        alertsBySeverityChartView->setChart(alertsBySeverityChart);
        alertsBySeverityChartView->setRenderHint(QPainter::Antialiasing);
        alertsBySeverityChartView->setVisible(true);

        QWidget *w1 = this->findChild<QWidget*>("_1_alertsBySeverity");
        QVBoxLayout *l1 = new QVBoxLayout(w1);
        l1->addWidget(alertsBySeverityChartView);

        alertsBySeveritySeries->slices().at(static_cast<int>(Severity::Alert))->setValue(1); //TESTING
    }


    { // Alerts By Device
        alertsByDeviceSeries = new QHorizontalStackedBarSeries();
        QList<QBarSet*> severitySets;
        for (const QString &severity : severities) {
            auto *set = new QBarSet(severity);
            severitySets.append(set);
        }

        // Example data — each severity set gets one value per device
        severitySets[static_cast<int>(Severity::Emergency)]->append({5, 2, 1, 3, 1});
        severitySets[static_cast<int>(Severity::Alert)]->append({3, 1, 0, 6, 3});
        severitySets[static_cast<int>(Severity::Critical)]->append({6, 3, 4, 10, 5});
        severitySets[static_cast<int>(Severity::Error)]->append({10, 5, 2, 4, 4});
        severitySets[static_cast<int>(Severity::Warning)]->append({4, 4, 3, 1, 0});
        severitySets[static_cast<int>(Severity::Notice)]->append({1, 0, 2, 7, 6});
        severitySets[static_cast<int>(Severity::Informational)]->append({7, 6, 3, 2, 1});
        severitySets[static_cast<int>(Severity::Debug)]->append({2, 1, 5, 5, 2});

        for (auto *set : severitySets) {
            alertsByDeviceSeries->append(set);
        }

        QChart *alertsByDeviceChart = new QChart();
        alertsByDeviceChart->addSeries(alertsByDeviceSeries);
        alertsByDeviceChart->legend()->setVisible(true);
        alertsByDeviceChart->setTitle("Alerts by Device");
        alertsByDeviceChart->setAnimationOptions(QChart::SeriesAnimations);
        alertsByDeviceChart->setTheme(QChart::ChartThemeDark);
        alertsByDeviceChart->setBackgroundVisible(false);
        alertsByDeviceChart->setVisible(true);

        QValueAxis *axisX = new QValueAxis();
        axisX->setTitleText("Alert Count");
        QBarCategoryAxis *axisY = new QBarCategoryAxis();
        axisY->append({"Device01", "Device02", "Device03", "Device04", "Device05"});
        alertsByDeviceChart->addAxis(axisX, Qt::AlignBottom);
        alertsByDeviceChart->addAxis(axisY, Qt::AlignLeft);
        alertsByDeviceSeries->attachAxis(axisX);
        alertsByDeviceSeries->attachAxis(axisY);

        QChartView *alertsByDeviceChartView = new QChartView();
        alertsByDeviceChartView->setChart(alertsByDeviceChart);
        alertsByDeviceChartView->setRenderHint(QPainter::Antialiasing);
        alertsByDeviceChartView->setVisible(true);

        QWidget *w2 = this->findChild<QWidget*>("_2_alertsByDevice");
        QVBoxLayout *l2 = new QVBoxLayout(w2);
        l2->addWidget(alertsByDeviceChartView);
    }


    { // Logs By Severity
        logsBySeveritySeries = new QPieSeries();
        logsBySeveritySeries->setHoleSize(0.5);
        logsBySeveritySeries->append("Emergency", 0);
        logsBySeveritySeries->append("Alert", 1);
        logsBySeveritySeries->append("Critical", 2);
        logsBySeveritySeries->append("Error", 3);
        logsBySeveritySeries->append("Warning", 4);
        logsBySeveritySeries->append("Notice", 5);
        logsBySeveritySeries->append("Informational", 6);
        logsBySeveritySeries->append("Debug", 7);

        logsBySeveritySeries->setLabelsVisible(false);
        for (auto slice : logsBySeveritySeries->slices()) {
            QString text = QString("%1: %2")
                            .arg(slice->label())
                            .arg(slice->value());
            slice->setLabel(text);
        }

        QChart *logsBySeverityChart = new QChart();
        logsBySeverityChart->addSeries(logsBySeveritySeries);
        logsBySeverityChart->legend()->setVisible(true);
        logsBySeverityChart->setTitle("Logs by Severity");
        logsBySeverityChart->setTheme(QChart::ChartThemeDark);
        logsBySeverityChart->legend()->setAlignment(Qt::AlignLeft);
        logsBySeverityChart->setAnimationOptions(QChart::SeriesAnimations);
        logsBySeverityChart->setBackgroundVisible(false);
        logsBySeverityChart->setVisible(true);

        QChartView *logsBySeverityChartView = new QChartView();
        logsBySeverityChartView->setChart(logsBySeverityChart);
        logsBySeverityChartView->setRenderHint(QPainter::Antialiasing);
        logsBySeverityChartView->setVisible(true);

        QWidget *w3 = this->findChild<QWidget*>("_3_logsBySeverity");
        QVBoxLayout *l3 = new QVBoxLayout(w3);
        l3->addWidget(logsBySeverityChartView);
    }


    { // Logs By Device
        logsByDeviceSeries = new QHorizontalStackedBarSeries();
        QList<QBarSet*> severitySets;
        for (const QString &severity : severities) {
            auto *set = new QBarSet(severity);
            severitySets.append(set);
        }

        // Example data — each severity set gets one value per device
        severitySets[static_cast<int>(Severity::Emergency)]->append({50, 20, 10, 30, 10});
        severitySets[static_cast<int>(Severity::Alert)]->append({30, 10, 00, 60, 30});
        severitySets[static_cast<int>(Severity::Critical)]->append({60, 30, 40, 100, 50});
        severitySets[static_cast<int>(Severity::Error)]->append({100, 50, 20, 40, 40});
        severitySets[static_cast<int>(Severity::Warning)]->append({40, 40, 30, 10, 0});
        severitySets[static_cast<int>(Severity::Notice)]->append({10, 0, 20, 70, 60});
        severitySets[static_cast<int>(Severity::Informational)]->append({70, 60, 30, 20, 10});
        severitySets[static_cast<int>(Severity::Debug)]->append({20, 10, 50, 50, 20});

        for (auto *set : severitySets) {
            logsByDeviceSeries->append(set);
        }

        QChart *logsByDeviceChart = new QChart();
        logsByDeviceChart->addSeries(logsByDeviceSeries);
        logsByDeviceChart->legend()->setVisible(true);
        logsByDeviceChart->setTitle("Logs by Device");
        logsByDeviceChart->setAnimationOptions(QChart::SeriesAnimations);
        logsByDeviceChart->setTheme(QChart::ChartThemeDark);
        logsByDeviceChart->setBackgroundVisible(false);
        logsByDeviceChart->setVisible(true);

        QValueAxis *axisX = new QValueAxis();
        axisX->setTitleText("Log Count");
        QBarCategoryAxis *axisY = new QBarCategoryAxis();
        axisY->append({"Device01", "Device02", "Device03", "Device04", "Device05"});
        logsByDeviceChart->addAxis(axisX, Qt::AlignBottom);
        logsByDeviceChart->addAxis(axisY, Qt::AlignLeft);
        logsByDeviceSeries->attachAxis(axisX);
        logsByDeviceSeries->attachAxis(axisY);

        QChartView *logsByDeviceChartView = new QChartView();
        logsByDeviceChartView->setChart(logsByDeviceChart);
        logsByDeviceChartView->setRenderHint(QPainter::Antialiasing);
        logsByDeviceChartView->setVisible(true);

        QWidget *w4 = this->findChild<QWidget*>("_4_logsByDevice");
        QVBoxLayout *l4 = new QVBoxLayout(w4);
        l4->addWidget(logsByDeviceChartView);
    }


    { // Alert Timeline
        alertTimelineSeries = new QLineSeries();
        QChart *alertTimelineChart = new QChart();
        alertTimelineChart->addSeries(alertTimelineSeries);
        alertTimelineChart->setTitle("Alert Timeline");
        alertTimelineChart->legend()->setVisible(true);
        alertTimelineChart->legend()->setAlignment(Qt::AlignRight);
        alertTimelineChart->setBackgroundVisible(false);
        alertTimelineChart->setMargins(QMargins(5, 5, 5, 5));

        updateAlertTimeline();

        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setFormat("hh");
        axisX->setTitleText("Time");
        axisX->setTickCount(13);
        axisX->setMin(QDateTime::currentDateTime().addDays(-1));
        axisX->setMax(QDateTime::currentDateTime());
        alertTimelineChart->addAxis(axisX, Qt::AlignBottom);
        alertTimelineSeries->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis;
        axisY->setLabelFormat("%i");
        axisY->setTitleText("Alerts");
        alertTimelineChart->addAxis(axisY, Qt::AlignLeft);
        alertTimelineSeries->attachAxis(axisY);

        QChartView *chartView = new QChartView(alertTimelineChart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setVisible(true);

        QWidget *w5 = this->findChild<QWidget*>("alertTimeline");
        QVBoxLayout *l5 = new QVBoxLayout(w5);
        l5->addWidget(chartView);
    }

    updateAlertCharts();
    updateLogCharts();
}

void Overview::updateAlertCharts()
{
    QList<int> alertCounts = DatabaseManager::instance().getSeverityCountReport(true);
    for (int i = 0; i < alertCounts.size(); ++i) {
        alertsBySeveritySeries->slices()[i]->setValue(alertCounts[i]);
    }

    auto noisyAlerts = DatabaseManager::instance().getNoisyDevices(true);
}

void Overview::updateLogCharts()
{
    QList<int> logCounts = DatabaseManager::instance().getSeverityCountReport(false);
    for (int i = 0; i < logCounts.size(); ++i) {
        logsBySeveritySeries->slices()[i]->setValue(logCounts[i]);
    }

    //auto noisyLogs = DatabaseManager::instance().getNoisyDevices(false);
}


void Overview::updateAlertTimeline()
{
    HourlyLogData logs = DatabaseManager::instance().alertCountPerHour();
    for (int i = 0; i < logs.hours.size(); ++i) {
        alertTimelineSeries->append(logs.hours[i].toMSecsSinceEpoch(), logs.counts[i]);
        //qDebug() << logs.hours[i] << " : " << logs.counts[i];
    }
}
