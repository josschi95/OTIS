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
                    timestamp TEXT,
                    source TEXT,
                    hostname TEXT,
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
        INSERT INTO logs (timestamp, source, hostname, message)
        VALUES (?, ?, ?, ?)
    )");
    query.addBindValue(logEntry.timestamp);
    query.addBindValue(logEntry.source);
    query.addBindValue(logEntry.hostname);
    query.addBindValue(logEntry.message);

    if (!query.exec()) {
        qDebug() << "Insert failed: " << query.lastError().text();
    }
}

QList<QList<QStandardItem*>> DatabaseManager::queryDB(const LogFilters& filters)
{
    QString sql = "SELECT timestamp, source, hostname, message FROM logs WHERE 1=1";

    if (filters.startDate.isValid())
        sql += " AND timestamp >= :start";
    if (filters.endDate.isValid())
        sql += " AND timestamp <= :end";
    if (!filters.sourceFilter.isEmpty())
        sql += " AND source LIKE :source";
    if (!filters.hostnameFilter.isEmpty())
        sql += " AND hostname LIKE :hostname";
    if (!filters.messageFilter.isEmpty())
        sql += " AND message LIKE :message";
    sql += " ORDER BY timestamp DESC";

    QSqlQuery query(instance());
    query.prepare(sql);

    if (filters.startDate.isValid())
        query.bindValue(":start", filters.startDate.toString(Qt::ISODate));
    if (filters.endDate.isValid())
        query.bindValue(":end", filters.endDate.toString(Qt::ISODate));
    if (!filters.sourceFilter.isEmpty())
        query.bindValue(":source", filters.sourceFilter);
    if (!filters.hostnameFilter.isEmpty())
        query.bindValue(":hostname", filters.hostnameFilter);
    if (!filters.messageFilter.isEmpty())
        query.bindValue(":message", filters.messageFilter);

    //QSqlQuery query("SELECT timestamp, source, hostname, message FROM logs ORDER BY timestamp DESC", instance());
    QList<QList<QStandardItem*>> rows;
    if (!query.exec()) {
        qWarning() << "Database query failed: " << query.lastError().text();
        return rows;
    }

    while (query.next()) {
        QList<QStandardItem*> row;
        row << new QStandardItem(query.value(0).toString()); // timestamp
        row << new QStandardItem(query.value(1).toString()); // source
        row << new QStandardItem(query.value(2).toString()); // host
        row << new QStandardItem(query.value(3).toString()); // message
        rows.append(row);
    }
    return rows;
}
