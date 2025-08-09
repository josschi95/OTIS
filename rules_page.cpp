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
    QTableWidgetItem* item = rulesTable->item(rulesTable->currentRow(), 0);
    int id = item->data(Qt::UserRole).value<int>();
    auto rule = ruleManager->getRuleById(id);
    if (rule != nullptr) {
        newRuleDialog->setRuleToEdit(rule);
        openNewRuleDialog();
    }
}
