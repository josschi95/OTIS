#pragma once

#include <QObject>
#include <QWidget>
#include <QPieSeries>
#include <QHorizontalStackedBarSeries>
#include <QLineSeries>


namespace Ui {
class Overview;
}

class Overview : public QWidget
{
    Q_OBJECT

public:
    explicit Overview(QWidget *parent = nullptr);
    void initialize();

private slots:
    void updateAlertCharts();
    void updateLogCharts();
    void updateAlertTimeline();

private:
    bool initialized = false;
    QPieSeries *alertsBySeveritySeries;
    QHorizontalStackedBarSeries *alertsByDeviceSeries;
    QPieSeries *logsBySeveritySeries;
    QHorizontalStackedBarSeries *logsByDeviceSeries;
    QLineSeries *alertTimelineSeries;
};

