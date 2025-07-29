#pragma once

#include <QHeaderView>
#include <QVector>

#include "database_manager.h"
#include "customdatetimerangedialog.h"


class FilterHeader : public QHeaderView
{
    Q_OBJECT

public:
    explicit FilterHeader(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void updateGeometries() override;

    void setFilterBoxes();
    LogFilters getFilters();
    void clearFilters();

signals:
    //void customDateTimeRangeActivated(const bool useCustom);

private slots:
    void adjustPositions();
    void dateTimeFilterActivated(int index);

private:
    QVector<QWidget*> editors;
    CustomDateTimeRangeDialog *dtDialog = nullptr;
    QDateTime customStart, customEnd;
    int padding = 4;
    static constexpr int timestampIndex = 1;
};
