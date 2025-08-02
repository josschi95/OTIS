#include "tablefiltersdialog.h"
#include "ui_tablefiltersdialog.h"

TableFiltersDialog::TableFiltersDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TableFiltersDialog)
{
    ui->setupUi(this);
    connect(ui->resetButton, &QPushButton::clicked, this, &TableFiltersDialog::resetFilters);

    connect(ui->startDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [&](const QDateTime &dateTime) {
        startDTEdited = true;
    });

    connect(ui->endDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [&](const QDateTime &dateTime) {
        endDTEdited = true;
    });
}

TableFiltersDialog::~TableFiltersDialog()
{
    delete ui;
}

LogFilters TableFiltersDialog::getFilters() const
{
    LogFilters filters;

    //TODO: Check if these are invalid values
    filters.startDate = ui->startDateTimeEdit->dateTime();
    filters.endDate = ui->endDateTimeEdit->dateTime();

    filters.hostFilter = ui->hostnameLineEdit->text();
    filters.appFilter = ui->appnameLineEdit->text();
    filters.procFilter = ui->procidLineEdit->text();
    filters.msgIDFilter = ui->msgidLineEdit->text();
    filters.messageFilter = ui->messageLineEdit->text();

    qDebug() << "start Date is valid: " << filters.startDate.isValid();
    return filters;
}

void TableFiltersDialog::applyFilters(LogFilters& filters) const
{
    //TODO: Check if these are invalid values
    filters.startDate = ui->startDateTimeEdit->dateTime();
    filters.endDate = ui->endDateTimeEdit->dateTime();

    filters.hostFilter = ui->hostnameLineEdit->text();
    filters.appFilter = ui->appnameLineEdit->text();
    filters.procFilter = ui->procidLineEdit->text();
    filters.msgIDFilter = ui->msgidLineEdit->text();
    filters.messageFilter = ui->messageLineEdit->text();
}

void TableFiltersDialog::resetFilters()
{
    ui->startDateTimeEdit->setDateTime(QDateTime());
    ui->endDateTimeEdit->setDateTime(QDateTime());

    ui->severityLineEdit->setText(QString());
    ui->facilityLineEdit->setText(QString());
    ui->hostnameLineEdit->setText(QString());
    ui->appnameLineEdit->setText(QString());
    ui->procidLineEdit->setText(QString());
    ui->msgidLineEdit->setText(QString());
    ui->messageLineEdit->setText(QString());
}
