-- Правки к детской недели
DELETE FROM `spell_script_names` WHERE `spell_id` IN (23012, 23013);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(23012, 'spell_cw_summon_orphan'),
(23013, 'spell_cw_summon_orphan');

UPDATE `creature_template` SET `npcflag`=1 WHERE `entry`=52585;
UPDATE `creature_template` SET `unit_flags`=768, `InhabitType`=3, `ScriptName`='npc_cw_redhound_twoseater_trigger' WHERE `entry`=52585;
UPDATE `creature_template` SET `minlevel`=5, `maxlevel`=5, `unit_flags`=768, `VehicleId`=1577, `InhabitType`=4, `ScriptName`='npc_cw_redhound_twoseater_vehicle' WHERE `entry`=52583;

DELETE FROM `script_waypoint` WHERE `entry` = '52583';
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `point_comment`) VALUES
(52583, 1, 2655.08, -6138.38, 131.92, 'Redhound Two-Seater (CW)'),
(52583, 2, 2646.82, -6110.06, 123.04, 'Redhound Two-Seater (CW)'),
(52583, 3, 2636.78, -6062.60, 100.38, 'Redhound Two-Seater (CW)'),
(52583, 4, 2629.27, -6009.33, 100.48, 'Redhound Two-Seater (CW)'),
(52583, 5, 2684.69, -5880.80, 97.14, 'Redhound Two-Seater (CW)'),
(52583, 6, 2706.82, -5847.92, 98.26, 'Redhound Two-Seater (CW)'),
(52583, 7, 2780.98, -5770.83, 111.83, 'Redhound Two-Seater (CW)'),
(52583, 8, 2802.95, -5733.89, 123.47, 'Redhound Two-Seater (CW)'),
(52583, 9, 2806.83, -5727.36, 125.16, 'Redhound Two-Seater (CW)'),
(52583, 10, 2824.59, -5686.51, 127.83, 'Redhound Two-Seater (CW)'),
(52583, 11, 2830.56, -5642.52, 127.36, 'Redhound Two-Seater (CW)'),
(52583, 12, 2840.71, -5529.31, 134.45, 'Redhound Two-Seater (CW)'),
(52583, 13, 2836.16, -5466.06, 143.58, 'Redhound Two-Seater (CW)'),
(52583, 14, 2840.19, -5418.35, 141.40, 'Redhound Two-Seater (CW)'),
(52583, 15, 2858.93, -5380.91, 132.63, 'Redhound Two-Seater (CW)'),
(52583, 16, 2904.34, -5310.00, 129.12, 'Redhound Two-Seater (CW)'),
(52583, 17, 2909.12, -5263.02, 134.93, 'Redhound Two-Seater (CW)'),
(52583, 18, 2908.08, -5223.09, 129.86, 'Redhound Two-Seater (CW)'),
(52583, 19, 2926.11, -5129.59, 131.76, 'Redhound Two-Seater (CW)'),
(52583, 20, 2930.41, -5065.10, 165.60, 'Redhound Two-Seater (CW)'),
(52583, 21, 2969.12, -4951.13, 166.10, 'Redhound Two-Seater (CW)'),
(52583, 22, 2985.53, -4930.43, 163.99, 'Redhound Two-Seater (CW)'),
(52583, 23, 3002.53, -4909.65, 162.17, 'Redhound Two-Seater (CW)'),
(52583, 24, 3022.45, -4880.55, 162.17, 'Redhound Two-Seater (CW)'),
(52583, 25, 3047.28, -4874.44, 142.96, 'Redhound Two-Seater (CW)'),
(52583, 26, 3130.56, -4933.71, 144.99, 'Redhound Two-Seater (CW)'),
(52583, 27, 3166.46, -4901.59, 144.99, 'Redhound Two-Seater (CW)'),
(52583, 28, 3165.47, -4774.27, 170.64, 'Redhound Two-Seater (CW)'),
(52583, 29, 3097.51, -4787.22, 170.61, 'Redhound Two-Seater (CW)'),
(52583, 30, 3094.41, -4825.83, 170.61, 'Redhound Two-Seater (CW)'),
(52583, 31, 3119.27, -4789.89, 170.61, 'Redhound Two-Seater (CW)'),
(52583, 32, 3014.86, -4805.83, 156.02, 'Redhound Two-Seater (CW)'),
(52583, 33, 3022.45, -4845.50, 156.56, 'Redhound Two-Seater (CW)'),
(52583, 34, 3042.77, -4848.20, 142.08, 'Redhound Two-Seater (CW)'),
(52583, 35, 3042.77, -4848.20, 126.44, 'Redhound Two-Seater (CW)'),
(52583, 36, 3077.72, -4850.58, 149.95, 'Redhound Two-Seater (CW)'),
(52583, 37, 3101.51, -4852.77, 143.87, 'Redhound Two-Seater (CW)'),
(52583, 38, 3185.97, -4839.00, 168.80, 'Redhound Two-Seater (CW)'),
(52583, 39, 3409.25, -4812.11, 153.66, 'Redhound Two-Seater (CW)'),
(52583, 40, 3501.25, -4793.60, 159.12, 'Redhound Two-Seater (CW)'),
(52583, 41, 3613.21, -4794.11, 157.88, 'Redhound Two-Seater (CW)');

