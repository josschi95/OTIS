#pragma once

#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QDateTime>
#include <QMap>

enum class Severity {
    Emergency,
    Alert,
    Critical,
    Error,
    Warning,
    Notice,
    Informational,
    Debug,
};

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
    QString hostname;
    QString appname;
    QString procid;
    QString msgid; // RFC 5424 only
    QString structureddata; // RFC 5424 only
    QString msg;

    int getFacility() const
    {
        return priority / 8;
    }

    int getSeverity() const
    {
        return priority % 8;
    }
};

struct LogFilters {
    int severity = static_cast<int>(Severity::Debug);

    QDateTime startDate = QDateTime();
    QDateTime endDate = QDateTime();
    QString hostFilter = QString();
    QString appFilter = QString();
    QString procFilter = QString();
    QString msgIDFilter = QString();
    QString messageFilter = QString();
};

class DatabaseManager
{
public:
    static QSqlDatabase& instance();
    static QStringList insertLog(const LogEntry& logEntry);
    static QList<QStringList> queryDB(const LogFilters& filters);

signals:
    void databaseUpdated();

private:
    DatabaseManager() = default;
    static QStringList getRow(const QSqlQuery& query);
};
