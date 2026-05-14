#include <enet/enet.h>
#include <iostream>
#include <map>
#include "Protocol.h"

int main() {
    if (enet_initialize() != 0) {
        std::cout << "[SERVER] Failed to initialize ENet!" << std::endl;
        return 1;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;  // Слушаем любой IP (включая интернет)
    address.port = 12345;

    ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);
    if (server == nullptr) {
        std::cout << "[SERVER] Failed to create server on port 12345!" << std::endl;
        return 1;
    }

    std::cout << "[SERVER] Started on port 12345. Waiting for players..." << std::endl;

    int nextPlayerId = 1;
    std::map<ENetPeer*, int> peerToId;

    ENetEvent event;
    while (true) {
        while (enet_host_service(server, &event, 10) > 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) {
                int newId = nextPlayerId++;
                peerToId[event.peer] = newId;

                // Первый - Лось, второй - Охотник
                EntityType role = (newId % 2 != 0) ? EntityType::MOOSE : EntityType::HUNTER;

                std::cout << "[SERVER] Client connected! ID: " << newId << " Role: " << (int)role << std::endl;

                // Отправляем игроку его роль
                PacketInit initData;
                initData.myId = newId;
                initData.myRole = role;

                ENetPacket* packet = enet_packet_create(&initData, sizeof(PacketInit), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(event.peer, 0, packet);
            } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                // Сервер получил позицию от игрока. Надо разослать остальным.
                PacketUpdate* update = (PacketUpdate*)event.packet->data;

                // Рассылаем всем КРОМЕ отправителя
                ENetPacket* broadcastPacket = enet_packet_create(update, sizeof(PacketUpdate), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
                for (size_t i = 0; i < server->peerCount; ++i) {
                    ENetPeer* targetPeer = &server->peers[i];
                    if (targetPeer->state == ENET_PEER_STATE_CONNECTED && targetPeer != event.peer) {
                        enet_peer_send(targetPeer, 1, broadcastPacket);
                    }
                }
                enet_packet_destroy(event.packet);
            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                std::cout << "[SERVER] Client disconnected! ID: " << peerToId[event.peer] << std::endl;
                peerToId.erase(event.peer);
            }
        }
    }

    enet_host_destroy(server);
    return 0;
}