#pragma once

#include <string>
#include <vector>

class Dynamic {
public:
    Dynamic(const std::string& name, double mass, double f_wheel_max,
            double p_drive_max, double p_recu_max,
            double cw, double front_surface, bool four_wheel_drive);

    static Dynamic getTeslaModel3Standard();
    static Dynamic getTeslaModel3Performance();
    static Dynamic getPorsche911G();
    static Dynamic getDefender();
    static Dynamic getLucidAir();
    static std::vector<Dynamic> getCars();

    void compute(double pedalSpeed, double pedalBrake, double recu);

    const std::string& getName() const { return m_Name; }
    double getMass() const { return m_Mass; }
    double getPDriveMax() const { return m_P_Drive_Max; }
    double getPRecuMax() const { return m_P_Recu_Max; }
    double getCw() const { return m_Cw; }
    double getFrontSurface() const { return m_Front_Surface; }

    double getSpeed() const;
    double getAcceleration() const;
    double getPower() const;
    double getDistance() const;
    double getPowerConsumption(double val) const;
    double getAvgPowerConsumption() const;
    bool getTraction() const;
    double getTime() const;
    double getSOC() const;
    double getCellTemp() const;
    double getInvTemp() const;
    double getStarterVolt() const;
    double getRecu() const; 
    void setRecu(double value){ m_Recu = value; }
    double getDisPower() const;
    static constexpr uint32_t SEND_CAN_ID = 0x123;  
private:
    double getTimeDelta() const { return 0.01; }

    
    std::string m_Name;
    double m_Mass;
    double m_F_Wheel_Max;
    double m_P_Drive_Max;
    double m_P_Recu_Max;
    double m_Cw;
    double m_Front_Surface;
    int m_Nr_Drive_Wheels;

    double m_Power_Request;
    double m_Speed;
    double m_Rel_Speed;
    double m_F_Aero;
    double m_F_Roll;
    double m_F_Drive;
    double m_F_Brake;
    double m_Acceleration;
    double m_Time;
    double m_Distance;
    double m_PowerConsumption;
    double m_Pedal_Gamma_Speed;
    double m_Pedal_Gamma_Recu;
    double m_Pedal_Speed_Neutral;
    bool   m_Traction;
    double m_C_RR;
    double m_Wind_Speed;
    double m_Rho;
    double m_G;
    double m_Max_Brake_Decelaration;

    double m_SOC;
    double m_CellTemp;
    double m_InverterTemp;
    double m_BatteryEnergy;
    double m_StarterVolt;
    double m_Recu;
};

