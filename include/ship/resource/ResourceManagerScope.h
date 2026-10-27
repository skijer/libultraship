#pragma once

#include <memory>
#include "ship/Context.h"
#include "ship/GameRegistry.h"
#include "ship/resource/ResourceManager.h"

namespace Ship {
/**
 * @brief RAII: makes `target` the ResourceManager seen by this scope, restoring on exit.
 *
 * A registered game's manager is selected per thread through GameScope; any other manager
 * swaps the Context's own, which is only safe on the thread that owns the Context.
 */
class ResourceManagerScope {
  public:
    explicit ResourceManagerScope(std::shared_ptr<ResourceManager> target)
        : mTargetGame(target != nullptr ? GameRegistry::FindByResources(target.get()) : nullptr),
          mGameScope(mTargetGame != nullptr ? mTargetGame : GameRegistry::GetCurrent()) {
        auto ctx = Context::GetRawInstance();
        if (ctx == nullptr || target == nullptr || mTargetGame != nullptr) {
            return;
        }
        mPrevious = ctx->GetResourceManager();
        if (mPrevious != target) {
            ctx->SetResourceManager(target);
            mCtx = ctx;
        }
    }
    ~ResourceManagerScope() {
        if (mCtx != nullptr && Context::GetRawInstance() == mCtx) {
            mCtx->SetResourceManager(mPrevious);
        }
    }
    ResourceManagerScope(const ResourceManagerScope&) = delete;
    ResourceManagerScope& operator=(const ResourceManagerScope&) = delete;
    ResourceManagerScope(ResourceManagerScope&&) = delete;
    ResourceManagerScope& operator=(ResourceManagerScope&&) = delete;

  private:
    GameServices* mTargetGame;
    GameScope mGameScope;
    std::shared_ptr<ResourceManager> mPrevious;
    Context* mCtx = nullptr;
};
} // namespace Ship
