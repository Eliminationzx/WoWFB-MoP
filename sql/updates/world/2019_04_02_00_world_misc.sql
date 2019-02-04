-- Fix Stove rotation
DELETE FROM gameobject WHERE guid=527460 AND id=222792;
INSERT INTO `gameobject` VALUES ('527460', '222792', '870', '0', '0', '1', '1', '-787.753', '-5087.27', '2.05181', '3,56178', '0', '0', '0,978011', '-0,208552', '300', '255', '1', '0', '55');
DELETE FROM gameobject WHERE guid=527094 AND id=222793;
INSERT INTO `gameobject` VALUES ('527094', '222793', '870', '0', '0', '1', '1', '-614.774', '-4911.24', '2.05349', '1.37008', '0', '0', '0', '1', '120', '255', '1', '0', '57');

-- Immolation for Abyssal and Infernal
UPDATE creature_template_addon SET auras='19483' WHERE entry IN (89, 58997);

-- Fix npc 59000
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE (`entry`='59000');
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES ('59000', '0', '0', '0', '0', '0', '100', '0', '500', '1000', '2500', '3000', '11', '85692', '0', '0', '0', '0', '0', '2', '0', '0', '0', '0', '0', '0', '0', '');
