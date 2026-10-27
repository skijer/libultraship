#include "ship/resource/CrossRMRegistry.h"
#include "ship/Context.h"

#include <algorithm>
#include <mutex>
#include <vector>

namespace Ship {

static std::mutex sTeardownMutex;
static std::vector<void (*)()> sTeardownListeners;

void CrossRMRegistry::Register(const std::string& name, std::shared_ptr<ResourceManager> rm) {
    GameRegistry::Register(name)->Resources = std::move(rm);
}

void CrossRMRegistry::RegisterTeardownListener(void (*listener)()) {
    if (listener == nullptr) {
        return;
    }
    std::lock_guard lock(sTeardownMutex);
    if (std::find(sTeardownListeners.begin(), sTeardownListeners.end(), listener) == sTeardownListeners.end()) {
        sTeardownListeners.push_back(listener);
    }
}

// Runs at deinit on the main thread: the manager's thread pool joins its workers when it dies,
// and doing that later, at module unload, deadlocks under the loader lock.
void CrossRMRegistry::Unregister(const std::string& name) {
    std::vector<void (*)()> listeners;
    {
        std::lock_guard lock(sTeardownMutex);
        listeners = sTeardownListeners;
    }
    for (auto listener : listeners) {
        listener();
    }
    GameRegistry::Unregister(name);
}

std::shared_ptr<ResourceManager> CrossRMRegistry::Get(const std::string& name) {
    auto game = GameRegistry::Get(name);
    return game != nullptr ? game->Resources : nullptr;
}

std::shared_ptr<ResourceManager> CrossRMRegistry::GetOrActive(const std::string& name) {
    auto resources = Get(name);
    if (resources != nullptr) {
        return resources;
    }
    auto ctx = Context::GetRawInstance();
    return ctx != nullptr ? ctx->GetResourceManager() : nullptr;
}
} // namespace Ship
