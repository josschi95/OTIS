#pragma once

#include <QHeaderView>
#include <QVector>
//#include <QString>
//#include <QLineEdit>


class FilterHeader : public QHeaderView
{
    Q_OBJECT

public:
    explicit FilterHeader(QWidget *parent = nullptr);

    void setFilterBoxes(const int count);
    QString filterText(int index) const;
    void setFilterText(int index, const QString &text);
    void clearFilters();


    QSize sizeHint() const override;
    void updateGeometries() override;

signals:
    void filterActivated();

private slots:
    void adjustPositions();

private:
    QVector<QWidget*> editors;
    int padding = 4;
};
