// C_AccountInfo (AccountInfo object 0x10bdb114, accessor FUN_1008c870) and C_Realm (RealmInfo 0x10bdb138).
//
// SMSG_ACCOUNT_INFO 0x9BB (handler_0x09bb): u32 (+0x04), u32 GM level (+0x08), u8 newcomer (+0x0C),
// u8 guide (+0x0D), then u32 n characters of 0x7C bytes: str name (+0x00), str sex (+0x18), u32 level
// (+0x30), str class (+0x34), str race (+0x4C), str faction (+0x64). +0x0E ("is GM") comes from SMSG 0x5C0.
#include <Ascension/AscAccount.hpp>
#include <Ascension/AscBindings.hpp>
#include <Ascension/AscRuntime.hpp>
#include <Ascension/AscScript.hpp>
#include <Ascension/RealmInfo.hpp>
#include <Client/CDataStore.hpp>
#include <Client/CNetClient.hpp>
#include <Misc/DataContainer.hpp>
#include <string>
#include <vector>
#include <cstring>
#include <windows.h>

using namespace AscScript;

namespace
{
    struct Character { std::string name, sex; uint32_t level = 0; std::string cls, race, faction; };
    struct AccountInfo
    {
        uint32_t field4 = 0, gmLevel = 0;
        uint8_t newcomer = 0, guide = 0, gm = 0;
        std::vector<Character> characters;
    } g_account;

    uint32_t ReadU32(CDataStore* p) { uint32_t v = *reinterpret_cast<const uint32_t*>(p->m_buffer + p->m_read); p->m_read += 4; return v; }
    uint8_t ReadU8(CDataStore* p)   { return static_cast<uint8_t>(p->m_buffer[p->m_read++]); }
    std::string ReadStr(CDataStore* p)
    {
        std::string s(reinterpret_cast<const char*>(p->m_buffer + p->m_read));
        p->m_read += static_cast<int32_t>(s.size() + 1);
        return s;
    }

    void __cdecl OnAccountInfo(void*, uint32_t, uint32_t, CDataStore* pkt)
    {
        g_account.field4 = ReadU32(pkt);
        g_account.gmLevel = ReadU32(pkt);
        g_account.newcomer = ReadU8(pkt);
        g_account.guide = ReadU8(pkt);
        g_account.characters.clear();
        for (uint32_t n = ReadU32(pkt); n; --n)
        {
            Character c;
            c.name = ReadStr(pkt);
            c.sex = ReadStr(pkt);
            c.level = ReadU32(pkt);
            c.cls = ReadStr(pkt);
            c.race = ReadStr(pkt);
            c.faction = ReadStr(pkt);
            g_account.characters.push_back(std::move(c));
        }
    }

    int GetGMLevel(lua_State* L)       { PushInt(L, static_cast<int32_t>(g_account.gmLevel)); return 1; }
    int IsGM(lua_State* L)             { PushBool(L, g_account.gm != 0); return 1; }
    int IsGuide(lua_State* L)          { PushBool(L, g_account.guide != 0); return 1; }
    int IsNewcomer(lua_State* L)       { PushBool(L, g_account.newcomer != 0); return 1; }
    int GetNumCharacters(lua_State* L) { PushInt(L, static_cast<int32_t>(g_account.characters.size())); return 1; }

    void SetStr(lua_State* L, const char* k, const std::string& v)
    {
        AscLua::lua_pushstring(L, k);
        AscLua::lua_pushstring(L, v.c_str());
        AscLua::lua_settable(L, -3);
    }

    // FUN_1008d1b0: 1-based; nothing is returned for index 0 or out of range.
    int GetCharacterAtIndex(lua_State* L)
    {
        if (!ValidateInput(L, {NUMBER}))
            return 0;
        const int32_t i = ToInt(CheckNumber(L, 1));
        if (i == 0 || static_cast<uint32_t>(i - 1) >= g_account.characters.size())
            return 0;
        const Character& c = g_account.characters[i - 1];
        AscLua::lua_createtable(L, 0, 0);
        AscLua::lua_checkstack(L, 2);
        SetStr(L, "CharacterName", c.name);
        SetStr(L, "Sex", c.sex);
        SetStrInt(L, "Level", static_cast<int32_t>(c.level));
        SetStr(L, "Class", c.cls);
        SetStr(L, "Race", c.race);
        SetStr(L, "Faction", c.faction);
        return 1;
    }

    // C_Realm: RealmInfo +0x40..+0x44 (FUN_102fc660 / 6b0 / 650 / 670 / 640, FUN_102fc680).
    const uint8_t* Gates() { return RealmInfoSvc::Get().gates; }
    int IsLive(lua_State* L)        { PushBool(L, Gates()[0] != 0); return 1; }
    int IsSeasonal(lua_State* L)    { PushBool(L, Gates()[1] != 0); return 1; }
    int IsLeague(lua_State* L)      { PushBool(L, Gates()[2] != 0); return 1; }
    int IsPTR(lua_State* L)         { PushBool(L, Gates()[3] != 0); return 1; }
    int IsDevelopment(lua_State* L) { PushBool(L, Gates()[4] != 0); return 1; }
    int IsProduction(lua_State* L)  { PushBool(L, Gates()[0] || Gates()[1] || Gates()[2]); return 1; }

