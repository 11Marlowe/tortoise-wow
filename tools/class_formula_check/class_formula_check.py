#!/usr/bin/env python3
"""Differential check for the ClassRegistry stat-formula refactor.

The refactor (commit "Move class stat formulas from hardcoded switches into
ClassRegistry descriptors") replaced hardcoded per-class switches with a
descriptor table.  This tool verifies behavior preservation in two ways:

1. Constant diff - the descriptor values parsed from the current
   src/game/ClassRegistry.cpp must equal the constants that were hardcoded in
   the pre-refactor switches (StatSystem.cpp, Player.cpp, Unit.cpp).
2. Structural check - the only expression whose *shape* changed is attack
   power (extra zero-valued terms and a flat coefficient replacing " - 20").
   Attack power uses only float multiply/add, so we emulate IEEE-754 single
   precision exactly and compare old vs new evaluation over a matrix of
   classes, levels and stats.

Lineage: to see the old code, run
    git show 66132ca~1:src/game/StatSystem.cpp
    git show 66132ca~1:src/game/Objects/Player.cpp
    git show 66132ca~1:src/game/Objects/Unit.cpp

Exit code 0 = no differences, 1 = differences found.
"""

import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REGISTRY_CPP = ROOT / "src" / "game" / "ClassRegistry.cpp"

CLASS_IDS = {
    "WARRIOR": 1, "PALADIN": 2, "HUNTER": 3, "ROGUE": 4, "PRIEST": 5,
    "SHAMAN": 7, "MAGE": 8, "WARLOCK": 9, "DRUID": 11,
}
ALL_CLASSES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]

# ClassDescriptor in-class defaults (mirror of ClassRegistry.h)
DEFAULTS = dict(
    meleeAp=[0.0, 0.0, 0.0, 0.0],
    rangedAp=[0.0, 0.0, 1.0, -10.0],
    apUsesDruidForms=False,
    apUsesLightningReflexes=False,
    baseCrit=0.0,
    baseDodge=0.0,
    meleeCritFromAgiAtLevel1=0.0,
    meleeCritFromAgiAtLevel60=0.0,
    dodgeFromAgiAtLevel1=0.0,
    dodgeFromAgiAtLevel60=0.0,
    spellCritBase=0.0,
    spellCritRate0=0.0,
    spellCritRate1=10.0,
    hpPerSpirit=0.0,
    hpRegenFlat=0.0,
    mpSpiritDivisor=1.0,
    mpRegenFlat=0.0,
    casterDefault=False,
)


def parse_registry(path):
    text = path.read_text(encoding="utf-8", errors="replace")
    parsed = {}
    for m in re.finditer(
        r"ClassDescriptor&\s+d\s*=\s*m_descriptors\[CLASS_(\w+)\];(.*?)\n    \}",
        text, re.S,
    ):
        name, body = m.group(1), m.group(2)
        cid = CLASS_IDS[name]
        fields = {}
        for fm in re.finditer(r"d\.(\w+)\s*=\s*\{([^}]*)\};", body):
            fields[fm.group(1)] = [
                float(x.strip().rstrip("f")) for x in fm.group(2).split(",")
            ]
        for fm in re.finditer(r"d\.(\w+)\s*=\s*([^;]+);", body):
            field, val = fm.group(1), fm.group(2).strip()
            if field in fields:
                continue
            fields[field] = (val == "true") if val in ("true", "false") else float(val.rstrip("f"))
        parsed[cid] = fields
    return parsed


def merged(parsed, cid):
    d = dict(DEFAULTS)
    d.update(parsed.get(cid, {}))
    return d


# ---------------------------------------------------------------------------
# Constants hardcoded before the refactor (transcribed from 66132ca~1).
# ---------------------------------------------------------------------------

OLD_BASE_CRIT = {11: 0.9, 8: 3.2, 2: 0.7, 5: 3.0, 7: 1.7, 9: 2.0}
OLD_AGI_CRIT = {1: (3.9, 20.0), 2: (4.6, 20.0), 3: (3.5, 53.0), 4: (2.2, 29.0),
                5: (11.0, 20.0), 7: (4.6, 20.0), 8: (12.9, 20.0), 9: (8.4, 20.0),
                11: (4.6, 20.0)}
