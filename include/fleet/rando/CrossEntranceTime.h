#pragma once
#include <cstdint>

namespace ComboRando::CrossDungeon {
inline bool MmForeignArrivalAllowed(bool canPlaySongOfTime, bool clockShuffle, uint8_t ownedClockMask,
                                    bool verifiedFrozenBossArrival = false) {
    return verifiedFrozenBossArrival || (canPlaySongOfTime && (!clockShuffle || (ownedClockMask & 0x3f) != 0));
}
} // namespace ComboRando::CrossDungeon