DELETE FROM `creature_text` WHERE `entry`=14444;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(14444,0,0,'This is gonna be fun!',12,0,100,0,0,0,'Comment'),
(14444,1,0,'I\'ve seen goblins riding this things. I know they can go faster!',12,0,100,0,0,0,'Comment'),
(14444,2,0,'I wonder what this button does...',12,0,100,0,0,0,'Comment'),
(14444,3,0,'Now THAT\'s more like it!',12,0,100,0,0,0,'Comment'),
(14444,4,0,'I\'ve figured out how this works. I\'m taking control!',12,0,100,0,0,0,'Comment'),
(14444,5,0,'Hang on onto your, um... helmet!',12,0,100,0,0,0,'Comment'),
(14444,6,0,'I\'m loosing control!',12,0,100,0,0,0,'Comment'),
(14444,7,0,'No, rocket! Don\'t go that way!',12,0,100,0,0,0,'Comment'),
(14444,8,0,'I think I found the emergency button. Rockets have those, don\'t they?',12,0,100,0,0,0,'Comment'),
(14444,9,0,'I did it! I Got us back to the track!',12,0,100,0,0,0,'Comment'),
(14444,10,0,'This is the only button I haven\'t pushed yet. Let\'s give it a try!',12,0,100,0,0,0,'Comment'),
(14444,11,0,'Whooooooooooa!',12,0,100,0,0,0,'Comment'),
(14444,12,0,'Wow, so you\'re the Banshee Queen? I kind of expected you to be taller!',12,0,100,0,0,0,'Comment'),
(14444,13,0,'Everyone says you had a fight with the warchief and you\'re going to abandon the Horde. Is that true?',12,0,100,0,0,0,'Comment'),
(14444,14,0,'So it\'s not true? Thank the ancestors! I wouldn\'t want you to be my enemy. Lady Sylvanas.',12,0,100,0,0,0,'Comment'),
(14444,15,0,'So this is how a chieftain is honored!',12,0,100,0,0,0,'Comment');

UPDATE `creature_template` SET `minlevel`=5, `maxlevel`=5, `npcflag`=1, `unit_flags`=768, `ScriptName`='npc_cw_rental_chopper_trigger' WHERE `entry`=52189;
UPDATE `creature_template` SET `minlevel`=5, `maxlevel`=5, `unit_flags`=768, `VehicleId`=335, `ScriptName`='npc_cw_rental_chopper_vehicle' WHERE `entry`=52188;

