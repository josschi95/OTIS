#include <QMessageBox>

#include "new_rule_dialog.h"
#include "ui_new_rule_dialog.h"
#include "database_manager.h"

NewRuleDialog::NewRuleDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewRuleDialog)
{
    ui->setupUi(this);

    QIntValidator *sevValidator = new QIntValidator(0, 7, this);
    ui->severityLineEdit->setValidator(sevValidator);

    QIntValidator *facValidator = new QIntValidator(0, 23, this);
    ui->facilityLineEdit->setValidator(facValidator);

    QIntValidator *threshValidator = new QIntValidator(0, INT_MAX, this);
    ui->limitLineEdit->setValidator(threshValidator);

    connect(this, &QDialog::accepted, this, &NewRuleDialog::parseNewRule);
    connect(this, &QDialog::rejected, this, &NewRuleDialog::reset);

    connect(ui->deleteRuleButton, &QPushButton::clicked, this, &NewRuleDialog::deleteRule);
}


NewRuleDialog::~NewRuleDialog()
{
    delete ui;
}


void NewRuleDialog::reset()
{
    ui->alertSeverityComboBox->setCurrentIndex(0);
    ui->severityComboBox->setCurrentIndex(0);
    ui->facilityComboBox->setCurrentIndex(0);
    ui->hostnameComboBox->setCurrentIndex(0);
    ui->appnameComboBox->setCurrentIndex(0);
    ui->procidComboBox->setCurrentIndex(0);
    ui->msgidComboBox->setCurrentIndex(0);
    ui->msgComboBox->setCurrentIndex(0);
    ui->limitComboBox->setCurrentIndex(0);

    ui->scriptPathLineEdit->setText(QString());
    ui->scriptArgsLineEdit->setText(QString());
    ui->severityLineEdit->setText(QString());
    ui->facilityLineEdit->setText(QString());
    ui->hostnameLineEdit->setText(QString());
    ui->appnameLineEdit->setText(QString());
    ui->procidLineEdit->setText(QString());
    ui->msgidLineEdit->setText(QString());
    ui->msgLineEdit->setText(QString());
    ui->limitLineEdit->setText(QString());

    ui->limitTimeEdit->setTime(QTime());
    ui->perHostCheckBox->setChecked(false);
    ui->ruleEnabledCheckBox->setChecked(true);
    ui->deleteRuleButton->setEnabled(false);
}


// TODO: Maybe check periodically and disable/enable Confirm button?
bool NewRuleDialog::ruleIsValid()
{
    // Just need to check that there is at least *some* value to compare
    if (!ui->severityLineEdit->text().isEmpty()) return true;
    if (!ui->facilityLineEdit->text().isEmpty()) return true;
    if (!ui->hostnameLineEdit->text().isEmpty()) return true;
    if (!ui->appnameLineEdit->text().isEmpty()) return true;
    if (!ui->procidLineEdit->text().isEmpty()) return true;
    if (!ui->msgidLineEdit->text().isEmpty()) return true;
    if (!ui->msgLineEdit->text().isEmpty()) return true;

    // if a threshold is declared, needs to have a valid time value
    if (!ui->limitLineEdit->text().isEmpty()) {
        // If it's less than 5 seconds, set it 5 seconds (RuleManager update time)
        int windowMs = QTime(0, 0).msecsTo(ui->limitTimeEdit->time());
        if (windowMs < 5000) ui->limitTimeEdit->time().setHMS(0, 0, 5);

        return true;
    }

    return false;
}


