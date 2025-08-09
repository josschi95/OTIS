#include <QTimer>

#include "rule_manager.h"
#include "database_manager.h"


RuleManager::RuleManager(QObject *parent) : QObject(parent)
{
    // Load in saved rules from db
    auto rules = DatabaseManager::instance().loadRules();
    for (const auto& rule : rules) addRule(rule);

    connect(&DatabaseManager::instance(), &DatabaseManager::ruleSaved, this, &RuleManager::updateRule);
    connect(&DatabaseManager::instance(), &DatabaseManager::ruleDeleted, this, &RuleManager::updateRule);

    // Start Update loop
    updateTimer = new QTimer(this);
    updateTimer->setInterval(5 * 1000); // 5 seconds
    connect(updateTimer, &QTimer::timeout, this, &RuleManager::update);
    updateTimer->start();
}


// Needed for tracking 'heartbeat' rules - those with a triggerCondition of < or <=
// Have to do this because logs *not* coming in doesn't call any function to check
void RuleManager::update()
{
    QDateTime now = QDateTime::currentDateTime();

    for (auto& group : ruleGroups) {
        if (!group.rule->enabled) continue; // user disabled
        if (group.rule->thresholdCount < 0) continue; // not time-based
        if (group.rule->triggerCondition != ComparisonOperator::lt && group.rule->triggerCondition != ComparisonOperator::lte) continue;
        //qDebug() << group.rule->name;

        int windowMs = QTime(0, 0).msecsTo(group.rule->timeWindow);

        auto it = group.entityTimestamps.begin();
        while (it != group.entityTimestamps.end()) {
            QList<QDateTime>& timestamps = it.value();
            clearOldTimestamps(timestamps, now, windowMs);

            int count = timestamps.size(); // Verify that there are the given number of logs remaining within the time window
            if (group.rule->triggerCondition == ComparisonOperator::lt && count < group.rule->thresholdCount) emit ruleViolated(group.rule);
            if (group.rule->triggerCondition == ComparisonOperator::lte && count <= group.rule->thresholdCount) emit ruleViolated(group.rule);

            ++it;
        }
    }

    updateTimer->start();
}


// DatabaseManager added new rule or changed existing one
void RuleManager::updateRule(std::shared_ptr<Rule> rule)
{
    // It's easier to just remove and re-add updated rule
    // addRule will handle changes that might affect RuleGroup and tracked timestamps
    ruleDeleted(rule);
    addRule(rule);
}


void RuleManager::ruleDeleted(std::shared_ptr<Rule> rule)
{
    auto index = rules.indexOf(rule);
    if (index >= 0) { // returns -1 if not in list
        //qDebug() << "(RuleManager) updated rule: " << rule->name;
        rules.removeAt(index);

        for (int i = 0; i < ruleGroups.size(); ++i) {
            if (ruleGroups[i].rule == rule) {
                ruleGroups.removeAt(i);
                break;
            }
        }
    }
}


QList<std::shared_ptr<Rule>> RuleManager::getRules() const
{
    return rules;
}


std::shared_ptr<Rule> RuleManager::getRuleById(int id) const
{
    for (const auto& rule : rules) {
        if (rule->id == id) return rule;
    }
    return NULL;
}


bool RuleManager::addRule(std::shared_ptr<Rule> rule)
{
    if (!rule) return false;
    //qDebug() << "Adding rule: " << rule->name;
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

        //qDebug() << "log evaluated true";

        bool timeBased = false;
        for (auto& group : ruleGroups) {
            if (group.rule != rule) continue;
            //qDebug() << "Timed rule";
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
            //qDebug() << "Not time based";
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
