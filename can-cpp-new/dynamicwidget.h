// dynamicwidget.h
#pragma once

#include <QtWidgets>
#include <QTimer>
#include <memory>
#include "dynamic.h"
#include "can_manager.h"

class DynamicWidget : public QWidget {
    Q_OBJECT

public:
    explicit DynamicWidget(QWidget* parent = nullptr);
    ~DynamicWidget();

private slots:
    void toggleSimulation();
    void updateSimulation();

private:
    std::unique_ptr<QTimer> m_timer;
    bool m_isRunning{false};
    std::vector<Dynamic> m_cars;
    Dynamic* m_car{nullptr};

    CANManager m_can;
    std::mutex m_canMutex;
    double m_recu{0.0};  // letztmals empfangene Reku‑Daten

    // GUI‑Widgets
    QLabel* carInfo{};
    QComboBox* m_carSelector{};
    QSlider* m_speedSlider{};
    QSlider* m_brakeSlider{};
    QSlider* m_rekuSlider{};
    QLabel* m_speedLabel{};
    QLabel* m_accelLabel{};
    QLabel* m_powerLabel{};
    QLabel* m_distanceLabel{};
    QLabel* m_powerConsLabel{};
    QLabel* m_avgPowerLabel{};
    QLabel* m_tractionLabel{};
    QLabel* m_recuLabel{};
    QLabel* m_timeLabel{};
    QLabel* m_socLabel{};
    QLabel* m_cellTempLabel{};
    QLabel* m_inverterTempLabel{};
    QPushButton* m_startButton{};   // CAN‑ID‑Konstanten wie in deinem Python‑Code
 
    static constexpr uint32_t RECU_CAN_ID = 0xCF090FE;
    static constexpr uint32_t SPEED_CAN_ID = 0xCFE6CFE;
    static constexpr uint32_t SOC_CAN_ID = 0xCF091FE;
    static constexpr uint32_t CELL_TEMP_CAN_ID = 0xCF092FE;
    static constexpr uint32_t INVERTER_TEMP_CAN_ID = 0x18FB72FE;
    static constexpr uint32_t SEND_CAN_ID = 0x123;


    void createUI();
    void createConnections();
    void selectCar(const QString& name);
    void updateCarInfo();
    void sendCANData();
    void updateLabels();
};
