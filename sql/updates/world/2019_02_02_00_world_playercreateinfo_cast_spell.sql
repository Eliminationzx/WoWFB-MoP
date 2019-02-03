-- ----------------------------
-- Table structure for `playercreateinfo_cast_spell`
-- ----------------------------
DROP TABLE IF EXISTS `playercreateinfo_cast_spell`;
CREATE TABLE `playercreateinfo_cast_spell` (
  `raceMask` int(10) unsigned NOT NULL DEFAULT '0',
  `classMask` int(10) unsigned NOT NULL DEFAULT '0',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `note` varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of playercreateinfo_cast_spell
-- ----------------------------
INSERT INTO `playercreateinfo_cast_spell` VALUES ('0', '32', '48263', 'Death Knight - Blood Presence');
INSERT INTO `playercreateinfo_cast_spell` VALUES ('0', '1', '2457', 'Warrior - Battle Stance');