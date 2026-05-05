// dynamicwidget.cpp
#include "dynamicwidget.h"
#include <algorithm>
#include <sstream>



DynamicWidget::DynamicWidget(QWidget* parent)
    : QWidget(parent)
    , m_can("can0")  // CAN‑Schnittstelle
{
    m_cars = Dynamic::getCars();

    setWindowTitle("Brake & Speed Control");
    resize(950, 560);

    createUI();
    createConnections();

    if (!m_can.open()) {
        qWarning() << "Kann CAN-Interface nicht öffnen:" << m_can.getInterface().c_str();
    }
}

DynamicWidget::~DynamicWidget() {
    m_can.close();
}

void DynamicWidget::toggleSimulation() {
    qDebug() << "toggleSimulation() called";

    if (m_isRunning) {
        m_isRunning = false;
        m_timer->stop();
        qDebug() << "timer stopped";
    } else {
        if (!m_car) {
            m_car = &m_cars.front();
            updateCarInfo();
        }
        m_isRunning = true;
        m_timer->start(10);
        qDebug() << "timer started, active =" << m_timer->isActive();
    }
}

void DynamicWidget::createUI() {
    m_timer = std::make_unique<QTimer>(this);
    connect(m_timer.get(), &QTimer::timeout, this, &DynamicWidget::updateSimulation);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    carInfo = new QLabel("Bitte Auto wählen", this);
    carInfo->setFrameStyle(QFrame::Panel | QFrame::Raised);
    carInfo->setLineWidth(2);
    carInfo->setStyleSheet("padding: 8px; background: #222; color: white;");
    mainLayout->addWidget(carInfo);

    m_carSelector = new QComboBox(this);
    m_carSelector->addItem("Select Car");
    for (const auto& c : m_cars)
        m_carSelector->addItem(QString::fromStdString(c.getName()));
    mainLayout->addWidget(m_carSelector);

    QHBoxLayout* slidersLayout = new QHBoxLayout();
    mainLayout->addLayout(slidersLayout);

    QVBoxLayout* speedLayout = new QVBoxLayout();
    QLabel* speedTitle = new QLabel("Pedal", this);
    m_speedSlider = new QSlider(Qt::Vertical, this);
    m_speedSlider->setRange(0, 100);
    m_speedSlider->setValue(0);
    m_speedSlider->setMinimumHeight(200);
    m_speedSlider->setFixedWidth(70);
    speedLayout->addWidget(speedTitle);
    speedLayout->addWidget(m_speedSlider);

    QVBoxLayout* brakeLayout = new QVBoxLayout();
    QLabel* brakeTitle = new QLabel("Brake", this);
    m_brakeSlider = new QSlider(Qt::Vertical, this);
    m_brakeSlider->setRange(0, 100);
    m_brakeSlider->setValue(0);
    m_brakeSlider->setMinimumHeight(200);
    m_brakeSlider->setFixedWidth(70);
    brakeLayout->addWidget(brakeTitle);
    brakeLayout->addWidget(m_brakeSlider);
	
    QVBoxLayout* rekuLayout = new QVBoxLayout();
    QLabel* rekuTitle = new QLabel("Rekuperation", this);
    m_rekuSlider = new QSlider(Qt::Vertical, this);
    m_rekuSlider->setRange(0,100);
    m_rekuSlider->setValue(0);
    m_rekuSlider->setMinimumHeight(200);
    m_rekuSlider->setFixedWidth(70);
    rekuLayout->addWidget(rekuTitle);
    rekuLayout->addWidget(m_rekuSlider);

    slidersLayout->addLayout(speedLayout);
    slidersLayout->addLayout(brakeLayout);
    slidersLayout->addLayout(rekuLayout);

    m_speedLabel = new QLabel("Speed: 0 km/h", this);
    m_accelLabel = new QLabel("Acceleration: 0 m/s²", this);
    m_powerLabel = new QLabel("Power: 0 kW", this);
    m_distanceLabel = new QLabel("Distance: 0 km", this);
    m_powerConsLabel = new QLabel("Powerconsumption: 0 kWh", this);
    m_avgPowerLabel = new QLabel("avg. Powerconsumption: 0 kWh/100 km", this);
    m_tractionLabel = new QLabel("Traction: ok", this);
    m_recuLabel = new QLabel("Reku (CAN): 0.0 %", this);
    m_timeLabel = new QLabel("Time: 0:00 min", this);
    m_socLabel = new QLabel("SOC: 100.0 %", this);
    m_cellTempLabel = new QLabel("Cell Temp: 25.0 °C", this);
    m_inverterTempLabel = new QLabel("Inverter Temp: 25.0 °C", this);

    mainLayout->addWidget(m_speedLabel);
    mainLayout->addWidget(m_accelLabel);
    mainLayout->addWidget(m_powerLabel);
    mainLayout->addWidget(m_distanceLabel);
    mainLayout->addWidget(m_powerConsLabel);
    mainLayout->addWidget(m_avgPowerLabel);
    mainLayout->addWidget(m_tractionLabel);
    mainLayout->addWidget(m_recuLabel);
    mainLayout->addWidget(m_timeLabel);
    mainLayout->addWidget(m_socLabel);
    mainLayout->addWidget(m_cellTempLabel);
    mainLayout->addWidget(m_inverterTempLabel);

    m_startButton = new QPushButton("Start/Stop", this);
    m_startButton->setStyleSheet("padding: 8px; font-size: 14px;");
    connect(m_startButton, &QPushButton::clicked, this, &DynamicWidget::toggleSimulation);
    mainLayout->addWidget(m_startButton);
}

