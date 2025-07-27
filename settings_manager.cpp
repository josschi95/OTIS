#include "settings_manager.h"


SettingsManager* SettingsManager::instance()
{
    static SettingsManager inst;
    return &inst;
}

SettingsManager::SettingsManager()
{
    QSettings settings;

    // Time Zone
    QString id = settings.value("timezone", QTimeZone::systemTimeZoneId()).toString();
    timeZone = QTimeZone(id.toUtf8());
    if (!timeZone.isValid())
        timeZone = QTimeZone::systemTimeZone();

    // Syslog Port
    port = settings.value("port", 5140).toInt();
}

/*** Syslog Port ***/
int SettingsManager::getPort() const
{
    return port;
}

void SettingsManager::setPort(const int num)
{
    if (port == num || num < 0 || num > 65535) return;

    port = num;
    QSettings settings;
    settings.setValue("port", port);

    emit syslogPortChanged();
}

/*** Time Zone ***/
QTimeZone SettingsManager::currentTimeZone() const
{
    return timeZone;
}

void SettingsManager::setTimeZone(const QTimeZone& tz)
{
    if (timeZone == tz || !tz.isValid()) return;

    timeZone = tz;

    QSettings settings;
    settings.setValue("timezone", QString::fromUtf8(timeZone.id()));
}




