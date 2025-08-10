#include <QPushButton>

#include "rules_page.h"
#include "database_manager.h"


RulesPage::RulesPage(QWidget *parent) : QWidget(parent) {}


void RulesPage::initialize()
{
    if (initialized) return;
    initialized = true;

    newRuleDialog = new NewRuleDialog(this);
    rulesTable = this->findChild<QTableWidget*>("rulesTable");

    QPushButton* newRuleButton = this->findChild<QPushButton*>("newRuleButton");
    connect(newRuleButton, &QPushButton::clicked, this, &RulesPage::openNewRuleDialog);

    editRuleButton = this->findChild<QPushButton*>("editRuleButton");
    connect(editRuleButton, &QPushButton::clicked, this, &RulesPage::openEditRuleDialog);
    connect(rulesTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
        editRuleButton->setEnabled(rulesTable->selectionModel()->hasSelection());
    });
    editRuleButton->setEnabled(false);

    connect(&DatabaseManager::instance(), &DatabaseManager::ruleSaved, this, &RulesPage::refreshRulesTable);
    connect(&DatabaseManager::instance(), &DatabaseManager::ruleDeleted, this, &RulesPage::refreshRulesTable);

    refreshRulesTable();
}


void RulesPage::addRow(const QStringList &row)
{
    const int newRow = rulesTable->rowCount();
    rulesTable->insertRow(newRow);

    int id = row.last().toInt(); // id is added to end of list
    bool enabled = (row[row.size() - 2].toInt() != 0); // enabled is second to last

    for (int c = 0; c < row.size() - 2; ++c) {
        rulesTable->setItem(newRow, c, new QTableWidgetItem(row[c]));

        // Need reference to rule id for when selecting a row to edit the rule
        if (c == 0) rulesTable->item(newRow, c)->setData(Qt::UserRole, id);

        // Can't use ItemIsEnabled (gives same appearance) because then it's not selectable
        if (!enabled) rulesTable->item(newRow, c)->setForeground(QBrush(Qt::gray));
    }
}


void RulesPage::setRulesManager(RuleManager &rm)
{
    ruleManager = &rm;
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

    const auto rows = DatabaseManager::instance().queryRules();
    //qDebug() << "DB query rows returned: " << rows.size();

    for (const auto& row : rows) {
        addRow(row);
    }
}


void RulesPage::openEditRuleDialog()
{
    QTableWidgetItem *item = rulesTable->item(rulesTable->currentRow(), 0);
    int id = item->data(Qt::UserRole).value<int>();
    auto rule = ruleManager->getRuleById(id);
    if (rule != nullptr) {
        newRuleDialog->setRuleToEdit(rule);
        openNewRuleDialog();
    }
}
