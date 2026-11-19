#pragma once

#include <memory>
#include <string>
#include "ship/GameRegistry.h"

namespace Ship {
class ResourceManager;

class CrossRMRegistry {
  public:
    static void Register(const std::string& name, std::shared_ptr<ResourceManager> rm);
    static void Unregister(const std::string& name);
    static void RegisterTeardownListener(void (*listener)());
    static std::shared_ptr<ResourceManager> Get(const std::string& name);
    static std::shared_ptr<ResourceManager> GetOrActive(const std::string& name);
};

class OwnRMScope : public GameScope {
  public:
    explicit OwnRMScope(const char* name) : GameScope(std::string(name)) {
    }
};
} // namespace Ship
