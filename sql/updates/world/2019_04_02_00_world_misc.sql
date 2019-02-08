-- Fix Stove rotation
DELETE FROM gameobject WHERE guid=527460 AND id=222792;
INSERT INTO `gameobject` VALUES ('527460', '222792', '870', '0', '0', '1', '1', '-787.753', '-5087.27', '2.05181', '3,56178', '0', '0', '0,978011', '-0,208552', '300', '255', '1', '0', '55');
DELETE FROM gameobject WHERE guid=527094 AND id=222793;
INSERT INTO `gameobject` VALUES ('527094', '222793', '870', '0', '0', '1', '1', '-614.774', '-4911.24', '2.05349', '1.37008', '0', '0', '0', '1', '120', '255', '1', '0', '57');

-- Immolation for Abyssal and Infernal
UPDATE creature_template_addon SET auras='19483' WHERE entry IN (89, 58997);

-- Fix npc 59000
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_terror_and_doom_guard' WHERE (`entry`='59000');
UPDATE `creature_template` SET `ScriptName`='npc_terror_and_doom_guard' WHERE (`entry`='11859');

-- delet dublicat
DELETE FROM `gameobject` WHERE `guid` in (527210, 527215, 527165, 527180, 527238, 527433, 527239, 527240, 527439, 527242, 527304, 527473, 527318, 527252, 527412, 527415, 527423, 527213, 527211, 527432);

UPDATE creature_template SET vehicleId = 2119 WHERE entry IN(60378, 60379, 60380); -- mini-map icon

UPDATE access_requirement SET level_min = 90 WHERE mapId=1098; -- Throne of Thunder