#pragma once

#include <QString>
#include <QMap>
#include <optional>

#include "database_manager.h"

class LogParser
{
public:
    static std::optional<LogEntry> parse(const QString &line);
};
