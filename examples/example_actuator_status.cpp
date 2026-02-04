/**
 * @file example_actuator_status.cpp
 * @brief Monitor actuator status via subscribeRobotState()
 *
 * Essential: state.actuators[], state.hasActuatorError()
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

void printActuatorTable(const raisin_sdk::ExtendedRobotState& state) {
    // Clear screen and move cursor to top
    std::cout << "\033[2J\033[H";

    std::cout << "=== Actuator Status ===" << std::endl;
    std::cout << std::left
              << std::setw(12) << "Name"
              << std::setw(12) << "Status"
              << std::setw(10) << "Temp(C)"
              << std::setw(12) << "Pos(rad)"
              << std::setw(12) << "Vel(rad/s)"
              << std::setw(12) << "Effort(Nm)"
              << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto& act : state.actuators) {
        // Use proper status interpretation:
        // 39 = OPERATION_ENABLED (normal running)
        // 33 = READY_TO_SWITCH_ON (normal standby)
        // 35 = SWITCHED_ON (normal)
        // 0, 8, 99 = error states
        std::string statusStr = raisin_sdk::getActuatorStatusName(act.status);
        bool isError = raisin_sdk::isActuatorStatusError(act.status);

        std::cout << std::left
                  << std::setw(12) << act.name
                  << std::setw(12) << (isError ? "\033[31m" + statusStr + "\033[0m" : statusStr)
                  << std::fixed << std::setprecision(1)
                  << std::setw(10) << act.temperature
                  << std::setprecision(3)
                  << std::setw(12) << act.position
                  << std::setw(12) << act.velocity
                  << std::setw(12) << act.effort
                  << std::endl;
    }

    std::cout << std::string(70, '-') << std::endl;

    if (state.hasActuatorError()) {
        auto errors = state.getActuatorsWithErrors();
        std::cout << "\033[31mWARNING: Actuator errors detected:\033[0m" << std::endl;
        for (const auto& err : errors) {
            std::cout << "  - " << err << std::endl;
        }
    } else if (state.allActuatorsOperational()) {
        std::cout << "\033[32mAll actuators OPERATIONAL (running)\033[0m" << std::endl;
    } else {
        std::cout << "All actuators OK (standby/ready)" << std::endl;
    }

    std::cout << std::endl << "(Ctrl+C to stop)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <robot_id>" << std::endl;
        std::cout << "Example: " << argv[0] << " 10.42.0.1" << std::endl;
        return 1;
    }

    std::signal(SIGINT, signalHandler);

    std::string robot_id = argv[1];
    raisin_sdk::RaisinClient client("actuator_example");

    std::cout << "Connecting to robot: " << robot_id << std::endl;
    if (!client.connect(robot_id)) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // ===== ESSENTIAL =====
    std::atomic<int> updateCounter{0};
    client.subscribeRobotState([&](const raisin_sdk::ExtendedRobotState& state) {
        if (++updateCounter % 10 == 0) {
            printActuatorTable(state);
        }
    });
    // ==================

    std::cout << "Monitoring actuator status..." << std::endl;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;
    return 0;
}
