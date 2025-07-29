#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <QFileInfo> // TESTING

#include "database_manager.h"


// Need to do this since Qt doesn't like a static non-POD db
// There's *probably* a better way to do this... I just don't know it
QSqlDatabase& DatabaseManager::instance()
{
    static bool initialized = false;
    static QSqlDatabase db;

    if (!initialized) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("logs.db");

        if (!db.open()) {
            qCritical() << "Failed to open DB:" << db.lastError().text();
        } else {
            //qDebug() << "Database opened successfully.";

            QSqlQuery query;
            query.exec(R"(
                CREATE TABLE IF NOT EXISTS logs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    priority NUMBER,
                    timestamp TEXT,
                    host TEXT,
                    app TEXT,
                    message TEXT
                )
            )");

            if (!query.exec()) {
                qCritical() << "Failed to create table:" << query.lastError().text();
                std::abort(); // Crash... gracefully
            }
        }
        initialized = true;
    }

    return db;
}

void DatabaseManager::insertLog(const LogEntry& logEntry)
{
    //qDebug() << "Database manager inserting log";
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO logs (priority, timestamp, host, app, message)
        VALUES (?, ?, ?, ?, ?)
    )");
    query.addBindValue(logEntry.priority);
    query.addBindValue(logEntry.timestamp);
    query.addBindValue(logEntry.host);
    query.addBindValue(logEntry.app);
    query.addBindValue(logEntry.message);

    if (!query.exec()) {
        qDebug() << "Insert failed: " << query.lastError().text();
    }
}

QList<QList<QStandardItem*>> DatabaseManager::queryDB(const LogFilters& filters)
{
    QString sql = "SELECT priority, timestamp, host, app, message FROM logs WHERE 1=1";

    if (filters.startDate.isValid())
        sql += " AND timestamp >= :start";
    if (filters.endDate.isValid())
        sql += " AND timestamp <= :end";
    if (!filters.hostFilter.isEmpty())
        sql += " AND host LIKE :host";
    if (!filters.appFilter.isEmpty())
        sql += " AND app LIKE :app";
    if (!filters.messageFilter.isEmpty())
        sql += " AND message LIKE :message";
    sql += " ORDER BY timestamp DESC";

    QSqlQuery query(instance());
    query.prepare(sql);

    if (filters.startDate.isValid())
        query.bindValue(":start", filters.startDate.toString(Qt::ISODate));
    if (filters.endDate.isValid())
        query.bindValue(":end", filters.endDate.toString(Qt::ISODate));
    if (!filters.hostFilter.isEmpty())
        query.bindValue(":host", filters.hostFilter);
    if (!filters.appFilter.isEmpty())
        query.bindValue(":app", filters.appFilter);
    if (!filters.messageFilter.isEmpty())
        query.bindValue(":message", filters.messageFilter);

    QList<QList<QStandardItem*>> rows;
    if (!query.exec()) {
        qWarning() << "Database query failed: " << query.lastError().text();
        return rows;
    }

    while (query.next()) {
        QList<QStandardItem*> row;
        row << new QStandardItem(query.value(0).toString()); // priority
        row << new QStandardItem(query.value(1).toString()); // timestamp
        row << new QStandardItem(query.value(2).toString()); // host
        row << new QStandardItem(query.value(3).toString()); // app
        row << new QStandardItem(query.value(4).toString()); // message
        rows.append(row);
    }
    return rows;
}
