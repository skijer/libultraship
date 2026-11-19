#include <gtest/gtest.h>
#include <memory>

#include "ship/Context.h"
#include "ship/resource/ResourceManager.h"
#include "ship/resource/ResourceManagerScope.h"

namespace {

Ship::Context& TestContext() {
    static Ship::Context* ctx = Ship::Context::CreateUninitializedInstance("LusTest", "lustest", "");
    return *ctx;
}

std::shared_ptr<Ship::ResourceManager> MakeRm() {
    return std::make_shared<Ship::ResourceManager>();
}

} // namespace

TEST(ResourceManagerScope, SwapsAndRestores) {
    auto& ctx = TestContext();

    auto original = MakeRm();
    ctx.SetResourceManager(original);
    ASSERT_EQ(ctx.GetResourceManager(), original);

    auto other = MakeRm();
    {
        Ship::ResourceManagerScope scope(other);
        EXPECT_EQ(ctx.GetResourceManager(), other);
    }
    EXPECT_EQ(ctx.GetResourceManager(), original);
}

TEST(ResourceManagerScope, NullTargetIsNoOp) {
    auto& ctx = TestContext();

    auto original = MakeRm();
    ctx.SetResourceManager(original);

    {
        Ship::ResourceManagerScope scope(nullptr);
        EXPECT_EQ(ctx.GetResourceManager(), original);
    }
    EXPECT_EQ(ctx.GetResourceManager(), original);
}
