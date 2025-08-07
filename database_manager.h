#pragma once

#include <QSqlDatabase>

#include "rules.h"


class DatabaseManager
{
public:
    static QSqlDatabase& instance();
    static QStringList insertLog(const LogEntry& logEntry);
    static QList<QStringList> queryLogs(const LogFilters& filters);
    static int logCount();
    static void addRule(const Rule& rule);
    static QList<QStringList> queryRules();

signals:
    void databaseUpdated();

private:
    DatabaseManager() = default;
    static QStringList getLogRow(const QSqlQuery& query);
    static QStringList getRuleRow(const QSqlQuery& query);
    static QString severityString(const int severity);
    static QString facilityString(const int facility);
};
