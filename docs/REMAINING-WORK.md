# Remaining work — post-pipeline analysis (2026-10-10)

*Result of the per-task pipeline that closed nine gaps and the deep re-audit that followed. Each item
states what it is, why it is still open, and what would close it. Branch names refer to this repository.*

*Update 2026-10-11 (completion pass): everything completable without a live game session is done —
fork fix committed and pushed; the scaffold-gate audit closed (all five sites verified as the
original's, `audit-step3-scope` updated); crash-reporter provenance tracked in
`THIRD_PARTY_NOTICES.md`; the `+0x274` record investigated (refined note below). What remains needs
VM-bytecode decode or a live measurement session; the product decisions stay deliberate.*

*Update 2026-10-11 (VM deep dive): the CA rule system was NOT VM-blocked — it is a static family of
polymorphic rule objects (`off_10BCCAF0..off_10BCCBFC`, installed by `sub_1015C9C0`) whose `ok()`
functions are plain code. Slot 18 is now fully rebuilt and slot 20 refined (branch
`ca-rules-dependencies`, `3d79d9c`); the `+0x274` byte is identified (the unlearn runner's path
selector). Scroll slots 12/20 are absent from the located installer (wildcard path to find); A3/A4
keep their notes with better routes.*

## Closed in this pipeline (each its own branch, ready for review)

| branch | what it does |
| --- | --- |
| `ca-rules-dependencies` | CA rules from the original rule objects: slot 20 wired + refined (talent-only, active-player-only), **slot 18 rebuilt** (`TemplateRankCheck`) — 2026-10-11 deep dive |
| `ca-pending-request-flow` | fills the manager `+0x198` request vector at the transcribed CMSG 0x61D sends and clears it on SMSG 0x61E / reset, so `InFlight`, `IsPendingEntryID` and the pending-build signal wait actually wait |
| `temporal-contracts-status` | records the confirmed VM-side un-recoverability of the contract-type fill with the byte-search evidence |
| `ca-layout-columns` | retires the superseded partial `GetAllEntries` pair (never bound; the live `AscCAMgr` FUN_1016f5b0 + PushEntry path already emits every column) |
| `aura-callback6-mentor` | reconstructs the original-only sixth aura-application callback (Mentor / Newcomer / Returning Player / overhead icon) in `AscAccount.cpp`; callback-order test: 3367 checks, 0 failures |
| `audit-step3-scope` | scopes and **closes** the remaining scaffold plumbing — audit (2026-10-11): all five sites and both bodies verify as the original's own |
| `docs-improvements-md` | adds `IMPROVEMENTS.md`, the defect log the code comments promise |
| `stale-notes-cleanup` | fixes the verified-stale notes (AscPatchData hand-work list, AscGameMode manager note, AscWildcardRolls header, DLL_REFERENCE mirror) |
| `third-party-notices` | adds `THIRD_PARTY_NOTICES.md` (crash-reporter GPL-2 lineage + vendored freetype/mapbox/nlohmann licenses) |
| `remaining-work-analysis` | this file |

## Still open — by category

### A. VM-mutated zone (blocked on bytecode decode)

The constants and bodies below are referenced only from the Themida-mutated region; the campaign's
byte-search across the deployed image finds no plain-code reference of any encoding (verified
2026-10-10: `0x10BDCDF0`, `0x1015B7C0`, `0x10D3C1F0`, `0x10BE2B6C`, `0x10BE28AC` → 0 hits).

1. **CA rule slot 18 (BUILD_CREATOR template relation)** — **fixed 2026-10-11**: rules decoded via the
   static rule-object family; slot 18's `ok()` = `sub_10160bc0` (template match by name; row must hold
   ≥ current rank + 1 ranks in it) — wired through `AscBuildCreator::TemplateRankCheck`
   (`ca-rules-dependencies`, `3d79d9c`). Slot 20 refined in the same commit (talent-only,
   active-player-only).
2. **Scroll-of-Fortune accounting for unlearn slots 12/20/21** — narrowed 2026-10-11: the unlearn rule
   arrays (`+0x1D0`/`+0x1DC`) install 14 objects (slots 3–19, per `sub_1015C9C0`); slots 12/20/21 are
   **not** among them, so their rules come from the wildcard-mode install path (not yet located). The
   claim/reward side exists in `AscScrollOfFortune.cpp`.
