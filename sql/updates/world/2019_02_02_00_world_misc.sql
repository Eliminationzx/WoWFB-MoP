UPDATE `creature_template` SET `ScriptName` = 'boss_slagmaw' WHERE `entry` =61463;
UPDATE `creature_template` SET `ScriptName` = 'boss_adarogg' WHERE `entry` =61408;
UPDATE `creature_template` SET `ScriptName` = 'boss_koranthal' WHERE `entry` =61412;

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(119405, 'spell_inferno_charge');

UPDATE `creature_template` SET `InhabitType` = '4' WHERE `entry` =61463;

UPDATE `creature_template` SET `minlevel` = '16', `maxlevel` = '16', `faction_A` = '16', `faction_H` = '16', `VehicleId` = '522', `ScriptName` = 'boss_slagmaw' WHERE `entry` =61463;

DELETE FROM `vehicle_template_accessory` WHERE `entry` = 61463;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(61463, 61800, 0, 1, 'Slagmaw', 6, 1000);

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 61463;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(61463, 88066, 1, 0);

UPDATE `creature_template` SET `minlevel` = '16', `maxlevel` = '16', `faction_A` = '16', `faction_H` = '16', `InhabitType` = '4' WHERE `entry` =61800;

UPDATE `creature_template` SET `ScriptName` = 'boss_slagmaw_head' WHERE `entry` =61800;

