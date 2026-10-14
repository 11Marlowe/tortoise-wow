-- FILE: playable_class.sql
-- Runtime playable-class registry (module extension point).  Classes listed
-- here are treated as playable by the core's player-info loaders in addition
-- to the built-in vanilla classes.  Module-provided classes insert their
-- class id here; an empty table means "vanilla classes only" (no behavior
-- change).  Loaded by ClassRegistry::LoadPlayableClasses() before DBC and
-- player-info loading.

CREATE TABLE IF NOT EXISTS `playable_class` (
  `id` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
