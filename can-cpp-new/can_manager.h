// can_manager.h
#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

struct CANMessage {
    uint32_t id{};
    uint8_t  data[8]{};
    uint8_t  dlc{};
    bool     is_extended{};

    CANMessage() = default;
    CANMessage(uint32_t id_, uint8_t* data_, uint8_t dlc_, bool is_ext_)
        : id(id_), dlc(dlc_), is_extended(is_ext_) {
        for (int i = 0; i < 8; ++i)
            data[i] = data_[i < dlc_ ? i : 0];
    }
};

class CANManager {
public:
    CANManager(const std::string& interface, int bitrate = 500000);
    ~CANManager();

    bool open();
    void close();
    bool send(const CANMessage& msg);
    bool receive(CANMessage& msg, int timeout_ms = 100);

    const std::string& getInterface() const { return m_interface; }

private:
    std::string  m_interface;
    int          m_bitrate;
    int          m_sock{ -1 };
    std::thread  m_recv_thread;
    std::mutex   m_mutex;
    std::atomic_bool m_running{ false };

    void recvLoop();
};
