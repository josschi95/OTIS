#pragma once

#include <QHeaderView>
#include <QLineEdit>
#include <QMap>

class FilterHeaderView : public QHeaderView {
    Q_OBJECT

public:
    explicit FilterHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

signals:
    void filterChanged(int column, const QString &text);

private slots:
    void adjustPositions();
    void onTextChanged();

private:
    QMap<int, QLineEdit*> filters;
};
