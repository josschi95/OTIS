#include <QApplication>
#include <QTimeZone> // TESTING

#include "mainwindow.h"
#include "syslog_receiver.h"
#include "log_parser.h"
#include "database_manager.h"

// Operational Technology Insight & Security

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DatabaseManager::instance();
    SyslogReceiver receiver;
    MainWindow window;
    LogParser parser;

    QObject::connect(&receiver, &SyslogReceiver::logReceived, &window, [&](const QString &line) {
        auto parsed = parser.parse(line);
        DatabaseManager::insertLog(parsed);
        window.updateLogTable(parsed);
    });

    window.show();
    return app.exec();
}
