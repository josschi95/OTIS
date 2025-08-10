#pragma once

#include <QObject>
#include <QDateTime>

#include "enums.h"
#include "rules.h"


struct Alert {
    int id = -1;
    const QDateTime timestamp;
    const Severity severity = Severity::Debug;
    const QString source;
    const QString ruleName;
    bool acknowledged = false;

    Alert(const QDateTime& ts, Severity se, const QString& so, const QString& rn)
        : timestamp(ts), severity(se), source(so), ruleName(rn) {}
};


class AlertManager : public QObject
{
    Q_OBJECT

public:
    static AlertManager& instance();

    void raiseAlert(std::shared_ptr<Alert> alert, std::shared_ptr<Rule> rule);
    std::shared_ptr<Alert> getAlertById(int id);
    QList<std::shared_ptr<Alert>> getAlerts(bool acknowledged = false);

signals:
    void alertRaised(std::shared_ptr<Alert>);

private slots:
    void alertDeleted(std::shared_ptr<Alert> alert);

private:
    explicit AlertManager(QObject *parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(AlertManager)
    QList<std::shared_ptr<Alert>> alerts;
    QString expandEnvVars(QString text);
};




