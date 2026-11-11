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

// One of a game's own randomizer items, drawn by that game into the running game's frame. The heads and tails are
// the running game's polyOpa/polyXlu arena pointers; the drawing game advances them. Both games share the MtxF layout.
typedef struct FleetItemDraw {
    const char* owner;
    const char* item;
    void* opaHead;
    void* opaTail;
    void* xluHead;
    void* xluTail;
    float model[16];
    float billboard[16];
    uint32_t frames;
} FleetItemDraw;

// "DrawForeignItem": forwards to the owner's "DrawItem"; false when the owner cannot draw `item` now.
typedef bool (*FleetDrawForeignItemFn)(FleetItemDraw* request);
// "ResetActiveGame": soft-resets the game being played, whichever game's reset command or button asked.
typedef void (*FleetResetActiveGameFn)(void);

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
// "DrawItem": draws one of this game's items for another game, with this game's own code and archives.
typedef bool (*FleetDrawItemFn)(FleetItemDraw* request);
// "Reset": the game's own soft reset, back to its boot and title screens.
typedef void (*FleetResetFn)(void);

// An ImGui texture of the game's GUI, valid for the current frame, drawn over uv0..uv1 and tinted.
typedef struct FleetItemIcon {
    void* texture;
    float uv0[2];
    float uv1[2];
    float tint[4];
} FleetItemIcon;

// "ItemIcon": the icon of one of this game's randomizer items, by its dump name; false when it has none.
typedef bool (*FleetItemIconFn)(const char* item, FleetItemIcon* icon);

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
