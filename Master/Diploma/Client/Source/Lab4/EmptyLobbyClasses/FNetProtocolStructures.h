#pragma once

#include <cstdint>

#pragma pack(push, 1)
enum class ServerCommandType : uint8_t
{
    REGISTER_SERVER = 0,
    UPDATE_SERVER = 1,
    UNKNOWN = 2
};

enum class ServerState : uint8_t
{
    LOBBY = 0,
    MATCH_STARTING = 1,
    MATCH_IN_PROGRESS = 2,
    SHTUTTING_DOWN = 3,
    RESTARTING = 4,
    UNKNOWN = 5
};

struct FServerRegisterMessage
{
    char Uri[32];
    char Uuid[37];
    uint16_t CurrentPlayers;
    uint16_t MaxPlayers;
    uint8_t ServerState;
};

struct FServerUpdateMessage
{
    char Uuid[37];
    uint16_t CurrentPlayers;
    uint8_t ServerState;
};

struct MessageFrameHeader
{
    uint8_t CommandType;
    uint16_t PayloadSize;
};
#pragma pack(pop)