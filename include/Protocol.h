#pragma once
#include <stdint.h>

#pragma pack(push, 1)

// Типы пакетов, которые понимает и сервер, и клиент
enum class PacketType : uint8_t {
    JOIN_REQUEST = 0,
    INIT = 1,
    UPDATE = 2,
    HIT,           // Клиент говорит серверу: "Я попал!"
    KILL_CONFIRM,  // Сервер говорит клиенту: "Ты убил, добавь очко!"
    RESPAWN        // Сервер говорит клиенту: "Тебя убили, возродись!"
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
    PacketHeader header = { PacketType::UPDATE };
    int playerId = 0;
    EntityType role = EntityType::MOOSE;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float yaw = 0.0f;
};

// пакет с попаданиями
struct PacketHit {
    PacketHeader header = {PacketType::HIT};
    // В PvP 1 на 1 ID можно не слать, но для будущего оставим:
    int victimId;
};

#pragma pack(pop)