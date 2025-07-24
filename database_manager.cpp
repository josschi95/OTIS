#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
                    hostname TEXT,
                    source TEXT,
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
    qDebug() << "Database manager inserting log";
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO logs (timestamp, hostname, source, message)
        VALUES (?, ?, ?, ?)
    )");
    query.addBindValue(logEntry.timestamp);
    query.addBindValue(logEntry.hostname);
    query.addBindValue(logEntry.source);
    query.addBindValue(logEntry.message);

    if (!query.exec()) {
        qDebug() << "Insert failed: " << query.lastError().text();
    }
}
