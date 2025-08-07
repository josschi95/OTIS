#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "database_manager.h"
#include "rules.h"


// Need to do this since Qt doesn't like a static non-POD db
// There's *probably* a better way to do this... I just don't know it
QSqlDatabase& DatabaseManager::instance()
{
    static bool initialized = false;
    static QSqlDatabase db;

    if (!initialized) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("otis.db");

        if (!db.open()) {
            qCritical() << "Failed to open DB:" << db.lastError().text();
        } else {
            QSqlQuery logsQuery;
            logsQuery.exec(R"(
                CREATE TABLE IF NOT EXISTS logs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    severity NUMBER,
                    facility NUMBER,
                    timestamp TEXT,
                    hostname TEXT,
                    appname TEXT,
                    procid TEXT,
                    msgid TEXT,
                    structured_data TEXT,
                    message TEXT
                )
            )");

            if (!logsQuery.exec()) {
                qCritical() << "Failed to create logs table:" << logsQuery.lastError().text();
                std::abort(); // Crash... gracefully
            }

            QSqlQuery rulesQuery;
            if (!rulesQuery.exec(R"(
                CREATE TABLE IF NOT EXISTS rules (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    name TEXT,
                    severity NUMBER,
                    severityOp NUMBER,
                    facility NUMBER,
                    facilityOp NUMBER,
                    hostname TEXT,
                    hostnameOp NUMBER,
                    appname TEXT,
                    appnameOp NUMBER,
                    procid TEXT,
                    procidOp NUMBER,
                    msgid TEXT,
                    msgidOp NUMBER,
                    message TEXT,
                    messageOp NUMBER,

                    threshold NUMBER,
                    timeWindow TEXT,
                    threshOp NUMBER,
                    perHost BOOLEAN
                )
            )")) {
                qCritical() << "Failed to create rules table:" << rulesQuery.lastError().text();
                std::abort(); // Crash... again
            }
        }
        initialized = true;
    }

    return db;
}

int DatabaseManager::logCount()
{
    QSqlQuery countQuery;
    if (countQuery.exec("SELECT COUNT(*) FROM logs")) {
        if (countQuery.next()) {
            return countQuery.value(0).toInt();
        }
    }
    return 0;
}


QStringList DatabaseManager::insertLog(const LogEntry& logEntry)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO logs (severity, facility, timestamp, hostname, appname, procid, msgid, structured_data, message)
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
    last.prepare("SELECT severity, facility, timestamp, hostname, appname, procid, msgid, structured_data, message FROM logs WHERE id = ?");
    last.addBindValue(lastId);
    if (!last.exec() || !last.next()) {
        qDebug() << "Failed to fetch inserted row: " << last.lastError().text();
        return {};
    }

    return getLogRow(last);
}


QList<QStringList> DatabaseManager::queryLogs(const LogFilters& filters)
{
    static const QMap<ComparisonOperator, QString> opToString = {
        { ComparisonOperator::eq, "==" },
        { ComparisonOperator::ne, "!=" },
        { ComparisonOperator::lt, "<" },
        { ComparisonOperator::lte, "<=" },
        { ComparisonOperator::gt, ">" },
        { ComparisonOperator::gte, ">=" },
    };

    //qDebug() << "Querying DB";
    QString sql = "SELECT severity, facility, timestamp, hostname, appname, procid, msgid, structured_data, message FROM logs WHERE 1=1";

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
        sql += " AND appname LIKE :appname";
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
        query.bindValue(":appname", filters.appFilter);
    if (!filters.messageFilter.isEmpty())
        query.bindValue(":message", filters.messageFilter);

    if (!query.exec()) {
        qWarning() << "Database logs query failed: " << query.lastError().text();
        return {};
    }

    QList<QStringList> rows;
    while (query.next()) rows << getLogRow(query);
    //qDebug() << rows.count();
    return rows;
}


QStringList DatabaseManager::getLogRow(const QSqlQuery& query)
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


void DatabaseManager::addRule(const Rule& rule)
{
    static const QMap<ComparisonOperator, QString> compMap = {
        { ComparisonOperator::eq, "==" },
        { ComparisonOperator::ne, "!=" },
        { ComparisonOperator::lt, "<" },
        { ComparisonOperator::lte, "<=" },
        { ComparisonOperator::gt, ">" },
        { ComparisonOperator::gte, ">=" }
    };

    static const QMap<StringComparison, QString> stringMap = {
        { StringComparison::ExactMatch, "Exact" },
        { StringComparison::Contains, "Contains" },
        { StringComparison::StartsWith, "Starts With" }
    };

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO rules (name, severity, severityOp, facility, facilityOp, hostname, hostnameOp, appname, appnameOp, procid, procidOp, msgid, msgidOp, message, messageOp, threshold, timeWindow, threshOp, perHost)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(rule.name);
    query.addBindValue(rule.severity);
    query.addBindValue(compMap[rule.severityOp]);
    query.addBindValue(rule.facility);
    query.addBindValue(compMap[rule.facilityOp]);
    query.addBindValue(rule.hostnameValue);
    query.addBindValue(stringMap[rule.hostnameOp]);
    query.addBindValue(rule.appnameValue);
    query.addBindValue(stringMap[rule.appnameOp]);
    query.addBindValue(rule.procIDValue);
    query.addBindValue(stringMap[rule.procIDOp]);
    query.addBindValue(rule.msgIDValue);
    query.addBindValue(stringMap[rule.msgIDOp]);
    query.addBindValue(rule.messageValue);
    query.addBindValue(stringMap[rule.messageOp]);
    query.addBindValue(rule.thresholdCount);
    query.addBindValue(rule.timeWindow);
    query.addBindValue(compMap[rule.triggerCondition]);
    query.addBindValue(rule.perHost);

    if (!query.exec()) {
        qDebug() << "Insert failed: " << query.lastError().text();
    }
}


QList<QStringList> DatabaseManager::queryRules()
{
    QString sql = "SELECT name, severity, severityOp, facility, facilityOp, hostname, hostnameOp, appname, appnameOp, procid, procidOp, msgid, msgidOp, message, messageOp, threshold, timeWindow, threshOp, perHost FROM rules";
    QSqlQuery query(instance());

    if (!query.exec(sql)) {
        qWarning() << "Database rules query failed: " << query.lastError().text();
        return {};
    }

    QList<QStringList> rows;
    while (query.next()) rows << getRuleRow(query);
    qDebug() << rows.count();
    return rows;
}


QStringList DatabaseManager::getRuleRow(const QSqlQuery& query)
{
    QStringList row;
    row << query.value(0).toString(); // name
    row << QString(query.value(2).toString() + query.value(1).toString()); // severity
    row << QString(query.value(4).toString() + query.value(3).toString()); // facility
    row << QString(query.value(6).toString() + ": " + query.value(5).toString()); // hostname
    row << QString(query.value(8).toString() + ": " + query.value(7).toString()); // appname
    row << QString(query.value(10).toString() + ": " + query.value(9).toString()); // procid
    row << QString(query.value(12).toString() + ": " + query.value(11).toString()); // msgid
    row << QString(query.value(14).toString() + ": " + query.value(13).toString()); // message
    //TODO: Change appearance for this if it's not being used
    row << QString(query.value(17).toString() + query.value(15).toString() + " in " + query.value(16).toString()); // limit
    row << query.value(18).toString(); // per-host

    return row;
}