    void PatchByte(uint32_t at, const void* bytes, size_t n)
    {
        DWORD old;
        if (!VirtualProtect(reinterpret_cast<void*>(at), n, PAGE_EXECUTE_READWRITE, &old))
            return;
        memcpy(reinterpret_cast<void*>(at), bytes, n);
        VirtualProtect(reinterpret_cast<void*>(at), n, old, &old);
    }

    // SMSG 0x5C0 (FUN_1008c2b0): u8 -> AccountInfo +0x0E. When set, the client's float at 0xA32904 becomes
    // FLT_MAX and the conditional jumps at 0x50D641 / 0x50D2B7 become jmp (never undone; measured by
    // emulation).
    void __cdecl OnGMStatus(void*, uint32_t, uint32_t, CDataStore* pkt)
    {
        g_account.gm = ReadU8(pkt);
        if (!g_account.gm)
            return;
        const uint32_t fltMax = 0x7F7FFFFF;
        const uint8_t jmp = 0xEB;
        PatchByte(0xA32904, &fltMax, 4);
        PatchByte(0x50D641, &jmp, 1);
        PatchByte(0x50D2B7, &jmp, 1);
    }

    // The original-only sixth entry of the aura-application list (0x10BE2B6C). The original registers
    // it from its runtime layer (a VM lane cell, no static site) and calls it last. Behaviour
    // byte-verified from the live original (2026-10-09/10): a staff/spectator gate for five custom
    // aura spells on PLAYER units -- Mentor (0x79BB0, 0x79C3F), the Ascension-logo overhead icon
    // (0x79BC5), New Player Help/Newcomer (0x79C40) and Returning Player (0x978960). Return false =
    // veto: the application stops and the aura is skipped. GM (+0x0E) and the GM PLAYER_FLAGS bit (8)
    // are allow paths, the developer flag (0x8000) suppresses, and the paired tags never apply to
    // the active player itself.
    bool __cdecl MentorAuraGate(void* unit, uint32_t, uint32_t rec)
    {
        if (!unit || !rec)
            return true;
        auto* u = static_cast<uint8_t*>(unit);
        if (*reinterpret_cast<const uint32_t*>(u + 0x14) != 4)   // interposes only on players
            return true;
        const uint32_t spell = *reinterpret_cast<const uint32_t*>(rec);
        const auto* desc = *reinterpret_cast<const uint32_t* const*>(u + 8);
        const uint32_t flags = desc[0x258 / 4];                  // PLAYER_FLAGS
        const auto activePlayer = [] { return reinterpret_cast<uint8_t*(__cdecl*)()>(0x4038F0)(); };
        switch (spell)
        {
        case 0x79BB0:   // Mentor
        case 0x79C3F:
            if (u == activePlayer())                    return false;
            if (flags & 0x8000)                         return false;
            if (!(flags & 8) && !g_account.newcomer)    return false;
            return g_account.gm == 0;
        case 0x79BC5:   // Overhead Icon - Ascension Logo
            return !(flags & 0x8000);
        case 0x79C40:   // New Player Help - Newcomer
        case 0x978960:  // Returning Player
            if (u == activePlayer())                    return false;
            if (flags & 0x8000)                         return false;
            if (!(flags & 8) && !g_account.guide)       return false;
            return g_account.gm == 0;
        default:
            return true;
        }
    }

    void Init()
    {
        sDC.AddPacketHandler(0x5C0, CNetClientCustomPacket((void*)&OnGMStatus, nullptr));
        sDC.AddPacketHandler(0x9BB, CNetClientCustomPacket((void*)&OnAccountInfo, nullptr));
        // Site 0 is not a captured site, so the registrar ranks it after the five captured entries --
        // the measured position of the original's runtime-registered sixth callback.
        AscRuntime::OnBefore724820(&MentorAuraGate, 0);
    }

    const AscBindings::Binding kBindings[] = {
        {"C_AccountInfo", "GetGMLevel", GetGMLevel},
        {"C_AccountInfo", "IsGM", IsGM},
        {"C_AccountInfo", "IsGuide", IsGuide},
        {"C_AccountInfo", "IsNewcomer", IsNewcomer},
        {"C_AccountInfo", "GetNumCharacters", GetNumCharacters},
        {"C_AccountInfo", "GetCharacterAtIndex", GetCharacterAtIndex},
        {"C_Realm", "IsLive", IsLive},
        {"C_Realm", "IsSeasonal", IsSeasonal},
        {"C_Realm", "IsLeague", IsLeague},
        {"C_Realm", "IsPTR", IsPTR},
        {"C_Realm", "IsDevelopment", IsDevelopment},
        {"C_Realm", "IsProduction", IsProduction},
    };
    AscBindings::Module s_module(kBindings, sizeof(kBindings) / sizeof(kBindings[0]), &Init);
}

namespace AscAccount
{
    uint32_t GmLevel() { return g_account.gmLevel; }
}
