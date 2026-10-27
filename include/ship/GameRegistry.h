#pragma once

#include <memory>
#include <string>

namespace Ship {
class ResourceManager;
class ConsoleVariable;
class Config;

/** @brief What one game owns inside a Context that several games share. */
struct GameServices {
    std::string Name;
    std::shared_ptr<ResourceManager> Resources;
    std::shared_ptr<Config> Settings;
    std::shared_ptr<ConsoleVariable> ConsoleVariables;
    /** @brief Replaces the app directory (archives, mods, saves, settings) while this game runs. */
    std::string AppDirectory;
};

/**
 * @brief Process-wide table of the games sharing one Context.
 *
 * Context resolves its per-game services through GetCurrent(), so code keeps calling
 * Context::GetResourceManager() / GetConsoleVariables() and lands in its own game.
 * With no game registered (a standalone build) everything falls back to the Context's own.
 */
class GameRegistry {
  public:
    static std::shared_ptr<GameServices> Register(const std::string& name);
    static void Unregister(const std::string& name);
    static std::shared_ptr<GameServices> Get(const std::string& name);
    static GameServices* FindByResources(const ResourceManager* resources);

    /** @brief Sets the game that owns threads without a GameScope (the game loop thread). */
    static void SetActive(const std::string& name);
    static GameServices* GetActive();

    /** @brief The game this thread runs as: its GameScope, else the active game, else null. */
    static GameServices* GetCurrent();
};

/** @brief RAII: runs the calling thread as the given game until the scope ends. */
class GameScope {
  public:
    explicit GameScope(GameServices* game);
    explicit GameScope(const std::string& name);
    ~GameScope();
    GameScope(const GameScope&) = delete;
    GameScope& operator=(const GameScope&) = delete;
    GameScope(GameScope&&) = delete;
    GameScope& operator=(GameScope&&) = delete;

  private:
    GameServices* mPrevious;
};
} // namespace Ship
