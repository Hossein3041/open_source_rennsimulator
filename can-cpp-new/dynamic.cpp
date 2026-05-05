// dynamic.cpp
#include "dynamic.h"
#include <algorithm>
#include <cmath>

Dynamic::Dynamic(const std::string& name, double mass, double f_wheel_max,
                 double p_drive_max, double p_recu_max,
                 double cw, double front_surface, bool four_wheel_drive)
    : m_Name(name)
    , m_Mass(mass)
    , m_F_Wheel_Max(f_wheel_max)
    , m_P_Drive_Max(p_drive_max)
    , m_P_Recu_Max(p_recu_max)
    , m_Cw(cw)
    , m_Front_Surface(front_surface)
    , m_Nr_Drive_Wheels(four_wheel_drive ? 4 : 2)
    , m_Power_Request(0.0)
    , m_Speed(0.0)
    , m_Rel_Speed(0.0)
    , m_F_Aero(0.0)
    , m_F_Roll(0.0)
    , m_F_Drive(0.0)
    , m_F_Brake(0.0)
    , m_Acceleration(0.0)
    , m_Time(0.0)
    , m_Distance(0.0)
    , m_PowerConsumption(0.0)
    , m_Pedal_Gamma_Speed(1.6)
    , m_Pedal_Gamma_Recu(1.2)
    , m_Pedal_Speed_Neutral(0.2)
    , m_Traction(true)
    , m_C_RR(0.01)
    , m_Wind_Speed(0.0)
    , m_Rho(1.225)
    , m_G(9.81)
    , m_Max_Brake_Decelaration(9.0)
    , m_SOC(100.0)
    , m_CellTemp(298.0)
    , m_InverterTemp(65.0)
    , m_BatteryEnergy(75e3)
    , m_StarterVolt(12.5)
    , m_Recu(0.0)
{}

void Dynamic::compute(double pedalSpeed, double pedalBrake, double recu) {
    pedalSpeed = std::clamp(pedalSpeed, 0.0, 1.0);
    pedalBrake = std::clamp(pedalBrake, 0.0, 1.0);
    recu = std::clamp(recu, 0.0, 1.0);

    if (pedalSpeed > 0.0 && pedalBrake > 0.0) {
        pedalBrake = 0.0;
    }

    m_Distance += m_Speed * getTimeDelta();

    if (m_Speed > 0.0) {
        m_PowerConsumption += m_Power_Request * getTimeDelta() / 3600.0;
    }

    double weight_pedal_speed_neutral = m_Pedal_Speed_Neutral * recu;

    if (pedalSpeed >= weight_pedal_speed_neutral) {
        double denom = (1.0 - weight_pedal_speed_neutral);
        double ratio = (denom > 1e-9) ? ((pedalSpeed - weight_pedal_speed_neutral) / denom) : 0.0;
        m_Power_Request = m_P_Drive_Max * std::pow(ratio, m_Pedal_Gamma_Speed);
    } else {
        double ratio = 0.0;
        if (weight_pedal_speed_neutral > 1e-9) {
            ratio = 1.0 - (pedalSpeed / weight_pedal_speed_neutral);
        }
        m_Power_Request = -m_P_Recu_Max * recu * std::pow(ratio, m_Pedal_Gamma_Recu);
    }

    m_Rel_Speed = std::max(m_Speed + m_Wind_Speed, 0.0);
    m_F_Aero = 0.5 * m_Rho * m_Cw * m_Front_Surface * std::pow(m_Rel_Speed, 2.0);
    m_F_Roll = (m_Rel_Speed == 0.0) ? 0.0 : m_Mass * m_G * m_C_RR;
    m_F_Brake = pedalBrake * m_Mass * m_Max_Brake_Decelaration;

    double maximal_drive_force = m_F_Wheel_Max * m_Nr_Drive_Wheels;

    if (m_Speed != 0.0 || m_Rel_Speed != 0.0) {
        m_F_Drive = std::min(maximal_drive_force,
                             m_Power_Request / std::max(m_Speed, m_Rel_Speed));
    } else {
        m_F_Drive = (m_Power_Request > 0.0) ? maximal_drive_force : 0.0;
    }
    double m_Max_Recu_Force = 5.0;
    double rekuForce = (getRecu() * m_Max_Recu_Force) / 100;
    double effectiveBrakeForce = m_F_Brake;
    
    if (getRecu() > 0.0 && pedalSpeed == 0.0) {
    	effectiveBrakeForce += rekuForce;
	m_Power_Request -= ((m_P_Recu_Max * getRecu()) / 100) + m_Power_Request;
    }

    m_Traction = m_F_Drive < maximal_drive_force;
    m_Acceleration = (m_F_Drive - m_F_Aero - m_F_Roll - effectiveBrakeForce) / m_Mass;
    m_Speed = std::max(0.0, m_Speed + m_Acceleration * getTimeDelta());
    m_Time += getTimeDelta();

    double dt_h = getTimeDelta() / 3600.0;
    m_SOC -= (m_Power_Request * dt_h) / m_BatteryEnergy * 100.0;
    m_SOC = std::clamp(m_SOC, 0.0, 100.0);

    double ambientTemp = 25.0;
    double dt = getTimeDelta();

    double P_inv_loss = std::abs(m_Power_Request) * 0.03;
    double C_th_inv = 10000.0;
    double tau_cool_inv = 1200.0;

    m_InverterTemp += (P_inv_loss / C_th_inv) * dt;
    m_InverterTemp -= ((m_InverterTemp - ambientTemp) / tau_cool_inv) * dt;

    m_CellTemp += (P_inv_loss / C_th_inv) * dt;
    m_CellTemp -= ((m_CellTemp - ambientTemp) / tau_cool_inv) * dt;
}



