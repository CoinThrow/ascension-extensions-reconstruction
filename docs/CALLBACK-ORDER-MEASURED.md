# Measured callback-list order — aura application, effect filtering, visual hiding

The three lists that used to insert by call-site order were measured from the original's live list
images. Call-site order cannot reproduce them: registration execution order does not follow the call
sites — and one original-only callback on the aura-application list has no call site at all.

## Captured order (call order of the original's detours and consumers)

| list (native object) | order |
| --- | --- |
| `0x10BE2B6C` aura application (before `0x724820`) | `AscAura137::BeforeApply`, `AscSpellVisuals::BeforeAuraApply`, `AscSpellMods::AuraFlagOn`, `AscAppearance::BeforeAuraApply`, `AscSpawnVisibility::BeforeAuraApply`, *+ the original-only callback (reconstructed below)* |
| `0x10BE2D78` effect filtering (`0x744AC0` replacement) | `AscSpellVisuals::RemoveEffect`, `AscQuestSpellHide::RemoveEffect` |
| `0x10BE2D98` visual hiding | `AscSpellVisuals::HideVisual`, `AscQuestSpellHide::HideVisual` |

Sites (reference image) are in `tests/fixtures/original_callback_order.json`; the registrars rank by them.
The two filter lists are exactly reversed vs ascending site order. The aura list is not any site sort:
`AscSpellMods` registered before `AscSpellVisuals`, and `AscSpawnVisibility` before `AscAppearance`,
although their sites are the other way round.

## The original-only sixth callback (aura application)

- Runs **last of six** (`AscSpawnVisibility`'s entry precedes it). The list count for `0x10BE2B6C` is 6.
- Registered through the original's runtime layer: no static registration site exists anywhere in the
  image; its address is only materialized at runtime (a VM lane cell, `.vm_sec` + `0xD70088`).
  **Reconstructed 2026-10-10** as `AscAccount::MentorAuraGate` (`src/Ascension/AscAccount.cpp`,
  original function `0x10A4D550`), registered with a non-captured site so `InsertByCapturedOrder`'s
  *unknown callbacks follow the captured entries* rule places it exactly where the original calls it.
- Behaviour — a spell veto called as `(unit, a, rec)`; `false` suppresses the aura application
  (the detour returns without calling the original `0x724820`):

```c
bool AuraVeto(void* unit, uint32_t, uint32_t rec)
{
    if (*(uint32_t*)((char*)unit + 0x14) != 4) return true;            // interposes only on players
    uint32_t spell = *(uint32_t*)rec;
    uint32_t flags = *(uint32_t*)(*(uint32_t*)((char*)unit + 8) + 0x258);   // PLAYER_FLAGS
    if (spell == 0x79BB0 || spell == 0x79C3F)
        return unit != ActivePlayer() && (flags & 0x8000) == 0
            && ((flags & 8) != 0 || AccountInfo()[0x0C] != 0) && AccountInfo()[0x0E] == 0;
    if (spell == 0x79BC5)
        return (flags & 0x8000) == 0;
    if (spell == 0x79C40 || spell == 0x978960)
        return unit != ActivePlayer() && (flags & 0x8000) == 0
            && ((flags & 8) != 0 || AccountInfo()[0x0D] != 0) && AccountInfo()[0x0E] == 0;
    return true;
}
```

(Flag `8` = GM, `0x8000` = developer; `AccountInfo()` is the TLS account object accessor —
our `FUN_1008c870` class. All five spell ids are Ascension custom spells, absent from the shipped
`Spell.dbc`.)

### Spell identification — the client's Mentor / Newcomer / Returning-Player / overhead-icon handling

The five ids are CoA's community and staff marker spells (community-archive identification, not server
data — AscensionDB preservation archive, "Jeff-Fro CoA in-game spell tooltip capture", 2026-09-02,
snapshot `2aa98857`):

