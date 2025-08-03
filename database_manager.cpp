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
                    severity NUMBER,
                    facility NUMBER,
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
        INSERT INTO logs (severity, facility, timestamp, hostname, app_name, procid, msgid, structured_data, message)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(logEntry.severity);
    query.addBindValue(logEntry.facility);
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
    last.prepare("SELECT severity, facility, timestamp, hostname, app_name, procid, msgid, structured_data, message FROM logs WHERE id = ?");
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
    static const QMap<FilterOperator, QString> opToString = {
        { FilterOperator::eq, "==" },
        { FilterOperator::ne, "!=" },
        { FilterOperator::lt, "<" },
        { FilterOperator::lte, "<=" },
        { FilterOperator::gt, ">" },
        { FilterOperator::gte, ">=" },
    };

    qDebug() << "Querying DB";
    QString sql = "SELECT severity, facility, timestamp, hostname, app_name, procid, msgid, structured_data, message FROM logs WHERE 1=1";

    if (filters.severity >= 0 && filters.severity <= 7)
        sql += QString(" AND severity %1 :severity").arg(opToString.value(filters.severityOp));
    if (filters.facility >= 0 && filters.facility <= 23)
        sql += QString(" AND facility %1 :facility").arg(opToString.value(filters.facilityOp));
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

    if (filters.severity >= 0 && filters.severity <= 7)
        query.bindValue(":severity", QString().number(filters.severity));
    if (filters.facility >= 0 && filters.facility <= 23)
        query.bindValue(":facility", QString().number(filters.facility));
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
    }
    qDebug() << rows.count();
    return rows;
}

QStringList DatabaseManager::getRow(const QSqlQuery& query)
{
    QStringList row;
    row << severityString(query.value(0).toInt()); // severity
    row << facilityString(query.value(1).toInt()); // facility
    row << query.value(2).toString(); // timestamp
    row << query.value(3).toString(); // hostname
    row << query.value(4).toString(); // app-name
    row << query.value(5).toString(); // procid
    row << query.value(6).toString(); // msgid
    //row << query.value(7).toString(); // structured-data
    row << query.value(8).toString(); // message
    return row;
}

QString DatabaseManager::severityString(const int severity)
{
    static const QStringList strings = {
        "0-emerg",
        "1-alert",
        "2-crit",
        "3-err",
        "4-warning",
        "5-notice",
        "6-info",
        "7-debug"
    };
    if (severity < 0) return strings[0];
    else if (severity >= strings.count()) return strings[strings.count() - 1];
    return strings[severity];
}

QString DatabaseManager::facilityString(const int facility)
{
    static const QStringList strings = {
        "00-kern",
        "01-user",
        "02-mail",
        "03-daemon",
        "04-auth",
        "05-syslog",
        "06-lpr",
        "07-news",
        "08-uucp",
        "09-cron",
        "10-authpriv",
        "11-ftp",
        "12-ntp",
        "13-security"
        "14-console",
        "15-solaris-cron",
        "16-local0",
        "17-local1",
        "18-local2",
        "19-local3",
        "20-local4",
        "21-local5",
        "22-local6",
        "23-local7",
    };
    if (facility < 0) return strings[0];
    else if (facility >= strings.count()) return strings[strings.count() - 1];
    return strings[facility];
}
