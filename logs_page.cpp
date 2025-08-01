#include <QPushButton>

#include "logs_page.h"


LogsPage::LogsPage(QWidget *parent)
    : QWidget(parent)
{
    //NOTE: parent and children are still apparently null here
}

void LogsPage::initialize()
{
    if (initialized) return;
    initialized = true;

    QPushButton* filtersButton = this->findChild<QPushButton*>("tableFiltersButton");
    if (filtersButton) {
        connect(filtersButton, &QPushButton::clicked, this, &LogsPage::openFiltersDialog);
    } else {
        qWarning() << "filtersButton not found";
    }
}

void LogsPage::openFiltersDialog()
{
    qDebug() << "Test";
}
