#include <QApplication>

#include "mainwindow.h"
#include "syslog_receiver.h"
#include "log_parser.h"
#include "database_manager.h"

// Operational Technology Insight & Security

int main(int argc, char *argv[])
{
    //QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    QApplication app(argc, argv);

    DatabaseManager::instance();
    SyslogReceiver receiver;
    MainWindow window;

    QObject::connect(&receiver, &SyslogReceiver::logReceived, &window, [&](const QString &line) {
        auto parsed = LogParser::parse(line);
        if (parsed.has_value()) {
            DatabaseManager::insertLog(parsed.value());
            window.updateLogTable(parsed.value());
        }
    });

    window.show();
    return app.exec();
}
