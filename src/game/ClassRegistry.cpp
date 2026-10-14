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
