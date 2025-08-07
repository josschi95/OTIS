#pragma once

#include <QDialog>

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

private:
    Ui::NewRuleDialog *ui;
};