DELETE FROM `script_waypoint` WHERE `entry` = '52188';
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `point_comment`) VALUES
(52188, 1, -10640.20, 1001.94, 32.82, 'Rental Chopper (CW)'),
(52188, 2, -10589.90, 989.42, 36.89, 'Rental Chopper (CW)'),
(52188, 3, -10522.21, 956.56, 41.08, 'Rental Chopper (CW)'),
(52188, 4, -10478.94, 952.79, 36.90, 'Rental Chopper (CW)'),
(52188, 5, -10460.65, 958.80, 35.24, 'Rental Chopper (CW)'),
(52188, 6, -10445.83, 970.43, 35.15, 'Rental Chopper (CW)'),
(52188, 7, -10430.26, 1073.13, 38.14, 'Rental Chopper (CW)'),
(52188, 8, -10425.89, 1104.92, 33.14, 'Rental Chopper (CW)'),
(52188, 9, -10420.99, 1150.15, 34.76, 'Rental Chopper (CW)'),
(52188, 10, -10424.04, 1212.71, 44.17, 'Rental Chopper (CW)'),
(52188, 11, -10367.50, 1297.64, 39.65, 'Rental Chopper (CW)'),
(52188, 12, -10347.39, 1392.32, 42.19, 'Rental Chopper (CW)'),
(52188, 13, -10338.14, 1464.01, 38.07, 'Rental Chopper (CW)'),
(52188, 14, -10298.95, 1530.28, 36.34, 'Rental Chopper (CW)'),
(52188, 15, -10286.43, 1591.41, 32.66, 'Rental Chopper (CW)'),
(52188, 16, -10267.06, 1689.00, 29.20, 'Rental Chopper (CW)'),
(52188, 17, -10266.66, 1702.39, 30.22, 'Rental Chopper (CW)'),
(52188, 18, -10257.89, 1805.32, 39.04, 'Rental Chopper (CW)'),
(52188, 19, -10271.18, 1831.94, 37.60, 'Rental Chopper (CW)'),
(52188, 20, -10343.12, 1843.74, 35.41, 'Rental Chopper (CW)'),
(52188, 21, -10374.55, 1845.68, 34.97, 'Rental Chopper (CW)'),
(52188, 22, -10425.91, 1840.07, 28.51, 'Rental Chopper (CW)'),
(52188, 23, -10449.71, 1809.51, 26.17, 'Rental Chopper (CW)'),
(52188, 24, -10454.22, 1760.43, -11.17, 'Rental Chopper (CW)'),
(52188, 25, -10463.60, 1741.76, -9.31, 'Rental Chopper (CW)'),
(52188, 26, -10479.39, 1725.47, -5.49, 'Rental Chopper (CW)'),
(52188, 27, -10485.98, 1687.28, -8.02, 'Rental Chopper (CW)'),
(52188, 28, -10472.75, 1643.07, -6.03, 'Rental Chopper (CW)'),
(52188, 29, -10474.73, 1614.02, 0.89, 'Rental Chopper (CW)'),
(52188, 30, -10436.75, 1591.60, -10.88, 'Rental Chopper (CW)'),
(52188, 31, -10429.54, 1595.66, -13.51, 'Rental Chopper (CW)'),
(52188, 32, -10408.86, 1612.89, -32.84, 'Rental Chopper (CW)'),
(52188, 33, -10393.64, 1637.09, -37.52, 'Rental Chopper (CW)'),
(52188, 34, -10391.53, 1660.76, -16.75, 'Rental Chopper (CW)'),
(52188, 35, -10391.03, 1680.28, -4.63, 'Rental Chopper (CW)'),
(52188, 36, -10409.94, 1676.71, 4.71, 'Rental Chopper (CW)'),
(52188, 37, -10415.67, 1653.93, 4.71, 'Rental Chopper (CW)'),
(52188, 38, -10380.07, 1653.35, 16.30, 'Rental Chopper (CW)'),
(52188, 39, -10368.33, 1696.55, 45.74, 'Rental Chopper (CW)'),
(52188, 40, -10411.31, 1714.82, 49.35, 'Rental Chopper (CW)'),
(52188, 41, -10447.91, 1679.72, 49.35, 'Rental Chopper (CW)'),
(52188, 42, -10390.10, 1592.19, 69.75, 'Rental Chopper (CW)'),
(52188, 43, -10369.55, 1519.53, 28.90, 'Rental Chopper (CW)'),
(52188, 44, -10370.10, 1306.63, 40.48, 'Rental Chopper (CW)'),
(52188, 45, -10391.57, 1208.17, 43.50, 'Rental Chopper (CW)'),
(52188, 46, -10453.90, 979.66, 36.82, 'Rental Chopper (CW)'),
(52188, 47, -10470.96, 954.90, 36.01, 'Rental Chopper (CW)'),
(52188, 48, -10514.11, 955.50, 40.79, 'Rental Chopper (CW)'),
(52188, 49, -10635.25, 1008.93, 32.03, 'Rental Chopper (CW)');

DELETE FROM `creature_text` WHERE `entry`=14305;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(14305,0,0,'Wow, this sure goes fast!',12,0,100,1,0,0,'Comment'),
(14305,1,0,'Look, someones\'s attacking Sentinel Hill. C\'mon guards, you just hafta win!',12,0,100,1,0,0,'Comment'),
(14305,2,0,'I thought the Park fell into big hole, but this one\'s way bigger and it has a whirlwind!',12,0,100,1,0,0,'Comment'),
(14305,3,0,'That looks fun! I wanna drive but the orphan matron says I can\'t get a mount \'til I\'m older. ',12,0,100,1,0,0,'Comment'),
(14305,4,0,'I\'ve never seen a flying tree before. Let\'s get closer! ',12,0,100,1,0,0,'Comment'),
(14305,5,0,'Look at all that slime. One is flying straight at me!',12,0,100,1,0,0,'Comment'),
(14305,6,0,'Eeeeeeeeeeeeeeew, it\'s on my head! Help! ',12,0,100,1,0,0,'Comment'),
(14305,7,0,'My tummy doesn\'t feel so good..',12,0,100,1,0,0,'Comment'),
(14305,8,0,'Whoa! Did you see how high we went?',12,0,100,1,0,0,'Comment'),
(14305,9,0,'That was great! Let\'s do it again! ',12,0,100,1,0,0,'Comment'),
(14305,10,0,'This has to be the biggest diamond ever!',12,0,100,1,0,0,'Comment'),
(14305,11,0,'Wow, you sure don\?t look like any night elf I\?ve seen before!',12,0,100,1,0,0,'Comment'),
(14305,12,0,'Our orphan matron says druids can turn in to birds, cats, and bears, but you look like a combination of all of them!',12,0,100,1,0,0,'Comment'),
(14305,13,0,'I see what you\?re doing. You\?re gonna take all of the druid forms and make them into one super-form!',12,0,100,1,0,0,'Comment'),
(14305,14,0,'The Emerald Dream doesn\?t sound like a place for me. What if it turns me into a turtle? I\?ve always wanted wings, though?',12,0,100,1,0,0,'Comment'),
(14305,15,0,'Wheeee! I?m a bird-boy!',12,0,100,1,0,0,'Comment');