// Getter‑Methoden als Inline‑Funktionen
double Dynamic::getSpeed() const { return m_Speed * 3.6; }          // km/h
double Dynamic::getAcceleration() const { return m_Acceleration; }
double Dynamic::getPower() const { return m_Power_Request / 1000.0; }
double Dynamic::getDistance() const { return m_Distance / 1000.0; }
double Dynamic::getPowerConsumption(double pedal) const { 
	if(pedal == 0.0) return (m_PowerConsumption * getRecu()) / 100;
	else return m_PowerConsumption / 1000.0; 
}
double Dynamic::getAvgPowerConsumption() const { return getDistance() > 0 ? getPowerConsumption(getRecu()) / getDistance() * 100 : 0.0; }
bool   Dynamic::getTraction() const { return m_Traction; }
double Dynamic::getTime() const { return m_Time; }
double Dynamic::getSOC() const { return m_SOC; }
double Dynamic::getCellTemp() const { return m_CellTemp; }
double Dynamic::getInvTemp() const { return m_InverterTemp ; }
double Dynamic::getStarterVolt() const { return m_StarterVolt; }
double Dynamic::getRecu() const { return m_Recu; }
double Dynamic::getDisPower() const
{
    return std::max(0.0, getPower());
}



Dynamic Dynamic::getTeslaModel3Standard() {
    return Dynamic("Tesla Model-3 Standard", 1610, 5000, 220000, 80000, 0.23, 2.22, false);
}

Dynamic Dynamic::getTeslaModel3Performance() {
    return Dynamic("Tesla Model 3 Performance", 1847, 5000, 393000, 80000, 0.23, 2.22, true);
}

Dynamic Dynamic::getPorsche911G() {
    return Dynamic("Porsche 911G", 1160, 5000, 220000, 70000, 0.39, 1.79, false);
}

Dynamic Dynamic::getDefender() {
    return Dynamic("Land Rover Defender (2010)", 2000, 5000, 112000, 50000, 0.62, 3.02, false);
}

Dynamic Dynamic::getLucidAir() {
    return Dynamic("Lucid Air", 2070, 5000, 325000, 50000, 0.197, 2.29, false);
}

std::vector<Dynamic> Dynamic::getCars() {
    return {
        Dynamic::getPorsche911G(),
        Dynamic::getTeslaModel3Standard(),
        Dynamic::getTeslaModel3Performance(),
        Dynamic::getDefender(),
        Dynamic::getLucidAir()
    };
}
