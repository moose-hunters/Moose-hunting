#pragma once
#include <stdint.h>

#pragma pack(push, 1)

// Типы пакетов, которые понимает и сервер, и клиент
enum class PacketType : uint8_t {
    JOIN_REQUEST = 0,
    INIT = 1,
    UPDATE = 2
};

// Роли игроков
enum class EntityType : uint8_t {
    MOOSE = 0,
    HUNTER = 1
};

struct PacketHeader {
    PacketType type;
};

// Пакет, который клиент отправляет при подключении, чтобы выбрать роль
struct PacketJoinRequest {
    PacketHeader header = {PacketType::JOIN_REQUEST};
    EntityType requestedRole;
};

// Пакет, который сервер отправляет клиенту для подтверждения ID и роли
struct PacketInit {
    PacketHeader header = {PacketType::INIT};
    int myId;
    EntityType myRole;
};

// Пакет с координатами
struct PacketUpdate {
    PacketHeader header = {PacketType::UPDATE};
    int playerId;
    EntityType role;
    float x, y, z;
    float yaw;
};

#pragma pack(pop)