void DynamicWidget::createConnections() {
    connect(m_carSelector, &QComboBox::currentTextChanged,
            this, [this](const QString& name) { selectCar(name); });
}

void DynamicWidget::selectCar(const QString& name) {
    if (name.isEmpty() || name == "Select Car")
        m_car = nullptr;
    else {
        auto it = std::find_if(m_cars.begin(), m_cars.end(),
                               [&name](const Dynamic& c) {
                                   return c.getName() == name.toStdString();
                               });
        if (it != m_cars.end())
            m_car = &*it;
        else
            m_car = &m_cars.front();
        updateCarInfo();
    }
}
void DynamicWidget::updateCarInfo() {
    if (!carInfo || !m_car) return;

    carInfo->setText(QString("🚗 <b>%1</b><br>"
                              "Masse: %2 kg | "
                              "Max Drive: %3 kW | "
                              "Max Recu: %4 kW<br>"
                              "Cw: %5 | Fläche: %6 m²")
                       .arg(QString::fromStdString(m_car->getName()))
                       .arg(m_car->getMass())
                       .arg(m_car->getPDriveMax() / 1000.0, 0, 'f', 0)
                       .arg(m_car->getPRecuMax() / 1000.0, 0, 'f', 0)
                       .arg(m_car->getCw(), 0, 'f', 2)
                       .arg(m_car->getFrontSurface(), 0, 'f', 2));
}

void DynamicWidget::updateSimulation() {
    if (!m_isRunning || !m_car) return;
    if (!m_speedSlider || !m_brakeSlider) return;

    double pedal = m_speedSlider->value() / 100.0;
    double brake = m_brakeSlider->value() / 100.0;
    m_car->setRecu(m_rekuSlider->value());

    qDebug() << "slider speed =" << m_speedSlider->value()
             << "brake =" << m_brakeSlider->value()
             << "pedal =" << pedal
             << "recu =" << m_car->getRecu();

    m_car->compute(pedal, brake, m_car->getRecu());

    qDebug() << "after compute:"
             << "speed =" << m_car->getSpeed()
             << "acc =" << m_car->getAcceleration()
             << "power =" << m_car->getPower()
             << "time =" << m_car->getTime()
             << "dist =" << m_car->getDistance();

    updateLabels();
    sendCANData();
}

