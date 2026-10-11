#pragma once
#include <cstdint>

// The Scroll of Fortune tier records behind C_WildcardRewards (SMSG 0x6b8's +0x58 list).
namespace AscScrollOfFortune
{
    // FUN_10a3af80: the tier's 28-byte record {u32 tier, v[6]}, or null when the tier has none.
    // v[0..2] are the ability / talent / plain caps, v[3..5] the matching used counts.
    const uint32_t* LimitRecord(uint32_t tier);
}
