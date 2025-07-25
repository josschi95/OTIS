#include <QHeaderView>
#include <QScrollBar>
#include <QAbstractScrollArea>
#include <QLineEdit>
#include <QComboBox>

#include "filter_header.h"

// https://stackoverflow.com/questions/44343738/how-to-inject-widgets-between-qheaderview-and-qtableview

FilterHeader::FilterHeader(QWidget *parent) : QHeaderView(Qt::Horizontal, parent)
{
    setStretchLastSection(true);
    setSectionResizeMode(QHeaderView::Stretch);
    setDefaultAlignment(Qt::AlignLeft);
    setSortIndicatorShown(false);

    connect(this, &QHeaderView::sectionResized, this, &FilterHeader::adjustPositions);

    if (auto scrollParent = qobject_cast<QAbstractScrollArea*>(parent)) {
        connect(scrollParent->horizontalScrollBar(), &QScrollBar::valueChanged, this, &FilterHeader::adjustPositions);
    }

    setFilterBoxes(4); //TODO: I think I can get rid of the count? I know exactly what I need
}

void FilterHeader::setFilterBoxes(const int count)
{
    while (!editors.isEmpty()) {
        QWidget* editor = editors.takeLast();
        editor->deleteLater();
    }

    for (int i = 0; i < count; ++i) {
        if (i == 0) {
            QComboBox* comboBox = new QComboBox(parentWidget());
            comboBox->addItems({"All Time", "Last Hour", "Last 24 Hours", "Last 7 Days", "Custom"});
            connect(comboBox, &QComboBox::currentIndexChanged, this, &FilterHeader::filterActivated);
            comboBox->show();
            editors.append(comboBox);
        } else {
            QLineEdit* lineEdit = new QLineEdit(parentWidget());
            lineEdit->setPlaceholderText("Filter");
            connect(lineEdit, &QLineEdit::returnPressed, this, &FilterHeader::filterActivated);
            lineEdit->show();
            editors.append(lineEdit);
        }
    }

    adjustPositions();
}

QSize FilterHeader::sizeHint() const
{
    QSize size = QHeaderView::sizeHint();
    if (!editors.isEmpty()) {
        int height = editors[0]->sizeHint().height();
        size.setHeight(size.height() + height + padding);
    }
    return size;
}

void FilterHeader::updateGeometries()
{
    if (!editors.isEmpty()) {
        int height = editors[0]->sizeHint().height();
        setViewportMargins(0, 0, 0, height + padding);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }

    QHeaderView::updateGeometries();
    adjustPositions();
}

void FilterHeader::adjustPositions()
{
    for (int i = 0; i < editors.size(); ++i) {
        QWidget* editor = editors[i];
        int height = editor->sizeHint().height();

        int x = sectionPosition(i) - offset() + 2;
        int y = height + (padding / 2);

        editor->move(x, y);
        editor->resize(sectionSize(i), height);
    }
}

QString FilterHeader::filterText(int index) const
{
    // index 0 is the QComboBox
    if (index >= 1 && index < editors.size()) {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editors[index]);
        return lineEdit->text();
    }
    return QString();
}

void FilterHeader::setFilterText(int index, const QString &text)
{
    // index 0 is the QComboBox
    if (index >= 1 && index < editors.size()) {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editors[index]);
        lineEdit->setText(text);
    }
}

void FilterHeader::clearFilters()
{
    for (int i = 0; i < editors.size(); ++i) {
        QWidget* editor = editors[i];
        if (auto* lineEdit = qobject_cast<QLineEdit*>(editor)) {
            lineEdit->clear();
        } else if (auto* comboBox = qobject_cast<QComboBox*>(editor)) {
            comboBox->setCurrentIndex(0);
        }
    }
}
