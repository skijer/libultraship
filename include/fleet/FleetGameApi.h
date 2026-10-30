#pragma once

// Contract between Fleet and a game module (soh, 2ship) sharing one process and one libultraship.
// Every service is a function looked up by name: Fleet asks a game through its exported Fleet_GetFunction, a game
// asks Fleet through the lookup it receives in Attach. NULL means the service is not offered, so either side just
// does without it; there is no contract version.

#include <stdbool.h>
#include <stdint.h>

#define FLEET_GET_FUNCTION_SYMBOL "Fleet_GetFunction"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*FleetGetFunctionFn)(const char* name);

typedef enum FleetLogLevel {
    FLEET_LOG_TRACE,
    FLEET_LOG_DEBUG,
    FLEET_LOG_INFO,
    FLEET_LOG_WARN,
    FLEET_LOG_ERROR,
} FleetLogLevel;

// A switch target. `payloadJson` carries game-agnostic state for the arrival (may be NULL): "portal" names a
// doorway both games know by name, so neither needs the other's entrance numbers, and "slot" the save slot.
typedef struct FleetDestination {
    const char* game;
    uint32_t entrance;
    const char* payloadJson;
} FleetDestination;

typedef struct FleetAttachInfo {
    FleetGetFunctionFn hostFunction;
    // The game's own install folder: its .o2r archives, mods/ and settings file live here.
    const char* installDirectory;
} FleetAttachInfo;

// Fleet's services. Strings passed in are only valid during the call.

// "Log"
typedef void (*FleetLogFn)(const char* game, FleetLogLevel level, const char* message);
// "RequestSwitch": leaves the calling game at its next FrameEnd and continues in `destination`.
typedef void (*FleetRequestSwitchFn)(const char* fromGame, const FleetDestination* destination);
// "FrameEnd": the running game calls this between frames. Fleet may park the game here and run the other one; the
// call returns once this game is active again.
typedef void (*FleetFrameEndFn)(const char* game);

// A game's services. Returned strings are owned by the game and released with "FreeString".

// "Name": "oot" or "mm", the GameRegistry name and the "__OTR__@<name>:" route of this game.
typedef const char* (*FleetNameFn)(void);
// "Attach": registers the game with Fleet without opening anything; false if its install cannot boot.
typedef bool (*FleetAttachFn)(const FleetAttachInfo* info);
// "Run": runs the game loop until the window closes; switches park it inside FrameEnd.
typedef void (*FleetRunFn)(const FleetDestination* arrival);
// "Arrive": Fleet calls this just before resuming a parked game; the game enters `arrival` on its next frames.
typedef void (*FleetArriveFn)(const FleetDestination* arrival);
// "Detach"
typedef void (*FleetDetachFn)(void);
// "FreeString"
typedef void (*FleetFreeStringFn)(char* text);

// Randomizer oracle, JSON in and out: "DumpRandoStaticData", "RandoReset", "RandoSetOwnedItems",
// "RandoGetReachableChecks", "RandoPlaceItem", and "RandoRequest" for every other service, named by `op`.
typedef char* (*FleetDumpRandoStaticDataFn)(void);
typedef void (*FleetRandoResetFn)(void);
typedef void (*FleetRandoSetOwnedItemsFn)(const char* itemsJson);
typedef char* (*FleetRandoGetReachableChecksFn)(void);
typedef bool (*FleetRandoPlaceItemFn)(const char* check, const char* item);
typedef char* (*FleetRandoRequestFn)(const char* op, const char* json);

#ifdef __cplusplus
}
#endif