void NewRuleDialog::parseNewRule()
{
    if (!ruleIsValid()) {
        // TODO: Don't let the user hit 'accept' when invalid
        ruleToEdit = nullptr;
        return;
    }

    std::shared_ptr<Rule> rule = ruleToEdit ? ruleToEdit : std::make_shared<Rule>();

    rule->name = ui->nameLineEdit->text();
    rule->scriptPath = ui->scriptPathLineEdit->text();
    rule->scriptArgs = ui->scriptArgsLineEdit->text();
    rule->alertSeverity = static_cast<Severity>(ui->alertSeverityComboBox->currentIndex());
    rule->enabled = ui->ruleEnabledCheckBox->isChecked();
    rule->perHost = ui->perHostCheckBox->isChecked();

    if (!ui->severityLineEdit->text().isEmpty()) {
        rule->severity = ui->severityLineEdit->text().toInt();
        rule->severityOp = static_cast<ComparisonOperator>(ui->severityComboBox->currentIndex());
    }

    if (!ui->facilityLineEdit->text().isEmpty()) {
        rule->facility = ui->facilityLineEdit->text().toInt();
        rule->facilityOp = static_cast<ComparisonOperator>(ui->facilityComboBox->currentIndex());
    }

    if (!ui->hostnameLineEdit->text().isEmpty()) {
        rule->hostname = ui->hostnameLineEdit->text();
        rule->hostnameOp = static_cast<StringComparison>(ui->hostnameComboBox->currentIndex());
    }

    if (!ui->appnameLineEdit->text().isEmpty()) {
        rule->appname = ui->appnameLineEdit->text();
        rule->appnameOp = static_cast<StringComparison>(ui->appnameComboBox->currentIndex());
    }

    if (!ui->procidLineEdit->text().isEmpty()) {
        rule->procid = ui->procidLineEdit->text();
        rule->procidOp = static_cast<StringComparison>(ui->procidComboBox->currentIndex());
    }

    if (!ui->msgidLineEdit->text().isEmpty()) {
        rule->msgid = ui->msgidLineEdit->text();
        rule->msgidOp = static_cast<StringComparison>(ui->msgidComboBox->currentIndex());
    }

    if (!ui->msgLineEdit->text().isEmpty()) {
        rule->message = ui->msgLineEdit->text();
        rule->messageOp = static_cast<StringComparison>(ui->msgComboBox->currentIndex());
    }

    if (!ui->limitLineEdit->text().isEmpty()) {
        rule->thresholdCount = ui->limitLineEdit->text().toInt();
        rule->timeWindow = ui->limitTimeEdit->time();
        // Plus 2 because == and != aren't valid nor included
        rule->triggerCondition = static_cast<ComparisonOperator>(ui->limitComboBox->currentIndex() + 2);
    }

    DatabaseManager::instance().saveRule(rule);
    ruleToEdit = nullptr; // Clear saved value for next time this is opened
    reset();
}


void NewRuleDialog::setRuleToEdit(std::shared_ptr<Rule> rule)
{
    //qDebug() << "Editing rule: " << rule->name;

    ruleToEdit = rule;
    ui->deleteRuleButton->setEnabled(true);

    ui->nameLineEdit->setText(rule->name);
    ui->scriptPathLineEdit->setText(rule->scriptPath);
    ui->scriptArgsLineEdit->setText(rule->scriptArgs);
    ui->ruleEnabledCheckBox->setChecked(rule->enabled);
    ui->perHostCheckBox->setChecked(rule->perHost);

    ui->alertSeverityComboBox->setCurrentIndex(static_cast<int>(rule->alertSeverity));
    ui->severityComboBox->setCurrentIndex(static_cast<int>(rule->severityOp));
    ui->facilityComboBox->setCurrentIndex(static_cast<int>(rule->facilityOp));
    ui->hostnameComboBox->setCurrentIndex(static_cast<int>(rule->hostnameOp));
    ui->appnameComboBox->setCurrentIndex(static_cast<int>(rule->appnameOp));
    ui->procidComboBox->setCurrentIndex(static_cast<int>(rule->procidOp));
    ui->msgidComboBox->setCurrentIndex(static_cast<int>(rule->msgidOp));
    ui->msgComboBox->setCurrentIndex(static_cast<int>(rule->messageOp));
    // Minus 2 because == and != aren't valid/included
    ui->limitComboBox->setCurrentIndex(static_cast<int>(rule->triggerCondition) - 2);

    if (rule->severity >= 0) ui->severityLineEdit->setText(QString::number(rule->severity));
    if (rule->facility >= 0) ui->facilityLineEdit->setText(QString::number(rule->facility));
    if (rule->thresholdCount >= 0) ui->limitLineEdit->setText(QString::number(rule->thresholdCount));

    ui->limitTimeEdit->setTime(rule->timeWindow);

    ui->hostnameLineEdit->setText(rule->hostname);
    ui->appnameLineEdit->setText(rule->appname);
    ui->procidLineEdit->setText(rule->procid);
    ui->msgidLineEdit->setText(rule->msgid);
    ui->msgLineEdit->setText(rule->message);
}


void NewRuleDialog::deleteRule()
{
    if (!ruleToEdit) return; // shouldn't happen?

    QMessageBox::StandardButton reply;
    if (QMessageBox::question(this, "Confirm Delete", "Are you sure?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        DatabaseManager::instance().deleteRule(ruleToEdit);
        reset();
        close();
    }
}

