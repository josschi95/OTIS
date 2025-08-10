#pragma once

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>


namespace Ui {
class AlertsPage;
}

class AlertsPage : public QWidget
{
    Q_OBJECT

public:
    explicit AlertsPage(QWidget *parent = nullptr);
    void initialize();

private slots:
    void refreshAlertsTable();
    void acknowledgeAlert();

private:
    bool initialized = false;
    QTableWidget *alertsTable;
    QPushButton *ackAlertButton;
    void addRow(const QStringList& row);
};
