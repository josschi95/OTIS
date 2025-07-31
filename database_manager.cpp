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
                    hostname TEXT,
                    app_name TEXT,
                    procid TEXT,
                    msgid TEXT,
                    structured_data TEXT,
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

QStringList DatabaseManager::insertLog(const LogEntry& logEntry)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO logs (priority, timestamp, hostname, app_name, procid, msgid, structured_data, message)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(logEntry.priority);
    query.addBindValue(logEntry.timestamp);
    query.addBindValue(logEntry.hostname);
    query.addBindValue(logEntry.appname);
    query.addBindValue(logEntry.procid);
    query.addBindValue(logEntry.msgid);
    query.addBindValue(logEntry.structureddata);
    query.addBindValue(logEntry.msg);

    if (!query.exec()) {
        qDebug() << "Insert failed: " << query.lastError().text();
        return {};
    }

    QVariant lastId = query.lastInsertId();
    if (!lastId.isValid()) {
        qDebug() << "Could not retrieve last insert ID.";
        return {};
    }

    QSqlQuery last;
    last.prepare("SELECT priority, timestamp, hostname, app_name, procid, msgid, structured_data, message FROM logs WHERE id = ?");
    last.addBindValue(lastId);
    if (!last.exec() || !last.next()) {
        qDebug() << "Failed to fetch inserted row: " << last.lastError().text();
        return {};
    }

    return getRow(last);
    /*QStringList row;
    for (int i = 0; i < last.record().count(); ++i) {
        row.append()
    }
    return row;*/
}

QList<QStringList> DatabaseManager::queryDB(const LogFilters& filters)
{
    QString sql = "SELECT priority, timestamp, hostname, app_name, procid, msgid, structured_data, message FROM logs WHERE 1=1";

    if (filters.startDate.isValid())
        sql += " AND timestamp >= :start";
    if (filters.endDate.isValid())
        sql += " AND timestamp <= :end";
    if (!filters.hostFilter.isEmpty())
        sql += " AND hostname LIKE :hostname";
    if (!filters.appFilter.isEmpty())
        sql += " AND app_name LIKE :app_name";
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
        query.bindValue(":hostname", filters.hostFilter);
    if (!filters.appFilter.isEmpty())
        query.bindValue(":app_name", filters.appFilter);
    if (!filters.messageFilter.isEmpty())
        query.bindValue(":message", filters.messageFilter);

    QList<QStringList> rows;
    if (!query.exec()) {
        qWarning() << "Database query failed: " << query.lastError().text();
        return rows;
    }

    while (query.next()) {
        rows.append(getRow(query));
        /*QStringList row;
        const int priority = query.value(0).toInt();
        row << QString::number(priority % 8); // severity
        row << QString::number(priority / 8); // facility
        row << query.value(1).toString(); // timestamp
        row << query.value(2).toString(); // hostname
        row << query.value(3).toString(); // app-name
        row << query.value(4).toString(); // procid
        row << query.value(5).toString(); // msgid
        row << query.value(6).toString(); // message
        rows.append(row);*/
    }
    return rows;
}

QStringList DatabaseManager::getRow(const QSqlQuery& query)
{
    QStringList row;
    const int priority = query.value(0).toInt();
    row << QString::number(priority % 8); // severity
    row << QString::number(priority / 8); // facility
    row << query.value(1).toString(); // timestamp
    row << query.value(2).toString(); // hostname
    row << query.value(3).toString(); // app-name
    row << query.value(4).toString(); // procid
    row << query.value(5).toString(); // msgid
    row << query.value(6).toString(); // message
    return row;
}
