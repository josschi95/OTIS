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
    connect(&DatabaseManager::instance(), &DatabaseManager::alertSaved, this, &Overview::updateAlertCharts);
    connect(&DatabaseManager::instance(), &DatabaseManager::logInserted, this, &Overview::updateLogCharts);

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
        severitySets[static_cast<int>(Severity::Emergency)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Alert)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Critical)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Error)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Warning)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Notice)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Informational)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Debug)]->append({0, 0, 0, 0, 0});

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
        axisX->setMin(0);
        axisX->setMax(20);
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
        severitySets[static_cast<int>(Severity::Emergency)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Alert)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Critical)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Error)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Warning)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Notice)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Informational)]->append({0, 0, 0, 0, 0});
        severitySets[static_cast<int>(Severity::Debug)]->append({0, 0, 0, 0, 0});

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
        axisX->setMin(0);
        axisX->setMax(100);
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
        alertTimelineChart->legend()->setVisible(false);
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
        axisY->setMax(20);
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
    // Line Chart
    updateAlertTimeline();

    // Pie Chart
    QList<int> alertCounts = DatabaseManager::instance().getSeverityCountReport(true);
    auto slices = alertsBySeveritySeries->slices();
    for (int i = 0; i < alertCounts.size(); ++i) {
        slices[i]->setValue(alertCounts[i]);
    }

    // Bar Chart
    auto noisyAlerts = DatabaseManager::instance().getNoisyDevices(true);

    QStringList hostnames;
    int index = 0;
    //int max = 0;
    auto it = noisyAlerts.begin();
    auto sets = alertsByDeviceSeries->barSets();
    while (it != noisyAlerts.end()) {
        hostnames << it.key();
        for (int i = 0; i < it.value().size(); ++i) {
            sets[i]->replace(index, it.value()[i]);
            //if (it.value()[i] > max) max = it.value()[i];
        }

        index++;
        it++;
    }

    auto axes = alertsByDeviceSeries->attachedAxes();
    //QValueAxis *valAxis = static_cast<QValueAxis*>(axes[0]);
    QBarCategoryAxis *catAxis = static_cast<QBarCategoryAxis*>(axes[1]);
    catAxis->clear();
    catAxis->append(hostnames);
    //valAxis->setMax(max);
}

void Overview::updateLogCharts()
{
    // Pie Chart
    QList<int> logCounts = DatabaseManager::instance().getSeverityCountReport(false);
    auto slices = logsBySeveritySeries->slices();
    for (int i = 0; i < logCounts.size(); ++i) {
        slices[i]->setValue(logCounts[i]);
    }

    // Bar Chart
    auto noisyLogs = DatabaseManager::instance().getNoisyDevices(false);

    QStringList hostnames;
    int index = 0;
    //int max = 0;
    auto it = noisyLogs.begin();
    auto sets = logsByDeviceSeries->barSets();
    while (it != noisyLogs.end()) {
        hostnames << it.key();
        for (int i = 0; i < it.value().size(); ++i) {
            sets[i]->replace(index, it.value()[i]);
            //if (it.value()[i] > max) max = it.value()[i];
        }

        //qDebug() << it.key() << ": " << it.value();
        index++;
        it++;
    }

    auto axes = logsByDeviceSeries->attachedAxes();
    //QValueAxis *valAxis = static_cast<QValueAxis*>(axes[0]);
    QBarCategoryAxis *catAxis = static_cast<QBarCategoryAxis*>(axes[1]);
    catAxis->clear();
    catAxis->append(hostnames);

    //valAxis->setMax(max);
}


void Overview::updateAlertTimeline()
{
    alertTimelineSeries->clear();
    HourlyLogData logs = DatabaseManager::instance().alertCountPerHour();

    for (int i = 0; i < logs.hours.size(); ++i) {
        alertTimelineSeries->append(logs.hours[i].toMSecsSinceEpoch(), logs.counts[i]);
        //qDebug() << logs.hours[i] << " : " << logs.counts[i];
    }
}
