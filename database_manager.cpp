#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <QFileInfo> // TESTING

#include "database_manager.h"


// Need to do this since Qt doesn't like a static non-POD db
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

        qDebug() << "DB Path:" << QFileInfo(db.databaseName()).absoluteFilePath();
        initialized = true;
    }

    return db;
}

void DatabaseManager::insertLog(const LogEntry& logEntry)
{
    qDebug() << "Database manager inserting log";
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

QList<QList<QStandardItem*>> DatabaseManager::queryDB(
    const QDateTime& startDate = QDateTime(),
    const QDateTime& endDate = QDateTime(),
    const QString& sourceFilter = QString(),
    const QString& hostnameFilter = QString(),
    const QString& messageFilter = QString())
{
    QString sql = "SELECT timestamp, source, hostname, message FROM logs WHERE 1=1";

    if (startDate.isValid()) sql += " AND timestamp >= :start";
    if (endDate.isValid()) sql += " AND timestamp <= :end";
    if (!sourceFilter.isEmpty()) sql += " AND source = :source";
    if (!hostnameFilter.isEmpty()) sql += " AND source = :hostname";
    //TODO messageFilter
    sql += " ORDER BY timestamp DESC";

    QSqlQuery query(instance());
    query.prepare(sql);

    if (startDate.isValid()) query.bindValue(":start", startDate.toString(Qt::ISODate));
    if (endDate.isValid()) query.bindValue(":end", endDate.toString(Qt::ISODate));
    if (!sourceFilter.isEmpty()) query.bindValue(":source", sourceFilter);
    if (!hostnameFilter.isEmpty()) query.bindValue(":hostname", hostnameFilter);

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
