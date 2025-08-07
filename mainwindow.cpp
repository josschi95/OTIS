#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "testwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pageSelectButtons = { ui->overviewSelectButton, ui->devicesSelectButton, ui->rulesSelectButton, ui->logsSelectButton };
    setActivePage(0);
    for (int i = 0; i < pageSelectButtons.count(); ++i) {
        int index = i;
        connect(pageSelectButtons[index], &QPushButton::clicked, this, [=]() {
            setActivePage(index);
        });
    }

    // Other Pages here
    ui->page2_rules->initialize();
    rulesPage = ui->page2_rules;

    ui->page3_logs->initialize();
    logsPage = ui->page3_logs; // Do I need to store this?

    onTestWindowButtonClicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTestWindowButtonClicked()
{
    if (!testWindow) {
        testWindow = new TestWindow(this);
    }
    testWindow->show();
    testWindow->raise();
    testWindow->activateWindow();
}

void MainWindow::setActivePage(int index)
{
    // alt underline 0078D7
    ui->stackedWidget->setCurrentIndex(index);
    for (int i = 0; i < pageSelectButtons.count(); ++i) {
        auto btn = pageSelectButtons[i];
        if (i == index) {
            btn->setStyleSheet(R"(
                QPushButton {
                    border: none;
                    background: transparent;
                    font-weight: bold;
                    color: white;
                    border-bottom: 2px solid #0553b2;
                }
            )");
        } else {
            btn->setStyleSheet(R"(
                QPushButton {
                    border: none;
                    background: transparent;
                    font-weight: normal;
                    color: white;
                }
            )");
        }
    }
}
