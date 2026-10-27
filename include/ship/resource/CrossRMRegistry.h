#pragma once

#include <memory>
#include <string>
#include "ship/GameRegistry.h"

namespace Ship {
class ResourceManager;

/**
 * @brief Name-to-ResourceManager view of GameRegistry, used to route "__OTR__@<game>:" paths
 * to the owning game's resources (cross-game item drawing).
 */
class CrossRMRegistry {
  public:
    static void Register(const std::string& name, std::shared_ptr<ResourceManager> rm);
    static void Unregister(const std::string& name);
    /**
     * @brief Called before every Unregister so modules release shared_ptrs into a game's
     * resources while all game modules are still loaded. Must tolerate repeated calls.
     */
    static void RegisterTeardownListener(void (*listener)());
    static std::shared_ptr<ResourceManager> Get(const std::string& name);
    /** @brief Get(name), else the Context's current ResourceManager. */
    static std::shared_ptr<ResourceManager> GetOrActive(const std::string& name);
};

/** @brief RAII: runs this thread against the named game's resources. */
class OwnRMScope : public GameScope {
  public:
    explicit OwnRMScope(const char* name) : GameScope(std::string(name)) {
    }
};
} // namespace Ship
