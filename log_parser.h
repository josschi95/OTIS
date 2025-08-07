#pragma once

#include <QMap>
#include <QString>

#include "logs.h"


class LogParser : public QObject
{
    Q_OBJECT

public:
    explicit LogParser(QObject *parent = nullptr);
    LogEntry parse(const QString &line);

private:
    QMap<QString, LogFormat> logFormats;
    QDateTime parseTimestamp(const QString& ts, const QString& format) const;

};