void DynamicWidget::updateLabels() {
    if (!m_car) return;

    m_speedLabel->setText(QString("Speed: %1 km/h").arg(m_car->getSpeed(), 0, 'f', 1));
    m_accelLabel->setText(QString("Acceleration: %1 m/s²").arg(m_car->getAcceleration(), 0, 'f', 2));
    m_powerLabel->setText(QString("Power: %1 kW").arg(m_car->getPower(), 0, 'f', 1));
    m_distanceLabel->setText(QString("Distance: %1 km").arg(m_car->getDistance(), 0, 'f', 2));
    m_powerConsLabel->setText(QString("Powerconsumption: %1 kWh").arg(m_car->getPowerConsumption(m_car->getRecu()), 0, 'f', 2));
    m_avgPowerLabel->setText(QString("avg. Powerconsumption: %1 kWh/100 km").arg(m_car->getAvgPowerConsumption(), 0, 'f', 2));

    m_tractionLabel->setText(m_car->getTraction() ? "Traction: OK" : "Traction: FAIL");
    m_tractionLabel->setStyleSheet(
        m_car->getTraction()
            ? "QLabel { background:#1f7a1f; color:white; padding:3px 8px; border-radius:6px; max-width:130px; }"
            : "QLabel { background:#8b1e1e; color:white; padding:3px 8px; border-radius:6px; max-width:130px; }"
    );

    double soc = m_car->getSOC();
    QString socColor = soc > 50 ? "#1f7a1f" : (soc > 20 ? "#ffaa00" : "#8b1e1e");
    m_socLabel->setText(QString("SOC: %1 %").arg(soc, 0, 'f', 1));
    m_socLabel->setStyleSheet(QString(
        "QLabel { background:%1; color:white; padding:3px 8px; border-radius:6px; }"
    ).arg(socColor));

    double cellTemp = m_car->getCellTemp() * 0.03125;
    QString cellColor = cellTemp < 40 ? "#1f7a1f" : (cellTemp < 60 ? "#ffaa00" : "#8b1e1e");
    m_cellTempLabel->setText(QString("Cell Temp: %1 °C").arg(cellTemp, 0, 'f', 1));
    m_cellTempLabel->setStyleSheet(QString(
        "QLabel { background:%1; color:white; padding:3px 8px; border-radius:6px; }"
    ).arg(cellColor));

    double invTemp = m_car->getInvTemp() - 40;
    QString invColor = invTemp < 60 ? "#1f7a1f" : (invTemp < 80 ? "#ffaa00" : "#8b1e1e");
    m_inverterTempLabel->setText(QString("Inverter Temp: %1 °C").arg(invTemp, 0, 'f', 1));
    m_inverterTempLabel->setStyleSheet(QString(
        "QLabel { background:%1; color:white; padding:3px 8px; border-radius:6px; }"
    ).arg(invColor));

    double recuPct = m_recu * 100.0;
    QString recuColor = recuPct < 50 ? "#1f7a1f" : (recuPct < 80 ? "#ffaa00" : "#8b1e1e");
    m_recuLabel->setText(QString("Reku (CAN): %1 %").arg(recuPct, 0, 'f', 1));
    m_recuLabel->setStyleSheet(QString(
        "QLabel { background:%1; color:white; padding:3px 8px; border-radius:6px; }"
    ).arg(recuColor));

    int minutes = static_cast<int>(m_car->getTime()) / 60;
    int seconds = static_cast<int>(m_car->getTime()) % 60;
    m_timeLabel->setText(QString("Time: %1:%2 min").arg(minutes).arg(seconds, 2, 10, QChar('0')));
}