OLD_AGI_DODGE = {1: (3.9, 20.0), 2: (4.6, 20.0), 3: (1.8, 26.5), 4: (1.1, 14.5),
                 5: (11.0, 20.0), 7: (4.6, 20.0), 8: (12.9, 20.0), 9: (8.4, 20.0),
                 11: (4.6, 20.0)}
# crit_data rows (base, rate0, rate1); index = class id
OLD_SPELL_CRIT = {1: (0.0, 0.0, 10.0), 2: (3.70, 14.77, 0.65), 3: (0.0, 0.0, 10.0),
                  4: (0.0, 0.0, 10.0), 5: (2.97, 10.03, 0.82), 6: (0.0, 0.0, 10.0),
                  7: (3.54, 11.51, 0.80), 8: (3.70, 14.77, 0.65), 9: (3.18, 11.30, 0.82),
                  10: (0.0, 0.0, 10.0), 11: (3.33, 12.41, 0.79)}
OLD_HP_REGEN = {11: (0.11, 1.0), 3: (0.43, -5.5), 8: (0.11, 1.0), 2: (0.25, 0.0),
                5: (0.15, 1.4), 4: (0.84, -13.0), 7: (0.28, -3.6), 9: (0.12, 1.5),
                1: (1.26, -22.6)}
OLD_MP_REGEN = {11: (5.0, 15.0), 3: (5.0, 15.0), 8: (4.0, 12.5), 2: (5.0, 15.0),
                5: (4.0, 12.5), 7: (5.0, 17.0), 9: (5.0, 15.0)}
OLD_CASTER = {1: False, 4: False, 3: True, 5: True, 8: True, 9: True}


# ---------------------------------------------------------------------------
# float32 emulation (exact for multiply/add)
# ---------------------------------------------------------------------------

def f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def fmul(a, b):
    return f32(a * b)


def fadd(a, b):
    return f32(a + b)


def fsub(a, b):
    return f32(a - b)


def bits(x):
    return struct.pack("<f", f32(x))


failures = []


def check(label, ok, detail=""):
    status = "ok  " if ok else "FAIL"
    print(f"  [{status}] {label}{(' -> ' + detail) if detail and not ok else ''}")
    if not ok:
        failures.append(label)


