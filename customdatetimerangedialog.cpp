#include <QPushButton>

#include "settings_manager.h"
#include "customdatetimerangedialog.h"
#include "ui_customdatetimerangedialog.h"

CustomDateTimeRangeDialog::CustomDateTimeRangeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CustomDateTimeRangeDialog)
{
    ui->setupUi(this);

    connect(ui->startDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &CustomDateTimeRangeDialog::onValuesChanged);
    connect(ui->endDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &CustomDateTimeRangeDialog::onValuesChanged);

    reset();
}

CustomDateTimeRangeDialog::~CustomDateTimeRangeDialog()
{
    delete ui;
}

// Should be called anytime the QComboBox is set to anyting but Custom
void CustomDateTimeRangeDialog::reset()
{
    QTimeZone tz = SettingsManager::instance().currentTimeZone();
    ui->startDateTimeEdit->setDateTime(QDateTime::currentDateTime(tz));
    ui->endDateTimeEdit->setDateTime(QDateTime::currentDateTime(tz));
}

QDateTime CustomDateTimeRangeDialog::getStartDateTime()
{
    return ui->startDateTimeEdit->dateTime();
}

QDateTime CustomDateTimeRangeDialog::getEndDateTime()
{
    return ui->endDateTimeEdit->dateTime();
}

// Checks that the chosen end date is AFTER the start date
void CustomDateTimeRangeDialog::onValuesChanged()
{
    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (ui->endDateTimeEdit->dateTime() < ui->startDateTimeEdit->dateTime()) {
        okButton->setEnabled(false);
    } else {
        okButton->setEnabled(true);
    }
}
