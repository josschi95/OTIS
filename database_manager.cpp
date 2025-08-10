#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "database_manager.h"
#include "rules.h"


// Need to do this since Qt doesn't like a static non-POD db
// There's *probably* a better way to do this... I just don't know it
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;

    static bool initialized = false;
    static QSqlDatabase db;

    if (!initialized) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("otis.db");

        if (!db.open()) {
            qCritical() << "Failed to open DB:" << db.lastError().text();
        } else {
            QSqlQuery logsQuery;
            if (!logsQuery.exec(R"(
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
            )")) {
                qCritical() << "Failed to create logs table:" << logsQuery.lastError().text();
                std::abort(); // Crash... gracefully
            }

            QSqlQuery rulesQuery;
            if (!rulesQuery.exec(R"(
                CREATE TABLE IF NOT EXISTS rules (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    name TEXT,
                    alertSeverity NUMBER,
                    scriptPath TEXT,
                    scriptArgs TEXT,

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
                    perHost BOOLEAN,
                    enabled BOOLEAN
                )
            )")) {
                qCritical() << "Failed to create rules table:" << rulesQuery.lastError().text();
                std::abort(); // Crash... again
            }

            QSqlQuery alertsQuery;
            if (!alertsQuery.exec(R"(
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp TEXT,
                    severity TEXT,
                    source TEXT,
                    rule_name TEXT,
                    acknowledged BOOLEAN
                )
            )")) {
                qCritical() << "Failed to create alerts table:" << alertsQuery.lastError().text();
                std::abort(); // Crash one last time
            }
        }
        initialized = true;
    }

    return instance;
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


int DatabaseManager::ruleCount()
{
    QSqlQuery countQuery;
    if (countQuery.exec("SELECT COUNT(*) FROM rules")) {
        if (countQuery.next()) {
            return countQuery.value(0).toInt();
        }
    }
    return 0;
}


int DatabaseManager::alertCount()
{
    QSqlQuery countQuery;
    if (countQuery.exec("SELECT COUNT(*) FROM alerts")) {
        if (countQuery.next()) {
            return countQuery.value(0).toInt();
        }
    }
    return 0;
}


int DatabaseManager::alertCount(Severity severity)
{
    QSqlQuery countQuery;
    countQuery.prepare("SELECT COUNT(*) FROM alerts WHERE severity = :severity");
    countQuery.bindValue(":severity", static_cast<int>(severity));
    if (!countQuery.exec()) {
        qWarning() << "Alert count query failed: " << countQuery.lastError().text();
    } else if (countQuery.next()){
        return countQuery.value(0).toInt();
    }
    return 0;
}

int DatabaseManager::unackAlertCount()
{
    QSqlQuery countQuery;
    countQuery.prepare("SELECT COUNT(*) FROM alerts WHERE acknowledged = :acknowledged");
    countQuery.bindValue(":acknowledged", false);
    if (!countQuery.exec()) {
        qWarning() << "Alert count query failed: " << countQuery.lastError().text();
    } else if (countQuery.next()){
        return countQuery.value(0).toInt();
    }
    return 0;
}

/********** Logs **********/

void DatabaseManager::insertLog(const LogEntry& logEntry)
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
        qDebug() << "Log insert failed: " << query.lastError().text();
        return;
    }

    QVariant lastId = query.lastInsertId();
    if (!lastId.isValid()) {
        qDebug() << "Could not retrieve last insert ID.";
        return;
    }

    QSqlQuery last;
    last.prepare("SELECT severity, facility, timestamp, hostname, appname, procid, msgid, structured_data, message FROM logs WHERE id = ?");
    last.addBindValue(lastId);
    if (!last.exec() || !last.next()) {
        qDebug() << "Failed to fetch inserted row: " << last.lastError().text();
        return;
    }

    emit logInserted(getLogRow(last));
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

    QSqlQuery query(db);
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


/********** Rules **********/

