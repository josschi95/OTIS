#include "settings_manager.h"


SettingsManager& SettingsManager::instance()
{
    static SettingsManager _instance;
    return _instance;
}

SettingsManager::SettingsManager()
{
    loadTimeZoneFromSettings();
}

QTimeZone SettingsManager::currentTimeZone() const
{
    return timeZone;
}

void SettingsManager::setTimeZone(const QTimeZone& tz)
{
    if (timeZone == tz || !tz.isValid()) return;

    timeZone = tz;
    saveTimeZoneToSettings();
    //emit timeZoneChanged(timeZone);
}

void SettingsManager::loadTimeZoneFromSettings()
{
    QSettings settings;
    QString id = settings.value("timezone", QTimeZone::systemTimeZoneId()).toString();
    timeZone = QTimeZone(id.toUtf8());
    if (!timeZone.isValid())
        timeZone = QTimeZone::systemTimeZone();
}

void SettingsManager::saveTimeZoneToSettings()
{
    QSettings settings;
    settings.setValue("timezone", QString::fromUtf8(timeZone.id()));
}
