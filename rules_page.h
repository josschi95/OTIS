#pragma once

#include <QWidget>
#include <QTableWidget>

#include "new_rule_dialog.h"


namespace Ui {
class RulesPage;
}

class RulesPage : public QWidget
{
    Q_OBJECT

public:
    explicit RulesPage(QWidget *parent = nullptr);
    void initialize();
    void addRow(const QStringList& log);

public slots:
    void openNewRuleDialog();

private slots:
    void refreshRulesTable();

private:
    bool initialized = false;
    QTableWidget *rulesTable;
    NewRuleDialog *newRuleDialog = nullptr;
};
