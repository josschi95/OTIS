#pragma once

#include <QObject>
#include <QTimer>
#include <QMap>

#include "rules.h"
//#include "database_manager.h"


class RuleManager : public QObject
{
    Q_OBJECT

public:
    explicit RuleManager(QObject *parent = nullptr);
    bool addRule(std::shared_ptr<Rule> rule);
    void deleteRule(const Rule& rule);
    QList<std::shared_ptr<Rule>> getRules() const;
    void checkRules(const LogEntry& log);

signals:
    void ruleViolated(std::shared_ptr<Rule> violdatedRule);

private slots:
    void update();

private:
    QList<std::shared_ptr<Rule>> rules;
    QList<RuleGroup> ruleGroups;
    //QMap<std::shared_ptr<Rule>, QList<QDateTime>> ruleTimers; //TODO: rename
    QTimer *updateTimer;
    void clearOldTimestamps(QList<QDateTime>& list, const QDateTime& now, int windowMs);
};