| id | name | captured tooltip |
| --- | --- | --- |
| `0x79BB0` (498608) | **Mentor** | `Mentor` |
| `0x79C3F` (498751) | **Mentor** | `Mentor \| I am a Mentor and I am here to help! New players can ask me questions about the game!` |
| `0x79BC5` (498629) | **Overhead Icon - Ascension Logo** | `Overhead Icon - Ascension Logo \| Instant` |
| `0x79C40` (498752) | **New Player Help - Newcomer** | `New Player Help - Newcomer \| Instant` |
| `0x978960` (9931104) | **Returning Player** | — |

The condition split mirrors the names: the two Mentor spells share the account byte `0x0C` branch, the
Newcomer / Returning-Player pair shares the `0x0D` branch, and the Ascension-logo overhead icon has its
own rule. Note the GM flag never suppresses here — it is only ever an *allow* path; the one suppressing
flag is `0x8000` (developer), and the tag pairs additionally never apply to your own active player.

## Measurement and validation

- Live walk of the original's list links in two independent game sessions of a sibling build
  (`md5 8b3dd390…`), identical results; direction follows the hooks: `node = *(head)` (the list
  head pointer), callback at `node+8`, next at `node+0`, stop when back at `head`.
- The same walk reproduces the two emulated fixtures (`Before71E930`, `After80B5D0`) byte-exactly in this
  reference image; addresses were translated with per-band deltas validated on all entries.
- `tests/native_callback_order.py` locks the orders in: the registrars run under all registration
  permutations, with vetoes, duplicate registrations, an off-array ("unknown") site, and — for visual
  hiding — the production consumer's first-true-wins short-circuit.

## The remaining lifecycle lists (measured 2026-10-11)

The eleven lifecycle lists with more than one registrant were measured the same way, after decoding
what the lists *are*:

