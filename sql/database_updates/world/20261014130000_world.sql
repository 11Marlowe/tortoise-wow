-- FILE: class_descriptor.sql
-- Optional overrides for the per-class stat descriptors (module extension
-- point).  NULL columns keep the built-in value; a row therefore only lists
-- the fields a module wants to change.  Loaded by
-- ClassRegistry::LoadDescriptors() before DBC and player-info loading.

CREATE TABLE IF NOT EXISTS `class_descriptor` (
  `class` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `ap_melee_level` float DEFAULT NULL,
  `ap_melee_str` float DEFAULT NULL,
  `ap_melee_agi` float DEFAULT NULL,
  `ap_melee_flat` float DEFAULT NULL,
  `ap_ranged_level` float DEFAULT NULL,
  `ap_ranged_str` float DEFAULT NULL,
  `ap_ranged_agi` float DEFAULT NULL,
  `ap_ranged_flat` float DEFAULT NULL,
  `ap_druid_forms` tinyint(1) DEFAULT NULL,
  `ap_lightning_reflexes` tinyint(1) DEFAULT NULL,
  `base_crit` float DEFAULT NULL,
  `base_dodge` float DEFAULT NULL,
  `melee_crit_agi_level1` float DEFAULT NULL,
  `melee_crit_agi_level60` float DEFAULT NULL,
  `dodge_agi_level1` float DEFAULT NULL,
  `dodge_agi_level60` float DEFAULT NULL,
  `spell_crit_base` float DEFAULT NULL,
  `spell_crit_rate0` float DEFAULT NULL,
  `spell_crit_rate1` float DEFAULT NULL,
  `hp_per_spirit` float DEFAULT NULL,
  `hp_regen_flat` float DEFAULT NULL,
  `mp_spirit_divisor` float DEFAULT NULL,
  `mp_regen_flat` float DEFAULT NULL,
  `caster_default` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`class`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
