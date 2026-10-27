#pragma once

// Contract between Fleet and a game module (soh, 2ship) sharing one process and one libultraship.
// Both tables are append-only: new fields go at the end and bump MINOR; anything else bumps MAJOR.
// A field may be read only if structSize covers it; a NULL function is a service not offered yet.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FLEET_GAME_API_MAJOR 1
#define FLEET_GAME_API_MINOR 0

// Each game module exports this symbol; it is resolved per module handle, so the name is shared.
#define FLEET_GET_GAME_API_SYMBOL "Fleet_GetGameApi"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FleetLogLevel {
    FLEET_LOG_TRACE,
    FLEET_LOG_DEBUG,
    FLEET_LOG_INFO,
    FLEET_LOG_WARN,
    FLEET_LOG_ERROR,
} FleetLogLevel;

// Items cross games as keys: "oot:ITEM_HOOKSHOT", "mm:ITEM_POWDER_KEG", or a mod key like "nei.whip".
typedef void (*FleetKeyVisitor)(void* userData, const char* key, int32_t count);

// A switch target. `payloadJson` carries game-agnostic state for the arrival (may be NULL).
typedef struct FleetDestination {
    const char* game;
    uint32_t entrance;
    const char* payloadJson;
} FleetDestination;

// What Fleet offers to the games. Strings passed in are only valid during the call.
typedef struct FleetHostApi {
    uint32_t structSize;
    uint16_t major;
    uint16_t minor;

    void (*Log)(const char* game, FleetLogLevel level, const char* message);

    // Leaves the calling game at the end of its frame and continues in `destination`.
    void (*RequestSwitch)(const char* fromGame, const FleetDestination* destination);
    // Where a scene exit leads when entrances are shuffled across games. False: keep the vanilla exit.
    bool (*ResolveExit)(const char* game, uint32_t exitEntrance, FleetDestination* destination);

    // Feeds the item mapper; Fleet answers by calling GrantKey on the paired games.
    void (*ReportObtained)(const char* game, const char* key, int32_t count);

    // The single network connection, multiplexed by channel ("oot", "mm", "fleet").
    void (*NetSend)(const char* game, const char* channel, const char* json);
} FleetHostApi;

// Everything a game needs to boot inside Fleet.
typedef struct FleetAttachInfo {
    uint32_t structSize;
    const FleetHostApi* host;
    // The game's own install folder: its .o2r archives, mods/ and settings file live here.
    const char* installDirectory;
    // Fleet owns saves: the game reads and writes its slots under this folder.
    const char* saveDirectory;
} FleetAttachInfo;

// What a game offers to Fleet. Returned strings are owned by the game and released with FreeString.
typedef struct FleetGameApi {
    uint32_t structSize;
    uint16_t major;
    uint16_t minor;
    // "oot" or "mm": the GameRegistry name and the "__OTR__@<name>:" route of this game.
    const char* name;

    // Lifecycle. Attach registers the game's GameServices, archives and mods without opening a window.
    bool (*Attach)(const FleetAttachInfo* info);
    // Runs the game loop until it requests a switch or the window closes.
    void (*Run)(const FleetDestination* arrival);
    void (*Detach)(void);

    // Saves.
    bool (*LoadSlot)(int32_t slot);
    bool (*SaveSlot)(int32_t slot);

    // Inventory as keys. GrantKey must work while the game is not running (no play state).
    void (*ListOwnedKeys)(FleetKeyVisitor visitor, void* userData);
    bool (*GrantKey)(const char* key, int32_t count);

    // Delivers a packet that arrived on this game's channel.
    void (*NetReceive)(const char* channel, const char* json);

    // Randomizer oracle; JSON in and out.
    char* (*DumpRandoStaticData)(void);
    void (*RandoReset)(void);
    void (*RandoSetOwnedItems)(const char* itemsJson);
    char* (*RandoGetReachableChecks)(void);
    bool (*RandoPlaceItem)(const char* check, const char* item);
    bool (*LoadSeed)(const char* seedJson);

    void (*FreeString)(char* text);
} FleetGameApi;

typedef const FleetGameApi* (*FleetGetGameApiFn)(uint16_t major);

#ifdef __cplusplus
}
#endif
