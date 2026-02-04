/**
 * @file example_joy_control.cpp
 * @brief Control mode switching and locomotion control
 *
 * This example demonstrates:
 * - Switching between manual (joystick) and autonomous control modes
 * - Stand up / sit down commands
 * - Starting/stopping patrol
 *
 * Essential APIs:
 * - setManualControl(): Switch to gamepad control
 * - setAutonomousControl(): Switch to autonomous patrol mode
 * - releaseControl(): Release control
 * - standUp(): Make robot stand up (stop movement)
 * - sitDown(): Make robot sit down (standby mode)
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

void printUsage() {
    std::cout << std::endl;
    std::cout << "=== Control Commands ===" << std::endl;
    std::cout << "  m - Set Manual control (gamepad)" << std::endl;
    std::cout << "  a - Set Autonomous control (patrol mode)" << std::endl;
    std::cout << "  r - Release control" << std::endl;
    std::cout << std::endl;
    std::cout << "=== Locomotion Commands ===" << std::endl;
    std::cout << "  u - Stand Up (stop movement)" << std::endl;
    std::cout << "  d - Sit Down (standby mode)" << std::endl;
    std::cout << std::endl;
    std::cout << "=== Other ===" << std::endl;
    std::cout << "  s - Show current state" << std::endl;
    std::cout << "  q - Quit" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <robot_id>" << std::endl;
        std::cout << "Example: " << argv[0] << " 10.42.0.1" << std::endl;
        return 1;
    }

    std::signal(SIGINT, signalHandler);

    std::string robot_id = argv[1];
    raisin_sdk::RaisinClient client("control_example");

    std::cout << "Connecting to robot: " << robot_id << std::endl;
    if (!client.connect(robot_id)) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // Subscribe to robot state to monitor control mode and locomotion state
    client.subscribeRobotState([](const raisin_sdk::ExtendedRobotState& state) {
        // State is updated internally
    });

    printUsage();

    while (running) {
        std::cout << "> ";
        char cmd;
        std::cin >> cmd;

        if (std::cin.eof() || cmd == 'q' || cmd == 'Q') {
            break;
        }

        raisin_sdk::ServiceResult result;

        switch (cmd) {
            // === Control Mode Commands ===
            case 'm':
            case 'M':
                std::cout << "Setting Manual control (gamepad)..." << std::endl;
                result = client.setManualControl();
                std::cout << (result.success ? "OK" : "FAIL") << ": " << result.message << std::endl;
                break;

            case 'a':
            case 'A':
                std::cout << "Setting Autonomous control (patrol mode)..." << std::endl;
                result = client.setAutonomousControl();
                std::cout << (result.success ? "OK" : "FAIL") << ": " << result.message << std::endl;
                break;

            case 'r':
            case 'R':
                std::cout << "Releasing control..." << std::endl;
                client.releaseControl("joy/gui");
                client.releaseControl("vel_cmd/autonomy");
                std::cout << "Control released" << std::endl;
                break;

            // === Locomotion Commands ===
            case 'u':
            case 'U':
                std::cout << "Standing up..." << std::endl;
                result = client.standUp();
                std::cout << (result.success ? "OK" : "FAIL") << ": " << result.message << std::endl;
                break;

            case 'd':
            case 'D':
                std::cout << "Sitting down..." << std::endl;
                result = client.sitDown();
                std::cout << (result.success ? "OK" : "FAIL") << ": " << result.message << std::endl;
                break;

            // === Status ===
            case 's':
            case 'S': {
                auto state = client.getExtendedRobotState();
                std::cout << std::endl;
                std::cout << "=== Current State ===" << std::endl;
                std::cout << "Locomotion: " << state.getLocomotionStateName()
                          << " (" << state.locomotion_state << ")" << std::endl;
                std::cout << "Control: " << state.getJoySourceName() << std::endl;
                std::cout << "Operational: " << (state.isOperational() ? "Yes" : "No") << std::endl;
                std::cout << std::endl;
                break;
            }

            default:
                printUsage();
                break;
        }
    }

    std::cout << "Shutting down..." << std::endl;
    return 0;
}
