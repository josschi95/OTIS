#include <QTimer>

#include "rule_manager.h"


RuleManager::RuleManager(QObject *parent) : QObject(parent)
{
    // Load Logs from JSON

    auto newRule = std::make_shared<Rule>();
    newRule->name = "Multiple Failed Logins";
    newRule->msgIDValue = "LOGIN_FAILURE";
    newRule->msgIDOp = StringComparison::ExactMatch;
    newRule->thresholdCount = 3;
    newRule->timeWindow = QTime().fromString("00:01:00");
    newRule->triggerCondition = FilterOperator::gte;
    addRule(newRule);

    // Start Update loop
    updateTimer = new QTimer(this);
    updateTimer->setInterval(5 * 1000);
    connect(updateTimer, &QTimer::timeout, this, &RuleManager::update);
    updateTimer->start();
}

// Needed for tracking 'heartbeat' rules (e.g. those with a triggerCondition of < or <=)
void RuleManager::update()
{
    //qDebug() << "Update";

    //TODO: Need to handle 'heartbeat' rules

    for (const auto& rule : rules) {
        if (!rule->enabled) continue;
        if (!ruleTimers.contains(rule)) continue;

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

    // To count as a 'timed rule' then thresholdCount must be > 0
    // and timeWindow must be >= 60 seconds

    rules << rule;

    if (rule->thresholdCount > 0 && (rule->timeWindow.minute() > 0 || rule->timeWindow.hour() > 0)) {
        ruleTimers.insert(rule, QList<QDateTime>());
    }

    return true;
}

void RuleManager::checkRules(const LogEntry& log)
{
    QDateTime now = QDateTime::currentDateTime();

    //TODO: need to have option to track separately for different hosts
    // at the moment, the test rule checks for ANY failed login, so 3 different people each failing once will trigger this alert
    for (const auto& rule : rules) {
        if (!rule->enabled) continue;
        if (!rule->evaluate(log)) continue;
        qDebug() << "log evaluated true";
        if (!ruleTimers.contains(rule)) {
            // Not time-based, Generate Alert
            emit ruleViolated(rule);
        } else {
            qDebug() << "Timed rule";
            QList<QDateTime>& timestamps = ruleTimers[rule];
            timestamps.append(now);

            // Remove old timestamps outside timeWindow
            auto it = timestamps.begin();
            while (it != timestamps.end()) {
                if (it->msecsTo(now) > QTime(0, 0).msecsTo(rule->timeWindow)) {
                    it = timestamps.erase(it);
                } else {
                    ++it;
                }
            }

            int count = timestamps.size();
            qDebug() << count;
            if (rule->triggerCondition == FilterOperator::gt && count > rule->thresholdCount) emit ruleViolated(rule);
            if (rule->triggerCondition == FilterOperator::gte && count >= rule->thresholdCount) emit ruleViolated(rule);
        }
    }
}
