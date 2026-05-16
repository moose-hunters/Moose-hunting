#include "Game.h"
#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    std::cout << "=== MOOSE HUNTING ===" << std::endl;
    std::cout << "1. Create Local Server (Host)" << std::endl;
    std::cout << "2. Connect to Internet/LAN Server" << std::endl;
    int choice;
    std::cin >> choice;

    std::string targetIP = "127.0.0.1";  // По умолчанию локальный
    if (choice == 2) {
        targetIP = "26.186.206.213";  // Radmin VPN IP
        // std::cout << "Enter Server IP address (e.g., 26.186.206.213): ";
        // std::cin >> targetIP;  // Считываем то, что ты введешь руками
    }
    if (choice == 1) {
        std::cout << "[SYSTEM] Starting GameServer.exe in background..." << std::endl;
        system("start GameServer.exe");
    }
    std::cout << "[SYSTEM] Launching Game..." << std::endl;

    Game game;

    if (!game.init(1600, 900, "Moose Hunting", targetIP)) {
        std::cerr << "Initialization error!" << std::endl;
        return -1;
    }

    game.run();

    std::cout << "Bye!" << std::endl;
    return 0;
}