#include "new_rule_dialog.h"
#include "ui_new_rule_dialog.h"

NewRuleDialog::NewRuleDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewRuleDialog)
{
    ui->setupUi(this);

    QIntValidator *sevValidator = new QIntValidator(0, 7, this);
    ui->severityLineEdit->setValidator(sevValidator);

    QIntValidator *facValidator = new QIntValidator(0, 23, this);
    ui->facilityLineEdit->setValidator(facValidator);
}

NewRuleDialog::~NewRuleDialog()
{
    delete ui;
}

void NewRuleDialog::reset()
{
    //TODO: reset all properties

    ui->severityComboBox->setCurrentIndex(0);
    ui->facilityComboBox->setCurrentIndex(0);
    ui->hostnameComboBox->setCurrentIndex(0);
    ui->appnameComboBox->setCurrentIndex(0);
    ui->procidComboBox->setCurrentIndex(0);
    ui->msgidComboBox->setCurrentIndex(0);
    ui->msgComboBox->setCurrentIndex(0);
    ui->limitComboBox->setCurrentIndex(0);

    ui->severityLineEdit->setText(QString());
    ui->facilityLineEdit->setText(QString());
    ui->hostnameLineEdit->setText(QString());
    ui->appnameLineEdit->setText(QString());
    ui->procidLineEdit->setText(QString());
    ui->msgidLineEdit->setText(QString());
    ui->msgLineEdit->setText(QString());
    ui->limitLineEdit->setText(QString());

    ui->limitTimeEdit->setTime(QTime());
    ui->perHostCheckBox->setCheckState(Qt::CheckState::Unchecked);
}
