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

#ifndef CLASS_REGISTRY_H
#define CLASS_REGISTRY_H

#include "Common.h"
#include "SharedDefines.h"

#include <array>

// Attack power formula: ap = level*levelCoef + strength*strengthCoef
//                            + agility*agilityCoef + flat
struct AttackPowerFormula
{
    float levelCoef;
    float strengthCoef;
    float agilityCoef;
    float flat;
};

// Per-class stat formulas used by the players' stat calculations.
//
// The built-in values reproduce the original hardcoded class switches
// exactly.  Module-provided classes do not need core edits: their behavior
// is described by a descriptor (registered before world loading, see
// ClassRegistry::GetMutableDescriptor).
struct ClassDescriptor
{
    uint32 classId = 0;

    // Melee/ranged attack power
    AttackPowerFormula meleeAp = { 0.0f, 0.0f, 0.0f, 0.0f };
    AttackPowerFormula rangedAp = { 0.0f, 0.0f, 1.0f, -10.0f };

    // Vanilla quirk gates (the generic coefficients cannot express these)
    bool apUsesDruidForms = false;          // AP depends on shapeshift form
    bool apUsesLightningReflexes = false;   // hunter agility melee-AP talent

    // Flat crit/dodge percentages added by class (before defense-skill
    // corrections)
    float baseCrit = 0.0f;
    float baseDodge = 0.0f;

    // Crit/dodge gained from agility, linearly interpolated between level 1
    // and 60:  rate = l1*(60-level)/59 + l60*(level-1)/59;  result = agi/rate
    float meleeCritFromAgiAtLevel1 = 0.0f;
    float meleeCritFromAgiAtLevel60 = 0.0f;
    float dodgeFromAgiAtLevel1 = 0.0f;
    float dodgeFromAgiAtLevel60 = 0.0f;

    // Spell crit from intellect: chance = base + intellect / (rate0 + rate1*level)
    float spellCritBase = 0.0f;
    float spellCritRate0 = 0.0f;
    float spellCritRate1 = 10.0f;

    // Spirit regeneration:
    //   HP per tick: max(0, spirit * hpPerSpirit + hpRegenFlat)
    //   MP per tick: (spirit / mpSpiritDivisor + mpRegenFlat) / 2
    float hpPerSpirit = 0.0f;
    float hpRegenFlat = 0.0f;
    float mpSpiritDivisor = 1.0f;
    float mpRegenFlat = 0.0f;

    // Answer of Unit::IsCaster for classes without special-case rules
    bool casterDefault = false;
};

// Runtime registry of the classes the core treats as playable, plus their
// stat descriptors.
//
// Vanilla hardcodes the playable classes in CLASSMASK_ALL_PLAYABLE and the
// stat formulas in switch statements.  This registry keeps the vanilla
// values as built-in defaults, lets the world database (the `playable_class`
// table) extend the mask, and exposes the descriptors for module-provided
// classes.  With an empty (or absent) table behavior is unchanged.
//
// The registry must be loaded before DBC and player-info loading because
// the playable mask gates data loading (see World::SetInitialWorldSettings).
class ClassRegistry
{
    public:
        ClassRegistry();

        // Restores the built-in vanilla mask and applies the
        // `playable_class` table on top of it.
        void LoadPlayableClasses();

        // Applies optional per-class descriptor overrides from the
        // `class_descriptor` table (NULL columns keep the built-in value).
        void LoadDescriptors();

        // Mask of the classes currently considered playable.
        uint32 GetPlayableMask() const { return m_playableMask; }

        // True when `classId` is a valid class index that the core treats
        // as playable (built-in classes plus any added via the database).
        bool IsPlayableClass(uint32 classId) const
        {
            return classId > 0 && classId < MAX_CLASSES &&
                (m_playableMask & (1u << (classId - 1))) != 0;
        }

        // Stat descriptor for a class id; never null for classId < MAX_CLASSES.
        ClassDescriptor const* GetDescriptor(uint32 classId) const
        {
            return classId < MAX_CLASSES ? &m_descriptors[classId] : nullptr;
        }

        // Mutable access for module registration of custom classes.
        ClassDescriptor* GetMutableDescriptor(uint32 classId)
        {
            return classId < MAX_CLASSES ? &m_descriptors[classId] : nullptr;
        }

    private:
        // Fills the built-in descriptors with the vanilla class formulas.
        void InitVanillaDescriptors();

        uint32 m_playableMask;
        std::array<ClassDescriptor, MAX_CLASSES> m_descriptors;
};

extern ClassRegistry sClassRegistry;

#endif
