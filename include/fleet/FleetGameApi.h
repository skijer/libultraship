#pragma once

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

typedef struct FleetDestination {
    const char* game;
    uint32_t entrance;
    const char* payloadJson;
} FleetDestination;

typedef struct FleetAttachInfo {
    FleetGetFunctionFn hostFunction;
    const char* installDirectory;
} FleetAttachInfo;

typedef void (*FleetLogFn)(const char* game, FleetLogLevel level, const char* message);
typedef void (*FleetRequestSwitchFn)(const char* fromGame, const FleetDestination* destination);
typedef void (*FleetFrameEndFn)(const char* game);

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

typedef bool (*FleetDrawForeignItemFn)(FleetItemDraw* request);
typedef void (*FleetResetActiveGameFn)(void);

typedef const char* (*FleetNameFn)(void);
typedef bool (*FleetAttachFn)(const FleetAttachInfo* info);
typedef void (*FleetRunFn)(const FleetDestination* arrival);
typedef void (*FleetArriveFn)(const FleetDestination* arrival);
typedef void (*FleetDetachFn)(void);
typedef void (*FleetFreeStringFn)(char* text);
typedef bool (*FleetDrawItemFn)(FleetItemDraw* request);
typedef void (*FleetResetFn)(void);

typedef struct FleetItemIcon {
    void* texture;
    float uv0[2];
    float uv1[2];
    float tint[4];
} FleetItemIcon;

typedef bool (*FleetItemIconFn)(const char* item, FleetItemIcon* icon);

typedef char* (*FleetDumpRandoStaticDataFn)(void);
typedef void (*FleetRandoResetFn)(void);
typedef void (*FleetRandoSetOwnedItemsFn)(const char* itemsJson);
typedef char* (*FleetRandoGetReachableChecksFn)(void);
typedef bool (*FleetRandoPlaceItemFn)(const char* check, const char* item);
typedef char* (*FleetRandoRequestFn)(const char* op, const char* json);

#ifdef __cplusplus
}
#endif
