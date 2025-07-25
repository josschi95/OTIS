#pragma once

#include <QSqlDatabase>


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

signals:
    void databaseUpdated();

private:
    DatabaseManager() = default;
};
