#pragma once

#include <QSqlDatabase>

#include "rules.h"


class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    //static QSqlDatabase& instance();
    void insertLog(const LogEntry& logEntry);
    QList<QStringList> queryLogs(const LogFilters& filters);
    QList<std::shared_ptr<Rule>> loadRules();
    QList<QStringList> queryRules();
    void addRule(Rule& rule);
    int logCount();
    int ruleCount();

signals:
    void databaseUpdated();
    void logInserted(const QStringList& row);

private:
    explicit DatabaseManager(QObject *parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(DatabaseManager)

    //DatabaseManager() = default;
    QStringList getLogRow(const QSqlQuery& query);
    QStringList getRuleRow(const QSqlQuery& query);
    QString severityString(const int severity);
    QString facilityString(const int facility);

    QSqlDatabase db;
};
