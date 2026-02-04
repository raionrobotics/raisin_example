/**
 * @file example_robot_state.cpp
 * @brief Monitor robot locomotion state via subscribeRobotState()
 *
 * Essential: client.subscribeRobotState(callback)
 */

#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>
#include "raisin_sdk/raisin_client.hpp"

std::atomic<bool> running{true};

void signalHandler(int) {
    running = false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <robot_id>" << std::endl;
        std::cout << "Example: " << argv[0] << " 10.42.0.1" << std::endl;
        return 1;
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::string robot_id = argv[1];
    raisin_sdk::RaisinClient client("robot_state_example");

    std::cout << "Connecting to robot: " << robot_id << std::endl;
    if (!client.connect(robot_id, 10, &running)) {
        if (!running) {
            std::cout << "Connection cancelled" << std::endl;
            return 0;
        }
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // ===== ESSENTIAL =====
    client.subscribeRobotState([](const raisin_sdk::ExtendedRobotState& state) {
        std::cout << "\rState: " << state.getLocomotionStateName()
                  << " (" << state.locomotion_state << ")"
                  << " | Operational: " << (state.isOperational() ? "Yes" : "No")
                  << " | Control: " << state.getJoySourceName()
                  << "          " << std::flush;
    });
    // ==================

    std::cout << "Monitoring robot state... (Ctrl+C to stop)" << std::endl;
    std::cout << std::endl;  // New line for status output

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;
    return 0;
}
