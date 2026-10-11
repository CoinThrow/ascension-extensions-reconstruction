import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def function(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    index = opening + 1
    while depth:
        depth += (source[index] == '{') - (source[index] == '}')
        index += 1
    return source[start:index]


HARNESS = r'''
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#define __cdecl

typedef const uint8_t* Row;

struct TokenCost
{
    uint32_t item = 0, count = 0, marks = 0, money = 0;
};

struct Build
{
    uint8_t wildcard = 0;
    uint32_t u44 = 0;
    uint32_t level = 0;
    // test knobs behind the real essence helpers
    uint32_t ga = 0, ge = 0, ab = 0, tb = 0;
    uint32_t GlobalAE(uint32_t) const { return ga; }
    uint32_t GlobalTE(uint32_t) const { return ge; }
    uint32_t AEBudget(uint32_t) const { return ab; }
    uint32_t TEBudget(uint32_t) const { return tb; }
    bool HasGroup(uint32_t, uint32_t) const { return false; }
};

std::map<std::string, int32_t> g_config;
namespace AscConfig
{
int32_t const* Int(char const* name)
{
    auto it = g_config.find(name);
    return it == g_config.end() ? nullptr : &it->second;
}
}

std::map<uint32_t, uint64_t> g_tokens;
namespace AscToken
{
uint64_t Amount64(uint32_t index)
{
    auto it = g_tokens.find(index);
    return it == g_tokens.end() ? 0 : it->second;
}
}

std::vector<uint32_t> g_limit;   // {tier, v0..v5} or empty
namespace AscScrollOfFortune
{
const uint32_t* LimitRecord(uint32_t tier)
{
    return (!g_limit.empty() && g_limit[0] == tier) ? g_limit.data() : nullptr;
}
}

Build g_build;
bool g_hasBuild = true;
Build* BuildOf(uint64_t) { return g_hasBuild ? &g_build : nullptr; }

uint64_t g_playerGuid = 42;
uint64_t ActivePlayerGuid() { return g_playerGuid; }
int g_spec = 0;
int ActiveSpecIndex() { return g_spec; }

std::array<uint8_t, 0x130> g_desc;
std::array<uint8_t, 16> g_unit;
const uint8_t* Descriptor(const uint8_t* unit) { return *reinterpret_cast<uint8_t const* const*>(unit + 8); }
uint64_t UnitGuid(const uint8_t* unit) { return *reinterpret_cast<uint64_t const*>(Descriptor(unit)); }
uint32_t RowU32(Row r, uint32_t off) { return *reinterpret_cast<uint32_t const*>(r + off); }

int g_rowType = 1;
bool IsTalent(Row) { return g_rowType == 2; }
bool IsAbilityLike(Row) { return g_rowType == 1 || g_rowType == 4; }
bool CoAClass(uint8_t) { return false; }
uint8_t UnitClassOf(const uint8_t*) { return 0; }
const uint8_t* CurrentMap() { return nullptr; }
uint32_t MapType(const uint8_t*) { return 0; }
bool InCombat(const uint8_t*) { return false; }
bool CanPay(const uint8_t*, const TokenCost&) { return true; }
TokenCost UnlearnCost(const Build&, Row) { return TokenCost{}; }

@RULES@

int failures = 0;
void Check(bool condition, char const* name)
{
    failures += !condition;
    std::printf("%s: %s\n", condition ? "PASS" : "FAIL", name);
}

struct UnitCtx
{
    std::array<uint8_t, 0x130> desc;
    std::array<uint8_t, 16> unit;
};
const uint8_t* MakeUnit(UnitCtx& ctx, uint64_t guid)
{
    auto desc = ctx.desc.data();
    memcpy(ctx.unit.data() + 8, &desc, sizeof(desc));
    memcpy(ctx.desc.data(), &guid, sizeof(guid));
    return ctx.unit.data();
}

std::array<uint8_t, 0x128> g_row;
Row RowOf()
{
    g_row.fill(0);
    return g_row.data();
}

int main()
{
    UnitCtx meCtx, otherCtx;
    const uint8_t* me = MakeUnit(meCtx, g_playerGuid);
    const uint8_t* other = MakeUnit(otherCtx, 7);
    Row row = RowOf();

    // ---- slot 12 NO_SCROLL_OF_FORTUNE (FUN_10164ab0) ---------------------------------------------
    g_config.clear();
    g_tokens.clear();
    g_limit.clear();
    g_build = Build{};
    g_build.wildcard = 1;
    g_build.u44 = 4;
    g_spec = 3;
    Check(UnlearnUnitRuleOk(12, me, row),
          "12: the starting allowance needs no scroll");

    g_build.u44 = 5;
    Check(!UnlearnUnitRuleOk(12, me, row),
          "12: an ability row outside the allowance without scrolls fails");

    g_tokens[23] = 1;   // tier 3 _ABILITIES
    Check(UnlearnUnitRuleOk(12, me, row),
          "12: the abilities token of the active tier passes");

    g_tokens.clear();
    g_tokens[43] = 1;   // tier 3 _TALENTS
    Check(!UnlearnUnitRuleOk(12, me, row),
          "12: the talents token does not cover an ability row");

    g_tokens.clear();
    g_rowType = 2;
    g_tokens[43] = 1;
    Check(UnlearnUnitRuleOk(12, me, row),
          "12: the talents token of the active tier passes for talent rows");
    g_tokens.clear();
    Check(!UnlearnUnitRuleOk(12, me, row),
          "12: a talent row outside the allowance without scrolls fails");

    g_rowType = 1;
    g_tokens[3] = 1;    // plain token fallback
    Check(UnlearnUnitRuleOk(12, me, row),
          "12: the plain token of the tier is the fallback");
    g_tokens.clear();

    Check(UnlearnUnitRuleOk(12, other, row),
          "12: another unit's unlearn never fails here");

    g_config["CONFIG_WILDCARD_STARTING_ABILITY_COUNT"] = 10;
    Check(UnlearnUnitRuleOk(12, me, row),
          "12: the starting allowance follows its config");
    g_config.clear();

    g_hasBuild = false;
    Check(!UnlearnUnitRuleOk(12, me, row),
          "12: no resolvable build fails");
    g_hasBuild = true;

    // ---- slot 20 SCROLL_OF_FORTUNE_LIMIT (FUN_10165260) -------------------------------------------
    g_build = Build{};
    g_build.wildcard = 1;
    g_build.u44 = 0;
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: the starting allowance passes");

    g_build.u44 = 5;
    g_build.ab = 10;    // budget 10, spent 0 -> one more ability roll (cost 2) fits
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: holding no scroll of the kind passes");

    g_rowType = 0;
    g_tokens[23] = 1;
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: non-ability/talent rows pass");
    g_rowType = 1;
    g_tokens.clear();
    g_tokens[23] = 1;

    g_limit = {3, 2, 5, 5, 2, 0, 0};   // tier 3: ability cap 2 used 2
    Check(!UnlearnUnitRuleOk(20, me, row),
          "20: a reached ability cap with a fitting roll fails");

    g_limit = {3, 2, 5, 5, 1, 0, 0};
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: an unreached ability cap passes");

    g_limit = {3, 0xFFFFFFFFu, 5, 5, 9, 0, 0};
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: a negative cap passes");

    g_limit = {3, 2, 5, 5, 2, 0, 0};
    g_build.ga = 100;    // spent above the budget -> no roll fits
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: a reached cap passes when no roll would fit");
    g_build.ga = 0;

    g_limit.clear();
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: no limit record of the tier passes");

    g_tokens.clear();
    g_tokens[3] = 1;    // plain token only
    g_limit = {3, 5, 5, 2, 0, 0, 2};
    Check(!UnlearnUnitRuleOk(20, me, row),
          "20: a reached plain cap fails with the plain token held");
    g_limit = {3, 5, 5, 2, 0, 0, 1};
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: an unreached plain cap passes");

    g_tokens.clear();
    g_rowType = 2;
    g_tokens[43] = 1;
    g_build.tb = 10;    // one more talent roll (cost 1) fits
    g_limit = {3, 5, 2, 5, 0, 2, 0};
    Check(!UnlearnUnitRuleOk(20, me, row),
          "20: a reached talent cap fails for talent rows");

    Check(UnlearnUnitRuleOk(20, other, row),
          "20: another unit's unlearn never fails here");

    g_hasBuild = false;
    Check(UnlearnUnitRuleOk(20, me, row),
          "20: no resolvable build passes");
    g_hasBuild = true;

    return failures ? 1 : 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description='Compile and exercise the wildcard unlearn scroll rules with client doubles.')
    parser.add_argument('--source-ref')
    args = parser.parse_args()
    name = 'src/Ascension/AscCARules.cpp'
    if args.source_ref:
        source = subprocess.check_output(['git', 'show', f'{args.source_ref}:{name}'], cwd=ROOT, text=True)
    else:
        source = (ROOT / name).read_text(encoding='utf-8')
    harness = HARNESS
    rules = '\n\n'.join(function(source, signature) for signature in [
        'uint32_t ConfigInt(const char* name, int32_t def)',
        'bool StartingAllowance(const Build& b)',
        'bool OneMoreRollFits(const Build& b, bool talent)',
        'bool UnlearnUnitRuleOk(uint32_t slot, const uint8_t* unit, Row r)',
    ])
    harness = harness.replace('@RULES@', rules)
    compiler = shutil.which(os.environ.get('CXX', 'cl.exe' if os.name == 'nt' else 'c++'))
    if not compiler:
        raise SystemExit('A C++20 compiler is required.')
    with tempfile.TemporaryDirectory(prefix='native-unlearn-scroll-') as directory:
        out = Path(directory)
        cpp = out / 'unlearn.cpp'
        cpp.write_text(harness, encoding='utf-8')
        executable = out / ('unlearn.exe' if os.name == 'nt' else 'unlearn')
        if Path(compiler).stem.lower() == 'cl':
            flags = ['/nologo', '/std:c++20', '/EHsc', str(cpp), '/Fe' + str(executable)]
        else:
            flags = ['-std=c++20', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter',
                     str(cpp), '-o', str(executable)]
        subprocess.run([compiler, *flags], cwd=out, check=True)
        return subprocess.run([str(executable)], cwd=out).returncode


if __name__ == '__main__':
    raise SystemExit(main())
