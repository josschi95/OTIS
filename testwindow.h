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
    void on_user_successfulLoginButton_clicked();
    void on_user_failedLoginButton_clicked();
    void on_user_firewallDisabledButton_clicked();
    void on_hmi_operatorLoginButton_clicked();
    void on_hmi_pumpStartButton_clicked();
    void on_hmi_pumpStopButton_clicked();
    void on_hmi_unauthAccessButton_clicked();
    void on_scada_plcStartButton_clicked();
    void on_scada_plcStopButton_clicked();
    void on_scada_periodicReadingButton_clicked();
    void on_scada_sensorOfflineButton_clicked();
    void on_tempSensorSlider_valueChanged(int value);
    void on_tempSensorSlider_sliderReleased();
    void on_vibrationSensorEnabledCheckbox_checkStateChanged(const Qt::CheckState &arg1);
    void on_user_failedLoginButton_2_clicked();
    void onTimerTrigger();

private:
    Ui::TestWindow *ui;
    QTimer *logTimer;
    void sendTestLog(QByteArray log);
};
