#pragma once

#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QDateTime>

//TODO: Add Severity Level (Debug, Info, Notice, Warning, Error, Critical, Alert, Emergency)
struct LogEntry {
    QString timestamp;
    QString hostname;
    QString source;
    QString message;
    bool valid = false;
};

class DatabaseManager
{
public:
    static QSqlDatabase& instance();
    static void insertLog(const LogEntry& logEntry);
    static QList<QList<QStandardItem*>> queryDB(
        const QDateTime& startDate = QDateTime(),
        const QDateTime& endDate = QDateTime(),
        const QString& sourceFilter = QString(),
        const QString& hostnameFilter = QString(),
        const QString& messageFilter = QString());

signals:
    void databaseUpdated();

private:
    DatabaseManager() = default;
};
