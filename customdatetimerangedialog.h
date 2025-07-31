#pragma once

#include <QDialog>
#include <QTimeZone>

namespace Ui {
class CustomDateTimeRangeDialog;
}

class CustomDateTimeRangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomDateTimeRangeDialog(QWidget *parent = nullptr);
    ~CustomDateTimeRangeDialog();
    void reset();
    QDateTime getStartDateTime() const;
    QDateTime getEndDateTime() const;

private slots:
    void onValuesChanged();

private:
    Ui::CustomDateTimeRangeDialog *ui;
};
