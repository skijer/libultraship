#pragma once

#include <memory>
#include <string>

namespace Ship {
class ResourceManager;
class ConsoleVariable;
class Config;

struct GameServices {
    std::string Name;
    std::shared_ptr<ResourceManager> Resources;
    std::shared_ptr<Config> Settings;
    std::shared_ptr<ConsoleVariable> ConsoleVariables;
    std::string AppDirectory;
};

class GameRegistry {
  public:
    static std::shared_ptr<GameServices> Register(const std::string& name);
    static void Unregister(const std::string& name);
    static std::shared_ptr<GameServices> Get(const std::string& name);
    static GameServices* FindByResources(const ResourceManager* resources);

    static void SetActive(const std::string& name);
    static GameServices* GetActive();

    static GameServices* GetCurrent();
};

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