DELETE FROM `creature_text` WHERE `entry`=22817;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(22817,0,0,'Hello sir, I\'m ready to rock!',12,0,100,0,0,0,'Comment'),
(22817,0,1,'Hey guys, I\'m your biggest fan! I\'m ready to rock!',12,0,100,0,0,0,'Comment'),
(22817,0,2,'Check this out guys! I can rock like you guys!',12,0,100,0,0,0,'Comment'),
(22817,1,0,'Salandria begins to play air guitar and sing The Power of the Horde.',16,0,100,0,0,0,'Comment'),
(22817,2,0,'Salandria Waves her hands in the air.',16,0,100,0,0,0,'Comment');

UPDATE `creature_template` SET `ScriptName`='npc_cw_salandria' WHERE `entry`=22817;
UPDATE `creature_template` SET `ScriptName`='' WHERE `entry`=36209;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_cw_oracle_orphan' WHERE `entry`=33533;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_cw_wolvar_orphan' WHERE `entry`=33532;

DELETE FROM `creature_text` WHERE `entry`=10181;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(10181,0,0,'%s kneels down and pick up the amulet.',16,0,100,0,0,0,'npc_lady_sylvanas_windrunner EMOTE_LAMENT_END'),
(10181,1,0,'Belore...',14,0,100,0,0,0,'npc_lady_sylvanas_windrunner SAY_LAMENT_END'),
(10181,2,0,'I do not require defending, ambassador. Young one, as I have told the warchief, my loyalties have never been in doubt.',12,0,100,0,0,0,'Comment'),
(10181,3,0,'The Horde embraced my people when we stood alone and we Forsaken never forget a debt...',12,0,100,0,0,0,'Comment'),
(10181,4,0,'You needn\'t worry, child. The warchief "observers" are here to make certain of that.',12,0,100,0,0,0,'Comment');

UPDATE `creature_template` SET `ScriptName`='npc_cw_orcish_orphan' WHERE `entry`=14444;

DELETE FROM `creature_text` WHERE `entry`=52700;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(52700,0,0,'Elders of Thunder Bluff, I have called you together in the sight of the ancestors to honor my father, Cairne Bloodhoof.',12,0,100,1,0,0,'Comment'),
(52700,1,0,'Father, you were a man who led wisely, who battled skillfully, and who always sought the best for your people.',12,0,100,1,0,0,'Comment'),
(52700,2,0,'May your spirit continue to guide us, father, as you did in life.',12,0,100,1,0,0,'Comment');

DELETE FROM `creature_text` WHERE `entry`=52729;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(52729,0,0,'When my druids and I were ambushed by renegade orcs, you challenged Warchief Garrosh to trial by combat to answer for their deaths, chieftain.',12,0,100,0,0,0,'Comment');

DELETE FROM `creature_text` WHERE `entry`=52787;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(52787,0,0,'You fought bravely, chieftain, but Grimtotem treachery poisoned Garrosh\'s weapon and denied you fair combat.',12,0,100,0,0,0,'Comment'),
(52787,1,0,'May your spirit know peace. Magatha and her kin were driven from the city and your worthy son carries on your legacy.',12,0,100,0,0,0,'Comment');

DELETE FROM `creature_text` WHERE `entry`=52777;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(52777,0,0,'We know you, Cairne Bloodhoof, and your people have told us of your deeds. You are welcome at the hearth of the ancestors.',12,0,100,0,0,0,'Comment');

UPDATE `creature_template` SET `ScriptName`='npc_cw_human_orphan' WHERE `entry`=14305;
UPDATE `creature_template` SET `speed_walk`=0.3, `speed_run`=0.45, `InhabitType`=4 WHERE `entry`=54116;

DELETE FROM `creature_text` WHERE `entry`=43845;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(43845,0,0,'No, little one, it\?s nothing like that. My spirit spent many years in the Emerald Dream while my body slept, and it started to take on attributes of our animal forms.',12,0,100,0,0,0,'Comment');

UPDATE `creature_template` SET  `flags_extra`='128' WHERE (`entry`='40789');
