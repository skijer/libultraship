#include "ship/GameRegistry.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace Ship {

// Entries live until Unregister at deinit, so raw pointers handed out stay valid while games run.
static std::shared_mutex sGamesMutex;
static std::unordered_map<std::string, std::shared_ptr<GameServices>> sGames;
static std::atomic<GameServices*> sActiveGame = nullptr;
static thread_local GameServices* tScopedGame = nullptr;

std::shared_ptr<GameServices> GameRegistry::Register(const std::string& name) {
    std::unique_lock lock(sGamesMutex);
    auto& game = sGames[name];
    if (game == nullptr) {
        game = std::make_shared<GameServices>();
        game->Name = name;
    }
    return game;
}

void GameRegistry::Unregister(const std::string& name) {
    std::shared_ptr<GameServices> removed;
    {
        std::unique_lock lock(sGamesMutex);
        auto it = sGames.find(name);
        if (it == sGames.end()) {
            return;
        }
        removed = std::move(it->second);
        sGames.erase(it);
    }
    GameServices* expected = removed.get();
    sActiveGame.compare_exchange_strong(expected, nullptr);
    // `removed` is released outside the lock: ~ResourceManager joins workers that may call Get().
}

std::shared_ptr<GameServices> GameRegistry::Get(const std::string& name) {
    std::shared_lock lock(sGamesMutex);
    auto it = sGames.find(name);
    return it != sGames.end() ? it->second : nullptr;
}

GameServices* GameRegistry::FindByResources(const ResourceManager* resources) {
    std::shared_lock lock(sGamesMutex);
    for (const auto& [name, game] : sGames) {
        if (game->Resources.get() == resources) {
            return game.get();
        }
    }
    return nullptr;
}

void GameRegistry::SetActive(const std::string& name) {
    sActiveGame = Get(name).get();
}

GameServices* GameRegistry::GetActive() {
    return sActiveGame;
}

GameServices* GameRegistry::GetCurrent() {
    if (tScopedGame != nullptr) {
        return tScopedGame;
    }
    return sActiveGame;
}

GameScope::GameScope(GameServices* game) : mPrevious(tScopedGame) {
    tScopedGame = game;
}

GameScope::GameScope(const std::string& name) : GameScope(GameRegistry::Get(name).get()) {
}

GameScope::~GameScope() {
    tScopedGame = mPrevious;
}
} // namespace Ship
