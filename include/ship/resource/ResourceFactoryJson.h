#pragma once

#include "ResourceFactory.h"

namespace Ship {
class ResourceFactoryJson : public ResourceFactory {
  protected:
    bool FileHasValidFormatAndReader(std::shared_ptr<File> file, std::shared_ptr<ResourceInitData> initData) override;
};
} // namespace Ship
