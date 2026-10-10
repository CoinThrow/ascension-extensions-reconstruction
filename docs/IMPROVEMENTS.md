# IMPROVEMENTS — fidelity notes, deliberate departures and original defects

*Assembled 2026-10-10 from the `(IMPROVEMENTS.md)` references scattered through `src/`. This file is the
index the code comments promise: three groups — (1) places where the reconstruction **deliberately
departs** from the original, (2) **original defects reproduced as-is** for fidelity, (3) provenance and
policy notes. Details always live at the referenced code sites; keep this file in sync when a marked
site changes.*

## 1. Deliberate departures from the original

- **Rune regeneration** (`AscCAMgr.cpp` ~299): the original's ready check degenerates to `cmp ebx, ebx`
  once a rune spell is known, so runes regenerate regardless of the display power. The reconstruction
  departs deliberately (see the site) — do not "fix" it back without re-measuring the live client.
- **Spell-token digit buffer** (`AscSpellTokens.cpp` ~210): the original copies digits into a 12-byte
  stack buffer without a terminator; the reconstruction terminates it. Visibility-only difference.
- **Keystone tooltip, one Dungeon Modifier** (`AscTooltipInsertFix.cpp`): **not in the genuine DLL** — a
  shipped fix, because no server-side route exists. It wraps both `GameTooltip` methods after world UI
  load to hide the line-past-`NumLines()` globals for the call's duration, so the original's own
  `else AddLine` branch runs.

## 2. Original defects reproduced as-is (fidelity)

Grouped by subsystem; each behaves exactly as the original does, defects included.

- **MMgr64.exe-gated dead paths** (the shipped loader never lets these run):
  - `AscAppearance.cpp` ~1130: the item-name/quality caches are only ever cleared by `FUN_101b5420`,
    which MMgr64.exe never allows to run.
  - `AscBridgeStore.cpp` ~831 and ~1028: the patch-key path (`0x73` appended at the end of the table)
    is refused by the shipped MMgr64.exe.
- **Character advancement**:
  - `AscCAMgr.cpp` ~5279: the purge-talents result's `ok` flag pushes a dword whose low byte alone is
    meaningful (upper bytes are a leftover pointer) — always true.
- **Data layer**:
  - `AscClientDbc.hpp` ~14 / `AscDbc.cpp` ~190: growing the index copies ids `min..max-1`, so the row
    that held the old maximum is lost.
  - `AscDbcReaders.cpp` ~8: the 41st reader is `ItemPurchaseGroup`, given `ItemLimitCategory`'s reader
    (80 bytes of a 104-byte row).
  - `AscHdDbc.cpp` ~98: a store whose file failed after its record count has `rows == null` and is read
    anyway.
  - `AscPatchData.cpp` ~103: `0x723` keeps `+0x18` as a pointer into the packet buffer.
- **Movement force** (`AscMovementForce.cpp` ~403/~465): the `0x762` stop is gated on a second erase of
  the same guid right after the first — it never fires.
- **Glue / CVars**:
  - `AscCVarHooks.cpp` ~70: the original steps character records by `0x188` while the client's are
    `0x198`, so only the first record is read correctly.
  - `AscUnitTokens.cpp` ~8: pointer checks against function entry points (`0x81B530`, `0x615020`,
    `0x6143F0`, `0x81AA00`) can never match a return address.
- **Small hooks** (`AscSmallHooks.cpp` ~179): `FUN_454070` builds a path and discards it — dead work,
  transcribed exactly.
- **Tooltips**:
  - `AscTooltipScaling.cpp` ~318: the damage block is filled from `+0xC0/+0xC8/+0xC4/+0xCC`, not the
    cache's `{min,max}` pairs — the original's order.
  - `AscTooltipScaling.cpp` ~689: `FUN_1027b070` leaves the trade flag set; `FUN_1027b0c0` clears it.
- **Events** (`AscEventPackets.cpp` ~14): `0x76B`/`0x76C` push the two trailing bytes with stale upper
  bytes.
- **Bindings** (`AscBindings.cpp` ~547): the original registers `"UnitNameBadge "` / `"UnitNameTag "`
  with a trailing space (strings `0x10B60B04` / `0x10B60AF4`); the reconstruction keeps the original
  names verbatim.

## 3. Provenance and policy notes

- **Crash reporter** (`AscCrashReport.cpp` ~14): transcribed from the decompile, bugs included. Source
  provenance: TrinityCore's `WheatyExceptionReport` is **GPL-2**; this repository is **MIT** — keep the
  provenance header with the file and resolve before any redistribution.
- **Original-only sixth aura callback**: see `docs/CALLBACK-ORDER-MEASURED.md` (reconstructed
  2026-10-10, `AscAccount.cpp`).
- **Logon hooks**: the three Ascension logon hooks are deliberately not installed (`AscMain.cpp`);
  the `native-logon` branch is the product answer.
- **Scaffold plumbing**: audit step 3's remaining scope is in `docs/AUDIT-STEP3-SCOPE.md`.
