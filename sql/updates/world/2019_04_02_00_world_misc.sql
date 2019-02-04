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
DELETE FROM `gameobject` WHERE `guid` in (527670, 527940, 528077, 527758, 527845, 527954, 528060, 527761, 527866, 527932, 528072, 528043, 527869, 527936, 527824, 527821, 527827, 527774, 527828, 527789, 527855, 528061, 527856, 527857, 528067, 527859, 527926, 527927, 527918, 527840, 527916, 528106, 528104, 527964, 527944, 527871, 528040, 527921, 528051, 527825, 527822, 527909, 527815, 527770, 527784, 527841, 527842, 527843, 528031, 527903, 527897, 527828, 527895, 528066, 528064, 527933, 527915, 527852, 528008, 528017, 527816);

