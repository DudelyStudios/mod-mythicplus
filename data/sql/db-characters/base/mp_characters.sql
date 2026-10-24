-- Mythic+ character data. Apply this to the characters database.

CREATE TABLE IF NOT EXISTS `mp_key` (
  `guid` INT UNSIGNED NOT NULL,
  `key_level` INT UNSIGNED NOT NULL DEFAULT 2,
  `week_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `finishes` INT UNSIGNED NOT NULL DEFAULT 0,
  `best_timed` INT UNSIGNED NOT NULL DEFAULT 0,
  `any_timed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `vault_level` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `mp_vault` (
  `guid` INT UNSIGNED NOT NULL,
  `week_id` INT UNSIGNED NOT NULL,
  `vault_key` INT UNSIGNED NOT NULL,
  `choices` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `claimed` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`, `week_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `mp_dungeon_best` (
  `guid` INT UNSIGNED NOT NULL,
  `season_id` INT UNSIGNED NOT NULL,
  `map_id` INT UNSIGNED NOT NULL,
  `best_key` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`guid`, `season_id`, `map_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `mp_leaderboard` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `guid` INT UNSIGNED NOT NULL,
  `char_name` VARCHAR(12) NOT NULL,
  `map_id` INT UNSIGNED NOT NULL,
  `theme_id` INT UNSIGNED NOT NULL,
  `key_level` INT UNSIGNED NOT NULL,
  `clear_time` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_map_time` (`map_id`, `clear_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `mp_season` (
  `id` TINYINT UNSIGNED NOT NULL,
  `season_id` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `mp_season` (`id`, `season_id`) VALUES (1, 1);
