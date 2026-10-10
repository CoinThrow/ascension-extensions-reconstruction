#pragma once
// C_BuildCreator -- the original's BuildCreatorEntryContainer (static 0x10BDCDF0, accessor FUN_10101370,
// ctor FUN_100f7580) and its bindings, transcribed from the decompile. See AscBuildCreator.cpp.
#include <cstdint>
#include <string>

namespace AscCA { struct BuildSeed; class Build; }

namespace AscBuildCreator
{
    // FUN_100fd750 + what FUN_1014f0f0 reads from the entry: false when no known build has this id.
    bool SeedOfBuild(const std::string& id, AscCA::BuildSeed& out);

    // Slot 18's template path (the rule object off_10BCCB40's ok(), sub_10160bc0): find the entry with
    // this id (sub_100fd850: category page, then the queried list), rebuild the template build from it
    // at `level` (sub_1014eec0 with its prepare flag), and report whether it holds more than
    // `currentRank` ranks of `rowId` -- false when there is no template, or it does not.
    bool TemplateRankCheck(const std::string& id, uint32_t rowId, uint32_t level, uint32_t currentRank);

    // The link codec, for C_CharacterAdvancement.ImportPendingBuild: FUN_1010f5e0 (URL decode) and
    // FUN_10109ba0 (base64 + raw inflate); and FUN_101001e0 into a scratch entry, as a CA seed.
    std::string UrlDecode(const std::string& in);
    std::string DecodeLink(const std::string& url);
    AscCA::BuildSeed SeedFromLink(const std::string& url);

    // FUN_100fd6c0(spec) +0x10: the spec's per-spec record (container +0x1D0) names an active build.
    bool SpecHasActiveBuild(int spec);

    // FUN_100fc240 + FUN_100fd8f0: the link of an entry made from a CA build (C_CharacterAdvancement.ExportBuild).
    std::string ExportBuildOf(const AscCA::Build& b, bool enchants);

    // What the C_BuildEditor entry filters ask of the pending entry (+0x10), one spell at a time:
    // FUN_100fac40 with {spell, FUN_102fc4d0()} and FUN_100fb9b0 without errors, and FUN_10100180's
    // lookup in its spell list (+0xFC).
    bool PendingCanAddSpell(uint32_t spell);
    bool PendingCanRemoveSpell(uint32_t spell);
    bool PendingHasSpell(uint32_t spell);
}
