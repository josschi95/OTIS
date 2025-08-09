#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>

#include "testwindow.h"
#include "rule_manager.h"


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
    void passRulesManager(RuleManager& ruleManager);

private slots:
    void onTestWindowButtonClicked();
    void setActivePage(int index);

private:
    Ui::MainWindow *ui;
    QList<QPushButton*> pageSelectButtons;

    //TESTING
    TestWindow *testWindow = nullptr;
};
