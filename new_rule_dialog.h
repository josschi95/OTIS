#pragma once

#include <QDialog>

#include "rules.h"


namespace Ui {
class NewRuleDialog;
}

class NewRuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewRuleDialog(QWidget *parent = nullptr);
    ~NewRuleDialog();
    void reset();
    void setRuleToEdit(std::shared_ptr<Rule> rule);

private slots:
    void parseNewRule();

private:
    Ui::NewRuleDialog *ui;
    bool ruleIsValid();
};
