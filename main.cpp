#include <QApplication>

#include "mainwindow.h"
#include "syslog_receiver.h"
#include "log_parser.h"
#include "database_manager.h"
#include "rule_manager.h"

// https://www.rfc-editor.org/rfc/rfc5424
// https://www.se.com/us/en/faqs/FA409423/


// Operational Technology Insight & Security

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DatabaseManager::instance();
    SyslogReceiver receiver;
    RuleManager ruleManager;
    MainWindow window;
    LogParser parser;

    // Log comes in -> Parsed into LogEntry -> Stored in DB -> Checked against Rules -> Added to Display
    QObject::connect(&receiver, &SyslogReceiver::logReceived, &window, [&](const QString &line) {
        const auto logEntry = parser.parse(line);
        // Returning row from DB here so it reuses same format as rest of table
        // LogsPage doesn't need to know what a LogEntry is
        const auto row = DatabaseManager::insertLog(logEntry);
        ruleManager.checkRules(logEntry);
        window.getLogsPage()->addRow(row);
    });

    QObject::connect(&ruleManager, &RuleManager::ruleViolated, &window, [&](std::shared_ptr<Rule> violatedRule) {
        qDebug() << "Rule Violated: " << violatedRule->name;
    });

    window.show();
    return app.exec();
}