- Every `On*` list is one **MSVC `std::unordered_set` of function pointers** (`sub_100997C0` of the
  original is the set's insert; nodes are `{next, prev, value}` with the callback at `+8`; iteration
  follows the set's linked list). Insertion **dedupes by pointer** — a repeated registration is a
  no-op — and new nodes are inserted at the front of their bucket run, so the list order is the set's
  **hash-bucket order of the runtime pointer values** (FNV-1a over the four relocated bytes, bucket
  count by element count). It equals registration order only while buckets are collision-free, and it
  **changes with the DLL load base**, because the hashed values are relocated addresses.
- Observed bases: `0x67E30000` (donor_charselect2, donor_v3 — the reference sessions the five lists
  above were pinned from), `0x66950000` (donor_full1), `0x63E40000` (donor_full2). The earlier note
  that hash-bucket relationships "remain identical at every allocation-aligned image base" did not
  hold — e.g. the six entries of `0x10BE5B68` walk in a different order at `0x66950000`. The pinned
  ranks below are the reference-base `0x67E30000` walk; they reproduce the original as observed in
  the campaign's session dumps.
- The walks needed a full-private memory dump (`re\donor_dump4.py`): the earlier curated dumps missed
  the secondary heap arena `0x26xxxxxx`, silently truncating GlueScreen, LeaveWorld, EnterWorld,
  After403340, After6E7F50 and the lua-setup lists. The pinned orders use complete, closed rings.
- Stock events (`0x81AA00` per-event dispatch, AscTutorial 0x258/0x1C0) and the direct-dispatch pairs
  GameObjectUse/ItemUse have a single registrant each — order is trivial. OnStockEvent's callbacks are
  per-event-id, so no cross-callback order exists there either.

| list | pinned order (measured walk at 0x67E30000) | notes |
| --- | --- | --- |
| EnterWorld | SpellCharges, WorldEntryTweaks::Send561, Graphics::CullingOn, Profiler::Arm, ExtraActionButton, Graphics::BuildOccluders, TemporalContracts, BugTracker, MysticEnchant, WildcardRolls, WildcardRapid, CAMgr::BuildIndex, SpellShadow, UnitSelect::LoadCones, SuperTrack, SpellActivationOverlay, SmallHooks (+1 original-only last) | 18 entries; ExtraActionButton / SpellShadow / SmallHooks pinned via their init pairings (see the fixture notes) |
| AfterEnterWorld | WET::Send561, WET::StanceForms, WET::StanceJump, WET::SkipCheck809572, Challenge, PlayerPoll, WET::LoadAntiCheat, WET::StartKillSwitch, Tutorial::SeedWatches | TooltipInsertFix::Apply is not in the original's set (reconstruction superset) |
| After403340 | Challenge, Profiler, SpellCharges, MysticEnchant, EventPackets, RealmData, Tutorial, NamePlates (+ 8 original-only) | pinned from donor_full1 (0x66950000): the reference-base walk was cut off by an unmapped heap block |
| Alternate6DF050 | ObjectAddon::KeepActivePlayer, UnitSelect::Tick, Graphics::MapCap, RaidVideo::Tick (+1 runtime-registered) | |
| LeaveWorld | NamePlates::ClearPlates, MovementForce::Reset, Profiler::Disarm (+1 aura-type-map clear) | |
| Before528F00 | CacheFiles::Prune, MovementForce::Reset, Profiler::Disarm (+1 aura-type-map clear) | |
| After402910 | CacheFiles::Save, MovementForce::Reset (+1 aura-type-map clear) | |
| After495810 | CharacterCreate::WatchComponent, RealmData::Tick (+1 runtime-registered) | |
| After4F6F90 | Graphics::DrawOcclusion, UnitSelect::Render (+the shared empty stub) | |
| ObjectFree | AuraAmounts::ForgetObject, ObjectAddon::OnObjectFree (+1 runtime-registered first) | |
| AfterTargetChange | Camera::AfterTargetChange, AddonTaint::AfterTargetChange | |

## The GlueScreen list (measured 2026-10-11)

`GlueScreen` (`0x10BE5868`, registrar `sub_1027A360`, 46 call sites — callback = the `push imm32`
immediately before the `call`) was pinned from the donor_full1 walk at base `0x66950000`, and
cross-checked against donor_full2 (`0x63E40000`). The raw ring slots 0..45 run:

| slot | callback | recon registration | rank |
| --- | --- | --- | --- |
| 0 | `0x1016EA30` (CA store `&10BE1440` + filter reset) | `AscCAMgr::OnGlueScreen` / `AscCAFilter::ResetOnGlue` | 0 / 1 |
| 1 | `0x100E72C0` | `AscBugTracker::Clear` | 2 |
| 2 | `0x1024B460` | `AscGMTicket::ResetTickets` | 3 |
| 3 | `0x10135020` | `AscChallenge::OnGlueScreen` | 4 |
| 4 | `0x102592F0` | `AscJson::ResetBuildPayloads` | 5 |
| 5 | `0x102552F0` | `AscGMTicket::ResetCurrent` | 6 |
| 6 | `0x102DA8E0` | `AscPlayerPoll::Reset` | 7 |
| 7 | `0x10336130` | `AscCAMgr::SuggestionsOnGlue` | 8 |
| 8 | `0x102F6DD0` | `AscSkillCardSlots::OnGlueScreen` | 9 |
| 9 | `0x10110350` | `AscBuildDraft::Reset` | 10 |
| 10 | `0x10327000` | `AscSpellMods::ClearModifiers` | 11 |
| 11 | `0x10296350` | `AscLootLockout::Reset` | 12 |
| 12 | `0x1026ECD0` | `AscGroupFinder::Reset` | 13 |
| 13 | `0x10119EF0` | `AscTrialCreator::OnGlueScreen` | 14 |
| 14 | `0x100FC170` (creator/editor-zone reset) | merged into `AscCAFilter::ResetOnGlue` | 1 |
| 15 | `0x102E5EE0` | `AscMysticEnchant::OnGlueScreen` | 15 |
| 16 | `0x102F4E40` | `AscMysticEnchantPreset::Reset` | 16 |
| 17 | `0x10A32510` (resets + cancels `WildcardReveal`) | `AscWildcardRolls::OnGlueScreen` / `AscWildcardRapid::OnGlueScreen` | 17 / 18 |
| 18 | `0x10339CF0` | `AscTemporalContracts::OnGlue` | 19 |
| 19 | `0x10226950` | `AscDraftMode::ResetState` | 20 |
| 20 | `0x10A36500` | `AscWildcardSets::OnGlueScreen` | 21 |
| 21 | `0x10238530` | `AscExtraActionButton::Stop` | 22 |
| 22 | `0x10A3AC30` | `AscScrollOfFortune::Reset` | 23 |
| 23 | `0x1031AA00` | `AscSkillCardCollection::OnGlueScreen` | 24 |
| 24 | `0x101A1980` | `AscCAMgr::ClearCredits` | 25 |
| 25 | `0x1018AF20` | `AscCharacterCreate::ResetPreview` | 26 |
| 26 | `0x102A29B0` | `AscManastorm::OnGlueScreen` | 27 |
| 27 | `0x100E1590` | `AscAssetQuery::Reset` | 28 |
| 28 | `0x10194F90` | `AscMisc::ApplyCommandLine` | 29 |
| 29 | `0x1033EE30` | `AscToken::ResetAmounts` | 30 |
| 30 | `0x10A20D00` | `AscCursor::LoadTable` | 31 |
| 31 | `0x10A20E10` | `AscCursor::ResetIndex` | 32 |
| 32 | `0x10A131F0` | `AscAuraAmounts::ClearUnits` | 33 |
| 33 | `0x100D3700` (zeroes the LFG-boss value) | **not reconstructed** (AscGlobalsB gap) | — |
| 34 | `0x102DF7A0` | `AscRaidVideo::Restore` | 34 |
| 35 | `0x10193800` | `AscEventPackets::Reset72D` | 35 |
| 36 | `0x101A83D0` | `AscSpellText::Reset` | 36 |
| 37 | `0x101A3CC0` | `AscCustomStore::OnGlueScreen` | 37 |
| 38 | `0x100C2250` | `AscAppearance::OnGlueScreen` | 38 |
| 39 | `0x101A1E50` (zeroes the CA-gossip word) | **not reconstructed** (gap) | — |
| 40 | `0x10A194E0` | `AscTutorial::ResetAll` | 39 |
| 41 | `0x102DCB00` | `AscGlobalsC::ResetPrestige` | 40 |
| 42 | `0x10321500` | `AscGameMode::Reset` | 41 |
| 43 | `0x10322470` | `AscSpellActivationOverlay::Stop` | 42 |
| 44 | `0x10276C40` | `AscHighRisk::ResetFel` | 43 |
| 45 | `0x102BDCA0` | `AscMythicPlus::ResetRun` | 44 |

Notes:

- The recon ranks are compact (0..44, monotonic in walk order). Two raw slots split into two recon
  callbacks each (`0x1016EA30`, `0x10A32510`); raw slot 14 merges into `AscCAFilter::ResetOnGlue`
  (one registration runs the CA-store, filter and creator-editor-zone resets); raw slots 33 and 39
  have no recon registration and are listed original-only in the fixture. Raw multiset invariant: all
  of 0..45 plus one extra 17.
- These resets all run before the glue screen shows; reordering between them is not user-visible, but
  the measured order is implemented exactly as captured.

Implementation: `AscRuntime::InsertRanked` keeps `{rank, callback}` pairs; each registrar takes
`uint32_t rank = 0xFFFFFFFF` and modules pass their callback's pinned rank at the registration site.
Callbacks without a measured rank sort after the ranked entries, keeping registration order; the
set's dedupe-by-pointer behaviour is reproduced. `native_callback_order.py` covers every registration
permutation for representative lists plus dedupe, unknown placement and execution order (4329 checks,
0 failures), including a GlueScreen permutation run over the pinned ranks.
