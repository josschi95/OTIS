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

//TODO: I think this is old...?
QDateTime LogParser::parseTimestamp(const QString& ts, const QString& format) const {
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
    //TESTING
    static QString rfc5424 = "^<(?<priority>\\d{1,2}|1[0-8]\\d|19[01])>"
                        "(?<version>[1-9]\\d?)\\s"
                        "(?<timestamp>-|(?<fullyear>[12]\\d{3})-(?<month>0\\d|1[0-2])-(?<mday>[0-2]\\d|3[01])T"
                            "(?<hour>[01]\\d|2[0-3]):(?<minute>[0-5]\\d):(?<second>[0-5]\\d|60)(?:\\.(?<secfrac>\\d{1,6}))?(?<numoffset>Z|[+-]\\d{2}:\\d{2}))\\s"
                        "(?<hostname>\\S{1,255})\\s"
                        "(?<app_name>\\S{1,48})\\s"
                        "(?<procid>\\S{1,128})\\s"
                        "(?<msgid>\\S{1,32})\\s"
                        "(?<structured_data>-|\\[.*\\])"
                        "(?:\\s(?<msg>.+))?$";
    QRegularExpression foo(rfc5424);
    if (!foo.isValid()) {
        //qDebug() << "Regex Error: " << foo.errorString();
    }
    //TESTING


    LogEntry entry;
    entry.raw = line;

    for (auto it = logFormats.begin(); it != logFormats.end(); ++it) {
        const QString& standard = it.key();
        const LogFormat& format = it.value();

        QRegularExpression re(format.pattern);
        QRegularExpressionMatch match = re.match(line);

        if (match.hasMatch()) {
            //qDebug() << "Found matching pattern: " << standard;

            const int priority = match.captured("priority").toInt();
            entry.severity = priority % 8;
            entry.facility = priority / 8;
            //entry.version = match.captured("version").toInt();
            entry.timestamp = parseTimestamp(match.captured("timestamp"), format.dateFormat).toString();
            entry.hostname = match.captured("hostname");
            entry.appname = match.captured("app_name");
            entry.procid = match.captured("procid");
            entry.msgid = match.captured("msgid");
            entry.structureddata = match.captured("structured_data");
            entry.msg = match.captured("msg");
            break;
        }
    }

    if (entry.timestamp.isEmpty() && entry.msg.isEmpty()) {
        qWarning() << "Could not parse log: " << line;
        auto tz = SettingsManager::instance()->currentTimeZone();
        entry.timestamp = QDateTime::currentDateTime(tz).toString(Qt::ISODate);
        entry.msg = line;
    }

    return entry;
}
