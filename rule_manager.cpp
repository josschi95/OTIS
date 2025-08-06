#include <QTimer>

#include "rule_manager.h"


RuleManager::RuleManager(QObject *parent) : QObject(parent)
{
    // Load Logs from JSON

    auto newRule = std::make_shared<Rule>();
    newRule->name = "Multiple Failed Logins";
    newRule->msgIDValue = "LOGIN_FAILURE";
    newRule->msgIDOp = StringComparison::ExactMatch;
    newRule->perHost = true;
    newRule->thresholdCount = 3;
    newRule->timeWindow = QTime().fromString("00:01:00");
    newRule->triggerCondition = ComparisonOperator::gte;
    addRule(newRule);

    // Start Update loop
    updateTimer = new QTimer(this);
    updateTimer->setInterval(5 * 1000); // 5 seconds
    connect(updateTimer, &QTimer::timeout, this, &RuleManager::update);
    updateTimer->start();
}

// Needed for tracking 'heartbeat' rules - those with a triggerCondition of < or <=
void RuleManager::update()
{
    //qDebug() << "Update";
    QDateTime now = QDateTime::currentDateTime();

    for (auto& group : ruleGroups) {
        if (!group.rule->enabled) continue; // user disabled
        if (group.rule->thresholdCount < 0) continue; // not time-based
        if (group.rule->triggerCondition != ComparisonOperator::lt && group.rule->triggerCondition != ComparisonOperator::lte) continue;

        int windowMs = QTime(0, 0).msecsTo(group.rule->timeWindow);

        auto it = group.entityTimestamps.begin();
        while (it != group.entityTimestamps.end()) {
            QList<QDateTime>& timestamps = it.value();
            clearOldTimestamps(timestamps, now, windowMs);

            // Verify that there are the given number of logs remaining within the time window
            int count = timestamps.size();
            if (group.rule->triggerCondition == ComparisonOperator::lt && count < group.rule->thresholdCount) emit ruleViolated(group.rule);
            if (group.rule->triggerCondition == ComparisonOperator::lte && count <= group.rule->thresholdCount) emit ruleViolated(group.rule);
        }
    }

    updateTimer->start();
}

QList<std::shared_ptr<Rule>> RuleManager::getRules() const
{
    return rules;
}

bool RuleManager::addRule(std::shared_ptr<Rule> rule)
{
    if (!rule) return false;

    //TODO: Check that rule is valid

    rules << rule;

    // If rule is time-based add it to the RuleGroup
    // To count as a 'timed rule' then thresholdCount must be > 0 and timeWindow must be >= 60 seconds
    if (rule->thresholdCount > 0 && (rule->timeWindow.minute() > 0 || rule->timeWindow.hour() > 0)) {
        RuleGroup newGroup { rule };
        ruleGroups.append(newGroup);
    }

    return true;
}

void RuleManager::checkRules(const LogEntry& log)
{
    QDateTime now = QDateTime::currentDateTime();

    for (const auto& rule : rules) {
        if (!rule->enabled) continue;
        if (!rule->evaluate(log)) continue;

        qDebug() << "log evaluated true";

        bool timeBased = false;
        for (auto& group : ruleGroups) {
            if (group.rule != rule) continue;
            qDebug() << "Timed rule";
            timeBased = true;

            QString host = rule->perHost ? log.hostname : "global";
            QList<QDateTime>& timestamps = group.entityTimestamps[host];
            timestamps.append(now);

            int windowMs = QTime(0, 0).msecsTo(group.rule->timeWindow);
            clearOldTimestamps(timestamps, now, windowMs);

            // Check if the threshold has been exceeded within the time window
            int count = timestamps.size();
            if (group.rule->triggerCondition == ComparisonOperator::gt && count > group.rule->thresholdCount) emit ruleViolated(group.rule);
            if (group.rule->triggerCondition == ComparisonOperator::gte && count >= group.rule->thresholdCount) emit ruleViolated(group.rule);
        }

        // Not time-based, Generate Alert
        if (!timeBased) {
            qDebug() << "Not time based";
            emit ruleViolated(rule);
        }
    }
}

void RuleManager::clearOldTimestamps(QList<QDateTime>& timestamps, const QDateTime& now, int windowMs)
{
    auto it = timestamps.begin();
    while (it != timestamps.end()) {
        if (it->msecsTo(now) > windowMs) {
            it = timestamps.erase(it);
        } else {
            ++it;
        }
    }
}
