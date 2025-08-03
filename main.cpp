#include <QApplication>

#include "mainwindow.h"
#include "syslog_receiver.h"
#include "log_parser.h"
#include "database_manager.h"

// https://www.rfc-editor.org/rfc/rfc5424
// https://www.se.com/us/en/faqs/FA409423/


// Operational Technology Insight & Security

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DatabaseManager::instance();
    SyslogReceiver receiver;
    MainWindow window;
    LogParser parser;

    QObject::connect(&receiver, &SyslogReceiver::logReceived, &window, [&](const QString &line) {
        const auto parsed = parser.parse(line);
        const auto row = DatabaseManager::insertLog(parsed);
        window.getLogsPage()->addRow(row);
    });

    window.show();
    return app.exec();
}
