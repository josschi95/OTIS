#pragma once

#include <QObject>
#include <QTimeZone>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager& instance();

    QTimeZone currentTimeZone() const;
    void setTimeZone(const QTimeZone& tz);

signals:
    //void timeZoneChanged(const QTimeZone& newZone);

private:
    SettingsManager();
    Q_DISABLE_COPY(SettingsManager)

    QTimeZone timeZone;
    void loadTimeZoneFromSettings();
    void saveTimeZoneToSettings();
};
