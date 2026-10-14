/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ClassRegistry.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"

ClassRegistry sClassRegistry;

ClassRegistry::ClassRegistry() : m_playableMask(CLASSMASK_ALL_PLAYABLE)
{
    InitVanillaDescriptors();
}

// The values below are a literal translation of the original hardcoded
// class switches (StatSystem.cpp, Player.cpp, Unit.cpp).  The struct
// defaults already reproduce the "no case in the switch" behavior, so only
// vanilla classes need explicit values.
void ClassRegistry::InitVanillaDescriptors()
{
    // Warrior
    {
        ClassDescriptor& d = m_descriptors[CLASS_WARRIOR];
        d.meleeAp = { 3.0f, 2.0f, 0.0f, -20.0f };
        d.rangedAp = { 1.0f, 0.0f, 1.0f, -10.0f };
        d.meleeCritFromAgiAtLevel1 = 3.9f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 3.9f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.hpPerSpirit = 1.26f;
        d.hpRegenFlat = -22.6f;
    }

    // Paladin
    {
        ClassDescriptor& d = m_descriptors[CLASS_PALADIN];
        d.meleeAp = { 3.0f, 2.0f, 0.0f, -20.0f };
        d.baseCrit = 0.7f;
        d.baseDodge = 0.7f;
        d.meleeCritFromAgiAtLevel1 = 4.6f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 4.6f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.spellCritBase = 3.70f;
        d.spellCritRate0 = 14.77f;
        d.spellCritRate1 = 0.65f;
        d.hpPerSpirit = 0.25f;
        d.mpSpiritDivisor = 5.0f;
        d.mpRegenFlat = 15.0f;
    }

    // Hunter
    {
        ClassDescriptor& d = m_descriptors[CLASS_HUNTER];
        d.meleeAp = { 2.0f, 1.0f, 1.0f, -20.0f };
        d.rangedAp = { 2.0f, 0.0f, 2.0f, -10.0f };
        d.apUsesLightningReflexes = true;
        d.meleeCritFromAgiAtLevel1 = 3.5f;
        d.meleeCritFromAgiAtLevel60 = 53.0f;
        d.dodgeFromAgiAtLevel1 = 1.8f;
        d.dodgeFromAgiAtLevel60 = 26.5f;
        d.hpPerSpirit = 0.43f;
        d.hpRegenFlat = -5.5f;
        d.mpSpiritDivisor = 5.0f;
        d.mpRegenFlat = 15.0f;
        d.casterDefault = true;
    }

    // Rogue
    {
        ClassDescriptor& d = m_descriptors[CLASS_ROGUE];
        d.meleeAp = { 2.0f, 1.0f, 1.0f, -20.0f };
        d.rangedAp = { 1.0f, 0.0f, 1.0f, -10.0f };
        d.meleeCritFromAgiAtLevel1 = 2.2f;
        d.meleeCritFromAgiAtLevel60 = 29.0f;
        d.dodgeFromAgiAtLevel1 = 1.1f;
        d.dodgeFromAgiAtLevel60 = 14.5f;
        d.hpPerSpirit = 0.84f;
        d.hpRegenFlat = -13.0f;
    }

    // Priest
    {
        ClassDescriptor& d = m_descriptors[CLASS_PRIEST];
        d.meleeAp = { 0.0f, 1.0f, 0.0f, -10.0f };
        d.baseCrit = 3.0f;
        d.baseDodge = 3.0f;
        d.meleeCritFromAgiAtLevel1 = 11.0f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 11.0f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.spellCritBase = 2.97f;
        d.spellCritRate0 = 10.03f;
        d.spellCritRate1 = 0.82f;
        d.hpPerSpirit = 0.15f;
        d.hpRegenFlat = 1.4f;
        d.mpSpiritDivisor = 4.0f;
        d.mpRegenFlat = 12.5f;
        d.casterDefault = true;
    }

    // Shaman
    {
        ClassDescriptor& d = m_descriptors[CLASS_SHAMAN];
        d.meleeAp = { 2.0f, 2.0f, 0.0f, -20.0f };
        d.baseCrit = 1.7f;
        d.baseDodge = 1.7f;
        d.meleeCritFromAgiAtLevel1 = 4.6f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 4.6f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.spellCritBase = 3.54f;
        d.spellCritRate0 = 11.51f;
        d.spellCritRate1 = 0.80f;
        d.hpPerSpirit = 0.28f;
        d.hpRegenFlat = -3.6f;
        d.mpSpiritDivisor = 5.0f;
        d.mpRegenFlat = 17.0f;
    }

    // Mage
    {
        ClassDescriptor& d = m_descriptors[CLASS_MAGE];
        d.meleeAp = { 0.0f, 1.0f, 0.0f, -10.0f };
        d.baseCrit = 3.2f;
        d.baseDodge = 3.2f;
        d.meleeCritFromAgiAtLevel1 = 12.9f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 12.9f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.spellCritBase = 3.70f;
        d.spellCritRate0 = 14.77f;
        d.spellCritRate1 = 0.65f;
        d.hpPerSpirit = 0.11f;
        d.hpRegenFlat = 1.0f;
        d.mpSpiritDivisor = 4.0f;
        d.mpRegenFlat = 12.5f;
        d.casterDefault = true;
    }

    // Warlock
    {
        ClassDescriptor& d = m_descriptors[CLASS_WARLOCK];
        d.meleeAp = { 0.0f, 1.0f, 0.0f, -10.0f };
        d.baseCrit = 2.0f;
        d.baseDodge = 2.0f;
        d.meleeCritFromAgiAtLevel1 = 8.4f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 8.4f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.spellCritBase = 3.18f;
        d.spellCritRate0 = 11.30f;
        d.spellCritRate1 = 0.82f;
        d.hpPerSpirit = 0.12f;
        d.hpRegenFlat = 1.5f;
        d.mpSpiritDivisor = 5.0f;
        d.mpRegenFlat = 15.0f;
        d.casterDefault = true;
    }

    // Druid
    {
        ClassDescriptor& d = m_descriptors[CLASS_DRUID];
        d.apUsesDruidForms = true;
        d.baseCrit = 0.9f;
        d.baseDodge = 0.9f;
        d.meleeCritFromAgiAtLevel1 = 4.6f;
        d.meleeCritFromAgiAtLevel60 = 20.0f;
        d.dodgeFromAgiAtLevel1 = 4.6f;
        d.dodgeFromAgiAtLevel60 = 20.0f;
        d.spellCritBase = 3.33f;
        d.spellCritRate0 = 12.41f;
        d.spellCritRate1 = 0.79f;
        d.hpPerSpirit = 0.11f;
        d.hpRegenFlat = 1.0f;
        d.mpSpiritDivisor = 5.0f;
        d.mpRegenFlat = 15.0f;
    }
}

