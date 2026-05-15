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
    address.host = ENET_HOST_ANY;
    address.port = 12345;

    ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);
    if (server == nullptr) {
        std::cout << "[SERVER] Failed to create server on port 12345!" << std::endl;
        return 1;
    }

    std::cout << "[SERVER] Started on port 12345. Waiting for players..." << std::endl;

    int nextPlayerId = 1;
    std::map<ENetPeer*, int> peerToId;

    bool isMooseTaken = false;
    bool isHunterTaken = false;

    ENetEvent event;
    while (true) {
        while (enet_host_service(server, &event, 10) > 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) {
                int newId = nextPlayerId++;
                peerToId[event.peer] = newId;
                std::cout << "[SERVER] Client connected! ID: " << newId << " waiting for role..." << std::endl;
            } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                PacketHeader* header = reinterpret_cast<PacketHeader*>(event.packet->data);

                if (header->type == PacketType::JOIN_REQUEST) {
                    PacketJoinRequest* req = reinterpret_cast<PacketJoinRequest*>(event.packet->data);

                    EntityType assignedRole;
                    // Проверяем, свободна ли роль
                    if (req->requestedRole == EntityType::MOOSE && !isMooseTaken) {
                        assignedRole = EntityType::MOOSE;
                        isMooseTaken = true;
                    } else if (req->requestedRole == EntityType::HUNTER && !isHunterTaken) {
                        assignedRole = EntityType::HUNTER;
                        isHunterTaken = true;
                    } else {
                        // Если занята, даем ту, что осталась
                        assignedRole = isMooseTaken ? EntityType::HUNTER : EntityType::MOOSE;
                        if (assignedRole == EntityType::MOOSE)
                            isMooseTaken = true;
                        else
                            isHunterTaken = true;
                    }

                    std::cout << "[SERVER] Assigned Role " << (int)assignedRole << " to ID " << peerToId[event.peer] << std::endl;

                    PacketInit initData;
                    initData.myId = peerToId[event.peer];
                    initData.myRole = assignedRole;
                    ENetPacket* packet = enet_packet_create(&initData, sizeof(PacketInit), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                } else if (header->type == PacketType::UPDATE) {
                    PacketUpdate* update = reinterpret_cast<PacketUpdate*>(event.packet->data);
                    ENetPacket* broadcastPacket = enet_packet_create(update, sizeof(PacketUpdate), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);

                    for (size_t i = 0; i < server->peerCount; ++i) {
                        ENetPeer* targetPeer = &server->peers[i];
                        if (targetPeer->state == ENET_PEER_STATE_CONNECTED && targetPeer != event.peer) {
                            enet_peer_send(targetPeer, 1, broadcastPacket);
                        }
                    }
                }
                enet_packet_destroy(event.packet);
            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                std::cout << "[SERVER] Client disconnected!" << std::endl;
                peerToId.erase(event.peer);
                // По-хорошему тут нужно освобождать isMooseTaken / isHunterTaken
            }
        }
    }

    enet_host_destroy(server);
    return 0;
}