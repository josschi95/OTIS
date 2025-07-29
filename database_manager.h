#pragma once

#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QDateTime>
#include <QMap>


struct LogFormat {
    QString pattern;
    QString dateFormat;
};

//TODO: Add Severity Level (Debug, Info, Notice, Warning, Error, Critical, Alert, Emergency)
struct LogEntry {
    QString raw;

    int priority; // (Facility * 8) + Severity
    int version; // RFC 5424 only
    QString timestamp;
    QString host;
    QString app;
    QString pid;
    QString msgid; // RFC 5424 only
    QString data; // RFC 5424 only
    QString message;

    int getFacility()
    {
        return priority / 8;
    }

    int getSeverity()
    {
        return priority % 8;
    }
};

struct LogFilters {
    QDateTime startDate = QDateTime();
    QDateTime endDate = QDateTime();
    QString hostFilter = QString();
    QString appFilter = QString();
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