void ClassRegistry::LoadPlayableClasses()
{
    // Start from the built-in vanilla mask; the table can only add to it.
    m_playableMask = CLASSMASK_ALL_PLAYABLE;

    // Check table existence first so servers without the table do not log
    // SQL errors (same pattern as ObjectMgr::LoadModuleStrings).
    std::unique_ptr<QueryResult> tableResult(WorldDatabase.PQuery("SHOW TABLES LIKE 'playable_class'"));
    if (!tableResult || tableResult->GetRowCount() == 0)
    {
        sLog.outString(">> `playable_class` table not present; using built-in playable class mask 0x%X", m_playableMask);
        return;
    }

    uint32 count = 0;
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `id` FROM `playable_class`"));
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 classId = fields[0].GetUInt32();

        if (classId == 0 || classId >= MAX_CLASSES)
        {
            sLog.outErrorDb("Table `playable_class`: ignoring invalid class id %u.", classId);
            continue;
        }

        if (m_playableMask & (1u << (classId - 1)))
        {
            sLog.outErrorDb("Table `playable_class`: class %u is already part of the built-in playable mask.", classId);
            continue;
        }

        m_playableMask |= (1u << (classId - 1));
        ++count;
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u additional playable class(es) from `playable_class` (mask 0x%X)", count, m_playableMask);
}

void ClassRegistry::LoadDescriptors()
{
    // Optional override table: NULL columns keep the built-in value, so a
    // module only lists the fields it wants to change.
    std::unique_ptr<QueryResult> tableResult(WorldDatabase.PQuery("SHOW TABLES LIKE 'class_descriptor'"));
    if (!tableResult || tableResult->GetRowCount() == 0)
        return;

    std::unique_ptr<QueryResult> result(WorldDatabase.Query(
        "SELECT `class`,"
        "`ap_melee_level`,`ap_melee_str`,`ap_melee_agi`,`ap_melee_flat`,"
        "`ap_ranged_level`,`ap_ranged_str`,`ap_ranged_agi`,`ap_ranged_flat`,"
        "`ap_druid_forms`,`ap_lightning_reflexes`,"
        "`base_crit`,`base_dodge`,"
        "`melee_crit_agi_level1`,`melee_crit_agi_level60`,"
        "`dodge_agi_level1`,`dodge_agi_level60`,"
        "`spell_crit_base`,`spell_crit_rate0`,`spell_crit_rate1`,"
        "`hp_per_spirit`,`hp_regen_flat`,`mp_spirit_divisor`,`mp_regen_flat`,"
        "`caster_default`"
        " FROM `class_descriptor`"));
    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 classId = fields[0].GetUInt32();

        if (classId == 0 || classId >= MAX_CLASSES)
        {
            sLog.outErrorDb("Table `class_descriptor`: ignoring invalid class id %u.", classId);
            continue;
        }

        ClassDescriptor* d = GetMutableDescriptor(classId);
        if (!d)
            continue;

        auto applyFloat = [&](int idx, float& target)
        {
            if (!fields[idx].IsNULL())
                target = fields[idx].GetFloat();
        };
        auto applyBool = [&](int idx, bool& target)
        {
            if (!fields[idx].IsNULL())
                target = fields[idx].GetBool();
        };

        applyFloat(1,  d->meleeAp.levelCoef);
        applyFloat(2,  d->meleeAp.strengthCoef);
        applyFloat(3,  d->meleeAp.agilityCoef);
        applyFloat(4,  d->meleeAp.flat);
        applyFloat(5,  d->rangedAp.levelCoef);
        applyFloat(6,  d->rangedAp.strengthCoef);
        applyFloat(7,  d->rangedAp.agilityCoef);
        applyFloat(8,  d->rangedAp.flat);
        applyBool(9,   d->apUsesDruidForms);
        applyBool(10,  d->apUsesLightningReflexes);
        applyFloat(11, d->baseCrit);
        applyFloat(12, d->baseDodge);
        applyFloat(13, d->meleeCritFromAgiAtLevel1);
        applyFloat(14, d->meleeCritFromAgiAtLevel60);
        applyFloat(15, d->dodgeFromAgiAtLevel1);
        applyFloat(16, d->dodgeFromAgiAtLevel60);
        applyFloat(17, d->spellCritBase);
        applyFloat(18, d->spellCritRate0);
        applyFloat(19, d->spellCritRate1);
        applyFloat(20, d->hpPerSpirit);
        applyFloat(21, d->hpRegenFlat);
        applyFloat(22, d->mpSpiritDivisor);
        applyFloat(23, d->mpRegenFlat);
        applyBool(24,  d->casterDefault);

        ++count;
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u class descriptor override(s) from `class_descriptor`", count);
}
