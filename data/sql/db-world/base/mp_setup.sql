-- Mythic Plus world setup.
-- NPC entry 500100. Does not touch creature 500000.

DELETE FROM `creature` WHERE `id` = 500100;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 500100;
DELETE FROM `creature_template` WHERE `entry` = 500100;

INSERT INTO `creature_template` (
    `entry`,
    `name`, `subname`,
    `minlevel`, `maxlevel`,
    `faction`, `npcflag`,
    `speed_walk`, `speed_run`,
    `unit_class`, `unit_flags`, `unit_flags2`,
    `type`, `flags_extra`,
    `ScriptName`
) VALUES (
    500100,
    'Mythic Keystone',
    'Choose a dungeon',
    80, 80,
    190,
    1,
    1.0, 1.14286,
    1,
    2,
    0,
    7,
    2,
    'npc_mythic_plus'
);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`
) VALUES (
    500100, 0, 3167, 1.2, 1.0
);

INSERT INTO `creature` (
    `guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`,
    `spawntimesecs`
) VALUES
(8000101, 500100, 0, 1, 1,  -8838.0, 626.0, 94.30, 0.44, 180),
(8000102, 500100, 0, 1, 1,  -4914.0, -957.0, 501.53, 2.26, 180),
(8000103, 500100, 1, 1, 1,  9873.0, 2494.0, 1316.20, 3.1, 180),
(8000104, 500100, 530, 1, 1,  -3858.7, -11645.8, -137.63, 2.38, 180),
(8000105, 500100, 1, 1, 1,  1680.0, -4316.0, 61.80, 5.7, 180),
(8000106, 500100, 1, 1, 1,  -1273.6, 73.0, 128.75, 5.96, 180),
(8000107, 500100, 0, 1, 1,  1641.2, 240.1, -43.10, 3.14, 180),
(8000108, 500100, 530, 1, 1,  9742.0, -7454.0, 13.56, 0.5, 180),
(8000109, 500100, 530, 1, 1,  -1846.0, 5436.0, -12.10, 5.5, 180),
(8000110, 500100, 571, 1, 1,  5811.0, 506.2, 657.58, 5.54, 180),
(8000111, 500100, 0, 1, 1,  -14402.0, 420.0, 23.70, 4.0, 180);
