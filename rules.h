#pragma once

#include <QMap>
#include <QString>
#include <QTime>

#include "enums.h"
#include "logs.h"


// Triggers an Alert when the condition is met
struct Rule {
    ~Rule() = default;

    int id = -1;
    QString name; // really just for user convenience
    bool enabled = true; // if true, will generate alerts

    int severity = -1;
    ComparisonOperator severityOp = ComparisonOperator::eq;

    int facility = -1;
    ComparisonOperator facilityOp = ComparisonOperator::eq;

    QString hostnameValue = QString();
    StringComparison hostnameOp = StringComparison::ExactMatch;

    QString appnameValue = QString();
    StringComparison appnameOp = StringComparison::ExactMatch;

    QString procIDValue = QString();
    StringComparison procIDOp = StringComparison::ExactMatch;

    QString msgIDValue = QString();
    StringComparison msgIDOp = StringComparison::ExactMatch;

    // structured data ?

    QString messageValue = QString();
    StringComparison messageOp = StringComparison::ExactMatch;

    /* So the way this should work...
     *  thresholdCount = 3, timeWindow = 5 minutes, triggerCondition = gte: triggers if 3 or more logs evaluate to True within 5 minutes
     *  thresholdCount = 1, timeWindow = 5 minutes, triggerCondition = lt: triggers if less than 1 log evaluate to True every 5 minutes
    */

    bool perHost = false; // if true, tracks separately per hostname (e.g. 3 failed logins from a single host, not *any* host)
    int thresholdCount = -1;
    QTime timeWindow = QTime();
    ComparisonOperator triggerCondition = ComparisonOperator::gte;

    bool compare(ComparisonOperator op, int ruleValue, int logValue) const {
        switch (op) {
            case ComparisonOperator::eq: return ruleValue == logValue;
            case ComparisonOperator::ne: return ruleValue != logValue;
            case ComparisonOperator::lt: return ruleValue < logValue;
            case ComparisonOperator::lte: return ruleValue <= logValue;
            case ComparisonOperator::gt: return ruleValue > logValue;
            case ComparisonOperator::gte: return ruleValue >= logValue;
            default: return false;
        }
    }

    // I suppose I could do a simple bool value for this, but I might add other options later?
    bool compare(StringComparison op, QString ruleValue, QString logValue) const {
        switch (op) {
            case StringComparison::ExactMatch: return logValue == ruleValue;
            case StringComparison::Contains: return logValue.contains(ruleValue);
            case StringComparison::StartsWith: return logValue.startsWith(ruleValue);
            default: return false;
        }
    }

    bool evaluate(const LogEntry& log) const {
        //NOTE: All specified fields must match, so can't return early unless a declared field value does NOT match
        // Also NOTE: this *does* mean that a blank rule would match on EVERY log... so that shouldn't be allowed
        if (severity >= 0 && severity <= 7 && !compare(severityOp, severity, log.severity)) return false;
        if (facility >= 0 && facility <= 23 && !compare(facilityOp, facility, log.facility)) return false;

        // I don't think I need to include date or time here? It wouldn't make sense to specify a single date for a log
        // HOWEVER... it would make sense to specify a QTime field, e.g. for logins after normal working hours

        if (!hostnameValue.isEmpty() && !compare(hostnameOp, hostnameValue, log.hostname)) return false;
        if (!appnameValue.isEmpty() && !compare(appnameOp, appnameValue, log.appname)) return false;
        if (!procIDValue.isEmpty() && !compare(procIDOp, procIDValue, log.procid)) return false;
        if (!msgIDValue.isEmpty() && !compare(msgIDOp, msgIDValue, log.msgid)) return false;
        if (!messageValue.isEmpty() && !compare(messageOp, messageValue, log.msg)) return false;

        return true;
    }
};

struct RuleGroup {
    ~RuleGroup() = default;

    std::shared_ptr<Rule> rule;
    QMap<QString, QList<QDateTime>> entityTimestamps;

    RuleGroup(std::shared_ptr<Rule> r) : rule(std::move(r)) {
        if (!rule->perHost) {
            entityTimestamps.insert("global", QList<QDateTime>());
            if (rule->triggerCondition == ComparisonOperator::lt || rule->triggerCondition == ComparisonOperator::lte) {
                // 'heartbeat' rule, add a log at creation so it doesn't immediately trip
                entityTimestamps["global"].append(QDateTime().currentDateTime());
            }
        }
    }
};
