#include "alerts_page.h"
#include "database_manager.h"


AlertsPage::AlertsPage(QWidget *parent) : QWidget{parent} {}


void AlertsPage::initialize()
{
    if (initialized) return;
    initialized = true;

    alertsTable = this->findChild<QTableWidget*>("alertsTable");
    ackAlertButton = this->findChild<QPushButton*>("ackButton");

    connect(ackAlertButton, &QPushButton::clicked, this, &AlertsPage::acknowledgeAlert);

    connect(alertsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
        ackAlertButton->setEnabled(alertsTable->selectionModel()->hasSelection());
    });
    ackAlertButton->setEnabled(false);

    connect(&DatabaseManager::instance(), &DatabaseManager::alertSaved, this, &AlertsPage::refreshAlertsTable);
    connect(&DatabaseManager::instance(), &DatabaseManager::alertDeleted, this, &AlertsPage::refreshAlertsTable);
    refreshAlertsTable();
}


void AlertsPage::acknowledgeAlert()
{
    QTableWidgetItem *item = alertsTable->item(alertsTable->currentRow(), 0);
    int id = item->data(Qt::UserRole).value<int>();
    auto alert = AlertManager::instance().getAlertById(id);
    if (alert != nullptr) {
        if (alert->acknowledged) return;
        alert->acknowledged = true;
        DatabaseManager::instance().saveAlert(alert);
    }
}


void AlertsPage::refreshAlertsTable()
{
    alertsTable->setRowCount(0); // Clear table
    const auto rows = DatabaseManager::instance().queryAlerts();
    for (const auto& row : rows) addRow(row);
}


void AlertsPage::addRow(const QStringList &row)
{
    const int newRow = alertsTable->rowCount();
    alertsTable->insertRow(newRow);

    int id = row.last().toInt(); // id is added to end of list
    bool acknowledged = (row[row.size() - 2].toInt() != 0); // acknowledged is second to last
    for (int c = 0; c < row.size() - 2; ++c) {
        alertsTable->setItem(newRow, c, new QTableWidgetItem(row[c]));

        // Need reference to alert id for when selecting a row to acknowledge
        if (c == 0) alertsTable->item(newRow, c)->setData(Qt::UserRole, id);

        // Can't use ItemIsEnabled (gives same appearance) because then it's not selectable
        if (acknowledged) alertsTable->item(newRow, c)->setForeground(QBrush(Qt::gray));
    }
}
