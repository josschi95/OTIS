#pragma once

#include <QSqlDatabase>

#include "rules.h"
#include "alert_manager.h"


class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    void insertLog(const LogEntry& logEntry);
    QList<QStringList> queryLogs(const LogFilters& filters);

    QList<QStringList> queryRules();
    QList<std::shared_ptr<Rule>> loadRules();
    void saveRule(std::shared_ptr<Rule> rule);
    void deleteRule(std::shared_ptr<Rule> rule);

    QList<QStringList> queryAlerts();
    QList<std::shared_ptr<Alert>> loadAlerts();
    void saveAlert(std::shared_ptr<Alert> alert);
    void deleteAlert(std::shared_ptr<Alert> alert);

    int logCount();
    int ruleCount();
    int alertCount();
    int alertCount(Severity severity);
    int unackAlertCount();

    HourlyLogData alertCountPerHour();
    QList<int> getSeverityCountReport(bool alerts);
    QMap<QString, QList<int>> getNoisyDevices(bool alerts);

signals:
    void databaseUpdated();
    void logInserted(const QStringList& row);
    void ruleSaved(std::shared_ptr<Rule>);
    void ruleDeleted(std::shared_ptr<Rule>);
    void alertSaved(std::shared_ptr<Alert>);
    void alertDeleted(std::shared_ptr<Alert>);

private:
    explicit DatabaseManager(QObject *parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(DatabaseManager)

    QString severityString(const int severity);
    QString facilityString(const int facility);
    QStringList getLogRow(const QSqlQuery& query);
    QStringList getRuleRow(const QSqlQuery& query);
    QStringList getAlertRow(const QSqlQuery& query);

    QSqlDatabase db;
};
