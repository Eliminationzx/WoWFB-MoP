DELETE FROM `spell_script_names` WHERE `spell_id`=108212 AND `ScriptName`='spell_rog_burst_of_speed';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (108212, 'spell_rog_burst_of_speed');

-- Transcendence
UPDATE creature_template SET ScriptName = 'npc_transcendence_spirit' WHERE entry = 54569;
DELETE FROM spell_script_names WHERE ScriptName = 'spell_monk_transcendence_transfer';
INSERT INTO spell_script_names VALUE (119996, 'spell_monk_transcendence_transfer');

-- Warrior Shouts
DELETE FROM spell_script_names WHERE ScriptName = 'spell_warr_shouts';
INSERT INTO spell_script_names VALUE 
(469, 'spell_warr_shouts'),
(6673, 'spell_warr_shouts');

-- Mount summon visual effects
DELETE FROM spell_linked_spell WHERE spell_trigger IN (5784, 13819, 23161, 23214, 34767, 34769, 48778);
INSERT INTO `spell_linked_spell` VALUES ('5784', '31725', '0', 'Summon felsteed');
INSERT INTO `spell_linked_spell` VALUES ('13819', '31726', '0', 'Summon Warhorse');
INSERT INTO `spell_linked_spell` VALUES ('23161', '31725', '0', 'Summon Nightmare');
INSERT INTO `spell_linked_spell` VALUES ('23214', '31726', '0', 'Summon Charger');
INSERT INTO `spell_linked_spell` VALUES ('34767', '31726', '0', 'Summon Charger - Blood Elf');
INSERT INTO `spell_linked_spell` VALUES ('34769', '31726', '0', 'Summon Warhorse - Blood Elf');
INSERT INTO `spell_linked_spell` VALUES ('48778', '50772', '0', 'Acherus Deathcharger - Summon Unholy Mount Visual');

-- PvP Opening visual
DELETE FROM spell_script_names WHERE ScriptName = 'spell_gen_pvp_opening';
DELETE FROM spell_script_names WHERE ScriptName = 'spell_gen_pvp_opening_trigger';
INSERT INTO spell_script_names VALUE 
(24390, 'spell_gen_pvp_opening'),
(24391, 'spell_gen_pvp_opening_trigger');

-- Fix spell proc 146183
DELETE FROM `spell_proc_event` WHERE `entry` IN (146183);
INSERT INTO `spell_proc_event` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `SpellFamilyMask3`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`) VALUES ('146183', '0', '0', '0', '0', '0', '0', '65876', '0', '0', '0', '7');

-- Power Word: Barrier
DELETE FROM spell_areatrigger WHERE ScriptName = 'spell_area_priest_power_word_barrier';
INSERT INTO `spell_areatrigger` VALUES ('62618', '1', '8', '8', 'spell_area_priest_power_word_barrier', 'Power Word: Barrier');

-- Immolation
DELETE FROM spell_script_names WHERE ScriptName = 'spell_warl_pet_immolation_aura';
INSERT INTO spell_script_names VALUE 
(19483, 'spell_warl_pet_immolation_aura');

-- Fix spell 147280
DELETE FROM spell_script_names WHERE ScriptName = 'spell_time_lost_wisdom';
INSERT INTO spell_script_names VALUE 
(24390, 'spell_time_lost_wisdom');

-- Eye of Kilrogg
DELETE FROM spell_script_names WHERE ScriptName = 'spell_warl_eye_of_kilrogg';
DELETE FROM spell_script_names WHERE ScriptName = 'spell_warl_eye_of_kilrogg_passive';
INSERT INTO spell_script_names VALUE 
(126, 'spell_warl_eye_of_kilrogg'),
(2585, 'spell_warl_eye_of_kilrogg_passive');

UPDATE creature_template_addon SET auras='2585' WHERE entry =4277;

-- Book of The Ages
DELETE FROM spell_script_names WHERE ScriptName = 'spell_item_book_of_the_ages';
INSERT INTO spell_script_names VALUE 
(147228, 'spell_item_book_of_the_ages');

INSERT INTO `spell_proc_event` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `SpellFamilyMask3`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`) VALUES ('147228', '0', '0', '0', '0', '0', '0', '0', '0', '0', '15', '0');

