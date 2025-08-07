#pragma once

#include <QDialog>

#include "logs.h"


namespace Ui {
class TableFiltersDialog;
}

class TableFiltersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableFiltersDialog(QWidget *parent = nullptr);
    ~TableFiltersDialog();
    void applyFilters(LogFilters& filters) const;

public slots:
    void resetFilters();

private:
    Ui::TableFiltersDialog *ui;
    bool startDTEdited = false;
    bool endDTEdited = false;
};
