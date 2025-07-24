#include <QApplication>

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

    QObject::connect(&receiver, &SyslogReceiver::logReceived, [&](const QString &line){
        auto parsed = LogParser::parse(line);
        if (parsed.has_value()) {
            DatabaseManager::insertLog(parsed.value());
        }
    });

    MainWindow window;
    window.show();

    return app.exec();
}
