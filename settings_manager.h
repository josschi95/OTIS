#pragma once

#include <QObject>
#include <QTimeZone>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager* instance();

    int getPort() const;
    void setPort(const int port);

    QTimeZone currentTimeZone() const;
    void setTimeZone(const QTimeZone& tz);

signals:
    void syslogPortChanged();

private:
    SettingsManager();
    Q_DISABLE_COPY(SettingsManager)

    QTimeZone timeZone;
    int port;
};