DELETE FROM spell_script_names WHERE spell_id=72276;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (72276, 'boss_amagalad_of_corruption');
UPDATE `creature_template` SET `ScriptName`='creature_lorewalker_cho_pre_norushen' WHERE (`entry`='72872');
UPDATE `creature_template` SET `ScriptName`='creature_norushen_manifestation_of_corruption' WHERE entry=72264;
UPDATE `creature_template` SET `ScriptName`='creature_norushen_manifestation_of_corruption_non_test' WHERE entry=71977;
UPDATE `creature_template` SET `ScriptName`='creature_norushen_essnece_of_corruption_non_test' WHERE entry=71976;
UPDATE `creature_template` SET `ScriptName`='creature_norushen_essnece_of_corruption' WHERE entry=72263;
UPDATE `creature_template` SET `ScriptName`='creature_residual_corruption' WHERE entry=72550;
UPDATE `creature_template` SET `ScriptName`='creature_blind_hatred_trigger' WHERE entry=72595;
UPDATE `creature_template` SET `ScriptName`='creature_look_within_trigger' WHERE entry=765321;
UPDATE `creature_template` SET `ScriptName`='creature_titanic_corruption' WHERE entry=72051;
UPDATE `creature_template` SET `ScriptName`='creature_greater_corruption' WHERE entry=72001;
UPDATE `creature_template` SET `ScriptName`='creature_norushen_sun' WHERE entry=72000;
UPDATE `creature_template` SET `ScriptName`='creature_norushen_rook' WHERE entry=71996;
UPDATE `creature_template` SET `ScriptName`='creature_norushen_leven' WHERE entry=71995;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_desecrated_weapon' WHERE entry=72154;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_empowered_desecrated_weapon' WHERE entry=72198;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_heart_of_yshaaraj' WHERE entry=72215;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_iron_star' WHERE entry=73059;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_minion_of_yshaaraj' WHERE entry=72272;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_second_phase_prop_heart_of_yshaaraj' WHERE entry=72228;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_siege_engineer' WHERE entry=71984;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_creature_thrall' WHERE entry=73483;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_embodied_despair' WHERE entry=72238;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_embodied_doubt' WHERE entry=72236;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_embodied_fear' WHERE entry=72237;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_farseer_wolf_rider' WHERE entry=71983;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_korkron_warbringer' WHERE entry=71979;
UPDATE `creature_template` SET `ScriptName`='garrosh_hellscream_heart_of_yshaaraj_protection_aura' WHERE entry=78097;
UPDATE `creature_template` SET `ScriptName`='boss_farseer_vicious_war_wolf' WHERE entry=71994;
DELETE FROM spell_script_names WHERE spell_id=144745;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (144745, 'spell_desecrate_dummy');
DELETE FROM spell_script_names WHERE spell_id=145199;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145199, 'spell_empowered_gripping_despair_debuff');
DELETE FROM spell_script_names WHERE spell_id=145195;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145195, 'spell_empowered_gripping_despair');
DELETE FROM spell_script_names WHERE spell_id=149252;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (149252, 'spell_empowered_gripping_despair');
DELETE FROM spell_script_names WHERE spell_id=145037;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145037, 'spell_whirling_corruption');
DELETE FROM spell_script_names WHERE spell_id=145043;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145043, 'spell_empowering_corruption_filter');
DELETE FROM spell_script_names WHERE spell_id=144868;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (144868, 'spell_enter_the_realm_yshaaraj');
DELETE FROM spell_script_names WHERE spell_id=145096;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145096, 'spell_touch_of_yshaaraj_proc_two');
DELETE FROM spell_script_names WHERE spell_id=144821;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (144821, 'spell_hellscream_warsong_filter');
DELETE FROM spell_script_names WHERE spell_id=144867;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (144867, 'spell_ancsetral_healing');
DELETE FROM spell_script_names WHERE spell_id=147266;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (147266, 'spell_enter_realm_of_yshaarj_purple_beam');
DELETE FROM spell_script_names WHERE spell_id=145735;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145735, 'spell_norushen_icy_fear');
DELETE FROM spell_script_names WHERE spell_id=145212;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145212, 'spell_norushen_unleahsed_anger');
DELETE FROM spell_script_names WHERE spell_id=145140;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145140, 'spell_norushen_extract_corruption');
DELETE FROM spell_script_names WHERE spell_id=145613;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145613, 'spell_norushen_blind_hatred_beam');
DELETE FROM spell_script_names WHERE spell_id=145073;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145073, 'spell_norushen_src_spells_aoe_filter');
DELETE FROM spell_script_names WHERE spell_id=145226;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (145226, 'spell_norushen_blind_hatred_beam_filter');
DELETE FROM creature_template WHERE entry=765321;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `difficulty_entry_4`, `difficulty_entry_5`, `difficulty_entry_6`, `difficulty_entry_7`, `difficulty_entry_8`, `difficulty_entry_9`, `difficulty_entry_10`, `difficulty_entry_11`, `difficulty_entry_12`, `difficulty_entry_13`, `difficulty_entry_14`, `difficulty_entry_15`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction_A`, `faction_H`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `rank`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Health_mod`, `Mana_mod`, `Mana_mod_extra`, `Armor_mod`, `RacialLeader`, `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`, `movementId`, `RegenHealth`, `equipment_id`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `WDBVerified`) VALUES (765321, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1126, 23257, 0, 0, 'Look Within', '', '', 0, 93, 93, 4, 0, 35, 35, 1, 0, 1, 1.14286, 1.14286, 1, 0, 1, 2, 0, 1, 1, 2000, 2000, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 10, 1091568640, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120268, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 128, 'creature_look_within_trigger', 16048);
UPDATE `creature_template` SET gossip_menu_id=55001 WHERE entry=62445;
DELETE FROM gameobject WHERE id IN (213251,210171,221446,188488,221611,221760,222681,221751,221758,221677,221678,183957,221750,221676,221757,
221759,221752,221761,221756,221753,221754,221755,211363,164839) AND map=1136;
INSERT INTO `gameobject` VALUES ('NULL', '213251', '1136', '6738', '6738', '1', '1', '0', '1459.18', '861.013', '248.993', '4.69923', '0', '0', '0.711744', '-0.702439', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '210171', '1136', '6738', '6738', '1', '1', '0', '1281.03', '1041.68', '434.87', '3.09378', '0', '0', '0.999714', '0.0239024', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221446', '1136', '0', '0', '1', '1', '0', '762.313', '1046.56', '357.151', '1.75204', '0', '0', '0.768196', '0.640214', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '188488', '1136', '6738', '6738', '1', '1', '0', '1283.8', '1043.54', '436.246', '0.0379198', '0', '0', '0.0189588', '0.99982', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '210171', '1136', '6738', '6738', '1', '1', '0', '1281.36', '1036.23', '434.868', '0.0460982', '0', '0', '0.0230471', '0.999734', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221611', '1136', '0', '0', '1', '1', '0', '1214.21', '984.966', '417.877', '1.61004', '0', '0', '0.720846', '0.693096', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221760', '1136', '0', '0', '1', '1', '0', '723.039', '1129.09', '354.612', '0.15709', '0', '0', '0.0784644', '0.996917', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '687.826', '1100.18', '356.072', '3.32319', '0', '0', '0.995881', '-0.0906735', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '723.069', '1128.54', '356.073', '0.622188', '0', '0', '0.3061', '0.951999', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221751', '1136', '0', '0', '1', '1', '0', '731.979', '1088.21', '354.612', '1.697', '0', '0', '0.750289', '0.66111', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221758', '1136', '0', '0', '1', '1', '0', '764.376', '1137.69', '354.613', '4.84044', '0', '0', '0.660417', '-0.750899', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '772.502', '1096.17', '356.072', '0.41078', '0', '0', '0.203949', '0.978982', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221677', '1136', '0', '0', '1', '1', '0', '772.832', '1096.46', '353.718', '4.0541', '0', '0', '0.89771', '-0.440586', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221678', '1136', '0', '0', '1', '1', '0', '722.446', '1128.39', '353.325', '1.19935', '0', '0', '0.564374', '0.825519', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '183957', '1136', '6738', '6738', '1', '1', '0', '723.545', '1126.77', '356.992', '1.41031', '0', '0', '0.648152', '0.761511', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '695.358', '1144.19', '356.072', '2.65858', '0', '0', '0.970978', '0.239168', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221750', '1136', '0', '0', '1', '1', '0', '772.917', '1096.36', '354.613', '3.15393', '0', '0', '0.999981', '-0.00616861', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221676', '1136', '0', '0', '1', '1', '0', '764.071', '1137.55', '353.717', '1.22768', '0', '0', '0.576011', '0.817442', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221678', '1136', '0', '0', '1', '1', '0', '731.574', '1087.69', '353.343', '0.824668', '0', '0', '0.400749', '0.916188', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '809.212', '1125.97', '356.072', '0.304753', '0', '0', '0.151787', '0.988413', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '731.317', '1087.67', '356.073', '1.31006', '0', '0', '0.609183', '0.793029', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221757', '1136', '0', '0', '1', '1', '0', '764.376', '1137.69', '354.613', '0.574437', '0', '0', '0.283286', '0.959036', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221759', '1136', '0', '0', '1', '1', '0', '764.376', '1137.69', '354.613', '2.80644', '0', '0', '0.985992', '0.166794', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221752', '1136', '0', '0', '1', '1', '0', '731.979', '1088.21', '354.612', '3.64956', '0', '0', '0.967919', '-0.251262', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221761', '1136', '0', '0', '1', '1', '0', '723.039', '1129.09', '354.612', '2.18659', '0', '0', '0.888146', '0.459561', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '735.192', '1173.21', '356.072', '1.7335', '0', '0', '0.76223', '0.647306', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221756', '1136', '0', '0', '1', '1', '0', '723.039', '1129.09', '354.612', '4.36578', '0', '0', '0.818448', '-0.57458', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221753', '1136', '0', '0', '1', '1', '0', '731.979', '1088.21', '354.612', '5.97543', '0', '0', '0.153272', '-0.988184', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '222681', '1136', '6738', '6738', '1', '1', '0', '763.404', '1137.97', '356.073', '5.75988', '0', '0', '0.258677', '-0.965964', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221754', '1136', '0', '0', '1', '1', '0', '772.917', '1096.36', '354.613', '5.19693', '0', '0', '0.516816', '-0.856096', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '221755', '1136', '0', '0', '1', '1', '0', '772.917', '1096.36', '354.613', '1.02543', '0', '0', '0.490545', '0.871416', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '188488', '1136', '6738', '6738', '1', '1', '0', '1281.3', '1046.46', '434.874', '3.14016', '0', '0', '1', '0.000718669', '300', '0', '1', '0', null);
INSERT INTO `gameobject` VALUES ('NULL', '183957', '1136', '6738', '6738', '1', '1', '0', '724.366', '1131.84', '356.072', '6.04649', '0', '0', '0.118069', '-0.993005', '300', '0', '1', '0', null);
UPDATE gameobject SET spawnmask=8 WHERE id IN (183957,188488,221755,221754,222681,221753,221756,221761,221752,221759,221757,221678,221676,221750,221677,221758,221751,221760,221611,210171,221446,213251) AND map=1136;