/**
 * @file example_connect.cpp
 * @brief Discover network nodes, connect, and list available messages
 *
 * This example demonstrates:
 * - Discovering available nodes on the network via getAllConnections()
 * - Connecting to a selected node
 * - Listing publishers and services on the connected node
 *
 * Essential APIs:
 * - network->getAllConnections(): Discover available nodes
 * - network->connect(id): Connect to a node
 * - connection->publishers / connection->services: Inspect available messages
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include "raisin_network/network.hpp"

using namespace raisin;

std::vector<std::string> splitInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void printConnections(const std::shared_ptr<Network>& network) {
    auto connections = network->getAllConnections();

    std::cout << std::left
              << std::setw(10) << "Type"
              << std::setw(16) << "IP"
              << std::setw(30) << "ID"
              << std::endl;
    std::cout << std::string(56, '-') << std::endl;

    bool found = false;
    for (const auto& conn : connections) {
        if (conn.port < 0) continue;
        found = true;
        std::cout << std::left
                  << std::setw(10) << "Server"
                  << std::setw(16) << conn.ip
                  << std::setw(30) << conn.id
                  << std::endl;
    }

    if (!found) {
        std::cout << "  No connections discovered yet." << std::endl;
    }
    std::cout << std::endl;
}

void printConnectionDetails(const std::shared_ptr<Remote::Connection>& connection) {
    std::lock_guard<std::mutex> lock(connection->mutex);

    std::cout << "\n=== Connection: " << connection->id << " ===" << std::endl;
    std::cout << "  IP:   " << connection->ip << std::endl;
    std::cout << "  Type: " << (connection->networkType == Remote::NetworkType::TCP ? "TCP" : "WebSocket") << std::endl;

    std::cout << "\n--- Services ---" << std::endl;
    std::cout << std::left << std::setw(30) << "Name" << "Type" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    if (connection->services.empty()) {
        std::cout << "  (none)" << std::endl;
    } else {
        for (const auto& [name, desc] : connection->services) {
            std::cout << std::left << std::setw(30) << name << desc.dataType << std::endl;
        }
    }

    std::cout << "\n--- Publishers ---" << std::endl;
    std::cout << std::left << std::setw(30) << "Name" << "Type" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    if (connection->publishers.empty()) {
        std::cout << "  (none)" << std::endl;
    } else {
        for (const auto& [name, desc] : connection->publishers) {
            std::cout << std::left << std::setw(30) << name << desc.dataType << std::endl;
        }
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    std::vector<std::string> netInterfaces;
    if (argc == 2) {
        netInterfaces.push_back(argv[1]);
    } else {
        netInterfaces.push_back("lo");
    }

    std::vector<std::vector<std::string>> threads = {{"main"}};
    auto network = std::make_shared<Network>(
        "connect_example", "example", threads, netInterfaces);
    std::shared_ptr<Remote::Connection> connection;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Node discovery active. Use 'list' to see available connections.\n" << std::endl;

    while (true) {
        if (connection && connection->connected) {
            std::cout << "Commands: list | dis | quit" << std::endl;
        } else {
            std::cout << "Commands: list | con <server_id> | quit" << std::endl;
        }

        std::cout << ">> ";
        std::string input;
        if (!std::getline(std::cin, input)) break;
        if (input.empty()) continue;

        auto tokens = splitInput(input);
        const std::string& cmd = tokens[0];

        if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "list") {
            if (!connection) {
                printConnections(network);
            } else {
                printConnectionDetails(connection);
            }
        } else if (cmd == "con") {
            if (connection) {
                std::cout << "Already connected. Disconnect first ('dis')." << std::endl;
                continue;
            }
            if (tokens.size() < 2) {
                std::cout << "Usage: con <server_id>" << std::endl;
                continue;
            }
            connection = network->connect(tokens[1]);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (connection && connection->connected) {
                std::cout << "Connected to: " << tokens[1] << std::endl;
            } else {
                std::cout << "Connection failed." << std::endl;
                connection = nullptr;
            }
        } else if (cmd == "dis") {
            if (!connection) {
                std::cout << "Not connected." << std::endl;
                continue;
            }
            connection->disconnect();
            connection = nullptr;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << "Disconnected." << std::endl;
        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }

    std::cout << "Program terminated." << std::endl;
    return 0;
}