3. **Temporal contracts type-vector fill** — `AscTemporalContracts.cpp`. Documented; empty-shape stands.
   Suggestion: re-check with the object-family technique that cracked the CA rules (look for a static
   object table over the manager before assuming the VM).
4. **`AscJson.cpp` map writer** — returns nothing, documented. Lead: the sibling
   `LoadAscensionContentJSON` reads `<client dir>\Content\<name>.json`; the manager's writer may be the
   same file path from the archives (candidate, unverified — needs a live check of the original).
5. **`AscModelMethods.cpp` missile tick** — resolved by design: the module drives the tick with its own
   16 ms timer (the VM drives it in the original); the note stands.
6. **`AscProfiler.cpp` formatter** — closed as won't-transcribe: unreachable by construction (no
   caller in the original either); the module note stands.

*What would close the group:* case-by-case — the 2026-10-11 deep dive cracked the CA rules with no
VM work at all (static rule-object family); try the same static-object route per item before assuming
VM. (The genuine VM set = the four virtualized functions, `VM-FULL-ANALYSIS.md`; live interception
remains the fallback.)

### B. Measurement sessions needed

7. **Callback-list orders for the non-five lists.** The five documented lists are handled (Before71E930 /
   After80B5D0 captured; Before724820 / EffectFilter / VisualHide fixed on `callback-order-measured` +
   `aura-callback6-mentor`). The other 23 lifecycle lists still append in call-site order
   (`AscRuntime.cpp`: OnStockEvent, OnAfter403340, OnLeaveWorld, OnBefore528F00, OnAfter6E7F50,
   OnBefore4943C0, OnAfterAddSpell, OnAfter402910, OnAfter5204C0, OnAfter4F6F90, OnAfter495810,
   OnEnterWorld, OnGlueScreen, OnAfterEnterWorld, OnObjectFree, OnAlternate6DF050, OnGameObjectUse,
   OnItemUse, OnAfter743EC0, OnBeforeTargetChange, OnAfter52A980, OnAfterTargetChange, OnBefore6ECF80).
   Closing needs another live walk (method + two-session protocol in `docs/CALLBACK-ORDER-MEASURED.md`,
   "Measurement and validation").
8. **`+0x274` record** — **identified 2026-10-11**: build byte 628 is the flag the unlearn rule runner
   (`sub_10151820`) branches on — `*(build+628) == 0` selects the direct path, else the map-resolved
   one (the `+0x254`-style map with the FNV hash). The session reset clears it. Modelling stays a
   choice (the recon hand-implements slots rather than the generic runner).

### C. Product decisions (deliberate, keep separate from fidelity)

9. **Three logon hooks not installed** (`AscMain.cpp`) — product decision; the `native-logon` branch is
   the answer. Do not merge VM-hook installation for this.
10. **Anti-cheat loader / account kill switch** (`AscWorldEntryTweaks.cpp`) — deliberately inert (exact
    reproduction would kill non-Ascension realms).
11. **`ApplyPlaintextWorldHeaders`** — implemented but call-commented (config = 0); enable only with the
    matching server flag.

### D. Housekeeping / legal

12. **Crash-reporter provenance** — **tracked 2026-10-11**: `THIRD_PARTY_NOTICES.md` now records the
    GPL-2 lineage of `AscCrashReport.cpp` (plus the vendored freetype / mapbox / nlohmann licenses);
    resolution before any redistribution remains a legal decision, but the flag now has its home.
13. **Scaffold config gates** — **closed 2026-10-11**: the audit ran; all five addresses are the
    original's own patch sites and the detour bodies transpose the original's (evidence in
    `AUDIT-STEP3-SCOPE.md`, "Audit results"). The options stay as ordinary config toggles.

### E. Fork side (`azerothcore-wotlk-coa`)

14. **`AuthSession.cpp` session-key fix** — **done 2026-10-11**: committed `2b14991ad` on
    `feature/ascension-native-login` (persist `_ascensionKeySession` as `account.session_key` for the
    world handoff, with `LOGIN_UPD_LOGONPROOF`) and pushed to `CoinThrow/azerothcore-wotlk-coa`. The
    branch is now PR-able.

## Suggested review order

1. `ca-rules-dependencies` (small, self-contained), 2. `ca-pending-request-flow`,
3. `aura-callback6-mentor` (stack on `callback-order-measured`), 4. `ca-layout-columns`,
5. `stale-notes-cleanup` + `docs-improvements-md` + `audit-step3-scope` (docs),
6. `temporal-contracts-status`, 7. this file's branches as the roadmap.
