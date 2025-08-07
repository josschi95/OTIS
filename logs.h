#pragma once

#include <QString>
#include <QDateTime>

#include "enums.h"


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
    ComparisonOperator severityOp = ComparisonOperator::eq;
    int facility = -1;
    ComparisonOperator facilityOp = ComparisonOperator::eq;

    QDateTime startDate = QDateTime();
    QDateTime endDate = QDateTime();

    QString hostFilter = QString();
    QString appFilter = QString();
    QString procFilter = QString();
    QString msgIDFilter = QString();
    // structured data ?
    QString messageFilter = QString();
};
