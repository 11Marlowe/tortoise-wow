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
