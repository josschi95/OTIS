#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>

#include "logs_page.h"
#include "database_manager.h"


//NOTE: parent and children are apparently still null here
LogsPage::LogsPage(QWidget *parent) : QWidget(parent) {}


void LogsPage::initialize()
{
    if (initialized) return;
    initialized = true;

    logTable = this->findChild<QTableWidget*>("logTable");

    QPushButton* filtersButton = this->findChild<QPushButton*>("tableFiltersButton");
    if (filtersButton) {
        connect(filtersButton, &QPushButton::clicked, this, &LogsPage::openFiltersDialog);
    } else {
        qWarning() << "filtersButton not found";
    }

    filtersDialog = new TableFiltersDialog(this);
    connect(filtersDialog, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            refreshLogTable();
        } else { // Rejected or closed
            filtersDialog->resetFilters();
        }
    });

    connect(&DatabaseManager::instance(), &DatabaseManager::logInserted, this, &LogsPage::addRow);

    refreshLogTable();
}


void LogsPage::openFiltersDialog()
{
    filtersDialog->show();
    filtersDialog->raise();
    filtersDialog->activateWindow();
}


void LogsPage::addRow(const QStringList& row)
{
    //TODO: Check if new rule matches current filter rules

    const int newRow = logTable->rowCount();
    logTable->insertRow(newRow);
    for (int c = 0; c < row.size(); ++c) {
        logTable->setItem(newRow, c, new QTableWidgetItem(row[c]));
    }
}


void LogsPage::refreshLogTable()
{
    logTable->setRowCount(0); // Clear table

    filtersDialog->applyFilters(currentFilters);

    const auto rows = DatabaseManager::instance().queryLogs(currentFilters);
    //qDebug() << "DB query rows returned: " << rows.size();

    for (const auto& row : rows) {
        addRow(row);
    }
}
