/**
 * @file example_battery.cpp
 * @brief Monitor battery status via subscribeRobotState()
 *
 * Essential: state.voltage, current, min_voltage, max_voltage, body_temperature
 */

#include <iostream>
#include <iomanip>
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

    std::string robot_id = argv[1];
    raisin_sdk::RaisinClient client("battery_example");

    std::cout << "Connecting to robot: " << robot_id << std::endl;
    if (!client.connect(robot_id)) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // ===== ESSENTIAL =====
    client.subscribeRobotState([](const raisin_sdk::ExtendedRobotState& state) {
        double percentage = (state.voltage - state.min_voltage) /
                           (state.max_voltage - state.min_voltage) * 100.0;

        std::cout << "\r" << std::fixed << std::setprecision(1)
                  << "Voltage: " << state.voltage << "V (" << percentage << "%)"
                  << " | Current: " << state.current << "A"
                  << " | Temp: " << state.body_temperature << "C          " << std::flush;
    });
    // ==================

    std::cout << "Monitoring battery status... (Ctrl+C to stop)" << std::endl;
    std::cout << std::endl;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;
    return 0;
}
