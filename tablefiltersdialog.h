#pragma once

#include <QDialog>

#include "database_manager.h"


namespace Ui {
class TableFiltersDialog;
}

class TableFiltersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableFiltersDialog(QWidget *parent = nullptr);
    ~TableFiltersDialog();
    LogFilters getFilters() const;
    void applyFilters(LogFilters& filters) const;

public slots:
    void resetFilters();

private:
    Ui::TableFiltersDialog *ui;
    bool startDTEdited = false;
    bool endDTEdited = false;
};
