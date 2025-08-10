#pragma once

#include <QObject>
#include <QTimer>
#include <QMap>

#include "rules.h"


class RuleManager : public QObject
{
    Q_OBJECT

public:
    explicit RuleManager(QObject *parent = nullptr);
    bool addRule(std::shared_ptr<Rule> rule);
    void deleteRule(const Rule& rule);
    QList<std::shared_ptr<Rule>> getRules() const;
    std::shared_ptr<Rule> getRuleById(int id) const;
    void checkRules(const LogEntry& log);    

private slots:
    void update();
    void updateRule(std::shared_ptr<Rule> rule);
    void ruleDeleted(std::shared_ptr<Rule> rule);

private:
    QList<std::shared_ptr<Rule>> rules;
    QList<RuleGroup> ruleGroups;
    QTimer *updateTimer;
    void ruleViolated(std::shared_ptr<Rule> rule,  QString source);
    void clearOldTimestamps(QList<QDateTime>& list, const QDateTime& now, int windowMs);
};
