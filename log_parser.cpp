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

        format.pattern = obj.value("pattern").toString();
        format.dateFormat = obj.value("dateFormat").toString();
        logFormats.insert(key, format);
    }
    //qDebug() << "Num LogFormats: " << logFormats.size();
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
    LogEntry entry;
    entry.raw = line;

    for (auto it = logFormats.begin(); it != logFormats.end(); ++it) {
        const QString& standard = it.key();
        const LogFormat& format = it.value();

        QRegularExpression re(format.pattern);
        QRegularExpressionMatch match = re.match(line);

        if (match.hasMatch()) {
            //qDebug() << "Found matching pattern: " << standard;

            entry.priority = match.captured("pri").toInt();
            //Version
            entry.timestamp = parseTimestamp(match.captured("timestamp"), format.dateFormat).toString();
            entry.host = match.captured("host");
            entry.app = match.captured("app_name");
            entry.pid = match.captured("procid");
            // msgid
            // data
            entry.message = match.captured("msg");
            break;
        }
    }

    if (entry.timestamp.isEmpty() && entry.message.isEmpty()) {
        qWarning() << "Could not parse log: " << line;
        auto tz = SettingsManager::instance()->currentTimeZone();
        entry.timestamp = QDateTime::currentDateTime(tz).toString(Qt::ISODate);
        entry.message = line;
    }

    return entry;
}
