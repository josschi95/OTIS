#pragma once

#include <QComboBox>
#include <QMainWindow>

#include "logs_page.h"
#include "rules_page.h"
#include "testwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    LogsPage* getLogsPage() const { return logsPage; }

private slots:
    void onTestWindowButtonClicked();
    void setActivePage(int index);

private:
    Ui::MainWindow *ui;
    QList<QPushButton*> pageSelectButtons;

    // Other pages
    RulesPage* rulesPage;
    LogsPage* logsPage;

    //TESTING
    TestWindow *testWindow = nullptr;
};
