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

    QIntValidator *sevValidator = new QIntValidator(0, 7, this);
    ui->severityLineEdit->setValidator(sevValidator);

    // Is there a reason I'm not doing this for facility?
}

TableFiltersDialog::~TableFiltersDialog()
{
    delete ui;
}

void TableFiltersDialog::applyFilters(LogFilters& filters) const
{
    static const QDateTime nullDT = QDateTime().fromString("Sat Jan 1 00:00:00 2000");

    // Have to do this fuckery because you can't have an invalid value inside a QDateTimeEdit
    if (ui->startDateTimeEdit->dateTime() == nullDT) {
        filters.startDate = QDateTime();
    } else {
        filters.startDate = ui->startDateTimeEdit->dateTime();
    }
    if (ui->endDateTimeEdit->dateTime() == nullDT) {
        filters.endDate = QDateTime();
    } else {
        filters.endDate = ui->endDateTimeEdit->dateTime();
    }

    bool severityOK;
    int severity = ui->severityLineEdit->text().toInt(&severityOK);
    if (severityOK && severity >= 0 && severity <= 7) {
        filters.severity = severity;
        filters.severityOp = static_cast<ComparisonOperator>(ui->severityComboBox->currentIndex());
    } else {
        filters.severity = -1;
        filters.severityOp = ComparisonOperator::eq;

        ui->severityLineEdit->setText(QString());
        ui->severityComboBox->setCurrentIndex(0);
    }

    bool facilityOK;
    int facility = ui->facilityLineEdit->text().toInt(&facilityOK);
    if (facilityOK && facility >= 0 && facility <= 23) {
        filters.facility = facility;
        filters.facilityOp = static_cast<ComparisonOperator>(ui->facilityComboBox->currentIndex());
    } else {
        filters.facility = -1;
        filters.facilityOp = ComparisonOperator::eq;

        ui->facilityLineEdit->setText(QString());
        ui->facilityComboBox->setCurrentIndex(0);
    }

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
    ui->severityComboBox->setCurrentIndex(0);
    ui->facilityLineEdit->setText(QString());
    ui->facilityComboBox->setCurrentIndex(0);

    ui->hostnameLineEdit->setText(QString());
    ui->appnameLineEdit->setText(QString());
    ui->procidLineEdit->setText(QString());
    ui->msgidLineEdit->setText(QString());
    ui->messageLineEdit->setText(QString());
}
