#include <QTableView>

#include "custom_header_view.h"

FilterHeaderView::FilterHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);

    // Create a QLineEdit for each section
    for (int i = 0; i < count(); ++i) {
        QLineEdit *edit = new QLineEdit(this);
        edit->setPlaceholderText("Filter...");
        edit->setFrame(false);
        edit->setAlignment(Qt::AlignLeft);
        filters[i] = edit;

        connect(edit, &QLineEdit::textChanged, this, &FilterHeaderView::onTextChanged);
    }

    // Handle resizing/reordering
    connect(this, &QHeaderView::sectionResized, this, &FilterHeaderView::adjustPositions);
    connect(this, &QHeaderView::sectionMoved, this, &FilterHeaderView::adjustPositions);
    connect(this, &QHeaderView::geometriesChanged, this, &FilterHeaderView::adjustPositions);
}

void FilterHeaderView::adjustPositions()
{
    for (int i = 0; i < count(); ++i) {
        if (filters.contains(i)) {
            QRect rect = sectionRect(i);
            filters[i]->setGeometry(rect.x() + 1, rect.height() / 2, rect.width() - 2, rect.height() / 2 - 2);
        }
    }
}

void FilterHeaderView::onTextChanged()
{
    QLineEdit *edit = qobject_cast<QLineEdit*>(sender());
    if (!edit)
        return;

    int col = filters.key(edit);
    emit filterChanged(col, edit->text());
}
