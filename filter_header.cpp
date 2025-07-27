#include <QHeaderView>
#include <QScrollBar>
#include <QAbstractScrollArea>
#include <QLineEdit>
#include <QComboBox>

#include "filter_header.h"
#include "database_manager.h"
#include "settings_manager.h"

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

    setFilterBoxes();
}

void FilterHeader::setFilterBoxes()
{
    while (!editors.isEmpty()) {
        QWidget* editor = editors.takeLast();
        editor->deleteLater();
    }

    for (int i = 0; i < 4; ++i) {
        if (i == 0) {
            QComboBox* comboBox = new QComboBox(parentWidget());
            comboBox->addItems({"All Time", "Last Hour", "Last 24 Hours", "Last 7 Days", "Custom"});
            comboBox->show();
            editors.append(comboBox);

            connect(comboBox, &QComboBox::activated, this, &FilterHeader::dateTimeFilterActivated);
        } else {
            QLineEdit* lineEdit = new QLineEdit(parentWidget());
            lineEdit->setPlaceholderText("Filter");
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

void FilterHeader::dateTimeFilterActivated(int index)
{
    if (!dtDialog) {
        dtDialog = new CustomDateTimeRangeDialog(this);

        connect(dtDialog, &QDialog::finished, this, [=](int result) {
            if (result == QDialog::Accepted) {
                customStart = dtDialog->getStartDateTime();
                customEnd = dtDialog->getEndDateTime();
            } else {
                // Rejected or Closed
                customStart = QDateTime();
                customEnd = QDateTime();
                dtDialog->reset();
            }
        });
    }

    if (index != 4) {
        dtDialog->reset();
        return;
    }

    dtDialog->show();
    dtDialog->raise();
    dtDialog->activateWindow();
}

LogFilters FilterHeader::getFilters()
{
    LogFilters filters;

    // Timestamp
    auto* comboBox = qobject_cast<QComboBox*>(editors[0]);
    QTimeZone tz = SettingsManager::instance()->currentTimeZone();
    switch (comboBox->currentIndex()) {
    case 0: break; // "All Time" leave default (invalid) QDateTimes
    case 1: // "Last Hour"
        filters.startDate = QDateTime::currentDateTime(tz).addSecs(-3600); // seconds in an hour
        break;
    case 2: // "Last 24 Hours"
        filters.startDate = QDateTime::currentDateTime(tz).addDays(-1);
        break;
    case 3: // "Last 7 Days"
        filters.startDate = QDateTime::currentDateTime(tz).addDays(-7);
        break;
    case 4: // "Custom"
        filters.startDate = customStart;
        filters.endDate = customEnd;
        break;
    }

    // Source
    auto* sourceFilter = qobject_cast<QLineEdit*>(editors[1]);
    if (!sourceFilter->text().isEmpty()) filters.sourceFilter = sourceFilter->text();

    // Hostname
    auto* hostnameFilter = qobject_cast<QLineEdit*>(editors[2]);
    if (!hostnameFilter->text().isEmpty()) filters.hostnameFilter = hostnameFilter->text();

    // Message
    auto* messageFilter = qobject_cast<QLineEdit*>(editors[3]);
    if (!messageFilter->text().isEmpty()) filters.messageFilter = messageFilter->text();

    // TESTING
    /*qDebug() << "Printing Filters";
    if (filters.startDate.isValid())
        qDebug() << "Timestamp Filter: " << filters.startDate.toString();
    if (!filters.sourceFilter.isEmpty())
        qDebug() << "Source Filter: " << filters.sourceFilter;
    if (!filters.hostnameFilter.isEmpty())
        qDebug() << "Hostname Filter: " << filters.hostnameFilter;
    if (!filters.messageFilter.isEmpty())
        qDebug() << "Message Filter: " << filters.messageFilter;*/
    // TESTING

    return filters;
}
