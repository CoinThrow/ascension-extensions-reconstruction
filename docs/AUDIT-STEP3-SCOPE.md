# Audit step 3 — remaining scaffold plumbing: scope

*Status note, 2026-10-10. `src/Ascension/AscMain.cpp`'s header says "The scaffold plumbing still in use:
custom opcode dispatch and glue CVars (audit step 3 continues)". This file scopes exactly what "still in
use" means, what the original does instead (where that is known), and what closing step 3 would take.*

## What the audit found (context)

The live exactness audit (`tools/live_audit.py`, in the campaign workspace) compared every patch the
reconstruction applies against the patches the original DLL makes to the running client. It removed the
upstream WotLK-Extensions feature patches the original never makes (WoWTime, CMap, Misc, CGPlayer,
CGTooltip, CMissile, MacroConditions, Spell/LuaUnlock, the CGlueMgr / CustomLua / FrameScript call-site
hooks, the 0xD415B8/0x10BC range writes) — see `AscMain.cpp` for the full removal list. Two scaffold
areas survived because the modules depend on them to function at all.

## Item 1 — custom opcode dispatch (`CNetClient::ApplyPatches`)

**What it is.** Four writes in `WotLKExtensions/src/Client/CNetClient.cpp`:

| site | kind | purpose |
| --- | --- | --- |
| `0x631FA0` | detour (6 bytes) | intercept the client's packet-handler table writes (`SetMessageHandlerDetour`) |
| `0x631FE0` | detour (10 bytes) | run custom opcode handlers from the scaffold's process map; unknown opcodes go to `AscProfiler::Unhandled` |
| `0x63200D` | byte patch | `cmp esi, 0x51F` → `0x9D4`: extends the accepted opcode bound so custom opcodes are not dropped |
| `0x632A40` | detour (5 bytes) | keep the client's `NetClient` pointer for registrations (`FUN_102c3a90` shape) |

Every `sDC.AddPacketHandler(...)` registration in every module lands through this machinery.

**What the original does.** Its registrar `FUN_102c4590` (`FUN_102c4400` with the kept NetClient — the
comment in `CNetClient.cpp` already names it) registers handlers the same way, and its own opcode path
lives in the DLL's packet dispatch (`AscRuntime`'s hook manager `FUN_100010f0` covers stock handlers it
replaces; custom opcodes flow through its own tables). The reconstruction reproduces the *effect* but
installs it by detouring four client internals the original may or may not touch — the audit did not
enumerate these four as original patches, which is exactly why they are still labelled scaffold.

**What closing step 3 needs.**
1. Re-run the audit specifically over `0x631FA0 / 0x631FE0 / 0x63200D / 0x632A40`: list which of the four
   the original patches, with its prologues, and where its equivalents live.
2. Where the original has no equivalent (e.g. the bound bump), find how the original's dispatch admits
   custom opcodes without it (candidate: its handlers are installed into the client's own table through
   the registrar path alone, so no bound change is needed).
3. Replace the scaffold detours with the original's exact install sequence; keep
   `CNetClient::RegisterHandler` (it already mirrors `FUN_102c4590`).
4. Acceptance: `CUSTOMPACKETS_PATCH` removed from `PatchConfig.hpp`; every module's packet tests
   (`tests/native_ca_packets.py`) still pass; live trace shows identical handler dispatch.

**Risk if done wrong:** packet loss for custom opcodes (silent `Unhandled`), which breaks large parts of
the UI — verify with the in-game flows the campaign used (CA window, wildcard rolls, realm replay).

## Item 2 — glue CVars (`CVar::ApplyPatches`)

**What it is.** One detour + one list:

- `0x401B60` detour (6 bytes) → `RegisterGlueCVarsCustom`: calls the client's glue-CVar registration
  first, then registers every queued custom entry. The comment already notes this **is** the original's
  detour (`FUN_10114180`, installer `FUN_101141e0`); before 2026-09-27 the scaffold rewrote the call at
  `0x404147` instead, and that was replaced.
- `FillCustomGlueCVarVector` / `RegisterWorldQueue`: the Ascension CVars the SharedXML options panels
  read (`useRaidVideoSettings` + `_raid` set, `loadUnknownAddOns`, C_Logger gates, graphics and camera
  sets, the queued module registrations at the original's own list `0x10BDD1AC..`).

**Assessment.** This area is already near-exact: the detour site is the original's, the queued-vector
addresses are the original's, and the registrar signature matches. The residual scaffold footprint is
the `#if GLUEMGREXTENSION` config gate and the `sDC` DataContainer list that feeds the custom entries
(the original's list is its own 0x10BDD1AC queue).

**What closing step 3 needs.**
1. Audit `0x401B60` and the queue addresses against the running original (expected: match; the campaign
   comments say they were measured).
2. Decide the config gate's fate: either prove the gate is always-on in shipping configurations and
   fold it out, or record why it stays.
3. Move the custom entries off `sDC` onto the recon's own queue object if the original's queue can be
   transcribed exactly.

## Related gap (separate from step 3): the three logon hooks

`AscMain.cpp` documents that the three hooks of Ascension's own logon protocol
(`0x8CCE00`, `0x9A83E0`, `0x9A88C0`) are deliberately not installed (SRP6-restore decision). That is a
product decision, not scaffold debt — and the `native-logon` branch now carries the alternative:
reconstructing the logon client-side (`AscNativeLogon.cpp`, build-gated) instead of installing the
original's VM hooks. Keep the two efforts separate in reviews.

## Suggested next actions

1. Run the audit over the five remaining addresses above; attach results to this file.
2. If all five match the original (likely for CVar, unknown for CNetClient), step 3 closes by removing
   the `CUSTOMPACKETS_PATCH` / `GLUEMGREXTENSION` gates and re-running the packet + in-game suites.
3. If any do not match, transcribe the original's install sequence at that site before touching gates.
