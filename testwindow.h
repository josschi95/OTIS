#pragma once

#include <QMainWindow>


namespace Ui {
class TestWindow;
}

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow();

private slots:
    void onFakeLogButtonClicked();

private:
    Ui::TestWindow *ui;
};
