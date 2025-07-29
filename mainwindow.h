#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <QStandardItemModel>

#include "database_manager.h"
#include "testwindow.h"
#include "filter_header.h"

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
    void updateLogTable(const LogEntry &log);
    void showLogs();

private slots:
    void onTestWindowButtonClicked();
    void onRefreshLogTable();

private:
    Ui::MainWindow *ui;

    FilterHeader *header;
    QStandardItemModel *model;

    //TODO: Priority Filter
    QComboBox *timestampFilter;
    QLineEdit *hostFilter;
    QLineEdit *tagFilter;
    QLineEdit *messageFilter;

    //TESTING
    TestWindow *testWindow = nullptr;
};