QList<std::shared_ptr<Rule>> DatabaseManager::loadRules()
{    
    QSqlQuery query;
    if (!query.exec(R"(SELECT
        id, name, alertSeverity,
        scriptPath, scriptArgs,
        severity, severityOp,
        facility, facilityOp,
        hostname, hostnameOp,
        appname, appnameOp,
        procid, procidOp,
        msgid, msgidOp,
        message, messageOp,
        threshold, timeWindow, threshOp,
        perHost, enabled
        FROM rules
    )")) {
        qWarning() << "Database rules query failed: " << query.lastError().text();
        return {};
    }

    QList<std::shared_ptr<Rule>> rules;
    while (query.next()) {
        auto rule = std::make_shared<Rule>();

        rule->id = query.value(0).toInt();
        rule->name = query.value(1).toString();
        rule->alertSeverity = static_cast<Severity>(query.value(2).toInt());
        rule->scriptPath = query.value(3).toString();
        rule->scriptArgs = query.value(4).toString();
        rule->severity = query.value(5).toInt();
        rule->severityOp = static_cast<ComparisonOperator>(query.value(6).toInt());
        rule->facility = query.value(7).toInt();
        rule->facilityOp = static_cast<ComparisonOperator>(query.value(8).toInt());
        rule->hostname = query.value(9).toString();
        rule->hostnameOp = static_cast<StringComparison>(query.value(10).toInt());
        rule->appname = query.value(11).toString();
        rule->appnameOp = static_cast<StringComparison>(query.value(12).toInt());
        rule->procid = query.value(13).toString();
        rule->procidOp = static_cast<StringComparison>(query.value(14).toInt());
        rule->msgid = query.value(15).toString();
        rule->msgidOp = static_cast<StringComparison>(query.value(16).toInt());
        rule->message = query.value(17).toString();
        rule->messageOp = static_cast<StringComparison>(query.value(18).toInt());
        rule->thresholdCount = query.value(19).toInt();
        rule->timeWindow = QTime::fromString(query.value(20).toString());
        rule->triggerCondition = static_cast<ComparisonOperator>(query.value(21).toInt());
        rule->perHost = query.value(22).toBool();
        rule->enabled = query.value(23).toBool();

        rules << rule;
    }

    return rules;
}


void DatabaseManager::saveRule(std::shared_ptr<Rule> rule)
{
    QSqlQuery query;

    if (rule->id > 0) { // Updating an existing rule
        qDebug() << "Updating existing rule";
        query.prepare(R"(
            UPDATE rules SET
            name = :name,
            alertSeverity = :alertSeverity,
            scriptPath = :scriptPath, scriptArgs = :scriptArgs,
            severity = :severity, severityOp = :severityOp,
            facility = :facility, facilityOp = :facilityOp,
            hostname = :hostname, hostnameOp = :hostnameOp,
            appname = :appname, appnameOp = :appnameOp,
            procid = :procid, procidOp = :procidOp,
            msgid = :msgid, msgidOp = :msgidOp,
            message = :message, messageOp = :messageOp,
            threshold = :threshold, timeWindow = :timeWindow, threshOp = :threshOp,
            perHost = :perHost,
            enabled = :enabled
            WHERE id = :id
        )");
        query.bindValue(":id", rule->id);
    } else { // Creating a new rule
        query.prepare(R"(
            INSERT INTO rules (
                name,
                alertSeverity,
                scriptPath, scriptArgs,
                severity, severityOp,
                facility, facilityOp,
                hostname, hostnameOp,
                appname, appnameOp,
                procid, procidOp,
                msgid, msgidOp,
                message, messageOp,
                threshold, timeWindow, threshOp,
                perHost, enabled
            ) VALUES (
                :name,
                :alertSeverity,
                :scriptPath, :scriptArgs,
                :severity, :severityOp,
                :facility, :facilityOp,
                :hostname, :hostnameOp,
                :appname, :appnameOp,
                :procid, :procidOp,
                :msgid, :msgidOp,
                :message, :messageOp,
                :threshold, :timeWindow, :threshOp,
                :perHost, :enabled
            )
        )");
    }

    query.bindValue(":name", rule->name);
    query.bindValue(":alertSeverity", static_cast<int>(rule->alertSeverity));
    query.bindValue(":scriptPath", rule->scriptPath);
    query.bindValue(":scriptArgs", rule->scriptArgs);
    query.bindValue(":severity", rule->severity);
    query.bindValue(":severityOp", static_cast<int>(rule->severityOp));
    query.bindValue(":facility", rule->facility);
    query.bindValue(":facilityOp", static_cast<int>(rule->facilityOp));
    query.bindValue(":hostname", rule->hostname);
    query.bindValue(":hostnameOp", static_cast<int>(rule->hostnameOp));
    query.bindValue(":appname", rule->appname);
    query.bindValue(":appnameOp", static_cast<int>(rule->appnameOp));
    query.bindValue(":procid", rule->procid);
    query.bindValue(":procidOp", static_cast<int>(rule->procidOp));
    query.bindValue(":msgid", rule->msgid);
    query.bindValue(":msgIDOp", static_cast<int>(rule->msgidOp));
    query.bindValue(":message", rule->message);
    query.bindValue(":messageOp", static_cast<int>(rule->messageOp));
    query.bindValue(":threshold", rule->thresholdCount);
    query.bindValue(":timeWindow", rule->timeWindow);
    query.bindValue(":threshOp", static_cast<int>(rule->triggerCondition));
    query.bindValue(":perHost", rule->perHost);
    query.bindValue(":enabled", rule->enabled);

    if (!query.exec()) {
        qDebug() << ((rule->id > 0) ? "Rule update failed: " : "Rule insert failed: ") << query.lastError().text();
    } else if (rule->id <= 0) { // Assign id to new rule
        QVariant lastId = query.lastInsertId();
        if (!lastId.isValid()) {
            qDebug() << "Could not retrieve last insert ID.";
            return;
        }
        rule->id = lastId.toInt();
    }

    emit ruleSaved(rule);
}