void DynamicWidget::sendCANData()
{
    if (!m_car) return;

    auto clamp16 = [](int v) -> uint16_t {
        if (v < 0) return 0;
        if (v > 65535) return 65535;
        return static_cast<uint16_t>(v);
    };

    try {
        
	/*double starterVolt = m_car->getStarterVolt() +1 ; 
	uint8_t voltRaw = static_cast<uint8_t>(std::max(0.0, std::min(255.0, starterVolt * 10.0)));

	uint8_t voltData[8] = {0}; 
	voltData[5] = voltRaw; // Byte 5 laut Bit 47

	//m_can.send(CANMessage(0x2F0A001, voltData, 8, true));
*/
	// --- SOC (Bytes 0-1) ---
        uint16_t socRaw = clamp16(static_cast<int>(m_car->getSOC() / 0.0015625));
        uint8_t socData[8] = {
            static_cast<uint8_t>(socRaw & 0xFF),
            static_cast<uint8_t>((socRaw >> 8) & 0xFF),
            0, 0, 0, 0, 0, 0
        };
        m_can.send(CANMessage(0x0CF091FE, socData, 8, true));

  /*      // --- Speed (Bytes 6-7) ---
        uint16_t speedRaw = clamp16(static_cast<int>(m_car->getSpeed() / 0.00390625));
	uint8_t speedData[8] = {
            0, 0, 0, 0, 0, 0,
            static_cast<uint8_t>(speedRaw & 0xFF),
            static_cast<uint8_t>((speedRaw >> 8) & 0xFF)
        };
        m_can.send(CANMessage(0x0CFE6CFE, speedData, 8, true));
*/
 

	uint8_t data_2F0A001[8] = {0};

	// 12V battery -> start bit 47 -> byte 5
	double starterVolt = m_car->getStarterVolt() + 1;
	data_2F0A001[5] = static_cast<uint8_t>(std::clamp(std::round(starterVolt * 10.0), 0.0, 255.0));

	// speed -> start bit 63 -> byte 7
	double speedRaw = m_car->getSpeed() * 0.621371;
	data_2F0A001[7] = static_cast<uint16_t>(std::clamp(speedRaw, 0.0, 255.0));

	m_can.send(CANMessage(0x2F0A001, data_2F0A001, 8, true));


	// --- Cell Temp (Bytes 0-1) ---
	uint16_t cellRaw = clamp16(static_cast<int>(m_car->getCellTemp() / 0.03125));
        uint8_t cellData[8] = {
            static_cast<uint8_t>(cellRaw & 0xFF),
            static_cast<uint8_t>((cellRaw >> 8) & 0xFF),
            0, 0, 0, 0, 0, 0
        };
	m_can.send(CANMessage(0x0CF092FE, cellData, 8, true));

        // --- Inverter Temp (Byte 0) ---
        int invRawInt = static_cast<int>(m_car->getInvTemp());
        uint8_t invData[8] = {
            static_cast<uint8_t>(invRawInt),
            0, 0, 0, 0, 0, 0, 0
        };
	m_can.send(CANMessage(0x18FB72FE, invData, 8, true));

        // --- Discharge Power (Bytes 0-1) ---
/*        uint16_t disRaw = clamp16(static_cast<uint16_t>(m_car->getDisPower() / 0.05));
        uint8_t disData[8] = {
            static_cast<uint8_t>(disRaw & 0xFF),
            static_cast<uint8_t>((disRaw >> 8) & 0xFF),
            0, 0, 0, 0, 0, 0
        };
        m_can.send(CANMessage(0x0CF090FE, disData, 8, true));
*/


	double powerKw = m_car->getPower();   // darf negativ sein
	int16_t powerRaw = static_cast<int16_t>(std::round(powerKw * 10.0));

	uint8_t powerData[8] = {0};
	powerData[6] = static_cast<uint8_t>((powerRaw >> 8) & 0xFF);
	powerData[7] = static_cast<uint8_t>(powerRaw & 0xFF);

	m_can.send(CANMessage(0x2F0A209, powerData, 8, true));
	
	uint16_t rawValue = static_cast<uint16_t>(2000 + (m_car->getRecu() * 20));

	// Big Endian (Motorola): MSB zuerst
	uint8_t data[8] = {0}; 
	data[6] = static_cast<uint8_t>((rawValue >> 8) & 0xFF); // MSB in Byte 6
	data[7] = static_cast<uint8_t>(rawValue & 0xFF);        // LSB in Byte 7

	// ID 0x2F0A200, Extended=false (da 0x2F0A200 ein 29-Bit-ID Raum sein kann, 
	// aber AEM SID's oft 29-Bit nutzen, setze true, falls es nicht klappt!)
	m_can.send(CANMessage(0x2F0A200, data, 8, true)); 

	// --- Starterbatterie (in V, max 14V) ---
	// Wenn du 12.5V willst, vielleicht: raw = volt * 10 (dann 125)


	qDebug() << "Speed[" << speedRaw
                 << "] SOC[" << socRaw
                 << "] CellTemp[" << cellRaw
                 << "] InvTemp[" << invRawInt
		 << "] Recu[" << m_car->getRecu()
                 << "] Dis[" << powerRaw << "]";
    }
    catch (...) {
        qDebug() << "CAN TX error";
    }
}
