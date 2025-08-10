#include <QRegularExpression>
#include <QApplication>
#include <QMainWindow>
#include <QProcess>
#include <QDir>

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

void AlertManager::raiseAlert(std::shared_ptr<Alert> alert, std::shared_ptr<Rule> rule)
{
    //qDebug() << "(AlertManager) raising alert for rule: " << alert->ruleName;
    DatabaseManager::instance().saveAlert(alert); // signal will refresh alert page

    // Flashing UI handled by MainWindow, triggered by alertRaised

    for (QWidget *w : QApplication::topLevelWidgets()) { // Flash in dock/tray
        if (QMainWindow *main = qobject_cast<QMainWindow*>(w)) {
            QApplication::alert(main, 500);
        }
    }

    QApplication::beep(); // audio cue

    // External communication?

    if (!rule->scriptPath.isEmpty()) { // Script hooks
        QString path = rule->scriptPath;
        path.replace("$HOME", QDir::homePath());
        if (path.startsWith("~")) path.replace(0, 1, QDir::homePath());

        QString args = rule->scriptArgs;
        args.replace("{hostname}", alert->source);

        QStringList argsList = QProcess::splitCommand(args);

        if (!QProcess::startDetached(path, argsList)) {
            qWarning() << "Failed to run script: " << rule->scriptPath << argsList;
        }
    }

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
