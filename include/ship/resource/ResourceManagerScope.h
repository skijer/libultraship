#pragma once

#include <memory>
#include "ship/Context.h"
#include "ship/GameRegistry.h"
#include "ship/resource/ResourceManager.h"

namespace Ship {
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
