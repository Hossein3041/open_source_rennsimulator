// can_manager.cpp
#include "can_manager.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>

CANManager::CANManager(const std::string& interface, int bitrate)
    : m_interface(interface), m_bitrate(bitrate)
{}

bool CANManager::open() {
    m_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_sock < 0) {
	std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return false;
    }

    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, m_interface.c_str(), IFNAMSIZ - 1);
    if (ioctl(m_sock, SIOCGIFINDEX, &ifr) < 0) {
       std::cerr << "ioctl(SIOCGIFINDEX) failed: " << strerror(errno) << std::endl;
        close();
        return false;
    }

    struct sockaddr_can addr{};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(m_sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
	    std::cerr << "bind() failed: " << strerror(errno) << std::endl;
        close();
        return false;
    }

    m_running = true;
    m_recv_thread = std::thread(&CANManager::recvLoop, this);
    return true;
}

void CANManager::close() {
    m_running = false;
    if (m_recv_thread.joinable())
        m_recv_thread.join();
    if (m_sock >= 0) {
        ::close(m_sock);
        m_sock = -1;
    }
}

CANManager::~CANManager() {
    close();
}

void CANManager::recvLoop() {
    while (m_running) {
        struct can_frame frame{};
        int nbytes = read(m_sock, &frame, sizeof(frame));
        if (nbytes == sizeof(frame)) {
            CANMessage msg(frame.can_id, frame.data, frame.can_dlc, frame.can_id & CAN_EFF_FLAG);
            std::lock_guard<std::mutex> lock(m_mutex);
            // Neue Nachrichten verarbeiten; für dein Projekt: recu‑Paket speichern
        }
    }
}


bool CANManager::send(const CANMessage& msg) {
    if (m_sock < 0) return false;

    struct can_frame frame{};
    frame.can_id = msg.is_extended ? (msg.id | CAN_EFF_FLAG) : msg.id;
    frame.can_dlc = msg.dlc;
    memcpy(frame.data, msg.data, msg.dlc);

    int nbytes = write(m_sock, &frame, sizeof(frame));
    if (nbytes == sizeof(frame)) return true;

    std::cerr << "send() failed: " << strerror(errno) << std::endl;
    return false;
}

bool CANManager::receive(CANMessage& msg, int timeout_ms) {
    (void)msg;
    (void)timeout_ms;
    return false;
}
