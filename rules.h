#pragma once

#include <QMap>
#include <QString>
#include <QTime>

#include "enums.h"
#include "logs.h"


/*/home/joe/Projects/OTIS/rules.h:12:8: Excessive padding in 'struct Rule' (34 padding bytes, where 2 is optimal).
 * Optimal fields order:
 * name, hostname, appnameValue, procIDValue, msgIDValue, messageValue, id, severity, severityOp,
 * facility, facilityOp, hostnameOp, appnameOp, procIDOp, msgIDOp, messageOp, thresholdCount, timeWindow,
 * triggerCondition, enabled, perHost
 *
 *
 */

// Triggers an Alert when the condition is met
struct Rule {
    ~Rule() = default;

    QString name; // really just for user convenience
    QString hostname = QString();
    QString appname = QString();
    QString procid = QString();
    QString msgid = QString();
    QString message = QString();

    QString scriptPath = QString();
    QString scriptArgs = QString();
    Severity alertSeverity = Severity::Debug;

    int id = -1;
    int severity = -1;
    ComparisonOperator severityOp = ComparisonOperator::eq;
    int facility = -1;
    ComparisonOperator facilityOp = ComparisonOperator::eq;
    StringComparison hostnameOp = StringComparison::ExactMatch;
    StringComparison appnameOp = StringComparison::ExactMatch;
    StringComparison procidOp = StringComparison::ExactMatch;
    StringComparison msgidOp = StringComparison::ExactMatch;
    // structured data ?
    StringComparison messageOp = StringComparison::ExactMatch;
    int thresholdCount = -1;
    QTime timeWindow = QTime();
    ComparisonOperator triggerCondition = ComparisonOperator::gte;
    bool enabled = true; // if true, will generate alerts
    bool perHost = false; // if true, tracks separately per hostname (e.g. 3 failed logins from a single host, not *any* host)

    /* So the way threshold/timeWindow should work...
     *  thresholdCount = 3, timeWindow = 5 minutes, triggerCondition = gte: triggers if 3 or more logs evaluate to True within 5 minutes
     *  thresholdCount = 1, timeWindow = 5 minutes, triggerCondition = lt: triggers if less than 1 log evaluate to True every 5 minutes
    */

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

        if (!hostname.isEmpty() && !compare(hostnameOp, hostname, log.hostname)) return false;
        if (!appname.isEmpty() && !compare(appnameOp, appname, log.appname)) return false;
        if (!procid.isEmpty() && !compare(procidOp, procid, log.procid)) return false;
        if (!msgid.isEmpty() && !compare(msgidOp, msgid, log.msgid)) return false;
        if (!message.isEmpty() && !compare(messageOp, message, log.msg)) return false;

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
