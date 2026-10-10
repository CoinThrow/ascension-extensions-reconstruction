# Remaining work — post-pipeline analysis (2026-10-10)

*Result of the per-task pipeline that closed nine gaps and the deep re-audit that followed. Each item
states what it is, why it is still open, and what would close it. Branch names refer to this repository.*

## Closed in this pipeline (each its own branch, ready for review)

| branch | what it does |
| --- | --- |
| `ca-rules-dependencies` | wires CA rule slot 20 (NO_TALENTS_CHALLENGE) through `AscChallenge::ActiveHasRule` — "rule 14" is `CHALLENGE_RULES_TYPE_NO_TALENTS`; refreshes the slot-18 dependency notes |
| `ca-pending-request-flow` | fills the manager `+0x198` request vector at the transcribed CMSG 0x61D sends and clears it on SMSG 0x61E / reset, so `InFlight`, `IsPendingEntryID` and the pending-build signal wait actually wait |
| `temporal-contracts-status` | records the confirmed VM-side un-recoverability of the contract-type fill with the byte-search evidence |
| `ca-layout-columns` | retires the superseded partial `GetAllEntries` pair (never bound; the live `AscCAMgr` FUN_1016f5b0 + PushEntry path already emits every column) |
| `aura-callback6-mentor` | reconstructs the original-only sixth aura-application callback (Mentor / Newcomer / Returning Player / overhead icon) in `AscAccount.cpp`; callback-order test: 3367 checks, 0 failures |
| `audit-step3-scope` | scopes the remaining scaffold plumbing (custom opcode dispatch + glue CVars) |
| `docs-improvements-md` | adds `IMPROVEMENTS.md`, the defect log the code comments promise |
| `stale-notes-cleanup` | fixes the verified-stale notes (AscPatchData hand-work list, AscGameMode manager note, AscWildcardRolls header, DLL_REFERENCE mirror) |
| `remaining-work-analysis` | this file |

## Still open — by category

### A. VM-mutated zone (blocked on bytecode decode)

The constants and bodies below are referenced only from the Themida-mutated region; the campaign's
byte-search across the deployed image finds no plain-code reference of any encoding (verified
2026-10-10: `0x10BDCDF0`, `0x1015B7C0`, `0x10D3C1F0`, `0x10BE2B6C`, `0x10BE28AC` → 0 hits).

1. **CA rule slot 18 (BUILD_CREATOR template relation)** — `AscCARules.cpp`. `AscBuildCreator` is
   transcribed; the draft↔template relation the original check consults is not mapped. The current
   "none → fail" emulation stays until the relation is recovered.
2. **Scroll-of-Fortune accounting for unlearn slots 12/20/21** — `AscCARules.cpp` ~1058. The item
   counting path the original uses is not transcribed (the claim/reward side exists in
   `AscScrollOfFortune.cpp`; the counting side does not).
3. **Temporal contracts type-vector fill** — `AscTemporalContracts.cpp`. Documented; empty-shape stands.
4. **`AscJson.cpp` map writer** (its only writer is in the VM region) — returns nothing, documented.
5. **`AscModelMethods.cpp` missile tick** (no plain-code caller) — documented.
6. **`AscProfiler.cpp` formatter** (3.7 KB, unreachable in plain code) — documented.

*What would close the group:* decode the `.vm_sec` bytecode (see the campaign's `VM-FULL-ANALYSIS.md`);
or intercept the live VM with the campaign's hardware-breakpoint tooling at the fill sites.

### B. Measurement sessions needed

7. **Callback-list orders still unmeasured.** The fixture's own limits say Before71E930 / After80B5D0
   are complete, Before724820 / EffectFilter / VisualHide are measured, and *the remaining lists are
   unmeasured* — those still order by call site (`AscRuntime.hpp`). Closing needs another live walk of
   the original's list objects (the campaign's tooling + the two-session protocol).
8. **`AscWildcardRapid.hpp` `+0x274` record** — "not modelled here" in the session-reset note; needs a
   decompile pass to decide whether it carries behaviour beyond the phase reset.

### C. Product decisions (deliberate, keep separate from fidelity)

9. **Three logon hooks not installed** (`AscMain.cpp`) — product decision; the `native-logon` branch is
   the answer. Do not merge VM-hook installation for this.
10. **Anti-cheat loader / account kill switch** (`AscWorldEntryTweaks.cpp`) — deliberately inert (exact
    reproduction would kill non-Ascension realms).
11. **`ApplyPlaintextWorldHeaders`** — implemented but call-commented (config = 0); enable only with the
    matching server flag.

### D. Housekeeping / legal

12. **Crash-reporter provenance** — `AscCrashReport.cpp` descends from TrinityCore's GPL-2
    `WheatyExceptionReport`; the repository is MIT. Resolve before redistribution (flag lives in
    `IMPROVEMENTS.md`).
13. **Scaffold config gates** — `CUSTOMPACKETS_PATCH` / `GLUEMGREXTENSION`: audit the five addresses in
    `AUDIT-STEP3-SCOPE.md`, then fold the gates out.

### E. Fork side (`azerothcore-wotlk-coa`)

14. **`AuthSession.cpp` session-key fix is uncommitted** on `feature/ascension-native-login` (+18 lines:
    persist `_ascensionKeySession` as `account.session_key` for the world handoff, with
    `LOGIN_UPD_LOGONPROOF`). Commit it before PR review; without it the world handoff fails after
    custom auth.

## Suggested review order

1. `ca-rules-dependencies` (small, self-contained), 2. `ca-pending-request-flow`,
3. `aura-callback6-mentor` (stack on `callback-order-measured`), 4. `ca-layout-columns`,
5. `stale-notes-cleanup` + `docs-improvements-md` + `audit-step3-scope` (docs),
6. `temporal-contracts-status`, 7. this file's branches as the roadmap.
