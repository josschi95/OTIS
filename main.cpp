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

    DatabaseManager::instance(); // has to be first since others will call on it
    MainWindow window; // has to be before RuleManager because this will create TestWindow which populates db will filler if empty
    SyslogReceiver receiver;
    RuleManager ruleManager;
    LogParser parser;

    window.passRulesManager(ruleManager);

    // Log comes in -> Parsed into LogEntry -> Stored in DB -> Checked against Rules -> Added to Display
    QObject::connect(&receiver, &SyslogReceiver::logReceived, &window, [&](const QString &line) {
        const auto logEntry = parser.parse(line);
        DatabaseManager::instance().insertLog(logEntry);
        ruleManager.checkRules(logEntry);
    });

    QObject::connect(&ruleManager, &RuleManager::ruleViolated, &window, [&](std::shared_ptr<Rule> violatedRule) {
        qDebug() << "Rule Violated: " << violatedRule->name << " " << QTime().currentTime();
    });

    window.show();
    return app.exec();
}
