#include "./ui_mainwindow.h"
#include "mainwindow.h"
#include "database_manager.h"
#include "testwindow.h"
#include "filter_header.h"

#include <QVBoxLayout>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->testWindowButton, &QPushButton::clicked, this, &MainWindow::onTestWindowButtonClicked);
    connect(ui->refreshLogsButton, &QPushButton::clicked, this, &MainWindow::onRefreshLogTable);

    auto view = ui->tableView;
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(view);

    header = new FilterHeader(view);
    view->setHorizontalHeader(header);
    // row numbers messes up alignment
    view->verticalHeader()->setVisible(false);

    model = new QStandardItemModel(view);
    model->setHorizontalHeaderLabels({"Priority", "Timestamp", "Host", "App", "Message"});
    view->setModel(model);

    onTestWindowButtonClicked();

    onRefreshLogTable();
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

void MainWindow::onRefreshLogTable()
{
    model->removeRows(0, model->rowCount());

    auto filters = header->getFilters();
    auto rows = DatabaseManager::queryDB(filters);
    qDebug() << "DB query rows returned: " << rows.size();
    for (int i = 0; i < rows.size(); ++i) {
        model->appendRow(rows[i]);
    }
}

//TODO: Pull from db, don't use logEntry directly
void MainWindow::updateLogTable(const LogEntry &logEntry)
{
    //auto view = ui->tableView;
    QList<QStandardItem*> row;
    row << new QStandardItem(QString::number(logEntry.priority))
        << new QStandardItem(logEntry.timestamp)
        << new QStandardItem(logEntry.host)
        << new QStandardItem(logEntry.app)
        << new QStandardItem(logEntry.message);
    model->appendRow(row);

    // Old
    //int row = ui->logTable->rowCount();
    //ui->logTable->insertRow(row);
    //ui->logTable->setItem(row, 0, new QTableWidgetItem(logEntry.timestamp));
    //ui->logTable->setItem(row, 1, new QTableWidgetItem(logEntry.host));
    //ui->logTable->setItem(row, 2, new QTableWidgetItem(logEntry.app));
    //ui->logTable->setItem(row, 3, new QTableWidgetItem(logEntry.message));
}