QList<QStringList> DatabaseManager::queryRules()
{    
    QSqlQuery rulesQuery;
    if (!rulesQuery.exec(R"(SELECT
        name, alertSeverity,
        scriptPath, scriptArgs,
        severity, severityOp,
        facility, facilityOp,
        hostname, hostnameOp,
        appname, appnameOp,
        procid, procidOp,
        msgid, msgidOp,
        message, messageOp,
        threshold, timeWindow, threshOp,
        perHost, enabled, id
        FROM rules
    )")) {
        qWarning() << "Database rules query failed: " << rulesQuery.lastError().text();
        return {};
    }

    QList<QStringList> rows;
    while (rulesQuery.next()) rows << getRuleRow(rulesQuery);
    return rows;
}


QStringList DatabaseManager::getRuleRow(const QSqlQuery& query)
{    
    static const QStringList opStrings = { "==", "!=", "<", "<=", ">", ">=" };
    static const QStringList compStrings = { "Exact", "Contains", "Starts With" };
    static const QStringList severityStrings = { "Emergency", "Alert", "Critical", "Error", "Warning", "Notice", "Informational", "Debug" };
    // Values are checked and replaced with "-" if empty
    // Trying to make things look somewhat presentable

    QStringList row;
    row << query.value(0).toString(); // name
    row << severityStrings[query.value(1).toInt()];
    row << QString("%1 %2").arg(query.value(2).toString(), query.value(3).toString());

    // Severity(4) and Facility(6)
    for (int i = 4; i < 7; i+=2) {
        int value = query.value(i).toInt();
        if (value >= 0) {
            row << QString(opStrings[query.value(i+1).toInt()] + QString::number(value));
        } else {
            row << "-";
        }
    }

    // hostname(8), appname, procid, msgid, and message(16) all follow the same rules/format
    // The string comparison = index + 1
    for (int i = 8; i < 17; i+=2) {
        QString fieldName = query.value(i).toString();
        if (!fieldName.isEmpty()) {
            row << QString(compStrings[query.value(i+1).toInt()] + ": " + fieldName);
        } else {
            row << "-";
        }
    }

    // threshold=18, timeWindow=19, threshOp=20
    int thresh = query.value(18).toInt();
    if (thresh >= 0) {
        row << QString(query.value(20).toString() + " " + QString::number(thresh) + " in " + query.value(19).toString());
    } else {
        row << "-";
    }

    row << (query.value(21).toBool() ? "True" : "False"); // perHost
    row << (query.value(22).toBool() ? "1" : "0");        // enabled

    row << query.value(23).toString(); // database id
    return row;
}


