#pragma once

#include <QWidget>
#include <QTableWidget>

#include "tablefiltersdialog.h"
#include "logs.h"


namespace Ui {
class LogsPage;
}

class LogsPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogsPage(QWidget *parent = nullptr);
    void initialize();
    void addRow(const QStringList& log);

public slots:
    void openFiltersDialog();

private slots:
    void refreshLogTable();

private:
    bool initialized = false;
    QTableWidget *logTable;
    TableFiltersDialog *filtersDialog = nullptr;
    LogFilters currentFilters;
};
