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

enum class FilterOperator {
    eq,  // ==
    ne,  // !=
    lt,  // <
    lte, // <=
    gt,  // >
    gte,  // >=
};

enum class StringComparison {
    ExactMatch, // == comparison
    Contains, // QString.contains()
    StartsWith, // QString.startsWith
    // Regex?
};

struct LogFormat {
    QString pattern;
    QString dateFormat;
};

struct LogEntry {
    QString raw;

    //int priority; // (Facility * 8) + Severity
    int severity;
    int facility;
    //int version; // RFC 5424 only
    QString timestamp;
    QString hostname;
    QString appname;
    QString procid;
    QString msgid; // RFC 5424 only
    QString structureddata; // RFC 5424 only
    QString msg;
};

struct LogFilters {
    int severity = -1;
    FilterOperator severityOp = FilterOperator::eq;
    int facility = -1;
    FilterOperator facilityOp = FilterOperator::eq;

    QDateTime startDate = QDateTime();
    QDateTime endDate = QDateTime();

    QString hostFilter = QString();
    QString appFilter = QString();
    QString procFilter = QString();
    QString msgIDFilter = QString();
    // structured data ?
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
    static QString severityString(const int severity);
    static QString facilityString(const int facility);
};
