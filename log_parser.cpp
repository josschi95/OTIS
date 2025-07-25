#include <QRegularExpression>
#include <optional>

#include "log_parser.h"
#include "database_manager.h"


std::optional<LogEntry> LogParser::parse(const QString &line)
{
    qDebug() << "LogParser parsing...";
    //static const QRegularExpression re(R"((\w+\s+\d+\s[\d:]+)\s+(\S+)\s+(\S+):\s+(.*))");
    static const QRegularExpression re(R"((\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z)\s+(\S+)\s+(\S+):\s+(.*))");

    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        LogEntry result;
        result.timestamp = match.captured(1);
        result.hostname = match.captured(2);
        result.source = match.captured(3);
        result.message = match.captured(4);

        qDebug() << "Log Timestamp: " << result.timestamp;
        qDebug() << "Log Hostname: " << result.hostname;
        qDebug() << "Log Source: " << result.source;
        qDebug() << "Log Message: " << result.message;

        return result;
    } else {
        qDebug() << "Could not parse " + line;
    }

    return std::nullopt;
}
