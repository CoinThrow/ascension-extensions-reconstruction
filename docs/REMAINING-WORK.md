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

*Update 2026-10-11 (final deep dive): three of the four remaining unknowns are closed with evidence
and the callback-order work is done for the multi-registrant lists.*
- **Callback orders — DONE for the eleven multi-registrant lifecycle lists** (`callback-order-all-lists`,
  `9b7a60b`): the lists are MSVC `unordered_set`s whose iteration is the hash-bucket order of the
  relocated pointers (base-dependent!); walked live in four full-memory sessions (new
  `re\donor_dump4.py` — the old curated dumps missed the `0x26xxxxxx` heap arena) and pinned at the
  reference base `0x67E30000`. Modules pass measured ranks; `native_callback_order.py`: 3849 checks, 0
  failures. Remaining: GlueScreen's 45 registrants stay unranked (mapping deferred; walk data ready).
- **A2 — the wildcard install path is FOUND**: `sub_1015C9C0`'s complete slot/gate table is decoded
  (the wildcard block = `mode & 4` gating the five `+436` slots 44/25/26/27/28; per-slot gates
  `mode&7`/`mode&3`/`mode&1`; unlearn vectors never receive indices 12/14/15/20/21 in any mode — so
  the recon's masks are right and only the rule BODIES for unlearn 12/20 remain unlocated). Evidence:
  `re\out\A2-WILDCARD-INSTALL-FOUND.md`.
- **A3 — CLOSED**: the TemporalContractMgr type vector (+0x24 at 0x10D3F238) has no writer in the
  deployed image (ctor/zeroing + readers only). Vestigial; the empty shape is faithful.
- **A4 — CLOSED**: the JSONContentMgr (0x10BE5FA0) has no writer and no reader (ctor + DllMain + dtor
  only; not the VM either). Vestigial; `DecodeJSONContent` returning nothing is faithful.
  (Branch `a-items-closure`, `fbdde94`; VM residual 5's JSONContentMgr item closed in
  `VM-FULL-ANALYSIS.md`.)
- **Drop-in**: a branch build booted the live client to a stable 40 s+ session (smoke test); the
  game folder was restored to the user's own recon build afterwards.*

## Closed in this pipeline (each its own branch, ready for review)

| branch | what it does |
| --- | --- |
| `ca-rules-dependencies` | CA rules from the original rule objects: slot 20 wired + refined (talent-only, active-player-only), **slot 18 rebuilt** (`TemplateRankCheck`) — 2026-10-11 deep dive |
| `callback-order-all-lists` | **measured ranks for the eleven multi-registrant lifecycle lists** (`InsertRanked`); fixture + tests extended (3849 checks) — 2026-10-11 final deep dive |
| `a-items-closure` | A3/A4 closed with deployed-image xref evidence (no writers exist; vestigial subsystems) — 2026-10-11 |
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
2. **Scroll-of-Fortune accounting for unlearn slots 12/20/21** — **resolved 2026-10-11 (see the update
   block)**: the wildcard install path is the `mode & 4` block of `sub_1015C9C0` (full gate table in
   `re\out\A2-WILDCARD-INSTALL-FOUND.md`) and the recon's masks match it. The unlearn vectors never
   receive rule objects at indices 12/14/15/20/21 in any mode, so the rule BODIES for unlearn
   slots 12 (NO_SCROLL_OF_FORTUNE) and 20 (SCROLL_OF_FORTUNE_LIMIT) are the only unlocated piece
   (they cannot be array objects; suspect the `+548` challenge family, slot 12 = obj BBDC, ok
   `0x1015A4B0`, undecoded).
3. **Temporal contracts type-vector fill** — **CLOSED 2026-10-11**: no writer exists in the deployed
   image (ctor/zeroing + readers only); the empty shape is faithful (`a-items-closure`).
4. **`AscJson.cpp` map writer** — **CLOSED 2026-10-11**: the JSONContentMgr has no writer and no
   reader anywhere (ctor + DllMain + dtor; not the VM); `DecodeJSONContent` returning nothing is
   faithful (`a-items-closure`).
5. **`AscModelMethods.cpp` missile tick** — resolved by design: the module drives the tick with its own
   16 ms timer (the VM drives it in the original); the note stands.
6. **`AscProfiler.cpp` formatter** — closed as won't-transcribe: unreachable by construction (no
   caller in the original either); the module note stands.

*What would close the group:* case-by-case — the 2026-10-11 deep dive cracked the CA rules with no
VM work at all (static rule-object family); try the same static-object route per item before assuming
VM. (The genuine VM set = the four virtualized functions, `VM-FULL-ANALYSIS.md`; live interception
remains the fallback.)

### B. Measurement sessions needed

7. **Callback-list orders for the non-five lists** — **DONE 2026-10-11 for the eleven multi-registrant
   lists** (`callback-order-all-lists`, `9b7a60b`; measured live, pinned at the reference load base;
   modules pass ranks; 3849 checks). Remaining: **GlueScreen's 45 registrants** stay call-site-ordered
   (complete walk data captured; the per-module attribution of 45 callbacks is the last mapping job)
   and the single-entry lists (After5204C0, Before/AfterTargetChange's one-sided pairs, After52A980,
   AfterAddSpell, Before4943C0, Before6ECF80, After743EC0, ObjectFree-adjacent…) are order-trivial.
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
