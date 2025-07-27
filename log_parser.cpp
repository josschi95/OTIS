#include <QRegularExpression>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "log_parser.h"
#include "database_manager.h"
#include "settings_manager.h"


LogParser::LogParser(QObject *parent) : QObject(parent)
{
    // Load logFormats

    QFile file("log_format.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open parser config file at";
        return;
    }

    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON: " << error.errorString();
        return;
    }

    QJsonObject root = doc.object();
    for (const QString& key : root.keys()) {
        if (key == "example") continue;

        QJsonObject obj = root.value(key).toObject();
        LogFormat format;

        format.method = obj.value("method").toString();
        format.dateFormat = obj.value("dateFormat").toString();

        if (obj.contains("pattern")) {
            format.pattern = obj.value("pattern").toString();
        } else if (obj.contains("kvp")) {
            QJsonObject kvpObj = obj.value("kvp").toObject();
            for (const QString& key : kvpObj.keys()) {
                format.kvp.insert(key, kvpObj.value(key).toString());
            }
        }

        logFormats.insert(key, format);
    }
    qDebug() << "Num LogFormats: " << logFormats.size();
}

QDateTime LogParser::parseTimestamp(const QString& ts, const QString& format) {
    if (format.toLower() == "iso8601") {
        return QDateTime::fromString(ts, Qt::ISODate);
    } else if (format.toLower() == "rfc2822") {
        return QDateTime::fromString(ts, Qt::RFC2822Date);
    } else if (format.toLower() == "textdate") {
        return QDateTime::fromString(ts, Qt::TextDate);
    }
    return QDateTime::fromString(ts, format);
}

LogEntry LogParser::parse(const QString &line)
{
    qDebug() << "Num LogFormats: " << logFormats.size();
    LogEntry entry;
    entry.raw = line;

    for (auto it = logFormats.begin(); it != logFormats.end(); ++it) {
        const QString& source = it.key();
        const LogFormat& format = it.value();

        if (format.method == "regex") {
            QRegularExpression re(format.pattern);
            QRegularExpressionMatch match = re.match(line);

            if (match.hasMatch()) {
                entry.timestamp = parseTimestamp(match.captured("timestamp"), format.dateFormat).toString();
                entry.hostname = match.captured("host");
                entry.source = source;
                entry.message = match.captured("message");
                entry.parsed = true;
                break;
            }
        } else if (format.method == "json") {
            // verify whether the log is actually a json format
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
            if (error.error != QJsonParseError::NoError || !doc.isObject()) continue;
            //NOTE: This will just use the first json-based LogFormat that is provided
            // One option is to require a 'unique field' and check for that...

            QJsonObject obj = doc.object();
            entry.timestamp = parseTimestamp(obj.value(format.kvp["timestamp"]).toString(), format.dateFormat).toString();
            entry.hostname = obj.value(format.kvp["hostname"]).toString();
            entry.message = obj.value(format.kvp["message"]).toString();
            entry.source = source;
            entry.parsed = true;
            break;
        } else {
            qWarning() << "Invalid method found for LogFormat: " << source;
        }

    }

    if (!entry.parsed) {
        auto tz = SettingsManager::instance()->currentTimeZone();
        entry.timestamp = QDateTime::currentDateTime(tz).toString(Qt::ISODate);
        entry.message = line;
    }

    return entry;

    /*static const QRegularExpression re(R"((\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(Z|[+-]\d{2}:\d{2}))\s+(\S+)\s+(\S+):\s+(.*))");

    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        LogEntry result;
        result.timestamp = match.captured(1);
        // Regex + ISOFormat means the TZ offset (e.g. -04:00) is match.captured(2)
        result.hostname = match.captured(3);
        result.source = match.captured(4);
        result.message = match.captured(5);

        qDebug() << "Log Timestamp: " << result.timestamp;
        qDebug() << "Log Hostname: " << result.hostname;
        qDebug() << "Log Source: " << result.source;
        qDebug() << "Log Message: " << result.message;
        return result;
    }

    qDebug() << "Could not parse " + line;
    return std::nullopt;*/
}
