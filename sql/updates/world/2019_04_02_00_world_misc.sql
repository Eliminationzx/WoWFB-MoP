-- Fix Stove rotation
DELETE FROM gameobject WHERE guid=527460 AND id=222792;
INSERT INTO `gameobject` VALUES ('527460', '222792', '870', '0', '0', '1', '1', '-787.753', '-5087.27', '2.05181', '3,56178', '0', '0', '0,978011', '-0,208552', '300', '255', '1', '0', '55');
DELETE FROM gameobject WHERE guid=527094 AND id=222793;
INSERT INTO `gameobject` VALUES ('527094', '222793', '870', '0', '0', '1', '1', '-614.774', '-4911.24', '2.05349', '1.37008', '0', '0', '0', '1', '120', '255', '1', '0', '57');

-- Immolation for Abyssal and Infernal
UPDATE creature_template_addon SET auras='19483' WHERE entry IN (89, 58997);