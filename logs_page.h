#pragma once

#include <QWidget>

#include "tablefiltersdialog.h"

namespace Ui {
class LogsPage;
}

class LogsPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogsPage(QWidget *parent = nullptr);
    void initialize();

public slots:
    void openFiltersDialog();

private:
    bool initialized = false;
    TableFiltersDialog *filtersDialog = nullptr;

};