def main():
    if not REGISTRY_CPP.exists():
        print(f"ClassRegistry.cpp not found at {REGISTRY_CPP}")
        return 2

    parsed = parse_registry(REGISTRY_CPP)
    if len(parsed) != 9:
        check("parsed 9 vanilla class blocks", False, f"got {len(parsed)}")
    else:
        check("parsed 9 vanilla class blocks", True)

    print("\n== constant diff (old hardcoded values vs descriptor table) ==")
    for cid in ALL_CLASSES:
        d = merged(parsed, cid)

        exp_crit = OLD_BASE_CRIT.get(cid, 0.0)
        check(f"class {cid}: baseCrit", d["baseCrit"] == exp_crit,
              f"{d['baseCrit']} != {exp_crit}")
        check(f"class {cid}: baseDodge", d["baseDodge"] == exp_crit,
              f"{d['baseDodge']} != {exp_crit}")

        exp_agi = OLD_AGI_CRIT.get(cid, (0.0, 0.0))
        check(f"class {cid}: agility crit rates",
              (d["meleeCritFromAgiAtLevel1"], d["meleeCritFromAgiAtLevel60"]) == exp_agi,
              f"{(d['meleeCritFromAgiAtLevel1'], d['meleeCritFromAgiAtLevel60'])} != {exp_agi}")
        exp_dodge = OLD_AGI_DODGE.get(cid, (0.0, 0.0))
        check(f"class {cid}: agility dodge rates",
              (d["dodgeFromAgiAtLevel1"], d["dodgeFromAgiAtLevel60"]) == exp_dodge,
              f"{(d['dodgeFromAgiAtLevel1'], d['dodgeFromAgiAtLevel60'])} != {exp_dodge}")

        if cid in OLD_SPELL_CRIT:
            exp_sc = OLD_SPELL_CRIT[cid]
            check(f"class {cid}: spell crit",
                  (d["spellCritBase"], d["spellCritRate0"], d["spellCritRate1"]) == exp_sc,
                  f"{(d['spellCritBase'], d['spellCritRate0'], d['spellCritRate1'])} != {exp_sc}")
        else:
            check(f"class {cid}: spell crit defaults",
                  (d["spellCritBase"], d["spellCritRate0"], d["spellCritRate1"]) == (0.0, 0.0, 10.0))

        exp_hp = OLD_HP_REGEN.get(cid, (0.0, 0.0))
        check(f"class {cid}: hp regen",
              (d["hpPerSpirit"], d["hpRegenFlat"]) == exp_hp,
              f"{(d['hpPerSpirit'], d['hpRegenFlat'])} != {exp_hp}")
        exp_mp = OLD_MP_REGEN.get(cid, (1.0, 0.0))
        check(f"class {cid}: mp regen",
              (d["mpSpiritDivisor"], d["mpRegenFlat"]) == exp_mp,
              f"{(d['mpSpiritDivisor'], d['mpRegenFlat'])} != {exp_mp}")

        if cid in OLD_CASTER:
            check(f"class {cid}: casterDefault",
                  d["casterDefault"] == OLD_CASTER[cid],
                  f"{d['casterDefault']} != {OLD_CASTER[cid]}")

    print("\n== attack power: old switch expression vs descriptor expression ==")

    def old_melee_ap(cid, level, s, a):
        if cid in (1, 2):
            return fsub(fadd(fmul(level, 3.0), fmul(s, 2.0)), 20.0)
        if cid in (4, 3):
            return fsub(fadd(fadd(fmul(level, 2.0), s), a), 20.0)
        if cid == 7:
            return fsub(fadd(fmul(level, 2.0), fmul(s, 2.0)), 20.0)
        if cid in (8, 5, 9):
            return fsub(s, 10.0)
        return 0.0  # no case in the old switch

    def old_ranged_ap(cid, level, s, a):
        if cid == 3:
            return fsub(fadd(fmul(level, 2.0), fmul(a, 2.0)), 10.0)
        if cid in (4, 1):
            return fsub(fadd(level, a), 10.0)
        return fsub(a, 10.0)  # old default

    def new_ap(f, level, s, a):
        # ((level*L + str*S) + agi*A) + flat
        v = fadd(fmul(level, f[0]), fmul(s, f[1]))
        v = fadd(v, fmul(a, f[2]))
        return fadd(v, f[3])

    levels = [1, 2, 10, 23, 40, 60]
    stats = [0.0, 1.0, 19.0, 20.0, 21.0, 50.0, 100.0, 200.0, 300.0]
    mismatch = 0
    for cid in ALL_CLASSES:
        if cid == 11:  # druid: special form code kept verbatim, not descriptor-driven
            continue
        d = merged(parsed, cid)
        for level in levels:
            for s in stats:
                for a in stats:
                    if bits(old_melee_ap(cid, level, s, a)) != bits(new_ap(d["meleeAp"], level, s, a)):
                        mismatch += 1
                    if bits(old_ranged_ap(cid, level, s, a)) != bits(new_ap(d["rangedAp"], level, s, a)):
                        mismatch += 1
    check(f"melee+ranged AP matrix ({len(levels)}x{len(stats)}x{len(stats)} per class)",
          mismatch == 0, f"{mismatch} mismatches")

    print("\n== caster / special flags ==")
    check("hunter lightning-reflexes flag",
          merged(parsed, 3)["apUsesLightningReflexes"] is True)
    check("druid form-AP flag", merged(parsed, 11)["apUsesDruidForms"] is True)
    for cid in (1, 2, 3, 4, 5, 7, 8, 9, 11):
        check(f"class {cid}: no stray special flags",
              not merged(parsed, cid)["apUsesDruidForms"] or cid == 11)
    for cid in (6, 10):
        check(f"class {cid}: unknown-class AP defaults",
              merged(parsed, cid)["meleeAp"] == [0.0, 0.0, 0.0, 0.0]
              and merged(parsed, cid)["rangedAp"] == [0.0, 0.0, 1.0, -10.0])

    print()
    if failures:
        print(f"RESULT: {len(failures)} difference(s) found.")
        return 1
    print("RESULT: no differences; descriptor table matches the pre-refactor constants")
    print("and the attack-power rewrite is bit-identical over the tested matrix.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
