# Retired sources (not compiled)

Moved out of `src/` on 2026-09-27. `CMakeLists.txt` globs `src/*.cpp`, so nothing here is built.

- `AscStubs.generated.cpp`: nil-returning stubs for every original binding, from before the exact
  transcriptions existed. By 2026-09-27 every entry was either overridden by an `AscBindings`
  transcription or registered a name the original does not have (for example 59 `C_BuildEditor`
  functions under `C_BuildCreator`, and `IsSpellIDKnown` returning nil where the UI calls the real global).
- `AscCharAdvancementApi.cpp/.hpp`: the first hand-written `C_CharacterAdvancement`, fed by the
  `Asc_SetPlayerContext` bootstrap hook. Fully superseded by `AscCAMgr.cpp`. Its only surviving entries
  were three names the original registers elsewhere (`GetReducedChanceToBeCrit`, `IsSpellHoldToCast`
  are globals, `HasRuneUI` is `C_Spell`).
- `AscClassInfo.cpp/.hpp` (retired 2026-09-27): a native `C_ClassInfo.GetSpecInfoByID`. In the original that
  function is Lua from `Interface\SharedXML\Util\ClassInfoUtil.lua`, defined only `if not
  C_ClassInfo.GetSpecInfoByID`, on top of the native `C_ClassInfo.GetSpecInfo`. Ours pre-empted it.
- `AscGlobalStrings.cpp` (retired 2026-09-27): the old every-row GlobalStrings.dbc loader. Superseded by
  AscPatchData's per-state applier (FUN_1025af40 / FUN_1025afb0), and no longer called.
- `AscensionCA.*`, `CharSelectOpcodes.*` (retired 2026-09-27): early hand-written handlers for 0x722/0x725/0x726
  and ten char-select opcodes. Every one was also registered by an exact module that initialises first, so
  they never ran. The one exception is 0x672, which is now exact in AscMisc (handler_0x0672 -> the
  friend-offline flag); the old catch-all dropped the byte.
- `AscOpcodes.generated.*`, `AscOpcodeParse.*` (retired 2026-09-27): atlas-driven parse-and-log handlers for
  every atlas opcode. Diagnostics, and they silently consumed 38 opcodes the original leaves unhandled.
- `AscCharacterAdvancement.cpp/.hpp` (retired 2026-10-10): the first file-reading
  `C_CharacterAdvancement.GetAllEntries`, partial by design -- it emitted only the columns with a
  server-confirmed map and no layout geometry, because the Lua-side key names had not been found yet.
  Fully superseded by `AscCAMgr.cpp`'s exact pair: `GetAllEntries` (FUN_1016f5b0) + `PushEntry`
  (FUN_10172df0), which emits the complete key set (Type, Flags, NodeType, ParentNode, Distance,
  Anchor, Color, Masteries, PositionX/Y, SizeX/Y, ConnectedNodes, ...) read from the parsed rows at
  their real struct offsets. Never registered: the binding table always pointed at AscCAMgr's version,
  and nothing else included the header (only the retired stubs referenced its symbols).
- Upstream WotLK-Extensions feature sources are NOT moved (they are part of the vendored scaffold); since
  2026-09-27 they are excluded from the build by CMakeLists.txt instead: Client/CGTooltip, WoWTime,
  CMissile, CMap, Misc, MacroConditions, Spell, GameObjects/CGPlayer and Misc/LuaUnlock. The live audit
  found the original client makes none of their patches.
