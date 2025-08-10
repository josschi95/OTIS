#include "alert_manager.h"
#include "database_manager.h"


AlertManager& AlertManager::instance()
{
    static AlertManager instance;
    static bool initialized = false;

    if (!initialized) {
        initialized = true; // Needs to be first or will crash since referencing instance() below
        instance.alerts = DatabaseManager::instance().loadAlerts();
        connect(&DatabaseManager::instance(), &DatabaseManager::alertDeleted, &AlertManager::instance(), &AlertManager::alertDeleted);
    }

    return instance;
}

void AlertManager::raiseAlert(std::shared_ptr<Alert> alert)
{
    qDebug() << "(AlertManager) raising alert for rule: " << alert->ruleName;
    DatabaseManager::instance().saveAlert(alert); // signal will refresh alert page

    // Flashing UI
    // External communication?
    // Script hooks

    emit alertRaised(alert);
}


std::shared_ptr<Alert> AlertManager::getAlertById(int id)
{
    for (const auto& alert : alerts) {
        if (alert->id == id) return alert;
    }
    return nullptr;
}


QList<std::shared_ptr<Alert>> AlertManager::getAlerts(bool acknowledged)
{
    if (acknowledged) return alerts;
    QList<std::shared_ptr<Alert>> unacknowledged;
    for (const auto& alert : alerts) {
        if (!alert->acknowledged) unacknowledged << alert;
    }
    return unacknowledged;
}


void AlertManager::alertDeleted(std::shared_ptr<Alert> alert)
{
    auto index = alerts.indexOf(alert); // returns -1 if not in list
    if (index >= 0) alerts.removeAt(index);
}
