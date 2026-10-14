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

// Runtime registry of the classes the core treats as playable.
//
// Vanilla hardcodes the playable classes in CLASSMASK_ALL_PLAYABLE.  This
// registry keeps that mask as its built-in default but lets the world
// database (the `playable_class` table) extend it, so module-provided
// classes can be enabled without patching the core.  With an empty (or
// absent) table the mask is exactly the vanilla one and behavior is
// unchanged.
//
// The registry must be loaded before DBC and player-info loading because
// the playable mask gates data loading (see World::SetInitialWorldSettings).
class ClassRegistry
{
    public:
        ClassRegistry() : m_playableMask(CLASSMASK_ALL_PLAYABLE) {}

        // Restores the built-in vanilla mask and applies the
        // `playable_class` table on top of it.
        void LoadPlayableClasses();

        // Mask of the classes currently considered playable.
        uint32 GetPlayableMask() const { return m_playableMask; }

        // True when `classId` is a valid class index that the core treats
        // as playable (built-in classes plus any added via the database).
        bool IsPlayableClass(uint32 classId) const
        {
            return classId > 0 && classId < MAX_CLASSES &&
                (m_playableMask & (1u << (classId - 1))) != 0;
        }

    private:
        uint32 m_playableMask;
};

extern ClassRegistry sClassRegistry;

#endif
