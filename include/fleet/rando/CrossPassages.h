#pragma once
#include "CrossDungeonEntrances.h"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace ComboRando::CrossDungeon {

struct PassageDefinition {
    const char* key;
    const char* label;
    int32_t oot;
    int32_t mm;
};

inline constexpr PassageDefinition kPassages[] = {
    { "ikana", "Graveyard - Road to Ikana", 0x0E4, 0xA000 },
    { "greatBay", "Lake Hylia - Great Bay Coast", 0x102, 0x6800 },
    { "snowhead", "Zora's Fountain - Mountain Village", 0x225, 0x9A00 },
    { "woods", "Lost Woods - Woods of Mystery", 0x11E, 0xC200 },
};

struct Passage {
    Endpoint from;
    Endpoint to;
};

inline const PassageDefinition* FindPassage(const std::string& key) {
    const auto found = std::find_if(std::begin(kPassages), std::end(kPassages),
                                    [&](const PassageDefinition& passage) { return key == passage.key; });
    return found != std::end(kPassages) ? found : nullptr;
}

inline std::vector<Passage> DecodePassages(const nlohmann::json& crossSeed) {
    std::vector<Passage> passages;
    if (!crossSeed.is_object() || !crossSeed.contains("passages")) {
        return passages;
    }
    for (const auto& key : crossSeed.at("passages")) {
        const PassageDefinition* passage = key.is_string() ? FindPassage(key.get<std::string>()) : nullptr;
        if (passage == nullptr) {
            throw std::invalid_argument("unknown cross-game passage in the seed");
        }
        passages.push_back({ { 0, passage->oot }, { 1, passage->mm } });
        passages.push_back({ { 1, passage->mm }, { 0, passage->oot } });
    }
    return passages;
}

} // namespace ComboRando::CrossDungeon
