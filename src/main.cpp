#include "Game.h"
#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    std::cout << "Starting MooseHunting..." << std::endl;
    std::cout << "=== MOOSE HUNTING ===" << std::endl;
    std::cout << "1. Create Local Server (Host)" << std::endl;
    std::cout << "2. Connect to Internet/LAN Server" << std::endl;
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;

    std::string ip = "127.0.0.1";

    if (choice == 1) {
        std::cout << "[SYSTEM] Starting GameServer.exe in background..." << std::endl;
        system("start GameServer.exe");  // Откроет сервер в новом окне
    } else {
        std::cout << "Enter Server IP (e.g. your friend's Radmin VPN IP): ";
        std::cin >> ip;
    }
    std::cout << "[SYSTEM] Launching Game..." << std::endl;

    Game game;

    if (!game.init(1024, 768, "Moose Hunting")) {
        std::cerr << "Initialization error!" << std::endl;
        return -1;
    }

    game.run();

    std::cout << "Bye!" << std::endl;
    return 0;
}