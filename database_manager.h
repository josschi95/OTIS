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

struct LogFilters {
    QDateTime startDate = QDateTime();
    QDateTime endDate = QDateTime();
    QString sourceFilter = QString();
    QString hostnameFilter = QString();
    QString messageFilter = QString();
};

class DatabaseManager
{
public:
    static QSqlDatabase& instance();
    static void insertLog(const LogEntry& logEntry);
    static QList<QList<QStandardItem*>> queryDB(const LogFilters& filters);

signals:
    void databaseUpdated();

private:
    DatabaseManager() = default;
};
