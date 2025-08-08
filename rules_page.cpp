#include <QPushButton>

#include "rules_page.h"
#include "database_manager.h"


RulesPage::RulesPage(QWidget *parent)
    : QWidget(parent)
{

}


void RulesPage::initialize()
{
    if (initialized) return;
    initialized = true;

    rulesTable = this->findChild<QTableWidget*>("rulesTable");

    QPushButton* newRuleButton = this->findChild<QPushButton*>("newRuleButton");
    if (newRuleButton) {
        connect(newRuleButton, &QPushButton::clicked, this, &RulesPage::openNewRuleDialog);
    } else {
        qWarning() << "filtersButton not found";
    }

    editRuleButton = this->findChild<QPushButton*>("editRuleButton");
    connect(editRuleButton, &QPushButton::clicked, this, &RulesPage::openEditRuleDialog);
    connect(rulesTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
        editRuleButton->setEnabled(rulesTable->selectionModel()->hasSelection());
    });

    newRuleDialog = new NewRuleDialog(this);
    // I will probably need to have a custom signal in NewRuleDialog so I don't have a race condition between refreshing and submitting rule
    connect(newRuleDialog, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            refreshRulesTable();
        }
        newRuleDialog->reset();
    });

    // TESTING
    // Uncomment these if erasing the db to add rules for testing
    /*auto newRule = Rule();
    newRule.name = "Multiple Failed Logins";
    newRule.msgIDValue = "LOGIN_FAILURE";
    newRule.msgIDOp = StringComparison::ExactMatch;
    newRule.perHost = true;
    newRule.thresholdCount = 3;
    newRule.timeWindow = QTime().fromString("00:01:00");
    newRule.triggerCondition = ComparisonOperator::gte;

    auto noLogRule = Rule();
    noLogRule.name = "No Incoming Logs";
    noLogRule.thresholdCount = 1;
    noLogRule.timeWindow = QTime().fromString("00:01:00");
    noLogRule.triggerCondition = ComparisonOperator::lt;

    DatabaseManager::addRule(newRule);
    DatabaseManager::addRule(noLogRule);*/
    // TESTING

    refreshRulesTable();
}

void RulesPage::addRow(const QStringList &row)
{
    const int newRow = rulesTable->rowCount();
    rulesTable->insertRow(newRow);

    int id = row.last().toInt(); // id is added to end of list
    for (int c = 0; c < row.size()-1; ++c) {
        rulesTable->setItem(newRow, c, new QTableWidgetItem(row[c]));
        // Need reference to rule id for when selecting a row to edit the rule
        if (c == 0) rulesTable->itemAt(newRow, c)->setData(Qt::UserRole, id);
    }
}

void RulesPage::openNewRuleDialog()
{
    newRuleDialog->show();
    newRuleDialog->raise();
    newRuleDialog->activateWindow();
}

void RulesPage::refreshRulesTable()
{
    rulesTable->setRowCount(0); // Clear table

    const auto rows = DatabaseManager::queryRules();
    //qDebug() << "DB query rows returned: " << rows.size();

    for (const auto& row : rows) {
        addRow(row);
    }
}

void RulesPage::openEditRuleDialog()
{

    openNewRuleDialog();
}