void DatabaseManager::deleteRule(std::shared_ptr<Rule> rule)
{
    qDebug() << "Deleting rule " << rule->name;
    QSqlQuery query;
    query.prepare("DELETE FROM rules WHERE id = :id");
    query.bindValue(":id", rule->id);
    if (!query.exec()) {
        qWarning() << "Failed to delete rule: " << rule->name << " - " << query.lastError().text();
        return;
    }
    emit ruleDeleted(rule);
}

/********** Alerts **********/

QList<std::shared_ptr<Alert>> DatabaseManager::loadAlerts()
{
    QSqlQuery query;
    if (!query.exec(R"(SELECT
        id, timestamp, severity,
        source, rule_name, acknowledged
        FROM alerts
    )")) {
        qWarning() << "Database alerts query failed: " << query.lastError().text();
        return {};
    }

    QList<std::shared_ptr<Alert>> alerts;
    while (query.next()) {
        auto alert = std::make_shared<Alert>(
            QDateTime::fromString(query.value(1).toString(), Qt::ISODate),
            static_cast<Severity>(query.value(2).toInt()),
            query.value(3).toString(),
            query.value(4).toString()
        );

        alert->id = query.value(0).toInt();
        alert->acknowledged = query.value(5).toBool();

        alerts << alert;
    }

    return alerts;
}


QList<QStringList> DatabaseManager::queryAlerts()
{
    QSqlQuery alertsQuery;
    if (!alertsQuery.exec(R"(SELECT
        severity, timestamp, rule_name, source,
        acknowledged, id
        FROM alerts
        ORDER BY timestamp DESC
    )")) {
        qWarning() << "Database alerts query failed: " << alertsQuery.lastError().text();
        return {};
    }

    QList<QStringList> rows;
    while (alertsQuery.next()) rows << getAlertRow(alertsQuery);
    return rows;
}


QStringList DatabaseManager::getAlertRow(const QSqlQuery& query)
{
    static const QStringList severityStrings = { "Emergency", "Alert", "Critical", "Error", "Warning", "Notice", "Informational", "Debug" };

    QStringList row;
    row << severityStrings[query.value(0).toInt()]; // severity
    row << query.value(1).toString(); // timestamp
    row << query.value(2).toString(); // rule_name
    row << query.value(3).toString(); // source
    row << query.value(4).toString(); // acknowledged
    row << query.value(5).toString(); // id
    return row;
}


void DatabaseManager::saveAlert(std::shared_ptr<Alert> alert)
{
    QSqlQuery query;

    if (alert->id > 0) { // Updating an existing alert
        query.prepare(R"(
            UPDATE alerts SET
            acknowledged = :acknowledged
            WHERE id = :id
        )");
        query.bindValue(":id", alert->id);
    } else { // Storing a new alert
        query.prepare(R"(
            INSERT INTO alerts (timestamp, severity, source, rule_name, acknowledged)
            VALUES (:timestamp, :severity, :source, :rule_name, :acknowledged)
        )");
        query.bindValue(":timestamp", alert->timestamp.toString(Qt::ISODate));
        query.bindValue(":severity", static_cast<int>(alert->severity));
        query.bindValue(":source", alert->source);
        query.bindValue(":rule_name", alert->ruleName);
    }
    query.bindValue(":acknowledged", alert->acknowledged);

    if (!query.exec()) {
        qDebug() << ((alert->id > 0) ? "Alert update failed: " : "Alert insert failed: ") << query.lastError().text();
    } else if (alert->id <= 0) { // Assign id to new alert
        QVariant lastId = query.lastInsertId();
        if (!lastId.isValid()) {
            qDebug() << "Could not retrieve last insert ID.";
            return;
        }
        alert->id = lastId.toInt();
    }

    emit alertSaved(alert);
}


void DatabaseManager::deleteAlert(std::shared_ptr<Alert> alert)
{
    qDebug() << "Deleting alert for " << alert->ruleName;
    QSqlQuery query;
    query.prepare("DELETE FROM alerts WHERE id = :id");
    query.bindValue(":id", alert->id);
    if (!query.exec()) {
        qWarning() << "Failed to delete alert for: " << alert->ruleName << " - " << query.lastError().text();
        return;
    }
    emit alertDeleted(alert);
}
