/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Unit.h"
#include "ObjectAccessor.h"
#include "Util.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "Battleground.h"
#include "OutdoorPvPMgr.h"
#include "Formulas.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ScriptMgr.h"
#include "Vehicle.h"
#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "WeatherMgr.h"
#include "BattlegroundKT.h"
#include "Guild.h"

class Aura;
//
// EFFECT HANDLER NOTES
//
// in aura handler there should be check for modes:
// AURA_EFFECT_HANDLE_REAL set
// AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK set
// AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK set - aura is recalculated or is just applied/removed - need to redo all things related to m_amount
// AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK - logical or of above conditions
// AURA_EFFECT_HANDLE_STAT - set when stats are reapplied
// such checks will speedup trinity change amount/send for client operations
// because for change amount operation packets will not be send
// aura effect handlers shouldn't contain any AuraEffect or Aura object modifications

pAuraEffectHandler AuraEffectHandler[TOTAL_AURAS]=
{
    &AuraEffect::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &AuraEffect::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &AuraEffect::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &AuraEffect::HandleNoImmediateEffect,                         //  3 SPELL_AURA_PERIODIC_DAMAGE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &AuraEffect::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &AuraEffect::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &AuraEffect::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &AuraEffect::HandleNoImmediateEffect,                         //  8 SPELL_AURA_PERIODIC_HEAL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &AuraEffect::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &AuraEffect::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &AuraEffect::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &AuraEffect::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &AuraEffect::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD    implemented in Unit::DoAttackDamage
    &AuraEffect::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &AuraEffect::HandleModStealthDetect,                          // 17 SPELL_AURA_MOD_DETECT
    &AuraEffect::HandleModInvisibility,                           // 18 SPELL_AURA_MOD_INVISIBILITY
    &AuraEffect::HandleModInvisibilityDetect,                     // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &AuraEffect::HandleNoImmediateEffect,                         // 20 SPELL_AURA_OBS_MOD_HEALTH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         // 21 SPELL_AURA_OBS_MOD_POWER implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         // 24 SPELL_AURA_PERIODIC_ENERGIZE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &AuraEffect::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &AuraEffect::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &AuraEffect::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &AuraEffect::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &AuraEffect::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &AuraEffect::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &AuraEffect::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &AuraEffect::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &AuraEffect::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &AuraEffect::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &AuraEffect::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &AuraEffect::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &AuraEffect::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &AuraEffect::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &AuraEffect::HandleNoImmediateEffect,                         // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &AuraEffect::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &AuraEffect::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &AuraEffect::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &AuraEffect::HandleNULL,                                      // 46 SPELL_AURA_46 (used in test spells 54054 and 54058, and spell 48050) (3.0.8a)
    &AuraEffect::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &AuraEffect::HandleNULL,                                      // 48 SPELL_AURA_48 spell Napalm (area damage spell with additional delayed damage effect)
    &AuraEffect::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 50 SPELL_AURA_MOD_CRITICAL_HEALING_AMOUNT implemented in Unit::SpellCriticalHealingBonus
    &AuraEffect::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &AuraEffect::HandleAuraModWeaponCritPercent,                  // 52 SPELL_AURA_MOD_WEAPON_CRIT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 53 SPELL_AURA_PERIODIC_LEECH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &AuraEffect::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &AuraEffect::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &AuraEffect::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &AuraEffect::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &AuraEffect::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &AuraEffect::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &AuraEffect::HandleNoImmediateEffect,                         // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleUnused,                                    // 63 unused (4.3.4) old SPELL_AURA_PERIODIC_MANA_FUNNEL
    &AuraEffect::HandleNoImmediateEffect,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleAuraModSpellHaste,                         // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &AuraEffect::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &AuraEffect::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &AuraEffect::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &AuraEffect::HandleNoImmediateEffect,                         // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleUnused,                                    // 70 SPELL_AURA_EXTRA_ATTACKS clientside
    &AuraEffect::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &AuraEffect::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &AuraEffect::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &AuraEffect::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &AuraEffect::HandleNoImmediateEffect,                         // 76 SPELL_AURA_FAR_SIGHT
    &AuraEffect::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &AuraEffect::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &AuraEffect::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &AuraEffect::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &AuraEffect::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &AuraEffect::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 84 SPELL_AURA_MOD_REGEN implemented in Player::RegenerateHealth
    &AuraEffect::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN implemented in Player::Regenerate
    &AuraEffect::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &AuraEffect::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT implemented in Player::RegenerateHealth
    &AuraEffect::HandleNoImmediateEffect,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &AuraEffect::HandleUnused,                                    // 90 unused (4.3.4) old SPELL_AURA_MOD_RESIST_CHANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE implemented in Creature::GetAttackDistance
    &AuraEffect::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &AuraEffect::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &AuraEffect::HandleNoImmediateEffect,                         // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::Regenerate
    &AuraEffect::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &AuraEffect::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Unit::SelectMagnetTarget
    &AuraEffect::HandleNoImmediateEffect,                         // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &AuraEffect::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &AuraEffect::HandleUnused,                                    //100 SPELL_AURA_AURAS_VISIBLE obsolete? all player can see all auras now, but still have spells including GM-spell
    &AuraEffect::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &AuraEffect::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &AuraEffect::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &AuraEffect::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &AuraEffect::HandleNoImmediateEffect,                         //107 SPELL_AURA_ADD_FLAT_MODIFIER implemented in AuraEffect::CalculateSpellMod()
    &AuraEffect::HandleNoImmediateEffect,                         //108 SPELL_AURA_ADD_PCT_MODIFIER implemented in AuraEffect::CalculateSpellMod()
    &AuraEffect::HandleNoImmediateEffect,                         //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &AuraEffect::HandleModPowerRegenPCT,                          //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT implemented in Player::Regenerate, Creature::Regenerate
    &AuraEffect::HandleNoImmediateEffect,                         //111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER implemented in Unit::SelectMagnetTarget
    &AuraEffect::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &AuraEffect::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //115 SPELL_AURA_MOD_HEALING                 implemented in Unit::SpellBaseHealingBonusForVictim
    &AuraEffect::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleUnused,                                    //119 unused (4.3.4) old SPELL_AURA_SHARE_PET_TRACKING
    &AuraEffect::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &AuraEffect::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &AuraEffect::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &AuraEffect::HandleModTargetResistance,                       //123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &AuraEffect::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &AuraEffect::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //128 SPELL_AURA_MOD_POSSESS_PET (only dummy effects since cataclysm addon)
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //129 SPELL_AURA_MOD_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &AuraEffect::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &AuraEffect::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &AuraEffect::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &AuraEffect::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &AuraEffect::HandleNoImmediateEffect,                         //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &AuraEffect::HandleModMeleeSpeedPct,                          //138 SPELL_AURA_MOD_MELEE_HASTE
    &AuraEffect::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &AuraEffect::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &AuraEffect::HandleUnused,                                    //141 SPELL_AURA_141
    &AuraEffect::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &AuraEffect::HandleAuraModResistanceExclusive,                //143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &AuraEffect::HandleNoImmediateEffect,                         //144 SPELL_AURA_SAFE_FALL                         implemented in WorldSession::HandleMovementOpcodes
    &AuraEffect::HandleAuraModPetTalentsPoints,                   //145 SPELL_AURA_MOD_PET_TALENT_POINTS
    &AuraEffect::HandleNoImmediateEffect,                         //146 SPELL_AURA_ALLOW_TAME_PET_TYPE
    &AuraEffect::HandleModStateImmunityMask,                      //147 SPELL_AURA_MECHANIC_IMMUNITY_MASK
    &AuraEffect::HandleAuraRetainComboPoints,                     //148 SPELL_AURA_RETAIN_COMBO_POINTS
    &AuraEffect::HandleNoImmediateEffect,                         //149 SPELL_AURA_REDUCE_PUSHBACK
    &AuraEffect::HandleShieldBlockValue,                          //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &AuraEffect::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &AuraEffect::HandleNoImmediateEffect,                         //152 SPELL_AURA_MOD_DETECTED_RANGE implemented in Creature::GetAttackDistance
    &AuraEffect::HandleUnused,                                    //153 Unused (4.3.4) old SPELL_AURA_SPLIT_DAMAGE_FLAT
    &AuraEffect::HandleModStealthLevel,                           //154 SPELL_AURA_MOD_STEALTH_LEVEL
    &AuraEffect::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &AuraEffect::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN
    &AuraEffect::HandleNULL,                                      //157 SPELL_AURA_PET_DAMAGE_MULTI
    &AuraEffect::HandleShieldBlockValue,                          //158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &AuraEffect::HandleNoImmediateEffect,                         //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &AuraEffect::HandleUnused,                                    //160 Unused (4.3.4) old SPELL_AURA_MOD_AOE_AVOIDANCE
    &AuraEffect::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //162 SPELL_AURA_POWER_BURN implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
    &AuraEffect::HandleUnused,                                    //164 unused (3.2.0), only one test spell
    &AuraEffect::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &AuraEffect::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonus, Unit::MeleeDamageBonus
    &AuraEffect::HandleUnused,                                    //169 Unused (4.3.4) old SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
    &AuraEffect::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       various spells that change visual of units for aura target (clientside?)
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &AuraEffect::HandleUnused,                                    //173 unused (4.3.4) no spells, old SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &AuraEffect::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonus
    &AuraEffect::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &AuraEffect::HandleCharmConvert,                              //177 SPELL_AURA_AOE_CHARM
    &AuraEffect::HandleUnused,                                    //178 old SPELL_AURA_MOD_DEBUFF_RESISTANCE unused 4.3.4
    &AuraEffect::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &AuraEffect::HandleNoImmediateEffect,                         //180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleUnused,                                    //181 unused (4.3.4) old SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS
    &AuraEffect::HandleAuraModResistenceOfStatPercent,            //182 SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT
    &AuraEffect::HandleNULL,                                      //183 SPELL_AURA_MOD_CRITICAL_THREAT only used in 28746 - miscvalue - spell school
    &AuraEffect::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::GetUnitCriticalChance
    &AuraEffect::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChance
    &AuraEffect::HandleModRating,                                 //189 SPELL_AURA_MOD_RATING
    &AuraEffect::HandleNoImmediateEffect,                         //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN     implemented in Player::CalculateReputationGain
    &AuraEffect::HandleAuraModUseNormalSpeed,                     //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
    &AuraEffect::HandleModMeleeRangedSpeedPct,                    //192 SPELL_AURA_MOD_MELEE_RANGED_HASTE
    &AuraEffect::HandleModCombatSpeedPct,                         //193 SPELL_AURA_MELEE_SLOW (in fact combat (any type attack) speed pct)
    &AuraEffect::HandleNoImmediateEffect,                         //194 SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleNoImmediateEffect,                         //195 SPELL_AURA_MOD_TARGET_ABILITY_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleNULL,                                      //196 SPELL_AURA_MOD_COOLDOWN - flat mod of spell cooldowns
    &AuraEffect::HandleNoImmediateEffect,                         //197 SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE implemented in Unit::SpellCriticalBonus Unit::GetUnitCriticalChance
    &AuraEffect::HandleUnused,                                    //198 unused (4.3.4) old SPELL_AURA_MOD_ALL_WEAPON_SKILLS
    &AuraEffect::HandleUnused,                                    //199 unused (4.3.4) old SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT
    &AuraEffect::HandleNoImmediateEffect,                         //200 SPELL_AURA_MOD_XP_PCT implemented in Player::RewardPlayerAndGroupAtKill
    &AuraEffect::HandleAuraAllowFlight,                           //201 SPELL_AURA_FLY                             this aura enable flight mode...
    &AuraEffect::HandleNoImmediateEffect,                         //202 SPELL_AURA_CANNOT_BE_DODGED                implemented in Unit::RollPhysicalOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //203 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE  implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &AuraEffect::HandleNoImmediateEffect,                         //204 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &AuraEffect::HandleNULL,                                      //205 SPELL_AURA_MOD_SCHOOL_CRIT_DMG_TAKEN
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //206 SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //207 SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //208 SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //209 SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //210 SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //211 SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK
    &AuraEffect::HandleUnused,                                    //212 Unused (4.3.4) old SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //213 SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT implemented in Player::RewardRage
    &AuraEffect::HandleNULL,                                      //214 Tamed Pet Passive
    &AuraEffect::HandleArenaPreparation,                          //215 SPELL_AURA_ARENA_PREPARATION
    &AuraEffect::HandleAuraModSpellHaste,                         //216 SPELL_AURA_HASTE_SPELLS
    &AuraEffect::HandleModMeleeSpeedPct,                          //217 SPELL_AURA_MOD_MELEE_HASTE_2
    &AuraEffect::HandleAuraModRangedHaste,                        //218 SPELL_AURA_HASTE_RANGED
    &AuraEffect::HandleModManaRegen,                              //219 SPELL_AURA_MOD_MANA_REGEN_FROM_STAT
    &AuraEffect::HandleModRatingFromStat,                         //220 SPELL_AURA_MOD_RATING_FROM_STAT
    &AuraEffect::HandleNULL,                                      //221 SPELL_AURA_MOD_DETAUNT
    &AuraEffect::HandleUnused,                                    //222 unused (3.2.0) only for spell 44586 that not used in real spell cast
    &AuraEffect::HandleNoImmediateEffect,                         //223 SPELL_AURA_RAID_PROC_FROM_CHARGE
    &AuraEffect::HandleUnused,                                    //224 unused (4.3.4)
    &AuraEffect::HandleNoImmediateEffect,                         //225 SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE
    &AuraEffect::HandleNoImmediateEffect,                         //226 SPELL_AURA_PERIODIC_DUMMY implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //227 SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //228 SPELL_AURA_DETECT_STEALTH stealth detection
    &AuraEffect::HandleNoImmediateEffect,                         //229 SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE
    &AuraEffect::HandleAuraModIncreaseHealth,                     //230 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &AuraEffect::HandleNoImmediateEffect,                         //231 SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
    &AuraEffect::HandleNoImmediateEffect,                         //232 SPELL_AURA_MECHANIC_DURATION_MOD           implement in Unit::CalculateSpellDuration
    &AuraEffect::HandleUnused,                                    //233 set model id to the one of the creature with id GetMiscValue() - clientside
    &AuraEffect::HandleNoImmediateEffect,                         //234 SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK implement in Unit::CalculateSpellDuration
    &AuraEffect::HandleNoImmediateEffect,                         //235 SPELL_AURA_MOD_DISPEL_RESIST               implement in Unit::MagicSpellHitResult
    &AuraEffect::HandleAuraControlVehicle,                        //236 SPELL_AURA_CONTROL_VEHICLE
    &AuraEffect::HandleModSpellDamagePercentFromAttackPower,      //237 SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleModSpellHealingPercentFromAttackPower,     //238 SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER implemented in Unit::SpellBaseHealingBonus
    &AuraEffect::HandleAuraModScale,                              //239 SPELL_AURA_MOD_SCALE_2 only in Noggenfogger Elixir (16595) before 2.3.0 aura 61
    &AuraEffect::HandleAuraModExpertise,                          //240 SPELL_AURA_MOD_EXPERTISE
    &AuraEffect::HandleForceMoveForward,                          //241 SPELL_AURA_FORCE_MOVE_FORWARD Forces the caster to move forward
    &AuraEffect::HandleNULL,                                      //242 SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING - 2 test spells: 44183 and 44182
    &AuraEffect::HandleAuraModFaction,                            //243 SPELL_AURA_MOD_FACTION
    &AuraEffect::HandleComprehendLanguage,                        //244 SPELL_AURA_COMPREHEND_LANGUAGE
    &AuraEffect::HandleNoImmediateEffect,                         //245 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL
    &AuraEffect::HandleNoImmediateEffect,                         //246 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK implemented in Spell::EffectApplyAura
    &AuraEffect::HandleAuraCloneCaster,                           //247 SPELL_AURA_CLONE_CASTER
    &AuraEffect::HandleNoImmediateEffect,                         //248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE         implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleAuraConvertRune,                           //249 SPELL_AURA_CONVERT_RUNE
    &AuraEffect::HandleAuraModIncreaseHealth,                     //250 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &AuraEffect::HandleNoImmediateEffect,                         //251 SPELL_AURA_MOD_ENEMY_DODGE
    &AuraEffect::HandleModCombatSpeedPct,                         //252 SPELL_AURA_252 Is there any difference between this and SPELL_AURA_MELEE_SLOW ? maybe not stacking mod?
    &AuraEffect::HandleNoImmediateEffect,                         //253 SPELL_AURA_MOD_BLOCK_CRIT_CHANCE  implemented in Unit::isBlockCritical
    &AuraEffect::HandleAuraModDisarm,                             //254 SPELL_AURA_MOD_DISARM_OFFHAND
    &AuraEffect::HandleNoImmediateEffect,                         //255 SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT    implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNoReagentUseAura,                          //256 SPELL_AURA_NO_REAGENT_USE Use SpellClassMask for spell select
    &AuraEffect::HandleNULL,                                      //257 SPELL_AURA_MOD_TARGET_RESIST_BY_SPELL_CLASS Use SpellClassMask for spell select
    &AuraEffect::HandleNULL,                                      //258 SPELL_AURA_MOD_SPELL_VISUAL
    &AuraEffect::HandleNoImmediateEffect,                         //259 SPELL_AURA_MOD_HOT_PCT implemented in Unit::SpellHealingBonusTaken
    &AuraEffect::HandleNoImmediateEffect,                         //260 SPELL_AURA_SCREEN_EFFECT (miscvalue = id in ScreenEffect.dbc) not required any code
    &AuraEffect::HandlePhase,                                     //261 SPELL_AURA_PHASE
    &AuraEffect::HandleNoImmediateEffect,                         //262 SPELL_AURA_ABILITY_IGNORE_AURASTATE implemented in spell::cancast
    &AuraEffect::HandleAuraAllowOnlyAbility,                      //263 SPELL_AURA_ALLOW_ONLY_ABILITY player can use only abilities set in SpellClassMask
    &AuraEffect::HandleUnused,                                    //264 unused (3.2.0)
    &AuraEffect::HandleUnused,                                    //265 unused (4.3.4)
    &AuraEffect::HandleUnused,                                    //266 unused (4.3.4)
    &AuraEffect::HandleNoImmediateEffect,                         //267 SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL         implemented in Unit::IsImmunedToSpellEffect
    &AuraEffect::HandleUnused,                                    //268 unused (4.3.4) old SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT.
    &AuraEffect::HandleNoImmediateEffect,                         //269 SPELL_AURA_MOD_IGNORE_TARGET_RESIST implemented in Unit::CalcAbsorbResist and CalcArmorReducedDamage
    &AuraEffect::HandleUnused,                                    //270 unused (4.3.4) old SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST
    &AuraEffect::HandleNoImmediateEffect,                         //271 SPELL_AURA_MOD_DAMAGE_FROM_CASTER    implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //272 SPELL_AURA_IGNORE_MELEE_RESET
    &AuraEffect::HandleUnused,                                    //273 clientside
    &AuraEffect::HandleUnused,                                    //274 unused (4.3.4)
    &AuraEffect::HandleNoImmediateEffect,                         //275 SPELL_AURA_MOD_IGNORE_SHAPESHIFT Use SpellClassMask for spell select
    &AuraEffect::HandleNULL,                                      //276 mod damage % mechanic?
    &AuraEffect::HandleUnused,                                    //277 unused (4.3.4) old SPELL_AURA_MOD_MAX_AFFECTED_TARGETS
    &AuraEffect::HandleAuraModDisarm,                             //278 SPELL_AURA_MOD_DISARM_RANGED disarm ranged weapon
    &AuraEffect::HandleAuraInitializeImages,                      //279 SPELL_AURA_INITIALIZE_IMAGES
    &AuraEffect::HandleNoImmediateEffect,                         //280 SPELL_AURA_MOD_ARMOR_PENETRATION_PCT implemented in Unit::CalcArmorReducedDamage
    &AuraEffect::HandleNoImmediateEffect,                         //281 SPELL_AURA_MOD_GUID_REP_GAIN_PCT implemented in Player::RewardGuildReputation
    &AuraEffect::HandleAuraIncreaseBaseHealthPercent,             //282 SPELL_AURA_INCREASE_BASE_HEALTH_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //283 SPELL_AURA_MOD_HEALING_ABSORB_RECEIVED       implemented in Unit::SpellHealingBonus and AuraEffect::CalculateAmount
    &AuraEffect::HandleAuraLinked,                                //284 SPELL_AURA_LINKED
    &AuraEffect::HandleAuraModAttackPowerOfArmor,                 //285 SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR  implemented in Player::UpdateAttackPowerAndDamage
    &AuraEffect::HandleNoImmediateEffect,                         //286 SPELL_AURA_ABILITY_PERIODIC_CRIT implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //287 SPELL_AURA_DEFLECT_SPELLS             implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //288 SPELL_AURA_IGNORE_HIT_DIRECTION  implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNULL,                                      //289 unused (3.2.0)
    &AuraEffect::HandleAuraModCritPct,                            //290 SPELL_AURA_MOD_CRIT_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //291 SPELL_AURA_MOD_XP_QUEST_PCT  implemented in Player::RewardQuest
    &AuraEffect::HandleAuraOpenStable,                            //292 SPELL_AURA_OPEN_STABLE
    &AuraEffect::HandleAuraOverrideSpells,                        //293 auras which probably add set of abilities to their target based on it's miscvalue
    &AuraEffect::HandleNoImmediateEffect,                         //294 SPELL_AURA_PREVENT_REGENERATE_POWER implemented in Player::Regenerate(Powers power)
    &AuraEffect::HandleUnused,                                    //295 unused (4.3.4)
    &AuraEffect::HandleAuraSetVehicle,                            //296 SPELL_AURA_SET_VEHICLE_ID sets vehicle on target
    &AuraEffect::HandleNULL,                                      //297 Spirit Burst spells
    &AuraEffect::HandleAuraStrangulate,                           //298 70569 - Strangulating, maybe prevents talk or cast
    &AuraEffect::HandleUnused,                                    //299 unused (4.3.4)
    &AuraEffect::HandleNoImmediateEffect,                         //300 SPELL_AURA_SHARE_DAMAGE_PCT implemented in Unit::DealDamage
    &AuraEffect::HandleNoImmediateEffect,                         //301 SPELL_AURA_SCHOOL_HEAL_ABSORB implemented in Unit::CalcHealAbsorb
    &AuraEffect::HandleUnused,                                    //302 unused (4.3.4)
    &AuraEffect::HandleNoImmediateEffect,                         //303 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE implemented in Unit::SpellDamageBonus, Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModFakeInebriation,                    //304 SPELL_AURA_MOD_DRUNK
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //305 SPELL_AURA_MOD_MINIMUM_SPEED
    &AuraEffect::HandleUnused,                                    //306 unused (4.3.4)
    &AuraEffect::HandleUnused,                                    //307 unused (4.3.4)
    &AuraEffect::HandleNULL,                                      //308 new aura for hunter traps
    &AuraEffect::HandleAuraModResiliencePct,                      //309 SPELL_AURA_MOD_RESILIENCE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //310 SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE implemented in Spell::CalculateDamageDone
    &AuraEffect::HandleNULL,                                      //311 0 spells in 3.3.5
    &AuraEffect::HandleNULL,                                      //312 0 spells in 3.3.5
    &AuraEffect::HandleUnused,                                    //313 unused (4.3.4)
    &AuraEffect::HandlePreventResurrection,                       //314 SPELL_AURA_PREVENT_RESURRECTION todo
    &AuraEffect::HandleNoImmediateEffect,                         //315 SPELL_AURA_UNDERWATER_WALKING todo
    &AuraEffect::HandleNoImmediateEffect,                         //316 unused (4.3.4) old SPELL_AURA_PERIODIC_HASTE
    &AuraEffect::HandleAuraModSpellPowerPercent,                  //317 SPELL_AURA_MOD_SPELL_POWER_PCT
    &AuraEffect::HandleNULL,                                      //318 SPELL_AURA_MASTERY
    &AuraEffect::HandleModAttackSpeed,                            //319 SPELL_AURA_MOD_ATTACKSPEED_2
    &AuraEffect::HandleAuraModRangedHaste,                        //320 SPELL_AURA_MOD_RANGED_HASTE_2
    &AuraEffect::HandleNULL,                                      //321 SPELL_AURA_321
    &AuraEffect::HandleNULL,                                      //322 SPELL_AURA_INTERFERE_TARGETTING
    &AuraEffect::HandleUnused,                                    //323 unused (4.3.4)
    &AuraEffect::HandleNULL,                                      //324 SPELL_AURA_324
    &AuraEffect::HandleUnused,                                    //325 unused (4.3.4)
    &AuraEffect::HandleNULL,                                      //326 SPELL_AURA_326
    &AuraEffect::HandleUnused,                                    //327 unused (4.3.4)
    &AuraEffect::HandleNoImmediateEffect,                         //328 SPELL_AURA_PROC_ON_POWER_AMOUNT implemented in Unit::HandleAuraProcOnPowerAmount
    &AuraEffect::HandleNoImmediateEffect,                         //329 SPELL_AURA_MOD_RUNE_REGEN_SPEED implemented in Spell::TakeRunePower
    &AuraEffect::HandleNoImmediateEffect,                         //330 SPELL_AURA_CAST_WHILE_WALKING
    &AuraEffect::HandleAuraForceWeather,                          //331 SPELL_AURA_FORCE_WEATHER
    &AuraEffect::HandleOverrideActionbarSpells,                   //332 SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS also implemented in WorldSession::HandleCastSpellOpcode
    &AuraEffect::HandleOverrideActionbarSpells,                   //333 SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS_2 also implemented in WorldSession::HandleCastSpellOpcode
    &AuraEffect::HandleNULL,                                      //334 SPELL_AURA_MOD_BLIND
    &AuraEffect::HandleNULL,                                      //335 SPELL_AURA_335
    &AuraEffect::HandleNoImmediateEffect,                         //336 SPELL_AURA_MOD_FLYING_RESTRICTIONS
    &AuraEffect::HandleNoImmediateEffect,                         //337 SPELL_AURA_MOD_VENDOR_ITEMS_PRICES
    &AuraEffect::HandleNoImmediateEffect,                         //338 SPELL_AURA_MOD_DURABILITY_LOSS
    &AuraEffect::HandleNULL,                                      //339 SPELL_AURA_INCREASE_SKILL_GAIN_CHANCE
    &AuraEffect::HandleNULL,                                      //340 SPELL_AURA_SPELL_AURA_MOD_RESURRECTED_HEALTH_BY_GUILD_MEMBER
    &AuraEffect::HandleModCategoryCooldown,                       //341 SPELL_AURA_MOD_SPELL_CATEGORY_COOLDOWN
    &AuraEffect::HandleModMeleeRangedSpeedPct,                    //342 SPELL_AURA_MOD_MELEE_RANGED_HASTE_2
    &AuraEffect::HandleNoImmediateEffect,                         //343 SPELL_AURA_MOD_AUTOATTACK_DAMAGE_TARGET implemented in Unit::MeleeDamageBonusTaken
    &AuraEffect::HandleNoImmediateEffect,                         //344 SPELL_AURA_MOD_AUTOATTACK_DAMAGE implemented in Unit::MeleeDamageBonusTaken
    &AuraEffect::HandleNoImmediateEffect,                         //345 SPELL_AURA_BYPASS_ARMOR_FOR_CASTER
    &AuraEffect::HandleProgressBar,                               //346 SPELL_AURA_ENABLE_ALT_POWER
    &AuraEffect::HandleNoImmediateEffect,                         //347 SPELL_AURA_MOD_COOLDOWN_BY_SPELL_HASTE
    &AuraEffect::HandleNoImmediateEffect,                         //348 SPELL_AURA_AURA_DEPOSIT_BONUS_MONEY_IN_GUILD_BANK_ON_LOOT implemented in WorldSession::HandleLootMoneyOpcode
    &AuraEffect::HandleNoImmediateEffect,                         //349 SPELL_AURA_MOD_CURRENCY_GAIN implemented in Player::ModifyCurrency (TODO?)
    &AuraEffect::HandleNULL,                                      //350 SPELL_AURA_MOD_GATHERING_ITEMS_GAINED_PERCENT
    &AuraEffect::HandleNULL,                                      //351 SPELL_AURA_351
    &AuraEffect::HandleNULL,                                      //352 SPELL_AURA_352
    &AuraEffect::HandleModCamouflage,                             //353 SPELL_AURA_MOD_CAMOUFLAGE
    &AuraEffect::HandleNULL,                                      //354 SPELL_AURA_354
    &AuraEffect::HandleUnused,                                    //355 unused (4.3.4)
    &AuraEffect::HandleNULL,                                      //356 SPELL_AURA_356
    &AuraEffect::HandleNULL,                                      //357 SPELL_AURA_ENABLE_BOSS1_UNIT_FRAME
    &AuraEffect::HandleNoImmediateEffect,                         //358 SPELL_AURA_WORGEN_ALTERED_FORM clientside
    &AuraEffect::HandleNULL,                                      //359 SPELL_AURA_359
    &AuraEffect::HandleNULL,                                      //360 SPELL_AURA_360
    &AuraEffect::HandleNULL,                                      //361 SPELL_AURA_OVERRIDE_AUTOATTACK_WITH_MELEE_SPELL
    &AuraEffect::HandleUnused,                                    //362 unused (4.3.4)
    &AuraEffect::HandleAuraModNextSpell,                          //363 SPELL_AURA_MOD_NEXT_SPELL
    &AuraEffect::HandleUnused,                                    //364 unused (4.3.4)
    &AuraEffect::HandleNULL,                                      //365 SPELL_AURA_365
    &AuraEffect::HandleOverrideSpellPowerByAttackPower,           //366 SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleNULL,                                      //367 SPELL_AURA_OVERRIDE_AUTOATTACK_WITH_RANGED_SPELL
    &AuraEffect::HandleUnused,                                    //368 unused (4.3.4)
    &AuraEffect::HandleNULL,                                      //369 SPELL_SPELL_AURA_ENABLE_POWER_BAR_TIMER
    &AuraEffect::HandleNULL,                                      //370 SPELL_AURA_SET_FAIR_FAR_CLIP
    &AuraEffect::HandleNULL,                                      //371 SPELL_AURA_372
    &AuraEffect::HandleNULL,                                      //372 SPELL_AURA_372
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //373 SPELL_AURA_INCREASE_MIN_SWIM_SPEED
    &AuraEffect::HandleNoImmediateEffect,                         //374 SPELL_AURA_REDUCE_FALL_DAMAGE_PERCENT implemente in Player::EnvironmentalDamage
    &AuraEffect::HandleNULL,                                      //375 SPELL_AURA_375
    &AuraEffect::HandleNULL,                                      //376 SPELL_AURA_MOD_CURRENCY_GAIN_2
    &AuraEffect::HandleNULL,                                      //377 SPELL_AURA_CAST_WHILE_WALKING_2
    &AuraEffect::HandleNULL,                                      //378 SPELL_AURA_378
    &AuraEffect::HandleAuraModIncreaseManaRegen,                  //379 SPELL_AURA_INCREASE_MANA_REGEN
    &AuraEffect::HandleNULL,                                      //380 SPELL_AURA_380
    &AuraEffect::HandleNULL,                                      //381 SPELL_AURA_381
    &AuraEffect::HandleAuraModPetStatPctFromMax,                  //382 SPELL_AURA_MOD_PET_STAT_PCT_FROM_MAX
    &AuraEffect::HandleNoImmediateEffect,                         //383 SPELL_AURA_ALLOW_CAST_WHILE_IN_COOLDOWN
    &AuraEffect::HandleNULL,                                      //384 SPELL_AURA_384
    &AuraEffect::HandleNoImmediateEffect,                         //385 SPELL_AURA_STRIKE_SELF in Unit::AttackerStateUpdate
    &AuraEffect::HandleNULL,                                      //386 SPELL_AURA_386
    &AuraEffect::HandleNULL,                                      //387 SPELL_AURA_387
    &AuraEffect::HandleNoImmediateEffect,                         //388 SPELL_AURA_MOD_TAXI_FLIGHT_SPEED
    &AuraEffect::HandleNoImmediateEffect,                         //389 SPELL_AURA_KIL_JAEDENS_CUNNING in SpellInfo::AttackerStateUpdate
    &AuraEffect::HandleNULL,                                      //390 SPELL_AURA_390
    &AuraEffect::HandleNULL,                                      //391 SPELL_AURA_391
    &AuraEffect::HandleNULL,                                      //392 SPELL_AURA_392
    &AuraEffect::HandleNoImmediateEffect,                         //393 SPELL_AURA_DEFLECT_FRONT_SPELLS
    &AuraEffect::HandleNoImmediateEffect,                         //394 SPELL_AURA_TRIGGER_BONUS_LOOT
    &AuraEffect::HandleNULL,                                      //395 SPELL_AURA_AREA_TRIGGER
    &AuraEffect::HandleNULL,                                      //396 SPELL_AURA_396
    &AuraEffect::HandleNULL,                                      //397 SPELL_AURA_397
    &AuraEffect::HandleNULL,                                      //398 SPELL_AURA_398
    &AuraEffect::HandleNULL,                                      //399 SPELL_AURA_399
    &AuraEffect::HandleAuraModSkillValue,                         //400 SPELL_AURA_MOD_SKILL_VALUE
    &AuraEffect::HandleNULL,                                      //401 SPELL_AURA_401
    &AuraEffect::HandleNULL,                                      //402 SPELL_AURA_402
    &AuraEffect::HandleChangeSpellVisualEffect,                   //403 SPELL_AURA_CHANGE_VISUAL_EFFECT
    &AuraEffect::HandleOverrideAttackPowerBySpellPower,           //404 SPELL_AURA_OVERRIDE_AP_BY_SPELL_POWER_PCT
    &AuraEffect::HandleIncreaseHasteFromItemsByPct,               //405 SPELL_AURA_INCREASE_HASTE_FROM_ITEMS_BY_PCT
    &AuraEffect::HandleNULL,                                      //406 SPELL_AURA_406
    &AuraEffect::HandleModFear,                                   //407 SPELL_AURA_MOD_FEAR_2      TODO : Find the difference between 7 & 407
    &AuraEffect::HandleNULL,                                      //408 SPELL_AURA_408
    &AuraEffect::HandleAllowTurnWhileFalling,                     //409 SPELL_AURA_ALLOW_TURN_WHILE_FALLING
    &AuraEffect::HandleNoImmediateEffect,                         //410 SPELL_AURA_STAMPEDE_ONLY_CURRENT_PET
    &AuraEffect::HandleNoImmediateEffect,                         //411 SPELL_AURA_MOD_CHARGES implemented in Player.cpp
    &AuraEffect::HandleModManaRegenByHaste,                       //412 SPELL_AURA_MOD_MANA_REGEN_BY_HASTE
    &AuraEffect::HandleNULL,                                      //413 SPELL_AURA_413
    &AuraEffect::HandleNULL,                                      //414 SPELL_AURA_414
    &AuraEffect::HandleNULL,                                      //415 SPELL_AURA_415
    &AuraEffect::HandleNoImmediateEffect,                         //416 SPELL_AURA_MOD_COOLDOWN_BY_MELEE_HASTE
    &AuraEffect::HandleNoImmediateEffect,                         //417 SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_MELEE_HASTE
    &AuraEffect::HandleNULL,                                      //418 SPELL_AURA_MOD_MAX_POWER
    &AuraEffect::HandleAuraModIncreaseEnergyPercent,              //419 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT_2
    &AuraEffect::HandleNULL,                                      //420 SPELL_AURA_MOD_ABSORB_EFFECTS_AMOUNT_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //421 SPELL_AURA_MOD_ABSORPTION_PCT implemented in Unit
    &AuraEffect::HandleNULL,                                      //422 SPELL_AURA_422
    &AuraEffect::HandleNULL,                                      //423 SPELL_AURA_423
    &AuraEffect::HandleAuraModPvPPowerHealing,                    //424 SPELL_AURA_MOD_PVP_POWER_HEALING
    &AuraEffect::HandleNULL,                                      //425 SPELL_AURA_425
    &AuraEffect::HandleNULL,                                      //426 SPELL_AURA_426
    &AuraEffect::HandleNULL,                                      //427 SPELL_AURA_427
    &AuraEffect::HandleTriggerAtApply,                            //428 SPELL_AURA_TRIGGER_AT_APPLY
    &AuraEffect::HandleNULL,                                      //429 SPELL_AURA_429
    &AuraEffect::HandleNULL,                                      //430 SPELL_AURA_430
    &AuraEffect::HandleNULL,                                      //431 SPELL_AURA_431
    &AuraEffect::HandleNULL,                                      //432 SPELL_AURA_432
    &AuraEffect::HandleNULL,                                      //433 SPELL_AURA_433
    &AuraEffect::HandleNULL,                                      //434 SPELL_AURA_434
    &AuraEffect::HandleNULL,                                      //435 SPELL_AURA_435
    &AuraEffect::HandleNULL,                                      //436 SPELL_AURA_436
    &AuraEffect::HandleNULL,                                      //437 SPELL_AURA_437
};

AuraEffect::AuraEffect(Aura* base, uint8 effIndex, int32 *baseAmount, Unit* caster):
m_base(base), m_spellInfo(base->GetSpellInfo()),
m_baseAmount(baseAmount ? *baseAmount : m_spellInfo->Effects[effIndex].BasePoints),
m_spellmod(NULL), m_periodicTimer(0), m_tickNumber(0), m_effIndex(effIndex),
m_canBeRecalculated(true), m_isPeriodic(false), hasFixedPeriodic(false), m_amount(0), m_amplitude(0)
{
    m_fixed_periodic.Clear();
}

AuraEffect::~AuraEffect()
{
    delete m_spellmod;
}

void AuraEffect::GetTargetList(std::list<Unit*> & targetList) const
{
    Aura::ApplicationMap const & targetMap = GetBase()->GetApplicationMap();
    // remove all targets which were not added to new list - they no longer deserve area aura
    for (Aura::ApplicationMap::const_iterator appIter = targetMap.begin(); appIter != targetMap.end(); ++appIter)
    {
        if (appIter->second->HasEffect(GetEffIndex()))
            targetList.push_back(appIter->second->GetTarget());
    }
}

void AuraEffect::GetApplicationList(std::list<AuraApplication*> & applicationList) const
{
    Aura::ApplicationMap const & targetMap = GetBase()->GetApplicationMap();
    for (Aura::ApplicationMap::const_iterator appIter = targetMap.begin(); appIter != targetMap.end(); ++appIter)
    {
        if (appIter->second->HasEffect(GetEffIndex()))
            applicationList.push_back(appIter->second);
    }
}

int32 AuraEffect::CalculateAmount(Unit* caster)
{
    int32 amount;
    // default amount calculation
    Item const* tempItemTarget = (caster && GetBase()->GetCastItemGUID() && caster->ToPlayer()) ? caster->ToPlayer()->GetItemByGuid(GetBase()->GetCastItemGUID()) : nullptr;
    amount = m_spellInfo->Effects[m_effIndex].CalcValue(caster, &m_baseAmount, GetBase()->GetOwner()->ToUnit(), tempItemTarget, GetBase()->GetCastItemLevel());

    // check item enchant aura cast
    if (!amount && caster)
        if (uint64 itemGUID = GetBase()->GetCastItemGUID())
            if (Player* playerCaster = caster->ToPlayer())
                if (Item* castItem = playerCaster->GetItemByGuid(itemGUID))
                    if (castItem->GetItemSuffixFactor())
                    {
                        ItemRandomSuffixEntry const* item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(castItem->GetItemRandomPropertyId()));
                        if (item_rand_suffix)
                        {
                            for (int k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; k++)
                            {
                                SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(item_rand_suffix->enchant_id[k]);
                                if (pEnchant)
                                {
                                    for (int t = 0; t < MAX_ITEM_ENCHANTMENT_EFFECTS; t++)
                                        if (pEnchant->spellid[t] == m_spellInfo->Id)
                                    {
                                        amount = uint32((item_rand_suffix->prefix[k]*castItem->GetItemSuffixFactor()) / 10000);
                                        break;
                                    }
                                }

                                if (amount)
                                    break;
                            }
                        }
                    }

    float DoneActualBenefit = 0.0f;

    // custom amount calculations go here
    switch (GetAuraType())
    {
        case SPELL_AURA_ADD_PCT_MODIFIER:
        {
            switch (GetSpellInfo()->Id)
            {
                case 117828:
                    if (GetEffIndex() == EFFECT_1 && caster)
                    {
                        if (caster->HasAura(111546))
                            amount *= 4;
                        if (caster->HasAura(108683))
                            amount = 0;
                    }
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_XP_PCT:
        {
            if (caster && caster->IsPlayer() && GetSpellInfo()->SpellIconID == 3037)
                caster->ToPlayer()->UpdateHeirloomBonus(nullptr);
            break;
        }
        case SPELL_AURA_MOD_STAT:
        {
            switch (GetSpellInfo()->Id)
            {
                case 108300:
                {
                    if (caster && caster->IsInFeralForm())
                        amount = caster->GetStat(STAT_INTELLECT);
                    m_canBeRecalculated = false;
                    break;
                }
            }
            break;
        }
        case SPELL_AURA_MOD_INCREASE_HEALTH_2:
        {
            switch (GetSpellInfo()->Id)
            {
                case 113072:
                case 106922:
                case 132413:
                {
                    if (caster)
                        amount = caster->CountPctFromMaxHealth(amount);
                    m_canBeRecalculated = false;
                    break;
                }
            }
            break;
        }
        case SPELL_AURA_MOD_RATING:
        {
            // Heart's Judgment, Heart of Ignacious trinket (Heroic) 
            if (m_spellInfo->Id == 92328 && caster)
            {
                if (Aura* pAura = caster->GetAura(92325))
                    amount *= pAura->GetStackAmount();
            }
            break;
        }
        // crowd control auras
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_MOD_STUN:
        case SPELL_AURA_MOD_ROOT:
        case SPELL_AURA_TRANSFORM:
        case SPELL_AURA_MOD_FEAR_2:
        {
            m_canBeRecalculated = false;
            bool customAmount = false;

            // Custom entries
            switch (GetSpellInfo()->Id)
            {
                case 118:    // Polymorph
                case 1776:   // Gouge
                case 2094:   // Blind
                case 3355:   // Freezing Trap
                case 20066:  // Repentance
                case 31661:  // Dragon's Breath
                case 105421: // Blinding Light
                case 115078: // Paralysis
                {
                    amount = 1;
                    customAmount = true;
                    break;
                }
                case 5246:  // Intimidating Shout
                case 5782:  // Fear
                case 51514: // Hex
                case 82691: // Ring of Frost
                {
                    amount = irand(1, GetBase()->GetUnitOwner()->CountPctFromMaxHealth(10));
                    customAmount = true;
                    break;
                }
                case 128405:// Narrow Escape
                {
                    amount = int32(GetBase()->GetUnitOwner()->CountPctFromMaxHealth(10));
                    customAmount = true;
                    break;
                }
                default:
                    break;
            }

            if (customAmount)
                break;
            if (!m_spellInfo->ProcFlags)
                break;

            amount = int32(GetBase()->GetUnitOwner()->CountPctFromMaxHealth(10));
            if (caster)
            {
                // Glyphs increasing damage cap
                Unit::AuraEffectList const& overrideClassScripts = caster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (Unit::AuraEffectList::const_iterator itr = overrideClassScripts.begin(); itr != overrideClassScripts.end(); ++itr)
                {
                    if ((*itr)->IsAffectingSpell(m_spellInfo))
                    {
                        // Glyph of Frost nova and similar auras
                        if ((*itr)->GetMiscValue() == 7801)
                        {
                            AddPct(amount, (*itr)->GetAmount());
                            break;
                        }
                    }
                }
            }
            break;
        }
        // Any damage breaks confusion
        case SPELL_AURA_MOD_CONFUSE:
            amount = 1;
            break;
        case SPELL_AURA_SCHOOL_ABSORB:
            m_canBeRecalculated = false;
            if (!caster)
                break;
            switch (GetSpellInfo()->SpellFamilyName)
            {
                case SPELLFAMILY_MONK:
                    // Life Cocoon
                    if (GetSpellInfo()->Id == 116849)
                    {
                        // +1100% from sp bonus
                        DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 11.0f;
                        DoneActualBenefit *= caster->ToPlayer()->GetPvpHealingBonus();
                    }
                    break;
                case SPELLFAMILY_MAGE:
                    // Ice Barrier
                    if (GetSpellInfo()->Id == 11426)
                    {
                        // +330% from sp bonus
                        DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 3.3f;
                    }
                    // Mage Ward
                    else if (GetSpellInfo()->SpellFamilyFlags[0] & 0x8 && GetSpellInfo()->SpellFamilyFlags[2] & 0x8)
                    {
                        // +80.68% from sp bonus
                        DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 0.8068f;
                    }
                    break;
                case SPELLFAMILY_WARLOCK:
                    // Twilight Ward
                    if (m_spellInfo->Id == 6229 || m_spellInfo->Id == 104048 || m_spellInfo->Id == 131623 || m_spellInfo->Id == 131624)
                    {
                        // +300% from sp bonus
                        DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 3.0f;
                        DoneActualBenefit *= caster->ToPlayer() ? caster->ToPlayer()->GetPvpHealingBonus() : 1.0f;
                    }
                    break;
                case SPELLFAMILY_PRIEST:
                    // Power Word : Shield
                    if (GetSpellInfo()->Id == 17 || GetSpellInfo()->Id == 123258)
                    {
                        if (Player* _plr = caster->ToPlayer())
                        {
                            // +187.1% from sp bonus
                            DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 1.871f;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case SPELL_AURA_MANA_SHIELD:
            m_canBeRecalculated = false;
            if (!caster)
                break;
            // Mana Shield
            if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_MAGE && GetSpellInfo()->SpellFamilyFlags[0] & 0x8000 && m_spellInfo->SpellFamilyFlags[2] & 0x8)
            {
                // +80.7% from +spd bonus
                DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 0.807f;
            }
            break;
        case SPELL_AURA_DUMMY:
            if (!caster)
                break;
            // Earth Shield
            if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_SHAMAN && m_spellInfo->SpellFamilyFlags[1] & 0x400)
            {
                amount = caster->SpellHealingBonusDone(GetBase()->GetUnitOwner(), GetSpellInfo(), amount, SPELL_DIRECT_DAMAGE);
                amount = GetBase()->GetUnitOwner()->SpellHealingBonusTaken(caster, GetSpellInfo(), GetEffIndex(), amount, SPELL_DIRECT_DAMAGE);
            }
            if (GetSpellInfo()->Id == 114637 && caster->ToPlayer())
                amount = 10 + caster->ToPlayer()->GetFloatValue(PLAYER_MASTERY);
            if (GetSpellInfo()->Id == 33763)
            {
                DoneActualBenefit += caster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()) * 0.75f;
                DoneActualBenefit *= caster->ToPlayer()->GetPvpHealingBonus();
            }
            break;
        case SPELL_AURA_PERIODIC_DAMAGE:
            if (!caster)
                break;

            switch (GetSpellInfo()->Id)
            {
                // Rupture
                case 1943:
                {
                    m_canBeRecalculated = false;

                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        break;

                    uint8 cp = caster->ToPlayer()->GetComboPoints();
                    float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);

                    switch (cp)
                    {
                        case 1:
                            amount += int32(ap * 0.1f / 4);
                            break;
                        case 2:
                            amount += int32(ap * 0.24f / 6);
                            break;
                        case 3:
                            amount += int32(ap * 0.40f / 8);
                            break;
                        case 4:
                            amount += int32(ap * 0.56f / 10);
                            break;
                        case 5:
                            amount += int32(ap * 0.744f / 12);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                // Rip
                case 1079:
                {
                    m_canBeRecalculated = false;

                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        break;

                    // Basepoint hotfix
                    amount *= 10;

                    uint8 cp = caster->ToPlayer()->GetComboPoints();
                    int32 AP = caster->GetTotalAttackPowerValue(BASE_ATTACK);

                    // In feral spec : 0.484 * $AP * cp
                    if (caster->ToPlayer()->GetSpecializationId() == SPEC_DRUID_CAT)
                        amount += int32(cp * AP * 0.484f);
                    // In other spec : 0.387 * $AP * cp
                    else
                        amount += int32(cp * AP * 0.387f);

                    // Idol of Feral Shadows. Cant be handled as SpellMod in SpellAura:Dummy due its dependency from CPs
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(34241, EFFECT_0))
                        amount += cp * aurEff->GetAmount();
                    // Idol of Worship. Cant be handled as SpellMod in SpellAura:Dummy due its dependency from CPs
                    else if (AuraEffect const* aurEff = caster->GetAuraEffect(60774, EFFECT_0))
                        amount += cp * aurEff->GetAmount();

                    amount /= int32(GetBase()->GetMaxDuration() / GetBase()->GetEffect(0)->GetAmplitude());
                    break;
                }
                // Unholy Blight damage over time effect
                case 50536:
                    m_canBeRecalculated = false;
                    // we're getting total damage on aura apply, change it to be damage per tick
                    amount = int32((float)amount / GetTotalTicks());
                    break;
            }
            break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            switch (m_spellInfo->Id)
            {
                case 57669: // Replenishment (0.2% from max)
                    amount = CalculatePct(GetBase()->GetUnitOwner()->GetMaxPower(POWER_MANA), amount);
                    break;
                case 61782: // Infinite Replenishment
                    amount = GetBase()->GetUnitOwner()->GetMaxPower(POWER_MANA) * 0.0025f;
                    break;
                case 29166: // Innervate
                    ApplyPct(amount, float(GetBase()->GetUnitOwner()->GetCreatePowers(POWER_MANA)) / GetTotalTicks());
                    break;
                case 48391: // Owlkin Frenzy
                    ApplyPct(amount, GetBase()->GetUnitOwner()->GetCreatePowers(POWER_MANA));
                    break;
                case 54428: // Divine Plea
                    if (!caster)
                        break;
                    amount = caster->GetStat(STAT_SPIRIT) * 1.35f;
                    break;
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        {
            if (!caster)
                break;

            switch (GetId())
            {
                case 746:   // First Aid
                case 1159:
                case 3267:
                case 3268:
                case 7926:
                case 7927:
                case 10838:
                case 10839:
                case 18608:
                case 18610:
                case 27030:
                case 27031:
                case 45543:
                case 45544:
                case 51803:
                case 51827:
                case 72996:
                case 74553:
                case 74554:
                case 74555:
                case 102694:
                case 102695:
                {
                    // The Doctor Is In
                    if (Aura* doctorIsIn = caster->GetAura(118076))
                    {
                        if (caster->GetTypeId() != TYPEID_PLAYER)
                            break;

                        Player* _plr = caster->ToPlayer();
                        if (_plr->GetMap()->IsBattlegroundOrArena())
                            break;

                        AddPct(amount, doctorIsIn->GetEffect(0)->GetAmount());
                    }

                    break;
                }
                case 90361: // Spirit Mend
                {
                    if (!caster->GetOwner())
                        break;

                    Player* m_owner = caster->GetOwner()->ToPlayer();
                    if (!m_owner)
                        break;

                    amount += int32(m_owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.35f * 0.335f);

                    break;
                }
                case 114163:// Eternal Flame
                {
                    amount += int32(0.0585f * caster->SpellBaseDamageBonusDone(SpellSchoolMask(m_spellInfo->SchoolMask)));

                    int32 holyPower = caster->GetPower(POWER_HOLY_POWER) + 1;

                    if (holyPower > 3)
                        holyPower = 3;

                    // Divine Purpose
                    if (caster->HasAura(90174))
                        holyPower = 3;

                    // Glyph of Word of Glory
                    if (caster->HasAura(54936))
                    {
                        Aura* aura = caster->AddAura(115522, caster);

                        if (aura)
                        {
                            aura->GetEffect(0)->ChangeAmount(aura->GetEffect(0)->GetAmount() * (holyPower + 1));
                            aura->SetNeedClientUpdateForTargets();
                        }
                    }

                    amount *= holyPower;

                    // If aura applies on paladin it should heal 50% more
                    if (caster->GetGUID() ==  GetBase()->GetUnitOwner()->GetGUID())
                        amount = int32(amount * 1.5f);

                    if (auto _effect = caster->GetAuraEffect(114637, EFFECT_0))
                    {
                        AddPct(amount, _effect->GetAmount());
                        if (auto _base = _effect->GetBase())
                            _base->Remove();
                    }

                    // If no Divine Purpose
                    if (!caster->HasAura(90174))
                        caster->ModifyPower(POWER_HOLY_POWER, (holyPower > 1) ? (-(holyPower - 1)) : 0);

                    break;
                }
                // Recuperate
                case 73651:
                {
                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        break;

                    int32 heal_pct = amount * 1000;

                    // Glyph of Recuperate - 56806
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_ROGUE, 4885, 0))
                        heal_pct += aurEff->GetAmount();
                    
                    amount = CalculatePct(caster->GetMaxHealth(), 0.001 * heal_pct);

                    // Pvp power
                    float PvPPower = caster->ToPlayer()->GetPvpHealingBonus();
                    amount = int32(amount * PvPPower);

                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_THREAT:
        {
            uint8 level_diff = 0;
            float multiplier = 0.0f;
            switch (GetId())
            {
                // Arcane Shroud
                case 26400:
                    level_diff = GetBase()->GetUnitOwner()->getLevel() - 60;
                    multiplier = 2;
                    break;
                // The Eye of Diminution
                case 28862:
                    level_diff = GetBase()->GetUnitOwner()->getLevel() - 60;
                    multiplier = 1;
                    break;
            }
            if (level_diff > 0)
                amount += int32(multiplier * level_diff);
            break;
        }
        case SPELL_AURA_MOD_INCREASE_HEALTH:
            // Vampiric Blood
            if (GetId() == 55233)
                amount = GetBase()->GetUnitOwner()->CountPctFromMaxHealth(amount);
            break;
        case SPELL_AURA_MOD_INCREASE_SPEED:
            // Dash - do not set speed if not in cat form
            if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_DRUID && GetSpellInfo()->SpellFamilyFlags[2] & 0x00000008)
                amount = GetBase()->GetUnitOwner()->GetShapeshiftForm() == FORM_CAT ? amount : 0;
            break;
        case SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE:
        {
            if (caster)
            {
                // if Level <= 70 resist = player level
                int32 resist = caster->getLevel();

                if (resist > 70 && resist < 81)
                    resist += (resist - 70) * 5;
                else if (resist > 80)
                    resist += ((resist-70) * 5 + (resist - 80) * 7);

                switch (GetId())
                {
                    case 20043: // Aspect of the Wild
                    case 8185:  // Elemental Resistance
                    case 19891: // Resistance Aura
                    case 79106: // Shadow Protection
                    case 79107: // Shadow Protection
                        amount = resist;
                        break;
                    case 79060: // Mark of the Wild
                    case 79061: // Mark of the Wild
                    case 79062: // Blessing of Kings
                    case 79063: // Blessing of Kings
                    case 90363: // Embrace of the Shale Spider
                        amount = resist / 2;
                        break;
                    }
                break;
            }
        }
        case SPELL_AURA_BYPASS_ARMOR_FOR_CASTER:
        {
            switch (GetId())
            {
                case 108126: // Colossus Smash
                    if (GetBase()->GetUnitOwner()->GetTypeId() == TYPEID_PLAYER)
                        amount /= 2;
                    break;
                case 91021: // Find Weakness
                    if (GetBase()->GetUnitOwner()->GetTypeId() == TYPEID_PLAYER)
                        amount /= 2;
                    break;
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
        {
            if (!caster)
                break;

            switch (GetId())
            {
                case 120954:// Fortifying Brew
                {
                    // Glyph of Fortifying Brew
                    if (caster->HasAura(124997))
                        amount = 10;

                    break;
                }
                case 108503://Grimoire of Sacrifice
                {
                    // If warlock doesn't have Soul Link talent we don't need to increase hp
                    if (!caster->HasAura(108446))
                        amount = 0;
                    
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
        {
            if (!caster)
                break;

            switch (GetId())
            {
                case 120954:// Fortifying Brew
                {
                    // Glyph of Fortifying Brew
                    if (caster->HasAura(124997))
                        amount = 25;

                    break;
                }
                case 132403:
                {
                    if (caster->ToPlayer())
                    {
                        int32 val = std::min(std::max(25 + int32(caster->ToPlayer()->GetFloatValue(PLAYER_MASTERY)), 25), 80);
                        amount = -val;
                    }
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_OBS_MOD_HEALTH:
        {
            if (!caster)
                break;

            switch (GetId())
            {
                case 6262:  // Healthstone
                    if (!caster->HasAura(56224)) // Glyph of Healthstone
                        amount = 0;
                    break;
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_DECREASE_SPEED:
        {
            if (!caster)
                break;

            switch (GetId())
            {
                case 73682: // Unleash Frost
                {
                    if (Unit* target = GetBase()->GetUnitOwner())
                    {
                        Unit::AuraEffectList const& auras = target->GetAuraEffectsByType(SPELL_AURA_MOD_DECREASE_SPEED);
                        for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                        {
                            if (GetId() != (*itr)->GetSpellInfo()->Id && (*itr)->GetSpellInfo()->GetSchoolMask() == SPELL_SCHOOL_MASK_FROST)
                            {
                                amount = 70;
                                break;
                            }
                        }
                    }

                    break;
                }
                // Glyph of Hurricane - 54831
                case 16914: // Hurricane
                case 106996:// Astral Storm
                {
                    if (caster->HasAura(54831))
                        amount = 50;
                    else
                        amount = 0;

                    break;
                }
                // Glyph of Death and Decay
                case 43265: // Death and Decay
                {
                    if (!caster->HasAura(58629))
                        amount = 0;
                    else
                        amount = -50;
                    
                    break;
                }
                case 5215:  // Prowl
                case 102547:// Prowl (overrided)
                {
                    // Glyph of Prowl, remove the speed malus
                    if (Unit* target = GetBase()->GetUnitOwner())
                        if (target->HasAura(116186))
                            amount = 0;
                    break;
                }
                case 15407: // Mind flay
                {
                    // Glyph of Mind Flay, remove the slow effect
                    if (caster->HasAura(120585))
                        amount = 0;
                    break;
                }
            }
        }
        case SPELL_AURA_MOD_SPEED_ALWAYS:
        {
            // Stealth / Prowl
            if (m_spellInfo->HasAura(SPELL_AURA_MOD_STEALTH))
            {
                Unit * target = GetBase()->GetUnitOwner();
                if (!target)
                    break;

                // Elusiveness (Racial Passive)
                if (AuraEffect* eff = target->GetAuraEffect(21009, 0))
                    amount += eff->GetAmount();
            }
            // Shapeshift Movement Speed (23218) doesn't stack with Feline Swiftness (131768)
            if (GetSpellInfo()->Id == 23218)
                amount = GetBase()->GetUnitOwner()->HasAura(131768) ? 0 : amount;
            // Soul Reaper movement speed effect should work just if dk has Glyph of Swift Death
            if (GetSpellInfo()->Id == 114868)
                amount = GetBase()->GetUnitOwner()->HasAura(146645) ? amount : 0;
            break;
        }
        case SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK:
        {
            switch (GetId())
            {
                case 12472: // Icy Veins
                {
                    if (!caster)
                        break;

                    // Glyph of Icy Veins, remove casting speed bonus effect
                    if (caster->HasAura(56364))
                        amount = 0;

                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_ATTACKSPEED_2:
        {
            switch (GetId())
            {
                case 5171:  // Slice and Dice
                {
                    if (!caster)
                        break;

                    Player* plr = caster->ToPlayer();
                    if (!plr)
                        break;

                    if (!plr->HasAura(76808) && !plr->HasAura(79152))
                        break;

                    float MasteryPCT = 1.0f + plr->GetFloatValue(PLAYER_MASTERY) * 3.0f;
                    AddPct(amount, MasteryPCT);

                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_PARRY_PERCENT:
        {
            switch (GetId())
            {
                case 113656:// Fists of Fury
                {
                    if (caster)
                        if (caster->HasAura(125671))
                            amount = 100;

                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_INCREASE_SWIM_SPEED:
        {
            switch (GetId())
            {
                case 86496: // Mount Speed Mod: Walking Speed Ground Mount
                {
                    if (!caster)
                        break;

                    if (uint32 mapId = caster->GetZoneId())
                    {
                        // Vashj'Ir
                        if (mapId == 4815 || mapId == 5144 ||mapId == 5145 || mapId == 5146)
                            amount = 300;
                        else
                            break;
                    }

                    break;
                }
                case 5421: // Aquatic Form
                case 1066:
                {
                    if (!caster)
                        break;

                    amount = 150;
                    
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            switch (GetId())
            {
                case 49016: // Unholy Frenzy
                {
                    if (!caster)
                        break;

                    // Glyph of Unholy Frenzy
                    if (caster->HasAura(58616))
                        return 0;

                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
        {
            switch (GetId())
            {
                case 112071: // Celestial Alignment
                {
                    if (!caster)
                        break;

                    amount = int32(caster->GetFloatValue(PLAYER_MASTERY) * 1.875f) + 15;

                    // Dream of Cenarius (balance) - 14515
                    if (caster->HasAura(145151))
                        amount += 25;

                    break;
                }
                default:
                    break;
            }

            break;
        }
        default:
            break;

    }
    if (DoneActualBenefit != 0.0f && caster)
    {
        DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellInfo());
        amount += (int32)DoneActualBenefit;
    }

    // Power Word: Shield and Ice Barrier increase from PVP-power
    if (caster && (GetSpellInfo()->Id == 17 || GetSpellInfo()->Id == 123258 || GetSpellInfo()->Id == 11426))
    {
        if (Player* _plr = caster->ToPlayer())
            amount *= _plr->GetPvpHealingBonus();
    }

    uint8 stackAmount = GetBase()->GetStackAmount();

    // Agony works in a different way, its damage are pigfixed by multiplying by stack amount in damage applying
    // This way, we don't need to recalculate its damage everytime it add a stack
    // But we need the final damage (with 10 stacks), so let's compute it now thay way (even if we are at 1 stack)
    if (GetId() == 980)
        stackAmount = 10;

    bool l_Prevented = GetBase()->CallScriptEffectCalcAmountHandlers(this, amount, m_canBeRecalculated);
    if (GetId() != 137639) // Earth, Storm and Fire
        amount *= stackAmount;

    if (Unit* target = GetBase()->GetOwner()->ToUnit())
    {
        if (caster && GetAuraType() == SPELL_AURA_SCHOOL_ABSORB)
        {
            for (AuraEffect * l_Effect : target->GetAuraEffectsByType(SPELL_AURA_MOD_HEALING_ABSORB_RECEIVED))
            {
                if (caster->GetGUID() == l_Effect->GetCasterGUID() && l_Effect->IsAffectingSpell(GetSpellInfo()))
                    amount *= (1.0f + CalculatePct(1.0f, l_Effect->GetAmount()));
            }
        }
    }

    // Fixate damage for periodic damage auras
    // It's only for players now
    if (!l_Prevented && caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE || 
        GetAuraType() == SPELL_AURA_PERIODIC_LEECH || 
        GetAuraType() == SPELL_AURA_PERIODIC_HEAL)
        {
            if (GetBase()->GetType() == UNIT_AURA_TYPE)
            {
                Unit* target = GetBase()->GetUnitOwner();
                int32 temp_damage = amount;
                float temp_crit = 0.0f;

                if (GetAuraType() == SPELL_AURA_PERIODIC_HEAL)
                    temp_damage = caster->SpellHealingBonusDone(target, GetSpellInfo(), temp_damage, DOT, stackAmount);
                else
                    temp_damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), temp_damage, DOT, stackAmount);
                
                if (!GetSpellInfo()->HasAttribute(SPELL_ATTR2_CANT_CRIT))
                    temp_crit = caster->GetSpellCrit(target, GetSpellInfo(), SpellSchoolMask(GetSpellInfo()->SchoolMask));

                m_fixed_periodic.SetFixedDamage(temp_damage);
                m_fixed_periodic.SetCriticalChance(temp_crit);

                // Seed of Corruption and Soulburn : Seed of corruption - Set Total damage for explode
                if (GetSpellInfo()->Id == 27243 || GetSpellInfo()->Id == 114790)
                    m_fixed_periodic.SetFixedTotalDamage(temp_damage * (GetBase()->GetMaxDuration() / GetAmplitude()));

                hasFixedPeriodic = true;
                amount = temp_damage;
            }
        }
    }

    return amount;
}

void AuraEffect::CalculatePeriodic(Unit* caster, bool resetPeriodicTimer /*= true*/, bool load /*= false*/)
{
    m_amplitude = m_spellInfo->Effects[m_effIndex].Amplitude;

    // prepare periodics
    switch (GetAuraType())
    {
        case SPELL_AURA_OBS_MOD_POWER:
            // 3 spells have no amplitude set
            if (!m_amplitude)
                m_amplitude = 1 * IN_MILLISECONDS;
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        case SPELL_AURA_PERIODIC_ENERGIZE:
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        case SPELL_AURA_POWER_BURN:
        case SPELL_AURA_PERIODIC_DUMMY:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
            m_isPeriodic = true;
            break;
        default:
            break;
    }

    GetBase()->CallScriptEffectCalcPeriodicHandlers(this, m_isPeriodic, m_amplitude);

    if (!m_isPeriodic)
        return;

    Player* modOwner = caster ? caster->GetSpellModOwner() : NULL;

    // Apply casting time mods
    if (m_amplitude)
    {
        // Apply periodic time mod
        if (modOwner)
            modOwner->ApplySpellMod(GetId(), SPELLMOD_ACTIVATION_TIME, m_amplitude);

        if (caster)
        {
            // Haste modifies periodic time of channeled spells
            if (m_spellInfo->IsChanneled())
            {
                if (m_spellInfo->AttributesEx5 & SPELL_ATTR5_HASTE_AFFECT_DURATION)
                    caster->ModSpellCastTime(m_spellInfo, m_amplitude);
            }
            else if (m_spellInfo->AttributesEx5 & SPELL_ATTR5_HASTE_AFFECT_DURATION)
            {
                m_amplitude = int32(m_amplitude * caster->GetFloatValue(UNIT_MOD_CAST_SPEED));
            }
        }
    }

    if (load) // aura loaded from db
    {
        m_tickNumber = m_amplitude ? GetBase()->GetDuration() / m_amplitude : 0;
        m_periodicTimer = m_amplitude ? GetBase()->GetDuration() % m_amplitude : 0;
        if (m_spellInfo->AttributesEx5 & SPELL_ATTR5_START_PERIODIC_AT_APPLY)
            ++m_tickNumber;
    }
    else // aura just created or reapplied
    {
        m_tickNumber = 0;
        // reset periodic timer on aura create or on reapply when aura isn't dot
        // possibly we should not reset periodic timers only when aura is triggered by proc
        // or maybe there's a spell attribute somewhere
        if (resetPeriodicTimer)
        {
            m_periodicTimer = 0;
            // Start periodic on next tick or at aura apply
            if (m_amplitude && !(m_spellInfo->AttributesEx5 & SPELL_ATTR5_START_PERIODIC_AT_APPLY))
                m_periodicTimer += m_amplitude;
        }
    }
}

void AuraEffect::CalculateSpellMod()
{
    switch (GetAuraType())
    {
        case SPELL_AURA_DUMMY:
            switch (GetSpellInfo()->SpellFamilyName)
            {
                case SPELLFAMILY_DRUID:
                    switch (GetId())
                    {
                        case 34246:                                 // Idol of the Emerald Queen
                        case 60779:                                 // Idol of Lush Moss
                        {
                            if (!m_spellmod)
                            {
                                m_spellmod = new SpellModifier(GetBase());
                                m_spellmod->op = SPELLMOD_DOT;
                                m_spellmod->type = SPELLMOD_FLAT;
                                m_spellmod->spellId = GetId();
                                m_spellmod->mask[1] = 0x0010;
                            }
                            m_spellmod->value = GetAmount()/7;
                        }
                        break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case SPELL_AURA_ADD_FLAT_MODIFIER:
        case SPELL_AURA_ADD_PCT_MODIFIER:
            if (!m_spellmod)
            {
                m_spellmod = new SpellModifier(GetBase());
                m_spellmod->op = SpellModOp(GetMiscValue());
                ASSERT(m_spellmod->op < MAX_SPELLMOD);

                m_spellmod->type = SpellModType(uint32(GetAuraType()));    // SpellModType value == spell aura types
                m_spellmod->spellId = GetId();
                m_spellmod->mask = GetSpellInfo()->Effects[GetEffIndex()].SpellClassMask;
                m_spellmod->charges = GetBase()->GetCharges();
            }
            m_spellmod->value = GetAmount();

            switch (GetId())
            {
                case 125671:// Glyph of Fists of Fury
                    // Increases Parry chance by 100% while channeling Fists of Fury
                    m_spellmod->mask = sSpellMgr->GetSpellInfo(113686)->SpellFamilyFlags;
                    break;
                case 114232:// Sanctified Wrath
                {
                    if (!GetCaster())
                        break;

                    Player* _player = GetCaster()->ToPlayer();
                    if (!_player)
                        break;

                    // Sanctity of Battle
                    float reducePct = (100 - _player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_MELEE) * _player->GetRatingMultiplier(CR_HASTE_MELEE)) / 100.0f;
                    int32 newValue = int32(float(m_spellmod->value) * reducePct);
                    m_spellmod->value = newValue;

                    switch (_player->GetSpecializationId(_player->GetActiveSpec()))
                    {
                        case SPEC_PALADIN_HOLY: // Holy Shock - Effect 0 : Cooldown
                        {
                            if (m_effIndex != 0)
                                m_spellmod->value = 0;

                            break;
                        }
                        case SPEC_PALADIN_PROTECTION: // Judgement - Effect 1 : Cooldown ...
                        {
                            if (m_effIndex != 1)
                                m_spellmod->value = 0;

                            break;
                        }
                        case SPEC_PALADIN_RETRIBUTION: // Hammer of Wrath - Effect 2 : Cooldown
                        {
                            if (m_effIndex != 2)
                                m_spellmod->value = 0;

                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        case SPELL_AURA_MOD_COOLDOWN_BY_MELEE_HASTE:
        case SPELL_AURA_MOD_COOLDOWN_BY_SPELL_HASTE:
        case SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_MELEE_HASTE:
        {
            if (!GetCaster()->ToPlayer())
                break;

            if (!m_spellmod)
            {
                m_spellmod = new SpellModifier(GetBase());
                m_spellmod->op = (GetAuraType() == SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_MELEE_HASTE) ? SPELLMOD_GLOBAL_COOLDOWN : SPELLMOD_COOLDOWN;
                m_spellmod->type = SPELLMOD_PCT;
                m_spellmod->spellId = GetId();
                m_spellmod->mask = m_spellInfo->Effects[GetEffIndex()].SpellClassMask;
            }

            // Pick the right haste value depending on aura type
            float l_HasteValue;
            switch (GetAuraType())
            {
                case SPELL_AURA_MOD_COOLDOWN_BY_SPELL_HASTE:
                    l_HasteValue = GetCaster()->GetFloatValue(UNIT_MOD_CAST_HASTE); //< Spell haste
                    break;

                default:
                    l_HasteValue = GetCaster()->GetFloatValue(UNIT_MOD_HASTE); //< Melee haste
                    break;
            }

            m_spellmod->value = -floor((GetAmount() * (1.0f - l_HasteValue)) + 0.5f);
            m_spellmod->value = std::max(std::min(m_spellmod->value, 0), -100);
            break;
        }
        default:
            break;
    }
    GetBase()->CallScriptEffectCalcSpellModHandlers(this, m_spellmod);
}

void AuraEffect::ChangeAmount(int32 newAmount, bool mark, bool onStackOrReapply)
{
    // Reapply if amount change
    uint8 handleMask = 0;
    if (newAmount != GetAmount())
        handleMask |= AURA_EFFECT_HANDLE_CHANGE_AMOUNT;
    if (onStackOrReapply)
        handleMask |= AURA_EFFECT_HANDLE_REAPPLY;

    if (!handleMask)
        return;

    std::list<AuraApplication*> effectApplications;
    GetApplicationList(effectApplications);

    for (std::list<AuraApplication*>::const_iterator apptItr = effectApplications.begin(); apptItr != effectApplications.end(); ++apptItr)
        if ((*apptItr)->HasEffect(GetEffIndex()))
            HandleEffect(*apptItr, handleMask, false);

    if (handleMask & AURA_EFFECT_HANDLE_CHANGE_AMOUNT)
    {
        if (!mark)
            m_amount = newAmount;
        else
            SetAmount(newAmount);
    }

    if (handleMask & AURA_EFFECT_HANDLE_REAPPLY)
        CalculateSpellMod();

    for (std::list<AuraApplication*>::const_iterator apptItr = effectApplications.begin(); apptItr != effectApplications.end(); ++apptItr)
        if ((*apptItr)->HasEffect(GetEffIndex()))
            HandleEffect(*apptItr, handleMask, true);
}

void AuraEffect::HandleEffect(AuraApplication * aurApp, uint8 mode, bool apply)
{
    // check if call is correct, we really don't want using bitmasks here (with 1 exception)
    ASSERT(mode == AURA_EFFECT_HANDLE_REAL
        || mode == AURA_EFFECT_HANDLE_SEND_FOR_CLIENT
        || mode == AURA_EFFECT_HANDLE_CHANGE_AMOUNT
        || mode == AURA_EFFECT_HANDLE_STAT
        || mode == AURA_EFFECT_HANDLE_SKILL
        || mode == AURA_EFFECT_HANDLE_REAPPLY
        || mode == (AURA_EFFECT_HANDLE_CHANGE_AMOUNT | AURA_EFFECT_HANDLE_REAPPLY));

    // register/unregister effect in lists in case of real AuraEffect apply/remove
    // registration/unregistration is done always before real effect handling (some effect handlers code is depending on this)
    if (mode & AURA_EFFECT_HANDLE_REAL)
        aurApp->GetTarget()->_RegisterAuraEffect(this, apply);

    // real aura apply/remove, handle modifier
    if (mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_REAPPLY))
        ApplySpellMod(aurApp->GetTarget(), apply);

    // call scripts helping/replacing effect handlers
    Aura* auraBase = GetBase();
    bool prevented = false;

    if (auraBase)
    {
        if (apply)
            prevented = auraBase->CallScriptEffectApplyHandlers(this, const_cast<AuraApplication const*>(aurApp), (AuraEffectHandleModes)mode);
        else
            prevented = auraBase->CallScriptEffectRemoveHandlers(this, const_cast<AuraApplication const*>(aurApp), (AuraEffectHandleModes)mode);
    }

    // check if script events have removed the aura or if default effect prevention was requested
    if ((apply && aurApp->GetRemoveMode()) || prevented)
        return;

    if (GetAuraType() >= TOTAL_AURAS)
        return;

    // TODO: We may reimplement this if it would be work bad. Should we?
    // Some Auras can stack from different caster but their amount should not stack
    //if (m_base->IsUniqueVisibleAuraBuff() && IsUniqueStackAuraType())
        //DoUniqueStackAura(aurApp, mode, apply);
    //else
        (*this.*AuraEffectHandler [GetAuraType()])(const_cast<AuraApplication const*>(aurApp), mode, apply);

    // check if script events have removed the aura or if default effect prevention was requested
    if (apply && aurApp->GetRemoveMode())
        return;

    // call scripts triggering additional events after apply/remove
    if (auraBase)
    {
        if (apply)
            auraBase->CallScriptAfterEffectApplyHandlers(this, const_cast<AuraApplication const*>(aurApp), (AuraEffectHandleModes)mode);
        else
            auraBase->CallScriptAfterEffectRemoveHandlers(this, const_cast<AuraApplication const*>(aurApp), (AuraEffectHandleModes)mode);
    }
}

void AuraEffect::HandleEffect(Unit* target, uint8 mode, bool apply)
{
    AuraApplication* aurApp = GetBase()->GetApplicationOfTarget(target->GetGUID());
    ASSERT(aurApp);
    HandleEffect(aurApp, mode, apply);
}

void AuraEffect::ApplySpellMod(Unit* target, bool apply)
{
    if (!m_spellmod)
        return;

    Player* modOwner = target->GetSpellModOwner();

    if (modOwner)
        modOwner->AddSpellMod(m_spellmod, apply);
    else
        return;

    Aura* checkBase = GetBase();
    // Auras with charges do not mod amount of passive auras
    if (checkBase && checkBase->IsUsingCharges())
        return;

    // reapply some passive spells after add/remove related spellmods
    // Warning: it is a dead loop if 2 auras each other amount-shouldn't happen
    switch (GetMiscValue())
    {
        case SPELLMOD_ALL_EFFECTS:
        case SPELLMOD_EFFECT1:
        case SPELLMOD_EFFECT2:
        case SPELLMOD_EFFECT3:
        case SPELLMOD_EFFECT5:
        {
            uint64 guid = target->GetGUID();
            Unit::AuraApplicationMap & auras = target->GetAppliedAuras();
            for (Unit::AuraApplicationMap::iterator iter = auras.begin(); iter != auras.end(); ++iter)
            {
                Aura* aura = iter->second->GetBase();
                // only passive and permament auras-active auras should have amount set on spellcast and not be affected
                // if aura is casted by others, it will not be affected
                if ((aura->IsPassive() || aura->IsPermanent()) && aura->GetCasterGUID() == guid && aura->GetSpellInfo()->IsAffectedBySpellMod(m_spellmod))
                {
                    if (GetMiscValue() == SPELLMOD_ALL_EFFECTS)
                    {
                        for (uint8 i = 0; i<MAX_SPELL_EFFECTS; ++i)
                        {
                            if (AuraEffect* aurEff = aura->GetEffect(i))
                                aurEff->RecalculateAmount();
                        }
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT1)
                    {
                       if (AuraEffect* aurEff = aura->GetEffect(0))
                            aurEff->RecalculateAmount();
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT2)
                    {
                       if (AuraEffect* aurEff = aura->GetEffect(1))
                            aurEff->RecalculateAmount();
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT3)
                    {
                       if (AuraEffect* aurEff = aura->GetEffect(2))
                            aurEff->RecalculateAmount();
                    }
                    else if (GetMiscValue() == SPELLMOD_EFFECT5)
                    {
                       if (AuraEffect* aurEff = aura->GetEffect(4))
                            aurEff->RecalculateAmount();
                    }
                }
            }
        }
        default:
            break;
    }
}

void AuraEffect::Update(uint32 diff, Unit* caster)
{
    Aura* baseAura = GetBase();
    if (baseAura)
        baseAura->CallScriptEffectUpdateHandlers(diff, this);

    if (m_isPeriodic && baseAura && (baseAura->GetDuration() >= 0 || baseAura->IsPassive() || baseAura->IsPermanent()))
    {
        if (m_periodicTimer > int32(diff))
            m_periodicTimer -= diff;
        else // tick also at m_periodicTimer == 0 to prevent lost last tick in case max m_duration == (max m_periodicTimer)*N
        {
            ++m_tickNumber;

            // update before tick (aura can be removed in TriggerSpell or PeriodicTick calls)
            m_periodicTimer += m_amplitude - diff;
            UpdatePeriodic(caster);

            std::list<AuraApplication*> effectApplications;
            GetApplicationList(effectApplications);
            // tick on targets of effects
            for (std::list<AuraApplication*>::const_iterator apptItr = effectApplications.begin(); apptItr != effectApplications.end(); ++apptItr)
                if ((*apptItr)->HasEffect(GetEffIndex()))
                    PeriodicTick(*apptItr, caster);

            // TEMPORARY HACKS FOR PERIODIC HANDLERS OF DYNAMIC OBJECT AURAS
            if (baseAura->GetType() == DYNOBJ_AURA_TYPE)
            {
                if (DynamicObject const* d_owner = baseAura->GetDynobjOwner())
                {
                    switch (GetSpellInfo()->Id)
                    {
                        // Death and Decay
                        case 43265:
                            GetCaster()->CastSpell(d_owner->GetPositionX(), d_owner->GetPositionY(), d_owner->GetPositionZ(), 52212, true);
                            break;
                        // Smoke Bomb
                        case 76577:
                            GetCaster()->CastSpell(d_owner->GetPositionX(), d_owner->GetPositionY(), d_owner->GetPositionZ(), 88611, true);
                            break;
                        // Consecration
                        case 36946:
                            GetCaster()->CastSpell(d_owner->GetPositionX(), d_owner->GetPositionY(), d_owner->GetPositionZ(), 81297, true);
                            break;
                        case 103558: // Choking Smoke Bomb, Asira Dawnslayer, Hour of Twilight
                            GetCaster()->CastSpell(d_owner->GetPositionX(), d_owner->GetPositionY(), d_owner->GetPositionZ(), 103790, true);
                            break;
                    }
                }
            }
        }
    }
}

void AuraEffect::UpdatePeriodic(Unit* caster)
{
    switch (GetAuraType())
    {
        case SPELL_AURA_PERIODIC_DUMMY:
            switch (GetSpellInfo()->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:
                    switch (GetId())
                    {
                        // Drink
                        /*case 430:
                        case 431:
                        case 432:
                        case 1133:
                        case 1135:
                        case 1137:
                        case 10250:
                        case 22734:
                        case 27089:
                        case 34291:
                        case 43182:
                        case 43183:
                        case 46755:
                        case 49472: // Drink Coffee
                        case 57073:
                        case 61830:
                        case 72623:
                        case 80166:
                        case 80167:
                        case 87958:
                        case 87959:
                        case 92736:
                        case 92797:
                        case 92800:
                        case 92803:
                        case 104262:
                        case 104270:
                        case 105230:
                        case 105232:
                        case 118358:
                        case 130336:
                        {
                            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                                return;
                            // Get SPELL_AURA_MOD_POWER_REGEN aura from spell
                            if (AuraEffect* aurEff = GetBase()->GetEffect(0))
                            {
                                if (aurEff->GetAuraType() != SPELL_AURA_MOD_POWER_REGEN)
                                    m_isPeriodic = false;
                                else
                                {
                                    // default case - not in arena
                                    if (!caster->ToPlayer()->InArena())
                                    {
                                        aurEff->ChangeAmount(GetAmount());
                                        m_isPeriodic = false;
                                    }
                                    else
                                    {
                                        // **********************************************
                                        // This feature uses only in arenas
                                        // **********************************************
                                        // Here need increase mana regen per tick (6 second rule)
                                        // on 0 tick -   0  (handled in 2 second)
                                        // on 1 tick - 166% (handled in 4 second)
                                        // on 2 tick - 133% (handled in 6 second)

                                        // Apply bonus for 1 - 4 tick
                                        switch (m_tickNumber)
                                        {
                                            case 1:   // 0%
                                                aurEff->ChangeAmount(0);
                                                break;
                                            case 2:   // 166%
                                                aurEff->ChangeAmount(GetAmount() * 5 / 3);
                                                break;
                                            case 3:   // 133%
                                                aurEff->ChangeAmount(GetAmount() * 4 / 3);
                                                break;
                                            default:  // 100% - normal regen
                                                aurEff->ChangeAmount(GetAmount());
                                                // No need to update after 4th tick
                                                m_isPeriodic = false;
                                                break;
                                        }
                                    }
                                }
                            }

                            break;
                        }*/
                        case 58549: // Tenacity
                        case 59911: // Tenacity (vehicle)
                           GetBase()->RefreshDuration();
                           break;
                        case 66823: case 67618: case 67619: case 67620: // Paralytic Toxin
                            // Get 0 effect aura
                            if (AuraEffect* slow = GetBase()->GetEffect(0))
                            {
                                int32 newAmount = slow->GetAmount() - 10;
                                if (newAmount < -100)
                                    newAmount = -100;
                                slow->ChangeAmount(newAmount);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case SPELLFAMILY_MAGE:
                    if (GetId() == 55342)// Mirror Image
                        m_isPeriodic = false;
                    break;
                case SPELLFAMILY_DEATHKNIGHT:
                    // Chains of Ice
                    if (GetSpellInfo()->SpellFamilyFlags[1] & 0x00004000)
                    {
                        // Get 0 effect aura
                        if (AuraEffect* slow = GetBase()->GetEffect(0))
                        {
                            int32 newAmount = slow->GetAmount() + GetAmount();
                            if (newAmount > 0)
                                newAmount = 0;
                            slow->ChangeAmount(newAmount);
                        }
                        return;
                    }
                    break;
                default:
                    break;
           }
       default:
           break;
    }
    GetBase()->CallScriptEffectUpdatePeriodicHandlers(this);
}

bool AuraEffect::IsPeriodicTickCrit(Unit* target, Unit const* caster) const
{
    ASSERT(caster);
    return caster->isSpellCrit(target, m_spellInfo, m_spellInfo->GetSchoolMask());
}

bool AuraEffect::IsAffectingSpell(SpellInfo const* spell) const
{
    if (!spell)
        return false;
    // Check family name
    if (spell->SpellFamilyName != m_spellInfo->SpellFamilyName)
        return false;

    // Check EffectClassMask
    if (m_spellInfo->Effects[m_effIndex].SpellClassMask & spell->SpellFamilyFlags)
        return true;

    return false;
}

void AuraEffect::SendTickImmune(Unit* target, Unit* caster) const
{
    if (caster)
        caster->SendSpellDamageImmune(target, m_spellInfo->Id, true);
}

void AuraEffect::PeriodicTick(AuraApplication * aurApp, Unit* caster) const
{
    bool prevented = GetBase()->CallScriptEffectPeriodicHandlers(this, aurApp);
    if (prevented)
        return;

    Unit* target = aurApp->GetTarget();

    switch (GetAuraType())
    {
        case SPELL_AURA_PERIODIC_DUMMY:
            HandlePeriodicDummyAuraTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
            HandlePeriodicTriggerSpellAuraTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
            HandlePeriodicTriggerSpellWithValueAuraTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            HandlePeriodicDamageAurasTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_LEECH:
            HandlePeriodicHealthLeechAuraTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            HandlePeriodicHealthFunnelAuraTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
            HandlePeriodicHealAurasTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
            HandlePeriodicManaLeechAuraTick(target, caster);
            break;
        case SPELL_AURA_OBS_MOD_POWER:
            HandleObsModPowerAuraTick(target, caster);
            break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
            HandlePeriodicEnergizeAuraTick(target, caster);
            break;
        case SPELL_AURA_POWER_BURN:
            HandlePeriodicPowerBurnAuraTick(target, caster);
            break;
        default:
            break;
    }
}

void AuraEffect::HandleProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    bool prevented = GetBase()->CallScriptEffectProcHandlers(this, const_cast<AuraApplication const*>(aurApp), eventInfo);
    if (prevented)
        return;

    switch (GetAuraType())
    {
        case SPELL_AURA_PROC_TRIGGER_SPELL:
            HandleProcTriggerSpellAuraProc(aurApp, eventInfo);
            break;
        case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
            HandleProcTriggerSpellWithValueAuraProc(aurApp, eventInfo);
            break;
        case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            HandleProcTriggerDamageAuraProc(aurApp, eventInfo);
            break;
        case SPELL_AURA_RAID_PROC_FROM_CHARGE:
            HandleRaidProcFromChargeAuraProc(aurApp, eventInfo);
            break;
        case SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE:
            HandleRaidProcFromChargeWithValueAuraProc(aurApp, eventInfo);
            break;
        default:
            break;
    }

    GetBase()->CallScriptAfterEffectProcHandlers(this, const_cast<AuraApplication const*>(aurApp), eventInfo);
}

void AuraEffect::CleanupTriggeredSpells(Unit* target)
{
    uint32 tSpellId = m_spellInfo->Effects[GetEffIndex()].TriggerSpell;
    if (!tSpellId)
        return;

    SpellInfo const* tProto = sSpellMgr->GetSpellInfo(tSpellId);
    if (!tProto)
        return;

    if (tProto->GetDuration() != -1)
        return;

    // needed for spell 43680, maybe others
    // TODO: is there a spell flag, which can solve this in a more sophisticated way?
    if (m_spellInfo->Effects[GetEffIndex()].ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL &&
            uint32(m_spellInfo->GetDuration()) == m_spellInfo->Effects[GetEffIndex()].Amplitude)
        return;

    // Hack for submerge, Ragnaros (Firelands)
    if (m_spellInfo->Id == 100051)
        return;

    // Hack for Pulsing Flames, Galakras (Siege of Orgrimmar)
    if (m_spellInfo->Id == 147042)
        return;

    target->RemoveAurasDueToSpell(tSpellId, GetCasterGUID());
}

void AuraEffect::HandleShapeshiftBoosts(Unit* target, bool apply) const
{
    uint32 spellId  = 0;
    uint32 spellId2 = 0;
    uint32 spellId3 = 0;
    uint32 spellId4 = 0;

    switch (GetMiscValue())
    {
        case FORM_CAT:
            spellId = 3025;
            spellId2 = 48629;
            spellId3 = 106840;
            spellId4 = 113636;
            target->RemoveAura(115034);
            break;
        case FORM_TRAVEL:
            spellId = 5419;
            target->RemoveAura(115034);

            if (target->HasAura(114338) && !target->HasAura(131113) && apply)
                spellId2 = 115034;
            break;
        case FORM_AQUA:
            spellId = 5421;
            target->RemoveAura(115034);
            break;
        case FORM_BEAR:
            spellId = 1178;
            spellId2 = 21178;
            spellId3 = 106829;
            spellId4 = 106899;
            target->RemoveAura(115034);
            break;
        case FORM_BATTLESTANCE:
            spellId = 21156;
            break;
        case FORM_DEFENSIVESTANCE:
            spellId = 7376;
            break;
        case FORM_BERSERKERSTANCE:
            spellId = 7381;
            break;
        case FORM_MOONKIN:
            spellId = 24905;
            spellId2 = 24907;
            target->RemoveAura(115034);
            break;
        case FORM_FLIGHT:
            spellId = 33948;
            spellId2 = 34764;
            target->RemoveAura(115034);
            break;
        case FORM_FLIGHT_EPIC:
            spellId  = 40122;
            spellId2 = 40121;
            target->RemoveAura(115034);
            break;
        case FORM_METAMORPHOSIS:
            spellId  = 103965;
            spellId2 = 54817;
            spellId3 = 54879;
            if (apply)
                target->RemoveAura(114168); // Dark Apotheosis
            break;
        case FORM_SPIRITOFREDEMPTION:
            spellId  = 27792;
            spellId2 = 27795;                               // must be second, this important at aura remove to prevent to early iterator invalidation.
            break;
        case FORM_SHADOW:
            spellId = 49868;
            spellId2 = 71167;
            break;
        case FORM_GHOSTWOLF:
            spellId = 67116;
            break;
        case FORM_WISE_SERPENT:
            spellId = 136336;
            //spellId2 = 130324 don't need, use base aura to handle procs
            break;
        case FORM_GHOUL:
        case FORM_AMBIENT:
        case FORM_STEALTH:
        case FORM_CREATURECAT:
        case FORM_CREATUREBEAR:
            break;
        default:
            break;
    }

    if (apply)
    {
        // Remove cooldown of spells triggered on stance change - they may share cooldown with stance spell
        if (spellId)
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ToPlayer()->RemoveSpellCooldown(spellId);
            target->CastSpell(target, spellId, true, NULL, this);
        }

        if (spellId2)
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ToPlayer()->RemoveSpellCooldown(spellId2);
            target->CastSpell(target, spellId2, true, NULL, this);
        }

        if (spellId3)
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ToPlayer()->RemoveSpellCooldown(spellId3);
            target->CastSpell(target, spellId3, true, NULL, this);
        }

        if (spellId4)
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ToPlayer()->RemoveSpellCooldown(spellId4);
            target->CastSpell(target, spellId4, true, NULL, this);
        }

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            Player* plrTarget = target->ToPlayer();

            PlayerSpellMap const& sp_list = plrTarget->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if (itr->second->state == PLAYERSPELL_REMOVED || itr->second->disabled)
                    continue;

                if (itr->first == spellId || itr->first == spellId2 || itr->first == spellId3 || itr->first == spellId4)
                    continue;

                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
                if (!spellInfo || !(spellInfo->Attributes & (SPELL_ATTR0_PASSIVE | SPELL_ATTR0_HIDDEN_CLIENTSIDE)))
                    continue;

                if (spellInfo->Stances & (1<<(GetMiscValue()-1)))
                    target->CastSpell(target, itr->first, true, NULL, this);
            }

            // Also do it for Glyphs
            for (uint32 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
            {
                if (uint32 glyphId = plrTarget->GetGlyph(plrTarget->GetActiveSpec(), i))
                {
                    if (GlyphPropertiesEntry const* glyph = sGlyphPropertiesStore.LookupEntry(glyphId))
                    {
                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(glyph->SpellId);
                        if (!spellInfo || !(spellInfo->Attributes & (SPELL_ATTR0_PASSIVE | SPELL_ATTR0_HIDDEN_CLIENTSIDE)))
                            continue;

                        if (spellInfo->Stances & (1<<(GetMiscValue()-1)))
                            target->CastSpell(target, glyph->SpellId, true, NULL, this);
                    }
                }
            }

            // Leader of the Pack
            if (plrTarget->HasSpell(17007))
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(24932);
                if (spellInfo && spellInfo->Stances & (1<<(GetMiscValue()-1)))
                    target->CastSpell(target, 24932, true, NULL, this);
            }

            switch (GetMiscValue())
            {
                case FORM_CAT:
                {
                    // Savage Roar
                    if (target->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_DRUID, 0, 0x10000000, 0))
                        target->CastSpell(target, 62071, true);

                    break;
                }
            }
        }
    }
    else
    {
        if (spellId)
            target->RemoveOwnedAura(spellId, target->GetGUID());
        if (spellId2)
            target->RemoveOwnedAura(spellId2, target->GetGUID());
        if (spellId3)
            target->RemoveOwnedAura(spellId3, target->GetGUID());
        if (spellId4)
            target->RemoveOwnedAura(spellId4, target->GetGUID());

        const Unit::AuraEffectList& shapeshifts = target->GetAuraEffectsByType(SPELL_AURA_MOD_SHAPESHIFT);
        AuraEffect* newAura = nullptr;
        // Iterate through all the shapeshift auras that the target has, if there is another aura with SPELL_AURA_MOD_SHAPESHIFT, then this aura is being removed due to that one being applied
        for (Unit::AuraEffectList::const_iterator itr = shapeshifts.begin(); itr != shapeshifts.end(); ++itr)
        {
            if ((*itr) != this)
            {
                newAura = *itr;
                break;
            }
        }
        Unit::AuraApplicationMap& tAuras = target->GetAppliedAuras();
        for (Unit::AuraApplicationMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
        {
            // Use the new aura to see on what stance the target will be
            uint32 newStance = (1<<((newAura ? newAura->GetMiscValue() : 0)-1));

            // If the stances are not compatible with the spell, remove it
            if (itr->second->GetBase()->IsRemovedOnShapeLost(target) && !(itr->second->GetBase()->GetSpellInfo()->Stances & newStance))
                target->RemoveAura(itr);
            else
                ++itr;
        }
    }
}

/*********************************************************/
/***               AURA EFFECT HANDLERS                ***/
/*********************************************************/

/**************************************/
/***       VISIBILITY & PHASES      ***/
/**************************************/

void AuraEffect::HandleModInvisibilityDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    InvisibilityType type = InvisibilityType(GetMiscValue());

    if (apply)
    {
        target->m_invisibilityDetect.AddFlag(type);
        target->m_invisibilityDetect.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY_DETECT))
            target->m_invisibilityDetect.DelFlag(type);

        target->m_invisibilityDetect.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModInvisibility(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    InvisibilityType type = InvisibilityType(GetMiscValue());

    if (GetId() == 32727)
    {
        if (auto const player = target->ToPlayer())
        {
            if (player->GetMap()->IsBattleground())
                return;
        }
    }

    if (apply)
    {
        // apply glow vision
        //if (target->GetTypeId() == TYPEID_PLAYER && GetSpellInfo()->GetMaxDuration() != -1)
        //{
        //    target->SetByteFlag(PLAYER_FIELD_OVERRIDE_SPELLS_ID, 3, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);
        //}

        if (GetBase()->GetId() == 32612) // invisible mage pet
            if (Unit* pet = target->GetGuardianPet())
                target->AddAura(32612,pet);

        target->m_invisibility.AddFlag(type);
        target->m_invisibility.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
        {
            // if not have different invisibility auras.
            // remove glow vision
            //if (target->GetTypeId() == TYPEID_PLAYER)
            //    target->RemoveByteFlag(PLAYER_FIELD_OVERRIDE_SPELLS_ID, 3, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            target->m_invisibility.DelFlag(type);
        }
        else
        {
            bool found = false;
            Unit::AuraEffectList const& invisAuras = target->GetAuraEffectsByType(SPELL_AURA_MOD_INVISIBILITY);
            for (Unit::AuraEffectList::const_iterator i = invisAuras.begin(); i != invisAuras.end(); ++i)
            {
                if (GetMiscValue() == (*i)->GetMiscValue())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                target->m_invisibility.DelFlag(type);
        }

        target->m_invisibility.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        // drop flag at invisibiliy in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
    }

    target->UpdateObjectVisibility();
}

//TODO: Finish this aura
void AuraEffect::HandleModCamouflage(AuraApplication const *aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        //
    }
    else if (!(target->isCamouflaged()))
    {
        if (!GetCaster())
            return;

        if (Unit* pet = GetCaster()->GetGuardianPet())
            pet->RemoveAurasByType(SPELL_AURA_MOD_CAMOUFLAGE);
    }
}

void AuraEffect::HandleModStealthDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    StealthType type = StealthType(GetMiscValue());

    if (apply)
    {
        target->m_stealthDetect.AddFlag(type);
        target->m_stealthDetect.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH_DETECT))
            target->m_stealthDetect.DelFlag(type);

        target->m_stealthDetect.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModStealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    StealthType type = StealthType(GetMiscValue());

    if (apply)
    {
        target->m_stealth.AddFlag(type);
        target->m_stealth.AddValue(type, GetAmount());

        target->SetStandFlags(UNIT_STAND_FLAGS_CREEP);

        WorldPacket data(SMSG_CLEAR_TARGET, 8);
        ObjectGuid casterGuid = target->GetGUID();

        uint8 bitsOrder[8] = { 0, 5, 7, 2, 1, 6, 3, 4 };
        data.WriteBitInOrder(casterGuid, bitsOrder);

        uint8 bytesOrder[8] = { 2, 1, 5, 7, 4, 0, 6, 3 };
        data.WriteBytesSeq(casterGuid, bytesOrder);
        target->SendMessageUnfriendlyToSetInRange(&data, true);

        Unit::AttackerSet const& attackers = target->getAttackers();
        for (Unit::AttackerSet::const_iterator itr = attackers.begin(); itr != attackers.end();)
        {
            if (!(*itr)->canSeeOrDetect(target))
                (*(itr++))->AttackStop();
            else
                ++itr;
        }
    }
    else
    {
        target->m_stealth.AddValue(type, -GetAmount());

        if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH) && !target->HasAura(115192)) // if last SPELL_AURA_MOD_STEALTH
        {
            target->m_stealth.DelFlag(type);

            target->RemoveStandFlags(UNIT_STAND_FLAGS_CREEP);
        }
    }

    // call functions which may have additional effects after chainging state of unit
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        // drop flag at stealth in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
        target->RemoveFlagsAuras();
    }

    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModStealthLevel(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    StealthType type = StealthType(GetMiscValue());

    if (apply)
        target->m_stealth.AddValue(type, GetAmount());
    else
        target->m_stealth.AddValue(type, -GetAmount());

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleSpiritOfRedemption(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // prepare spirit state
    if (apply)
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            // disable breath/etc timers
            target->ToPlayer()->StopMirrorTimers();

            // set stand state (expected in this form)
            if (!target->IsStandState())
                target->SetStandState(UNIT_STAND_STATE_STAND);
        }

        target->SetHealth(1);
    }
    // die at aura end
    else if (target->isAlive())
        // call functions which may have additional effects after chainging state of unit
        target->setDeathState(JUST_DIED);

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    else
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
}

void AuraEffect::HandleAuraGhost(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
    {
        target->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
        target->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
        target->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
    }
    else
    {
        if (target->HasAuraType(SPELL_AURA_GHOST))
            return;

        target->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
        target->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        target->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
    }
}

void AuraEffect::HandlePhase(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();


    if (Player* player = target->ToPlayer())
    {
        if (apply)
            player->GetPhaseMgr().RegisterPhasingAuraEffect(this);
        else
            player->GetPhaseMgr().UnRegisterPhasingAuraEffect(this);
    }
    else
    {
        uint32 newPhase = 0;
        Unit::AuraEffectList const& phases = target->GetAuraEffectsByType(SPELL_AURA_PHASE);
        if (!phases.empty())
            for (Unit::AuraEffectList::const_iterator itr = phases.begin(); itr != phases.end(); ++itr)
                newPhase |= (*itr)->GetMiscValue();

        if (!newPhase)
        {
            newPhase = PHASEMASK_NORMAL;
            if (Creature* creature = target->ToCreature())
                if (CreatureData const* data = sObjectMgr->GetCreatureData(creature->GetSpawnId()))
                    newPhase = data->phaseMask;
        }

        target->SetPhaseMask(newPhase, false);
    }

    // call functions which may have additional effects after chainging state of unit
    // phase auras normally not expected at BG but anyway better check
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        // drop flag at invisibiliy in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
    }

    // need triggering visibility update base at phase update of not GM invisible (other GMs anyway see in any phases)
    if (target->IsVisible())
    {
        if (target->GetTypeId() == TYPEID_PLAYER && target->ToPlayer()->IsBeingTeleportedFar())
            return;

        target->UpdateObjectVisibility();
    }
}

/**********************/
/***   UNIT MODEL   ***/
/**********************/

void AuraEffect::HandleAuraModShapeshift(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    uint32 modelid = 0;
    Powers PowerType = POWER_MANA;
    ShapeshiftForm form = ShapeshiftForm(GetMiscValue());

    switch (form)
    {
        case FORM_FIERCE_TIGER:
        case FORM_STURDY_OX:
        case FORM_CAT:                                      // 0x01
        case FORM_GHOUL:                                    // 0x07
            PowerType = POWER_ENERGY;
            break;
        case FORM_BEAR:                                     // 0x05
        case FORM_BATTLESTANCE:                             // 0x11
        case FORM_DEFENSIVESTANCE:                          // 0x12
        case FORM_BERSERKERSTANCE:                          // 0x13
            PowerType = POWER_RAGE;
            break;
        case FORM_TREE:                                     // 0x02
        case FORM_TRAVEL:                                   // 0x03
        case FORM_AQUA:                                     // 0x04
        case FORM_AMBIENT:                                  // 0x06
        case FORM_STEVES_GHOUL:                             // 0x09
        case FORM_THARONJA_SKELETON:                        // 0x0A
        case FORM_TEST_OF_STRENGTH:                         // 0x0B
        case FORM_BLB_PLAYER:                               // 0x0C
        case FORM_SHADOW_DANCE:                             // 0x0D
        case FORM_CREATUREBEAR:                             // 0x0E
        case FORM_CREATURECAT:                              // 0x0F
        case FORM_GHOSTWOLF:                                // 0x10
            break;
        case FORM_WISE_SERPENT:                             // 0x14
            PowerType = POWER_MANA;
            break;
        case FORM_ZOMBIE:                                   // 0x15
        case FORM_METAMORPHOSIS:                            // 0x16
        case FORM_UNDEAD:                                   // 0x19
        case FORM_MASTER_ANGLER:                            // 0x1A
        case FORM_FLIGHT_EPIC:                              // 0x1B
        case FORM_SHADOW:                                   // 0x1C
        case FORM_FLIGHT:                                   // 0x1D
        case FORM_STEALTH:                                  // 0x1E
        case FORM_MOONKIN:                                  // 0x1F
        case FORM_SPIRITOFREDEMPTION:                       // 0x20
            break;
        default:
            break;
    }

    modelid = target->GetModelForForm(form);

    if (apply)
    {
        // remove polymorph before changing display id to keep new display id
        switch (form)
        {
            case FORM_CAT:
            case FORM_TREE:
            case FORM_TRAVEL:
            case FORM_AQUA:
            case FORM_BEAR:
            case FORM_FLIGHT_EPIC:
            case FORM_FLIGHT:
            case FORM_MOONKIN:
            {
                // remove vengeance tank mastery for all form except bear
                if (form != FORM_BEAR)
                    target->RemoveAurasDueToSpell(76691);

                // Burning Seed - item 94604
                if (form == FORM_CAT && target->HasAura(138927))
                    modelid = 38150;

                target->RemoveAurasDueToSpell(114282);

                // remove movement affects
                target->RemoveMovementImpairingAuras();

                // and polymorphic affects
                if (target->IsPolymorphed())
                    target->RemoveAurasDueToSpell(target->getTransForm());
                break;
            }
            default:
               break;
        }

        // remove other shapeshift before applying a new one
        target->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT, 0, GetBase());

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;

        if (modelid > 0)
            target->SetDisplayId(modelid);

        if (PowerType != POWER_MANA)
        {
            int32 oldPower = target->GetPower(PowerType);
            // reset power to default values only at power change
            if (target->getPowerType() != PowerType)
            {
                target->setPowerType(PowerType);

                // Fix visual bug with empty power
                if (target->getClass() == CLASS_DRUID && PowerType == POWER_ENERGY)
                    target->SetPower(POWER_ENERGY, target->GetPower(POWER_ENERGY));
            }
        }

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;

        target->SetShapeshiftForm(form);
    }
    else
    {
        // reset model id if no other auras present
        // may happen when aura is applied on linked event on aura removal
        if (!target->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        {
            target->SetShapeshiftForm(FORM_NONE);
            if (target->getClass() == CLASS_DRUID)
            {
                target->setPowerType(POWER_MANA);
                target->SetPower(POWER_MANA, target->GetPower(POWER_MANA), false);
                // Remove movement impairing effects also when shifting out
                target->RemoveMovementImpairingAuras();
            }
        }

        if (modelid > 0)
            target->RestoreDisplayId();

        switch (form)
        {
            // Nordrassil Harness - bonus
            case FORM_BEAR:
            case FORM_CAT:
                if (AuraEffect* dummy = target->GetAuraEffect(37315, 0))
                    target->CastSpell(target, 37316, true, NULL, dummy);
                //target->RemoveAurasDueToSpell(1850); // Dash
                break;
            // Nordrassil Regalia - bonus
            case FORM_MOONKIN:
                if (AuraEffect* dummy = target->GetAuraEffect(37324, 0))
                    target->CastSpell(target, 37325, true, NULL, dummy);

                target->RemoveAurasDueToSpell(114302); // Astral Form
                break;
            default:
                break;
        }
    }

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(target, apply);

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->InitDataForForm();

    if (target->getClass() == CLASS_DRUID)
    {
        // Dash
        if (AuraEffect* aurEff = target->GetAuraEffect(SPELL_AURA_MOD_INCREASE_SPEED, SPELLFAMILY_DRUID, 0, 0, 0x8))
            aurEff->RecalculateAmount();

        // Disarm handling
        // If druid shifts while being disarmed we need to deal with that since forms aren't affected by disarm
        // and also HandleAuraModDisarm is not triggered
        if (!target->CanUseAttackType(BASE_ATTACK))
        {
            if (Item* pItem = target->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            {
                uint32 minDamage = 0;
                uint32 maxDamage = 0;

                if (!pItem->GetTemplate()->ScalingStatDistribution)
                    pItem->GetTemplate()->CalculateMinMaxDamageScaling(target->ToPlayer()->GetEquipItemLevelFor(pItem->GetTemplate(), pItem), minDamage, maxDamage);

                target->ToPlayer()->_ApplyWeaponDamage(EQUIPMENT_SLOT_MAINHAND, pItem, NULL, apply, minDamage, maxDamage);
            }
        }
    }

    // stop handling the effect if it was removed by linked event
    if (apply && aurApp->GetRemoveMode())
        return;

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        SpellShapeshiftFormEntry const* shapeInfo = sSpellShapeshiftFormStore.LookupEntry(form);
        // Learn spells for shapeshift form - no need to send action bars or add spells to spellbook
        for (uint8 i = 0; i<MAX_SHAPESHIFT_SPELLS; ++i)
        {
            if (!shapeInfo->stanceSpell[i])
                continue;
            if (apply)
                target->ToPlayer()->AddTemporarySpell(shapeInfo->stanceSpell[i]);
            else
                target->ToPlayer()->RemoveTemporarySpell(shapeInfo->stanceSpell[i]);
        }
        // Update the Mastery percentage for Shapeshift
        target->ToPlayer()->UpdateMasteryPercentage();
    }
}

void AuraEffect::HandleAuraTransform(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        // update active transform spell only when transform or shapeshift not set or not overwriting negative by positive case
        if (!target->GetModelForForm(target->GetShapeshiftForm()) || !GetSpellInfo()->IsPositive())
        {
            // special case (spell specific functionality)
            if (GetMiscValue() == 0)
            {
                switch (GetId())
                {
                    // Orb of Deception
                    case 16739:
                    {
                        if (target->GetTypeId() != TYPEID_PLAYER)
                            return;

                        switch (target->getRace())
                        {
                            // Blood Elf
                            case RACE_BLOODELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 17829 : 17830);
                                break;
                            // Orc
                            case RACE_ORC:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10139 : 10140);
                                break;
                            // Troll
                            case RACE_TROLL:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10135 : 10134);
                                break;
                            // Tauren
                            case RACE_TAUREN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10136 : 10147);
                                break;
                            // Undead
                            case RACE_UNDEAD_PLAYER:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10146 : 10145);
                                break;
                            // Draenei
                            case RACE_DRAENEI:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 17827 : 17828);
                                break;
                            // Dwarf
                            case RACE_DWARF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10141 : 10142);
                                break;
                            // Gnome
                            case RACE_GNOME:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10148 : 10149);
                                break;
                            // Human
                            case RACE_HUMAN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10137 : 10138);
                                break;
                            // Night Elf
                            case RACE_NIGHTELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 10143 : 10144);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    // Murloc costume
                    case 42365:
                        target->SetDisplayId(21723);
                        break;
                    // Dread Corsair
                    case 50517:
                    // Corsair Costume
                    case 51926:
                    {
                        if (target->GetTypeId() != TYPEID_PLAYER)
                            return;

                        switch (target->getRace())
                        {
                            // Blood Elf
                            case RACE_BLOODELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25032 : 25043);
                                break;
                            // Orc
                            case RACE_ORC:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25039 : 25050);
                                break;
                            // Troll
                            case RACE_TROLL:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25041 : 25052);
                                break;
                            // Tauren
                            case RACE_TAUREN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25040 : 25051);
                                break;
                            // Undead
                            case RACE_UNDEAD_PLAYER:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25042 : 25053);
                                break;
                            // Draenei
                            case RACE_DRAENEI:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25033 : 25044);
                                break;
                            // Dwarf
                            case RACE_DWARF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25034 : 25045);
                                break;
                            // Gnome
                            case RACE_GNOME:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25035 : 25046);
                                break;
                            // Human
                            case RACE_HUMAN:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25037 : 25048);
                                break;
                            // Night Elf
                            case RACE_NIGHTELF:
                                target->SetDisplayId(target->getGender() == GENDER_MALE ? 25038 : 25049);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    // Pygmy Oil
                    case 53806:
                        target->SetDisplayId(22512);
                        break;
                    // Honor the Dead
                    case 65386:
                    case 65495:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 29203 : 29204);
                        break;
                    // Darkspear Pride
                    case 75532:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 31737 : 31738);
                        break;
                    // Gnomeregan Pride
                    case 75531:
                        target->SetDisplayId(target->getGender() == GENDER_MALE ? 31654 : 31655);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(GetMiscValue());
                if (!ci)
                {
                    target->SetDisplayId(16358);              // pig pink ^_^
                }
                else
                {
                    uint32 model_id = 0;

                    if (uint32 modelid = ci->GetRandomValidModelId())
                        model_id = modelid;                     // Will use the default model here

                    // Leyara's Locket
                    if (GetSpellInfo()->Id == 101185)
                        if (Unit* caster = GetCaster())
                            if (caster->getGender() == GENDER_FEMALE)
                                model_id = 38744;

                    // Polymorph (sheep)
                    if (GetSpellInfo()->Id == 118)
                    {
                        if (Unit* caster = GetCaster())
                        {
                            if (caster->HasAura(52648))     // Glyph of the Penguin
                                model_id = 26452;
                            else if (caster->HasAura(58136))// Glyph of the Bear Cub
                                model_id = 16189;
                            else if (caster->HasAura(57927))// Glyph of the Monkey
                                model_id = 21360; // should be the correct one
                            else if (caster->HasAura(57924))//Glyph of the Percupine
                                model_id = 45700;
                        }
                    }

                    target->SetDisplayId(model_id);

                    // Dragonmaw Illusion (set mount model also)
                    if (GetId() == 42016 && target->GetMountID() && !target->GetAuraEffectsByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED).empty())
                        target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 16314);
                }
            }
        }

        // update active transform spell only when transform or shapeshift not set or not overwriting negative by positive case
        SpellInfo const* transformSpellInfo = sSpellMgr->GetSpellInfo(target->getTransForm());
        if (!transformSpellInfo || !GetSpellInfo()->IsPositive() || transformSpellInfo->IsPositive())
            target->setTransForm(GetId());

        // polymorph case
        if ((mode & AURA_EFFECT_HANDLE_REAL) && target->GetTypeId() == TYPEID_PLAYER && target->IsPolymorphed())
        {
            // for players, start regeneration after 1s (in polymorph fast regeneration case)
            // only if caster is Player (after patch 2.4.2)
            if (IS_PLAYER_GUID(GetCasterGUID()))
                target->ToPlayer()->setRegenTimerCount(1*IN_MILLISECONDS);

            //dismount polymorphed target (after patch 2.4.2)
            if (target->IsMounted())
                target->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }
    else
    {
        // HandleEffect(this, AURA_EFFECT_HANDLE_SEND_FOR_CLIENT, true) will reapply it if need
        if (target->getTransForm() == GetId())
            target->setTransForm(0);

        target->RestoreDisplayId();

        // Dragonmaw Illusion (restore mount model)
        if (GetId() == 42016 && target->GetMountID() == 16314)
        {
            if (!target->GetAuraEffectsByType(SPELL_AURA_MOUNTED).empty())
            {
                uint32 cr_id = target->GetAuraEffectsByType(SPELL_AURA_MOUNTED).front()->GetMiscValue();
                if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(cr_id))
                {
                    uint32 team = 0;
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        team = target->ToPlayer()->GetTeam();

                    uint32 displayID = sObjectMgr->ChooseDisplayId(team, ci);
                    sObjectMgr->GetCreatureModelRandomGender(&displayID);

                    target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, displayID);
                }
            }
        }
    }
}

void AuraEffect::HandleAuraModScale(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X, (float)GetAmount(), apply);
}

void AuraEffect::HandleAuraCloneCaster(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        Unit* caster = GetCaster();
        if (caster)
        {
            Unit* displayOwner = caster->GetSimulacrumTarget() ? caster->GetSimulacrumTarget() : caster;

            m_data.resize(sizeof(AppareanceData));

            AppareanceData* appareanceData = reinterpret_cast<AppareanceData*>(&m_data[0]);
            appareanceData->Save(displayOwner);

            target->SetDisplayId(caster->GetDisplayId(), false);
        }
        else
        {
            AppareanceData const* appareanceData = reinterpret_cast<AppareanceData const*>(m_data.data());
            if (m_data.size() != sizeof(AppareanceData) || appareanceData->Version != AppareanceData::GetCurrentVersion())
                return;

            target->SetDisplayId(appareanceData->DisplayId, false);
        }

        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
        //target->SetObjectScale(caster->GetFloatValue(OBJECT_FIELD_SCALE_X)); // we need retail info about how scaling is handled (aura maybe?)
    }
    else
    {
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
        target->SetDisplayId(target->GetNativeDisplayId(), false);
    }
}

void AuraEffect::HandleAuraInitializeImages(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        Unit* caster = GetCaster();
        if (!caster || caster == target)
            return;

        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
    else
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
}

/************************/
/***      FIGHT       ***/
/************************/

void AuraEffect::HandleFeignDeath(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 0);
        target->SendMessageToSet(&data, true);
        */

        UnitList targets;
        JadeCore::AnyUnfriendlyUnitInObjectRangeCheck u_check(target, target, target->GetMap()->GetVisibilityRange());
        JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(target, targets, u_check);
        target->VisitNearbyObject(target->GetMap()->GetVisibilityRange(), searcher);
        for (UnitList::iterator iter = targets.begin(); iter != targets.end(); ++iter)
        {
            if (!(*iter)->HasUnitState(UNIT_STATE_CASTING))
                continue;

            for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
            {
                if ((*iter)->GetCurrentSpell(i)
                && (*iter)->GetCurrentSpell(i)->m_targets.GetUnitTargetGUID() == target->GetGUID())
                {
                    (*iter)->InterruptSpell(CurrentSpellTypes(i), false);
                }
            }
        }
        target->CombatStop();
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        // prevent interrupt message
        if (GetCasterGUID() == target->GetGUID() && target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            target->FinishSpell(CURRENT_GENERIC_SPELL, false);
        target->InterruptNonMeleeSpells(true);
        target->getHostileRefManager().deleteReferences();

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;
                                                            // blizz like 2.0.x
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
                                                            // blizz like 2.0.x
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                                                            // blizz like 2.0.x
        target->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        target->AddUnitState(UNIT_STATE_DIED);
    }
    else
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 0);
        target->SendMessageToSet(&data, true);
        */
                                                            // blizz like 2.0.x
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
                                                            // blizz like 2.0.x
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                                                            // blizz like 2.0.x
        target->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        target->ClearUnitState(UNIT_STATE_DIED);

        target->SendPowerUpdate(target->getPowerType());
    }
}

void AuraEffect::HandleModUnattackable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
    if (!apply && target->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
        return;

    target->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, apply);

    // call functions which may have additional effects after chainging state of unit
    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        if (GetSpellInfo()->Id != 122949)// Unseen Strike, Blade Lord Tayak, Heart of Fear
        {
            target->CombatStop();
            target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
        }
    }
}

void AuraEffect::HandleAuraModDisarm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    AuraType type = GetAuraType();

    //Prevent handling aura twice
    if ((apply) ? target->GetAuraEffectsByType(type).size() > 1 : target->HasAuraType(type))
        return;

    // Adaptation
    if (apply && target->HasAura(126046))
        target->CastSpell(target, 126050, true);

    uint32 field, flag, slot;
    WeaponAttackType attType;
    switch (type)
    {
        case SPELL_AURA_MOD_DISARM:
            field=UNIT_FIELD_FLAGS;
            flag=UNIT_FLAG_DISARMED;
            slot=EQUIPMENT_SLOT_MAINHAND;
            attType=BASE_ATTACK;
            break;
        case SPELL_AURA_MOD_DISARM_OFFHAND:
            field=UNIT_FIELD_FLAGS_2;
            flag=UNIT_FLAG2_DISARM_OFFHAND;
            slot=EQUIPMENT_SLOT_OFFHAND;
            attType=OFF_ATTACK;
            break;
        case SPELL_AURA_MOD_DISARM_RANGED:
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                if (target->ToPlayer()->getClass() != CLASS_HUNTER)
                    return;

            field=UNIT_FIELD_FLAGS_2;
            flag=UNIT_FLAG2_DISARM_RANGED;
            slot=EQUIPMENT_SLOT_MAINHAND;
            attType=RANGED_ATTACK;
            break;
        }
        default:
            return;
    }

    // if disarm aura is to be removed, remove the flag first to reapply damage/aura mods
    if (!apply)
        target->RemoveFlag(field, flag);

    // Handle damage modification, shapeshifted druids are not affected
    if (target->GetTypeId() == TYPEID_PLAYER && !target->IsInFeralForm())
    {
        if (Item* pItem = target->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            uint8 attacktype = Player::GetAttackBySlot(slot);

            if (attacktype < MAX_ATTACK)
            {
                uint32 minDamage = 0;
                uint32 maxDamage = 0;

                if (!pItem->GetTemplate()->ScalingStatDistribution)
                    pItem->GetTemplate()->CalculateMinMaxDamageScaling(target->ToPlayer()->GetEquipItemLevelFor(pItem->GetTemplate(), pItem), minDamage, maxDamage);

                target->ToPlayer()->_ApplyWeaponDamage(slot, pItem, NULL, !apply);
                target->ToPlayer()->_ApplyWeaponDependentAuraMods(pItem, WeaponAttackType(attacktype), !apply);
            }
        }
    }

    // if disarm effects should be applied, wait to set flag until damage mods are unapplied
    if (apply)
        target->SetFlag(field, flag);

    if (target->GetTypeId() == TYPEID_UNIT && target->ToCreature()->GetCurrentEquipmentId())
        target->UpdateDamagePhysical(attType);

    if (target->GetTypeId() == TYPEID_PLAYER && target->getClass() == CLASS_HUNTER && apply)
    {
        if (target->IsNonMeleeSpellCasted(true))
            target->InterruptNonMeleeSpells(true);
    }
}

void AuraEffect::HandleAuraModSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);

        bool interrupted = false;
        // call functions which may have additional effects after chainging state of unit
        // Stop cast only spells vs PreventionType == SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
        {
            if (Spell* spell = target->GetCurrentSpell(CurrentSpellTypes(i)))
            {
                if (spell->m_spellInfo->PreventionType & (SPELL_PREVENTION_TYPE_SILENCE | SPELL_PREVENTION_TYPE_PACIFY))
                {
                    // Stop spells on prepare or casting state
                    target->InterruptSpell(CurrentSpellTypes(i), false);
                    interrupted = true;
                }
            }
        }

        // Glyph of Strangulate - 58618
        // Increases the Silence duration of your Strangulate ability by 2 sec when used on a target who is casting a spell.
        if (m_spellInfo->Id == 47476 && interrupted && GetCaster() && GetCaster()->HasAura(58618))
        {
            aurApp->GetBase()->SetMaxDuration(aurApp->GetBase()->GetMaxDuration() + 2000);
            aurApp->GetBase()->RefreshDuration();
        }
    }
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_SILENCE) || target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;

        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void AuraEffect::HandleAuraModPacify(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
        target->AttackStop();
    }
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_PACIFY) || target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }
}

void AuraEffect::HandleAuraModPacifyAndSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // Vengeance of the Blue Flight (TODO: REMOVE THIS!)
    if (m_spellInfo->Id == 45839)
    {
        if (apply)
            target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        else
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }
    if (!(apply))
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            return;
    }
    HandleAuraModPacify(aurApp, mode, apply);
    HandleAuraModSilence(aurApp, mode, apply);
}

void AuraEffect::HandleAuraAllowOnlyAbility(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (apply)
            target->SetFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
        else
        {
            // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
            if (target->HasAuraType(SPELL_AURA_ALLOW_ONLY_ABILITY))
                return;
            target->RemoveFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
        }
    }
}

/****************************/
/***      TRACKING        ***/
/****************************/

void AuraEffect::HandleAuraTrackCreatures(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 mask = ((uint32)1) << (GetMiscValue() - 1);
    if (GetBase()->GetId() == 5225 && target->HasAura(114280))
        mask |= (uint32)1;

    target->ApplyModFlag(PLAYER_TRACK_CREATURES, mask, apply);
}

void AuraEffect::HandleAuraTrackResources(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->SetUInt32Value(PLAYER_TRACK_RESOURCES, (apply) ? ((uint32)1)<<(GetMiscValue()-1): 0);
}

void AuraEffect::HandleAuraTrackStealthed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!(apply))
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }
    target->ApplyModFlag(PLAYER_FIELD_LIFETIME_MAX_RANK, PLAYER_FIELD_BYTE_TRACK_STEALTHED, apply);
}

void AuraEffect::HandleAuraModStalked(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if (apply)
        target->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (!target->HasAuraType(GetAuraType()))
            target->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleAuraUntrackable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetByteFlag(UNIT_FIELD_BYTES_1, 2, UNIT_STAND_FLAGS_UNTRACKABLE);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
        target->RemoveByteFlag(UNIT_FIELD_BYTES_1, 2, UNIT_STAND_FLAGS_UNTRACKABLE);
    }
}

/****************************/
/***  SKILLS & TALENTS    ***/
/****************************/

void AuraEffect::HandleAuraModPetTalentsPoints(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;
}

void AuraEffect::HandleAuraModSkill(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_SKILL)))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    uint32 prot = GetMiscValue();
    int32 points = GetAmount();

    if (prot == SKILL_DEFENSE)
        return;

    target->ModifySkillBonus(prot, (apply ? points : -points), GetAuraType() == SPELL_AURA_MOD_SKILL_TALENT);
}

void AuraEffect::HandleAuraModSkillValue(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_SKILL)))
        return;
    
    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;
    
    uint32 skill = GetMiscValue();
    int32 points = GetAmount();
    
    target->ModifySkillBonus(skill, (apply ? points : -points), GetAuraType() == SPELL_AURA_MOD_SKILL_VALUE);
}

/****************************/
/***       MOVEMENT       ***/
/****************************/

void AuraEffect::HandleAuraMounted(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        uint32 creatureEntry = GetMiscValue();
        uint32 displayId = 0;
        uint32 vehicleId = 0;

        // Festive Holiday Mount
        if (target->HasAura(62061))
        {
            if (GetBase()->HasEffectType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                creatureEntry = 24906;
            else
                creatureEntry = 15665;
        }

        if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(creatureEntry))
        {
            uint32 team = 0;
            if (target->GetTypeId() == TYPEID_PLAYER)
                team = target->ToPlayer()->GetTeam();

            displayId = ObjectMgr::ChooseDisplayId(team, ci);
            sObjectMgr->GetCreatureModelRandomGender(&displayId);

            vehicleId = ci->VehicleId;

            //some spell has one aura of mount and one of vehicle
            for (uint32 i = 0; i < GetSpellInfo()->EffectCount; ++i)
                if (GetSpellInfo()->Effects[i].Effect == SPELL_EFFECT_SUMMON
                    && GetSpellInfo()->Effects[i].MiscValue == GetMiscValue())
                    displayId = 0;
        }

        // Glyph of the Mounted King
        if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PALADIN && target->HasAura(57958))
            target->CastSpell(target, 20217, true);

        if (GetBase()->GetId() == 87840)
        {
            displayId = target->getGender() == GENDER_FEMALE ? 29423 : 29422;
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ToPlayer()->SwitchToWorgenForm();
            target->SetDisplayId(displayId);
        }

        target->Mount(displayId, vehicleId, GetMiscValue());
    }
    else
    {
        if (GetBase()->GetId() == 87840)
            target->DeMorph();

        target->Dismount();
        //some mounts like Headless Horseman's Mount or broom stick are skill based spell
        // need to remove ALL arura related to mounts, this will stop client crash with broom stick
        // and never endless flying after using Headless Horseman's Mount
        if (mode & AURA_EFFECT_HANDLE_REAL)
            target->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateMount();
}

void AuraEffect::HandleAuraAllowFlight(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    //! Not entirely sure if this should be sent for creatures as well, but I don't think so.
    target->SetCanFly(apply);
    if (!apply)
    {
        target->RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING_FLY);
        target->m_movementInfo.SetFallTime(0);
    }

    Player* player = target->ToPlayer();
    if (!player)
        player = target->m_movedPlayer;

    if (player)
        player->SendMovementCanFlyChange();

    //! We still need to initiate a server-side MoveFall here,
    //! which requires MSG_MOVE_FALL_LAND on landing.
}

void AuraEffect::HandleAuraWaterWalk(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    target->SendMovementWaterWalking(apply);
}

void AuraEffect::HandleAuraFeatherFall(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        if (m_spellInfo->Id == 31884)
        {
            if (!target->HasAura(115931))
                return; // don't let apply feather effect on target that not have a glyph
        }
    }

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    if (apply)
        target->AddUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW);
    else
        target->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW);

    target->SendMovementFeatherFall();

    // start fall from current height
    if (!apply && target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->SetFallInformation(0, target->GetPositionZ());
}

void AuraEffect::HandleAuraHover(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    target->SetHover(apply);    //! Sets movementflags
    target->SendMovementHover(apply);
}

void AuraEffect::HandleWaterBreathing(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    // update timers in client
    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateMirrorTimers();
}

void AuraEffect::HandleForceMoveForward(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVEMENT);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVEMENT);
    }
}

void AuraEffect::HandleAllowTurnWhileFalling(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    target->SendCanTurnWhileFalling(apply);
}

/****************************/
/***        THREAT        ***/
/****************************/

void AuraEffect::HandleModThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    for (int8 i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1 << i))
            ApplyPercentModFloatVar(target->m_threatModifier[i], float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModTotalThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->isAlive() || target->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();
    if (caster && caster->isAlive())
        target->getHostileRefManager().addTempThreat((float)GetAmount(), apply);
}

void AuraEffect::HandleModTaunt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->isAlive())
        return;

    Unit* caster = GetCaster();
    if (!caster || !caster->isAlive())
        return;

    // Custom MoP Script - Glyph of Distracting Shot
    if (caster && caster->isAlive() && caster->getClass() == CLASS_HUNTER && caster->HasAura(123632))
        if (Pet* pet = caster->ToPlayer()->GetPet())
            caster = pet;

    if (!target->CanHaveThreatList())
    {
        if (target->getVictim() != caster)
        {
            if (target->GetTypeId() == TYPEID_UNIT && target->ToCreature()->IsAIEnabled 
                && (!target->ToCreature()->HasReactState(REACT_PASSIVE) || (target->IsPetGuardianStuff() && IS_PLAYER_GUID(target->GetCharmerOrOwnerGUID()))))
            {
                // taken from case COMMAND_ATTACK:                        //spellid=1792  //ATTACK PetHandler.cpp
                if (CharmInfo* charmInfo = target->GetCharmInfo())
                {
                    target->AttackStop();
                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsReturning(false);
                }
                target->ToCreature()->AI()->AttackStart(caster);
            }
        }
        return;
    }

    if (apply)
        target->TauntApply(caster);
    else
    {
        // When taunt aura fades out, mob will switch to previous target if current has less than 1.1 * secondthreat
        target->TauntFadeOut(caster);
    }
}

/*****************************/
/***        CONTROL        ***/
/*****************************/

void AuraEffect::HandleModConfuse(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->SetControlled(apply, UNIT_STATE_CONFUSED);
}

void AuraEffect::HandleModFear(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    switch (m_spellInfo->Id)
    {
        case 5246: ///< Intimidating Shout
            target->RemoveAura(115804); ///< Intimidating Shout removes Deep Wounds 
            break;
    }

    target->SetControlled(apply, UNIT_STATE_FLEEING);
}

void AuraEffect::HandleAuraModStun(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    switch (m_spellInfo->Id)
    {
        case 117436:// Lightning Prison
            target->RemoveAura(111850);
            break;
        case 127266:// Amber Prison
            if (target->GetTypeId() == TYPEID_PLAYER)
                return;
            break;
        default:
            break;
    }

    target->SetControlled(apply, UNIT_STATE_STUNNED);
}

void AuraEffect::HandleAuraModRoot(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    // Earthgrab totem - Immunity
    if (apply && target->HasAura(116946))
        return;

    if (apply)
    {
        switch (m_spellInfo->Id)
        {
            // Fear
            case 5782:
                // Glyph of Fear
                if (!GetCaster() || !GetCaster()->HasAura(56244))
                    return;
                break;
            // Mental Scream
            case 8122:
                // Glyph of Mental Scream
                if (!GetCaster() || !GetCaster()->HasAura(55676))
                    return;
                break;
            // Intimidating Shout
            case 5246:
                // Glyph of Intimidating Shout
                if (!GetCaster() || !GetCaster()->HasAura(63327))
                    return;
                break;
        }
    }

    target->SetControlled(apply, UNIT_STATE_ROOT);
}

void AuraEffect::HandlePreventFleeing(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->isFeared())
        target->SetControlled(!(apply), UNIT_STATE_FLEEING);
}

/***************************/
/***        CHARM        ***/
/***************************/

void AuraEffect::HandleModPossess(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    // no support for posession AI yet
    if (caster && caster->GetTypeId() == TYPEID_UNIT)
    {
        HandleModCharm(aurApp, mode, apply);
        return;
    }

    if (apply)
    {
        target->SetCharmedBy(caster, target->IsVehicle() ? CHARM_TYPE_VEHICLE : CHARM_TYPE_POSSESS, aurApp);
        target->SetCombatTimer(GetBase()->GetDuration());
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        if (caster)
        {
            caster->SetCombatTimer(GetBase()->GetDuration());
            caster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        }
    }
    else
        target->RemoveCharmedBy(caster);
}

// only one spell has this aura
void AuraEffect::HandleModPossessPet(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    //seems it may happen that when removing it is no longer owner's pet
    //if (caster->ToPlayer()->GetPet() != target)
    //    return;

    Unit* target = aurApp->GetTarget();
    if (target->GetTypeId() != TYPEID_UNIT || !target->ToCreature()->isPet())
        return;

    Pet* pet = target->ToPet();

    if (apply)
    {
        if (caster->ToPlayer()->GetPet() != pet)
            return;

        pet->SetCharmedBy(caster, CHARM_TYPE_POSSESS, aurApp);
    }
    else
    {
        pet->RemoveCharmedBy(caster);

        if (!pet->IsWithinDistInMap(caster, pet->GetMap()->GetVisibilityRange()))
            pet->Remove(PET_SLOT_OTHER_PET, true, pet->m_Stampeded);
        else
        {
            // Reinitialize the pet bar and make the pet come back to the owner
            caster->ToPlayer()->PetSpellInitialize();
            if (!pet->getVictim())
            {
                pet->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, pet->GetFollowAngle());
            }
        }
    }
}

void AuraEffect::HandleModCharm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    if (apply)
        target->SetCharmedBy(caster, CHARM_TYPE_CHARM, aurApp);
    else
        target->RemoveCharmedBy(caster);
}

void AuraEffect::HandleCharmConvert(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->IsVehicle())
        return;

    Unit* caster = GetCaster();

    if (apply)
        target->SetCharmedBy(caster, CHARM_TYPE_CONVERT, aurApp);
    else
        target->RemoveCharmedBy(caster);
}

/**
 * Such auras are applied from a caster(=player) to a vehicle.
 * This has been verified using spell #49256
 */
void AuraEffect::HandleAuraControlVehicle(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsVehicle())
        return;

    Unit* caster = GetCaster();

    if (!caster || caster == target)
        return;

    if (apply)
        caster->_EnterVehicle(target->GetVehicleKit(), m_amount - 1, aurApp);
    else
    {
        if (GetId() == 53111) // Devour Humanoid
        {
            target->Kill(caster);
            if (caster->GetTypeId() == TYPEID_UNIT)
                caster->ToCreature()->RemoveCorpse();
        }
        caster->_ExitVehicle();
        // some SPELL_AURA_CONTROL_VEHICLE auras have a dummy effect on the player - remove them
        caster->RemoveAurasDueToSpell(GetId());
    }
}

/*********************************************************/
/***                  MODIFY SPEED                     ***/
/*********************************************************/
void AuraEffect::HandleAuraModIncreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    switch (m_spellInfo->Id)
    {
        case 114868:// Soul Reaper - Haste
            // Don't increase speed if caster doesn't have glyph
            if (!target->HasAura(146645))
                return;
            break;
        default:
            break;
    }

    if (GetAuraType() == SPELL_AURA_INCREASE_MIN_SWIM_SPEED)
    {
        target->UpdateSpeed(MOVE_SWIM, true);
        return;
    }

    target->UpdateSpeed(MOVE_WALK, true);
    target->UpdateSpeed(MOVE_RUN, true);

    if (GetAuraType() == SPELL_AURA_MOD_MINIMUM_SPEED)
    {
        target->UpdateSpeed(MOVE_RUN_BACK, true);
        target->UpdateSpeed(MOVE_FLIGHT, true);
    }
}

void AuraEffect::HandleAuraModIncreaseMountedSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleAuraModIncreaseSpeed(aurApp, mode, apply);
}

void AuraEffect::HandleAuraModIncreaseFlightSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (GetAuraType() == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
    {
        //! Someone should clean up these hacks and remove it from this function. It doesn't even belong here.
        if (mode & AURA_EFFECT_HANDLE_REAL)
        {
            //Players on flying mounts must be immune to polymorph
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);

            // Dragonmaw Illusion (overwrite mount model, mounted aura already applied)
            if (apply && target->HasAuraEffect(42016, 0) && target->GetMountID())
                target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 16314);
        }
    }

    target->UpdateSpeed(MOVE_FLIGHT, true);
}

void AuraEffect::HandleAuraModIncreaseSwimSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    target->UpdateSpeed(MOVE_SWIM, true);
}

void AuraEffect::HandleAuraModDecreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_WALK, true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT, true);
    target->UpdateSpeed(MOVE_RUN_BACK, true);
    target->UpdateSpeed(MOVE_SWIM_BACK, true);
    target->UpdateSpeed(MOVE_FLIGHT_BACK, true);
}

void AuraEffect::HandleAuraModUseNormalSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->UpdateSpeed(MOVE_RUN,  true);
    target->UpdateSpeed(MOVE_WALK, true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT,  true);
    
    switch (GetId())
    {
        case 121164: // PURPLE
        case 121175: // ORANGE
        case 121176: // GREEN
        case 121177: // YELLOW
        {
            Player* player = target->ToPlayer();
            ASSERT(player != nullptr);
            if (BattlegroundKT* bg = dynamic_cast<BattlegroundKT*>(player->GetBattleground()))
            {
                uint8 const index = bg->GetOrbIndex(player->GetGUID());
                if (apply)
                    bg->TakeOrb(player, index);
                else
                    bg->ReleaseOrb(player, index);
            }
            break;
        }
    }
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void AuraEffect::HandleModStateImmunityMask(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    std::list <AuraType> aura_immunity_list;
    uint32 mechanic_immunity_list = 0;
    int32 miscVal = GetMiscValue();

    switch (miscVal)
    {
        case 96:
        case 1615:
        {
            if (GetAmount())
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }
            break;
        }
        case 679:
        case 1921:
        {
            if (GetId() == 57742)
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }
            else if (GetId() == 108201 || GetId() == 115018)
            {
                mechanic_immunity_list = (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }
            break;
        }
        case 1557:
        {
            if (GetId() == 64187)
            {
                mechanic_immunity_list = (1 << MECHANIC_STUN);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
            }
            else
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }
            break;
        }
        case 1614:
        case 1694:
        {
            target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, apply);
            aura_immunity_list.push_back(SPELL_AURA_MOD_TAUNT);
            break;
        }
        case 1630:
        {
            if (!GetAmount())
            {
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_TAUNT);
            }
            else
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }
            break;
        }
        case 477:
        case 1733:
        {
            if (!GetAmount())
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT)
                    | (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);


                // Bladestorm
                if (GetId() == 46924)
                {
                    mechanic_immunity_list = (1 << MECHANIC_DISARM);
                    aura_immunity_list.push_back(SPELL_AURA_MOD_DISARM);
                }
            }
            break;
        }
        case 878:
        {
            if (GetAmount() == 1)
            {
                mechanic_immunity_list = (1 << MECHANIC_SNARE) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_DISORIENTED) | (1 << MECHANIC_FREEZE);

                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
            }
            break;
        }
        case 1887:
        {
            // Frost Pillar
            if (GetId() == 51271)
            {
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, SPELL_EFFECT_JUMP_DEST, apply);
                target->ApplySpellImmune(GetId(), IMMUNITY_ID, 49560, apply);   // Glyph of Resilient Grip hack
                miscVal = 0;

                // Glyph of Frost Pillar
                if (target->HasAura(58635))
                {
                    mechanic_immunity_list = (1 << MECHANIC_FEAR) | (1 << MECHANIC_STUN)
                    | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_CHARM)
                    | (1 << MECHANIC_SAPPED) | (1 << MECHANIC_HORROR)
                    | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_DISORIENTED)
                    | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_TURN);

                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SLEEP, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_TURN, apply);
                    aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
                    aura_immunity_list.push_back(SPELL_AURA_TRANSFORM);
                    aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
                    aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                    aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);

                    // Apply Root !
                    if (apply)
                        target->CastSpell(target, 90259, true);
                    else
                        target->RemoveAura(90259);
                }
            }

            break;
        }
        case 1849:
        {
            if (GetId() == 8143)  // Tremor Totem
            {
                aura_immunity_list.push_back(SPELL_AURA_MOD_POSSESS);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }

            break;
        }
        default:
            break;
    }

    if (aura_immunity_list.empty())
    {
            if (miscVal & (1<<10))
                aura_immunity_list.push_back(SPELL_AURA_MOD_STUN);
            if (miscVal & (1<<1))
                aura_immunity_list.push_back(SPELL_AURA_TRANSFORM);

            // These flag can be recognized wrong:
            if (miscVal & (1<<6))
                aura_immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
            if (miscVal & (1<<0))
                aura_immunity_list.push_back(SPELL_AURA_MOD_ROOT);
            if (miscVal & (1<<2))
                aura_immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
            if (miscVal & (1<<9))
            {
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR);
                aura_immunity_list.push_back(SPELL_AURA_MOD_FEAR_2);
            }
            if (miscVal & (1<<7))
                aura_immunity_list.push_back(SPELL_AURA_MOD_DISARM);
    }

    // apply immunities
    for (std::list <AuraType>::iterator iter = aura_immunity_list.begin(); iter != aura_immunity_list.end(); ++iter)
        target->ApplySpellImmune(GetId(), IMMUNITY_STATE, *iter, apply);

    if (apply && GetSpellInfo()->AttributesEx & SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY)
    {
        target->RemoveAurasWithMechanic(mechanic_immunity_list, AURA_REMOVE_BY_DEFAULT, GetId());
        for (std::list <AuraType>::iterator iter = aura_immunity_list.begin(); iter != aura_immunity_list.end(); ++iter)
            target->RemoveAurasByType(*iter);
    }
}

void AuraEffect::HandleModMechanicImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    uint32 mechanic = 0;

    switch (GetId())
    {
        case 108212:// Burst of Speed
        case 137573:// Burst of Speed (triggered)
        {
            return;
            break;
        }
        case 42292: // PvP trinket
        case 59752: // Every Man for Himself
        case 65547: // PvP trinket (Trial of Crusader)
        case 53490: // Bullheaded
        {
            mechanic = IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;
            // Actually we should apply immunities here, too, but the aura has only 100 ms duration, so there is practically no point
            break;
        }
        case 54508: // Demonic Empowerment
        {
            mechanic = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT) | (1 << MECHANIC_FEAR) | (1 << MECHANIC_BANISH);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_BANISH, apply);
            break;
        }
        case 129020:// Avatar
        {
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_SNARE, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_DAZE, apply);
            break;
        }
        default:
        {
            if (GetMiscValue() < 1)
                return;
            mechanic = 1 << GetMiscValue();
            target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
            break;
        }
    }

    if (apply && GetSpellInfo()->AttributesEx & SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY)
        target->RemoveAurasWithMechanic(mechanic, AURA_REMOVE_BY_DEFAULT, GetId());
}

void AuraEffect::HandleAuraModEffectImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

   target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, GetMiscValue(), apply);

    // when removing flag aura, handle flag drop
    if (!apply && target->GetTypeId() == TYPEID_PLAYER
        && (GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION))
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (target->ToPlayer()->InBattleground())
            {
                if (Battleground* bg = target->ToPlayer()->GetBattleground())
                    bg->EventPlayerDroppedFlag(target->ToPlayer());
            }
            else
                sOutdoorPvPMgr->HandleDropFlag((Player*)target, GetSpellInfo()->Id);
        }
    }
}

void AuraEffect::HandleAuraModStateImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellImmune(GetId(), IMMUNITY_STATE, GetMiscValue(), apply);

    if (apply && GetSpellInfo()->AttributesEx & SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY)
        target->RemoveAurasByType(AuraType(GetMiscValue()), 0, GetBase());
}

void AuraEffect::HandleAuraModSchoolImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellImmune(GetId(), IMMUNITY_SCHOOL, GetMiscValue(), (apply));

    if (GetSpellInfo()->Mechanic == MECHANIC_BANISH)
    {
        if (apply)
            target->AddUnitState(UNIT_STATE_ISOLATED);
        else
        {
            bool banishFound = false;
            Unit::AuraEffectList const& banishAuras = target->GetAuraEffectsByType(GetAuraType());
            for (Unit::AuraEffectList::const_iterator i = banishAuras.begin(); i != banishAuras.end(); ++i)
                if ((*i)->GetSpellInfo()->Mechanic == MECHANIC_BANISH)
                {
                    banishFound = true;
                    break;
                }
            if (!banishFound)
                target->ClearUnitState(UNIT_STATE_ISOLATED);
        }
    }

    if (apply && GetMiscValue() == SPELL_SCHOOL_MASK_NORMAL)
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    // remove all flag auras (they are positive, but they must be removed when you are immune)
    if (GetSpellInfo()->AttributesEx & SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY
        && GetSpellInfo()->AttributesEx2 & SPELL_ATTR2_DAMAGE_REDUCED_SHIELD)
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    // TODO: optimalize this cycle - use RemoveAurasWithInterruptFlags call or something else
    if ((apply)
        && GetSpellInfo()->AttributesEx & SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY
        && GetSpellInfo()->IsPositive())                       //Only positive immunity removes auras
    {
        uint32 school_mask = GetMiscValue();
        Unit::AuraApplicationMap& Auras = target->GetAppliedAuras();
        for (Unit::AuraApplicationMap::iterator iter = Auras.begin(); iter != Auras.end();)
        {
            SpellInfo const* spell = iter->second->GetBase()->GetSpellInfo();
            if ((spell->GetSchoolMask() & school_mask)//Check for school mask
                && GetSpellInfo()->CanDispelAura(spell)
                && !iter->second->IsPositive()          //Don't remove positive spells
                && spell->Id != GetId()
                && !spell->IsPassive())               //Don't remove self
            {
                target->RemoveAura(iter);
            }
            else
                ++iter;
        }
    }
}

void AuraEffect::HandleAuraModDmgImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellImmune(GetId(), IMMUNITY_DAMAGE, GetMiscValue(), apply);
}

void AuraEffect::HandleAuraModDispelImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplySpellDispelImmunity(m_spellInfo, DispelType(GetMiscValue()), (apply));
}

/*********************************************************/
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void AuraEffect::HandleAuraModResistanceExclusive(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if (GetMiscValue() & int32(1<<x))
        {
            int32 amount = target->GetMaxPositiveAuraModifierByMiscMask(SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE, 1<<x, this);
            if (amount < GetAmount())
            {
                float value = float(GetAmount() - amount);
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_VALUE, value, apply);
                if (target->GetTypeId() == TYPEID_PLAYER)
                    target->ApplyResistanceBuffModsMod(SpellSchools(x), aurApp->IsPositive(), value, apply);
            }
        }
    }
}

void AuraEffect::HandleAuraModResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if (GetMiscValue() & int32(1<<x))
        {
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(GetAmount()), apply);
            if (target->GetTypeId() == TYPEID_PLAYER || target->ToCreature()->isPet())
                target->ApplyResistanceBuffModsMod(SpellSchools(x), GetAmount() > 0, (float)GetAmount(), apply);
        }
    }
}

void AuraEffect::HandleAuraModBaseResistancePCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // only players have base stats
    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        //pets only have base armor
        if (target->ToCreature()->isPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            target->HandleStatModifier(UNIT_MOD_ARMOR, BASE_PCT, float(GetAmount()), apply);
    }
    else
    {
        for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
        {
            if (GetMiscValue() & int32(1<<x) && aurApp->GetBase()->GetId() == 5487 && target->HasAura(16931))
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(330), apply);
            else if (GetMiscValue() & int32(1<<x))
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(GetAmount()), apply);
        }
    }
}

void AuraEffect::HandleModResistancePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    for (int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if (GetMiscValue() & int32(1<<i))
        {
            target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, float(GetAmount()), apply);
            if (target->GetTypeId() == TYPEID_PLAYER || target->ToCreature()->isPet())
            {
                target->ApplyResistanceBuffModsPercentMod(SpellSchools(i), true, (float)GetAmount(), apply);
                target->ApplyResistanceBuffModsPercentMod(SpellSchools(i), false, (float)GetAmount(), apply);
            }
        }
    }
}

void AuraEffect::HandleModBaseResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // only players have base stats
    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        //only pets have base stats
        if (target->ToCreature()->isPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            target->HandleStatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, float(GetAmount()), apply);
    }
    else
    {
        for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if (GetMiscValue() & (1<<i))
                target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(GetAmount()), apply);
    }
}

void AuraEffect::HandleModTargetResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // applied to damage as HandleNoImmediateEffect in Unit::CalcAbsorbResist and Unit::CalcArmorReducedDamage

    // show armor penetration
    if (target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE, GetAmount(), apply);

    // show as spell penetration only full spell penetration bonuses (all resistances except armor and holy
    if (target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_SPELL) == SPELL_SCHOOL_MASK_SPELL)
        target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, GetAmount(), apply);
}

/********************************/
/***           STAT           ***/
/********************************/

void AuraEffect::HandleAuraModStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (GetMiscValue() < -2 || GetMiscValue() > 4)
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        // -1 or -2 is all stats (misc < -2 checked in function beginning)
        if (GetMiscValue() < 0 || GetMiscValue() == i)
        {
            //target->ApplyStatMod(Stats(i), m_amount, apply);
            target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(GetAmount()), apply);
            if (target->GetTypeId() == TYPEID_PLAYER || target->ToCreature()->isPet())
                target->ApplyStatBuffMod(Stats(i), (float)GetAmount(), apply);
        }
    }
}

void AuraEffect::HandleModPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (GetMiscValue() < -1 || GetMiscValue() > 4)
        return;

    // only players have base stats
    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if (GetMiscValue() == i || GetMiscValue() == -1)
            target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, float(m_amount), apply);
    }
}

void AuraEffect::HandleModSpellDamagePercentFromStat(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromStat(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleAuraModSpellPowerPercent(AuraApplication const * aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit *target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellDamagePercentFromAttackPower(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromAttackPower(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModHealingDone(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;
    // implemented in Unit::SpellHealingBonus
    // this information is for client side only
    target->ToPlayer()->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModTotalPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // save current health state
    float healthPct = target->GetHealthPct();
    bool alive = target->isAlive();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        if (GetMiscValueB() & 1 << i || !GetMiscValueB()) // 0 is also used for all stats
        {
            // test fix for health
            if (i == STAT_STAMINA)
            {
                float oldModifier = target->GetModifierValue(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT);
                float staminaModifier = target->GetTotalAuraMultiplierByMiscBMask(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, 1 << i);
                target->SetAndHandleModifierValue(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, staminaModifier);
                target->ApplyStatPercentBuffMod(Stats(i), oldModifier, !apply);
                target->ApplyStatPercentBuffMod(Stats(i), staminaModifier, apply);
            }
            else
            {
                target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(GetAmount()), apply);
                if (target->GetTypeId() == TYPEID_PLAYER || target->ToCreature()->isPet())
                    target->ApplyStatPercentBuffMod(Stats(i), float(GetAmount()), apply);
            }
        }
    }

    // recalculate current HP/MP after applying aura modifications (only for spells with SPELL_ATTR0_UNK4 0x00000010 flag)
    // this check is total bullshit i think
    if (alive)
        if(GetMiscValueB() & 1 << STAT_STAMINA && (m_spellInfo->Attributes & SPELL_ATTR0_ABILITY))
            target->SetHealth(std::max<uint32>(uint32(healthPct * target->GetMaxHealth() * 0.01f), (alive ? 1 : 0)));
}

void AuraEffect::HandleAuraModResistenceOfStatPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (GetMiscValue() != SPELL_SCHOOL_MASK_NORMAL)
    {
        // support required adding replace UpdateArmor by loop by UpdateResistence at intellect update
        // and include in UpdateResistence same code as in UpdateArmor for aura mod apply.
        return;
    }

    // Recalculate Armor
    target->UpdateArmor();
}

void AuraEffect::HandleAuraModExpertise(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ToPlayer()->UpdateExpertise(BASE_ATTACK);
    target->ToPlayer()->UpdateExpertise(OFF_ATTACK);
    target->ToPlayer()->UpdateExpertise(RANGED_ATTACK);
}

/********************************/
/***      HEAL & ENERGIZE     ***/
/********************************/
void AuraEffect::HandleModPowerRegen(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    switch (GetMiscValue())
    {
        case POWER_MANA:
            target->ToPlayer()->UpdateManaRegen();
            break;
        case POWER_RUNES:
            target->ToPlayer()->UpdateAllRunesRegen();
            break;
        case POWER_ENERGY:
            target->ToPlayer()->UpdateEnergyRegen();
            break;
        default:
            // other powers are not immediate effects - implemented in Player::Regenerate, Creature::Regenerate
            break;
    }
}

void AuraEffect::HandleModPowerRegenPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleModPowerRegen(aurApp, mode, apply);
}

void AuraEffect::HandleModManaRegen(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    //Note: an increase in regen does NOT cause threat.
    target->ToPlayer()->UpdateManaRegen();
}

void AuraEffect::HandleAuraModIncreaseManaRegen(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ToPlayer()->UpdateManaRegen();
}

void AuraEffect::HandleAuraModIncreaseHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(GetAmount()), apply);
        target->ModifyHealth(GetAmount());
    }
    else
    {
        // Hackfix Nature's Guardian
        if (aurApp->GetBase()->GetId() == 31616)
        {
            int32 newCurrentHp = CalculatePct(int32(target->GetMaxHealth() / 1.25f), target->GetHealthPct());

            target->SetHealth(newCurrentHp);
        }
        else if (aurApp->GetBase()->GetId() != 106922 && aurApp->GetBase()->GetId() != 113072)
        {
            if (int32(target->GetHealth()) > GetAmount())
                target->ModifyHealth(-GetAmount());
            else
                target->SetHealth(1);
        }
        target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(GetAmount()), apply);
    }
}

void AuraEffect::HandleAuraModIncreaseMaxHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    uint32 oldhealth = target->GetHealth();
    double healthPercentage = (double)oldhealth / (double)target->GetMaxHealth();

    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(GetAmount()), apply);

    // refresh percentage
    if (oldhealth > 0)
    {
        uint32 newhealth = uint32(ceil((double)target->GetMaxHealth() * healthPercentage));
        if (newhealth == 0)
            newhealth = 1;

        target->SetHealth(newhealth);
    }
}

void AuraEffect::HandleAuraModIncreaseEnergy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    Powers powerType = Powers(GetMiscValue());
    // do not check power type, we can always modify the maximum
    // as the client will not see any difference
    // also, placing conditions that may change during the aura duration
    // inside effect handlers is not a good idea
    //if (int32(powerType) != GetMiscValue())
    //    return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    target->HandleStatModifier(unitMod, TOTAL_VALUE, float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModIncreaseEnergyPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    Powers powerType = Powers(GetMiscValue());
    // do not check power type, we can always modify the maximum
    // as the client will not see any difference
    // also, placing conditions that may change during the aura duration
    // inside effect handlers is not a good idea
    //if (int32(powerType) != GetMiscValue())
    //    return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);
    float amount = float(GetAmount());

    if (apply)
    {
        target->HandleStatModifier(unitMod, TOTAL_PCT, amount, apply);
        target->ModifyPowerPct(powerType, amount, apply);
    }
    else
    {
        target->ModifyPowerPct(powerType, amount, apply);
        target->HandleStatModifier(unitMod, TOTAL_PCT, amount, apply);
    }
}

void AuraEffect::HandleAuraModIncreaseHealthPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    float amount = float(GetAmount());

    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    float percent = target->GetHealthPct();
    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, amount, apply);
    if (target->isAlive())
        target->SetHealth(target->CountPctFromMaxHealth(int32(percent)));
}

void AuraEffect::HandleAuraIncreaseBaseHealthPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    target->HandleStatModifier(UNIT_MOD_HEALTH, BASE_PCT, float(GetAmount()), apply);
}

/********************************/
/***          FIGHT           ***/
/********************************/

void AuraEffect::HandleAuraModParryPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ToPlayer()->UpdateParryPercentage();
}

void AuraEffect::HandleAuraModDodgePercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ToPlayer()->UpdateDodgePercentage();
}

void AuraEffect::HandleAuraModBlockPercent(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ToPlayer()->UpdateBlockPercentage();
}

void AuraEffect::HandleAuraModRegenInterrupt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleModManaRegen(aurApp, mode, apply);
}

void AuraEffect::HandleAuraModWeaponCritPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int i = 0; i < MAX_ATTACK; ++i)
        if (Item* pItem = target->ToPlayer()->GetWeaponForAttack(WeaponAttackType(i), true))
            target->ToPlayer()->_ApplyWeaponDependentAuraCritMod(pItem, WeaponAttackType(i), this, apply);

    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if (GetSpellInfo()->EquippedItemClass == -1)
    {
        //target->ToPlayer()->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float (GetAmount()), apply);
        //target->ToPlayer()->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float (GetAmount()), apply);
        //target->ToPlayer()->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float (GetAmount()), apply);
        target->ToPlayer()->UpdateFlatModCritValue();
    }
    else
    {
        // done in Player::_ApplyWeaponDependentAuraMods
    }
}

void AuraEffect::HandleModHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        target->ToPlayer()->UpdateMeleeHitChances();
        target->ToPlayer()->UpdateRangedHitChances();
    }
    else
    {
        target->m_modMeleeHitChance += (apply) ? GetAmount() : (-GetAmount());
        target->m_modRangedHitChance += (apply) ? GetAmount() : (-GetAmount());
    }
}

void AuraEffect::HandleModSpellHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateSpellHitChances();
    else
        target->m_modSpellHitChance += (apply) ? GetAmount(): (-GetAmount());
}

void AuraEffect::HandleModSpellCritChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateAllSpellCritChances();
    else
        target->m_baseSpellCritChance += (apply) ? GetAmount():-GetAmount();
}

void AuraEffect::HandleModSpellCritChanceShool(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
        if (GetMiscValue() & (1<<school))
            target->ToPlayer()->UpdateSpellCritChance(school);
}

void AuraEffect::HandleAuraModCritPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->GetTypeId() != TYPEID_PLAYER)
    {
        target->m_baseSpellCritChance += (apply) ? GetAmount():-GetAmount();
        return;
    }

    //target->ToPlayer()->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float (GetAmount()), apply);
    //target->ToPlayer()->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float (GetAmount()), apply);
    //target->ToPlayer()->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float (GetAmount()), apply);

    target->ToPlayer()->UpdateFlatModCritValue();

    // included in Player::UpdateSpellCritChance calculation
    target->ToPlayer()->UpdateAllSpellCritChances();
}

void AuraEffect::HandleAuraModResiliencePct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = target->ToPlayer();

    float baseValue = _player->GetFloatValue(PLAYER_FIELD_MOD_RESILIENCE_PCT);

    if (apply)
        baseValue += GetAmount() / 100.0f;
    else
        baseValue -= GetAmount() / 100.0f;

    _player->SetFloatValue(PLAYER_FIELD_MOD_RESILIENCE_PCT, baseValue);
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void AuraEffect::HandleAuraModSpellHaste(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateRating(CR_HASTE_SPELL);
    else
        target->ApplyCastTimePercentMod((float) GetAmount(), apply);
}

void AuraEffect::HandleModMeleeRangedSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    //! ToDo: Haste auras with the same handler _CAN'T_ stack together
    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
    {
        target->ToPlayer()->UpdateRating(CR_HASTE_MELEE);
        target->ToPlayer()->UpdateRating(CR_HASTE_RANGED);
    }
    else
    {
        float value = (float) GetAmount();

        target->ApplyAttackTimePercentMod(BASE_ATTACK, value, apply);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, value, apply);
        target->ApplyAttackTimePercentMod(RANGED_ATTACK, value, apply);
    }
}

void AuraEffect::HandleModCombatSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
    {
        target->ToPlayer()->UpdateRating(CR_HASTE_MELEE);
        target->ToPlayer()->UpdateRating(CR_HASTE_RANGED);
        target->ToPlayer()->UpdateRating(CR_HASTE_SPELL);
    }
    else
    {
        float value = (float) GetAmount();

        target->ApplyCastTimePercentMod(value, apply);
        target->ApplyAttackTimePercentMod(BASE_ATTACK, value, apply);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, value, apply);
        target->ApplyAttackTimePercentMod(RANGED_ATTACK, value, apply);
    }
}

void AuraEffect::HandleModAttackSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplyAttackTimePercentMod(BASE_ATTACK, (float)GetAmount(), apply);
    target->ApplyAttackTimePercentMod(OFF_ATTACK, (float)GetAmount(), apply);
    target->UpdateDamagePhysical(BASE_ATTACK);
    target->UpdateDamagePhysical(OFF_ATTACK);
}

void AuraEffect::HandleModMeleeSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    //! ToDo: Haste auras with the same handler _CAN'T_ stack together
    Unit* target = aurApp->GetTarget();

    if (target->IsPlayer())
        target->ToPlayer()->UpdateRating(CR_HASTE_MELEE);
    else
    {
        float value = (float) GetAmount();

        target->ApplyAttackTimePercentMod(BASE_ATTACK, value, apply);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, value, apply);
    }
}

void AuraEffect::HandleAuraModRangedHaste(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    //! ToDo: Haste auras with the same handler _CAN'T_ stack together
    Unit* target = aurApp->GetTarget();

    target->ApplyAttackTimePercentMod(RANGED_ATTACK, (float)GetAmount(), apply);

    if (target->IsPlayer())
        target->ToPlayer()->UpdateRating(CR_HASTE_RANGED);
    else
        target->ApplyAttackTimePercentMod(RANGED_ATTACK, (float) GetAmount(), apply);
}

/********************************/
/***       COMBAT RATING      ***/
/********************************/

void AuraEffect::HandleModRating(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    switch (m_spellInfo->Id)
    {
        // Heart's Judgment, Heart of Ignacious 
        case 91041:
            target->RemoveAurasDueToSpell(91027);
            break;
        // Heart's Judgment, Heart of Ignacious (Heroic)
        case 92328:
            target->RemoveAurasDueToSpell(92325);
            break;
    }

    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            target->ToPlayer()->ApplyRatingMod(CombatRating(rating), GetAmount(), apply);
}

void AuraEffect::HandleModRatingFromStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Just recalculate ratings
    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            target->ToPlayer()->ApplyRatingMod(CombatRating(rating), 0, apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void AuraEffect::HandleAuraModAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModRangedAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if ((target->getClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // Don't apply Markmanship aura twice on pet
    if (GetCaster() && GetCaster()->ToPlayer() && aurApp->GetBase()->GetId() == 19506)
        if (Pet* pet = GetCaster()->ToPlayer()->GetPet())
            if (target->GetGUID() == pet->GetGUID())
                return;

    //UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    //target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, float(GetAmount()), apply);
    float modifier = target->GetTotalAuraMultiplier(SPELL_AURA_MOD_ATTACK_POWER_PCT);
    target->SetAndHandleModifierValue(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, modifier);
}

void AuraEffect::HandleAuraModRangedAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if ((target->getClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    //UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    //target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, float(GetAmount()), apply);
    float modifier = target->GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT);
    target->SetAndHandleModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, modifier);
}

void AuraEffect::HandleAuraModAttackPowerOfArmor(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // Recalculate bonus
    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateAttackPowerAndDamage(false);
}

void AuraEffect::HandleOverrideAttackPowerBySpellPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // Recalculate bonus
    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateAttackPowerAndDamage(false);

    // Magic damage modifiers implemented in Unit::MeleeDamageBonusDone
    // This information for client side use only
    // Get attack power bonus for all attack type
    target->SetFloatValue(PLAYER_FIELD_OVERRIDE_AP_BY_SPELL_POWER_PCT, float(GetAmount()));
}

void AuraEffect::HandleOverrideSpellPowerByAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Get healing bonus for all schools
    target->SetFloatValue(PLAYER_FIELD_OVERRIDE_SPELL_POWER_BY_AP_PCT, float(GetAmount()));
}

void AuraEffect::HandleIncreaseHasteFromItemsByPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->ToPlayer())
        return;

    for (uint8 i = 0; i < MAX_COMBAT_RATING; i++)
    {
        if (GetMiscValue() & (1 << i))
            target->ToPlayer()->UpdateRating((CombatRating)i);
    }
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/
void AuraEffect::HandleModDamageDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // apply item specific bonuses for already equipped weapon
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = target->ToPlayer()->GetWeaponForAttack(WeaponAttackType(i), true))
                target->ToPlayer()->_ApplyWeaponDependentAuraDamageMod(pItem, WeaponAttackType(i), this, apply);
    }

    // GetMiscValue() is bitmask of spell schools
    // 1 (0-bit) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if ((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellInfo()->EquippedItemClass == -1 || target->GetTypeId() != TYPEID_PLAYER)
        {
            target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(GetAmount()), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(GetAmount()), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(GetAmount()), apply);

            if (target->GetTypeId() == TYPEID_PLAYER)
            {
                if (GetAmount() > 0)
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS, GetAmount(), apply);
                else
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, GetAmount(), apply);
            }
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }
    }

    // Skip non magic case for speedup
    if ((GetMiscValue() & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if (GetSpellInfo()->EquippedItemClass != -1 || (GetSpellInfo()->EquippedItemInventoryTypeMask != 0 && GetSpellInfo()->EquippedItemInventoryTypeMask != -1))
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        target->ToPlayer()->ApplySpellPowerBonus(GetAmount(), apply);

        if (Guardian* pet = target->ToPlayer()->GetGuardianPet())
            pet->UpdateAttackPowerAndDamage();
    }
}

void AuraEffect::HandleModDamagePercentDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
            if (Item* item = target->ToPlayer()->GetWeaponForAttack(WeaponAttackType(i), false))
                target->ToPlayer()->_ApplyWeaponDependentAuraDamageMod(item, WeaponAttackType(i), this, apply);
    }

    if ((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL) && (GetSpellInfo()->EquippedItemClass == -1 || target->GetTypeId() != TYPEID_PLAYER))
    {
        target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND,         TOTAL_PCT, float (GetAmount()), apply);
        target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND,          TOTAL_PCT, float (GetAmount()), apply);
        target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED,           TOTAL_PCT, float (GetAmount()), apply);

        if (target->GetTypeId() == TYPEID_PLAYER)
            target->ToPlayer()->ApplyPercentModFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, float (GetAmount()), apply);
    }
    else
    {
        // done in Player::_ApplyWeaponDependentAuraMods for SPELL_SCHOOL_MASK_NORMAL && EquippedItemClass != -1 and also for wand case
    }
}

void AuraEffect::HandleModOffhandDamagePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(GetAmount()), apply);
}

void AuraEffect::HandleShieldBlockValue(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    BaseModType modType = FLAT_MOD;
    if (GetAuraType() == SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT)
        modType = PCT_MOD;

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->HandleBaseModValue(SHIELD_BLOCK_VALUE, modType, float(GetAmount()), apply);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void AuraEffect::HandleModPowerCostPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    float amount = CalculatePct(1.0f, GetAmount());
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1 << i))
            target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i, amount, apply);
}

void AuraEffect::HandleModPowerCost(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1<<i))
            target->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+i, GetAmount(), apply);
}

void AuraEffect::HandleArenaPreparation(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
        target->ModifyAuraState(AURA_STATE_BG_PREPARATION, true);
    }
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
        target->ModifyAuraState(AURA_STATE_BG_PREPARATION, false);
    }
}

void AuraEffect::HandleNoReagentUseAura(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    flag128 mask;
    Unit::AuraEffectList const& noReagent = target->GetAuraEffectsByType(SPELL_AURA_NO_REAGENT_USE);
        for (Unit::AuraEffectList::const_iterator i = noReagent.begin(); i != noReagent.end(); ++i)
            mask |= (*i)->m_spellInfo->Effects[(*i)->m_effIndex].SpellClassMask;

    target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1  , mask[0]);
    target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+1, mask[1]);
    target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+2, mask[2]);
    target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+3, mask[3]);
}

void AuraEffect::HandleAuraRetainComboPoints(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // combo points was added in SPELL_EFFECT_ADD_COMBO_POINTS handler
    // remove only if aura expire by time (in case combo points amount change aura removed without combo points lost)
    if (!(apply) && GetBase()->GetDuration() == 0 && target->ToPlayer()->GetComboTarget())
        if (Unit* unit = ObjectAccessor::GetUnit(*target, target->ToPlayer()->GetComboTarget()))
            target->ToPlayer()->AddComboPoints(unit, -GetAmount());
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void AuraEffect::HandleModCategoryCooldown(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 categoryId = uint32(GetMiscValue());
    int32 value = apply ? -GetAmount() : 0;

    SpellCategoriesEntry* spellCategory = const_cast<SpellCategoriesEntry*>(sSpellCategoriesStore.LookupEntry(categoryId));
    if (!sSpellCategoriesStore.LookupEntry(categoryId))
        return;

    target->ToPlayer()->SendCategoryCooldown(categoryId, value);
}

void AuraEffect::HandleOverrideActionbarSpells(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (GetId() == 77616)
    {
        target->ToPlayer()->m_HasDarkSimulacrumAura = apply;
    }
}

void AuraEffect::HandleAuraDummy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_REAPPLY)))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    if (mode & AURA_EFFECT_HANDLE_REAL)
    {
        // pet auras
        if (PetAura const* petSpell = sSpellMgr->GetPetAura(GetId(), m_effIndex))
        {
            if (apply)
                target->AddPetAura(petSpell);
            else
                target->RemovePetAura(petSpell);
        }
    }

    if (mode & (AURA_EFFECT_HANDLE_REAL | AURA_EFFECT_HANDLE_REAPPLY))
    {
        // AT APPLY
        if (apply)
        {
            switch (GetId())
            {
                // Satisfied
                case 87649:
                {
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        break;
                    if (Aura* aur = target->GetAura(87649))
                        if (aur->GetStackAmount() >= 91)
                            target->ToPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, 99041);

                    break;
                }
                case 1515:                                      // Tame beast
                    // FIX_ME: this is 2.0.12 threat effect replaced in 2.1.x by dummy aura, must be checked for correctness
                    if (caster && target->CanHaveThreatList())
                        target->AddThreat(caster, 10.0f);
                    break;
                case 13139:                                     // net-o-matic
                    // root to self part of (root_target->charge->root_self sequence
                    if (caster)
                        caster->CastSpell(caster, 13138, true, NULL, this);
                    break;
                case 37096:                                     // Blood Elf Illusion
                {
                    if (caster)
                    {
                        switch (caster->getGender())
                        {
                            case GENDER_FEMALE:
                                caster->CastSpell(target, 37095, true, NULL, this); // Blood Elf Disguise
                                break;
                            case GENDER_MALE:
                                caster->CastSpell(target, 37093, true, NULL, this);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                }
                case 39850:                                     // Rocket Blast
                    if (roll_chance_i(20))                       // backfire stun
                        target->CastSpell(target, 51581, true, NULL, this);
                    break;
                case 43873:                                     // Headless Horseman Laugh
                    target->PlayDistanceSound(11965);
                    break;
                case 46354:                                     // Blood Elf Illusion
                    if (caster)
                    {
                        switch (caster->getGender())
                        {
                            case GENDER_FEMALE:
                                caster->CastSpell(target, 46356, true, NULL, this);
                                break;
                            case GENDER_MALE:
                                caster->CastSpell(target, 46355, true, NULL, this);
                                break;
                        }
                    }
                    break;
                case 46361:                                     // Reinforced Net
                    if (caster)
                        target->GetMotionMaster()->MoveFall();
                    break;
                case 71563:
                    {
                        Aura* newAura = target->AddAura(71564, target);
                        if (newAura != nullptr)
                            newAura->SetStackAmount(newAura->GetSpellInfo()->StackAmount);
                    }
                    break;
                case 59628: // Tricks of the Trade
                    if (caster && caster->GetMisdirectionTarget())
                        target->SetReducedThreatPercent(100, caster->GetMisdirectionTarget()->GetGUID());
                    break;
            }
        }
        // AT REMOVE
        else
        {
            if ((GetSpellInfo()->IsQuestTame()) && caster && caster->isAlive() && target->isAlive())
            {
                uint32 finalSpelId = 0;
                switch (GetId())
                {
                    case 19548: finalSpelId = 19597; break;
                    case 19674: finalSpelId = 19677; break;
                    case 19687: finalSpelId = 19676; break;
                    case 19688: finalSpelId = 19678; break;
                    case 19689: finalSpelId = 19679; break;
                    case 19692: finalSpelId = 19680; break;
                    case 19693: finalSpelId = 19684; break;
                    case 19694: finalSpelId = 19681; break;
                    case 19696: finalSpelId = 19682; break;
                    case 19697: finalSpelId = 19683; break;
                    case 19699: finalSpelId = 19685; break;
                    case 19700: finalSpelId = 19686; break;
                    case 30646: finalSpelId = 30647; break;
                    case 30653: finalSpelId = 30648; break;
                    case 30654: finalSpelId = 30652; break;
                    case 30099: finalSpelId = 30100; break;
                    case 30102: finalSpelId = 30103; break;
                    case 30105: finalSpelId = 30104; break;
                }

                if (finalSpelId)
                    caster->CastSpell(target, finalSpelId, true, NULL, this);
            }

            switch (m_spellInfo->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:
                    switch (GetId())
                    {
                        case 2584: // Waiting to Resurrect
                            // Waiting to resurrect spell cancel, we must remove player from resurrect queue
                            if (target->GetTypeId() == TYPEID_PLAYER)
                            {
                                if (Battleground* bg = target->ToPlayer()->GetBattleground())
                                    bg->RemovePlayerFromResurrectQueue(target->GetGUID());
                                if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(target->GetZoneId()))
                                    bf->RemovePlayerFromResurrectQueue(target->GetGUID());
                            }
                            break;
                        case 36730:                                     // Flame Strike
                        {
                            target->CastSpell(target, 36731, true, NULL, this);
                            break;
                        }
                        case 44191:                                     // Flame Strike
                        {
                            if (target->GetMap()->IsDungeon())
                            {
                                uint32 spellId = target->GetMap()->IsHeroic() ? 46163 : 44190;

                                target->CastSpell(target, spellId, true, NULL, this);
                            }
                            break;
                        }
                        case 43681: // Inactive
                        {
                            if (target->GetTypeId() != TYPEID_PLAYER || aurApp->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                                return;

                            if (target->GetMap()->IsBattleground())
                                target->ToPlayer()->LeaveBattleground();
                            break;
                        }
                        case 42783: // Wrath of the Astromancer
                            target->CastSpell(target, GetAmount(), true, NULL, this);
                            break;
                        case 46308: // Burning Winds casted only at creatures at spawn
                            target->CastSpell(target, 47287, true, NULL, this);
                            break;
                        case 52172:  // Coyote Spirit Despawn Aura
                        case 60244:  // Blood Parrot Despawn Aura
                            target->CastSpell((Unit*)NULL, GetAmount(), true, NULL, this);
                            break;
                        case 58600: // Restricted Flight Area
                        case 58730: // Restricted Flight Area
                        case 91604: // Restricted Flight Area
                            if (aurApp->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                                target->CastSpell(target, 58601, true);
                            break;
                    }
                    break;
                case SPELLFAMILY_DEATHKNIGHT:
                    // Summon Gargoyle (Dismiss Gargoyle at remove)
                    if (GetId() == 61777)
                        target->CastSpell(target, GetAmount(), true);
                    break;
                case SPELLFAMILY_ROGUE:
                    //  Tricks of the trade
                    switch (GetId())
                    {
                        case 59628: //Tricks of the trade buff on rogue (6sec duration)
                            target->SetReducedThreatPercent(0,0);
                            break;
                        case 57934: //Tricks of the trade buff on rogue (30sec duration)
                            if (aurApp->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE || (caster && !caster->GetMisdirectionTarget()))
                                target->SetReducedThreatPercent(0,0);
                            else
                                target->SetReducedThreatPercent(0, (caster ? caster->GetMisdirectionTarget()->GetGUID() : 0));
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // AT APPLY & REMOVE

    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            if (!(mode & AURA_EFFECT_HANDLE_REAL))
                break;
            switch (GetId())
            {
                // Recently Bandaged
                case 11196:
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
                    break;
                // Unstable Power
                case 24658:
                {
                    uint32 spellId = 24659;
                    if (apply && caster)
                    {
                        SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);

                        for (uint32 i = 0; i < spell->StackAmount; ++i)
                            caster->CastSpell(target, spell->Id, true, NULL, nullptr, GetCasterGUID());
                        break;
                    }
                    target->RemoveAurasDueToSpell(spellId);
                    break;
                }
                // Restless Strength
                case 24661:
                {
                    uint32 spellId = 24662;
                    if (apply && caster)
                    {
                        SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);
                        for (uint32 i = 0; i < spell->StackAmount; ++i)
                            caster->CastSpell(target, spell->Id, true, NULL, nullptr, GetCasterGUID());
                        break;
                    }
                    target->RemoveAurasDueToSpell(spellId);
                    break;
                }
                // Tag Murloc
                case 30877:
                {
                    // Tag/untag Blacksilt Scout
                    target->SetEntry(apply ? 17654 : 17326);
                    break;
                }
                case 57819: // Argent Champion
                case 57820: // Ebon Champion
                case 57821: // Champion of the Kirin Tor
                case 57822: // Wyrmrest Champion
                case 93339: // Champion of the Earthen Ring
                case 94158: // Champion of the Dragonmaw Clan
                case 93337: // Champion of Ramkahen
                case 93341: // Champion of the Guardians of Hyjal
                case 93368: // Champion of the Wildhammer Clan
                case 93347: // Champion of Therazane
                case 93830: // Bilgewater Champion
                case 93827: // Darkspear Champion
                case 93806: // Darnassus Champion
                case 93811: // Exodar Champion
                case 93816: // Gilneas Champion
                case 93821: // Gnomeregan Champion
                case 93805: // Ironforge Champion
                case 93825: // Orgrimmar Champion
                case 93795: // Stormwind Champion
                case 94463: // Thunder Bluff Champion
                case 94462: // Undercity Champion
                case 93828: // Silvermoon Champion
                case 126434: // Tushui Tabard
                case 126436: // Huojin Tabard
                {
                    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                        break;

                    uint32 FactionID = 0;

                    if (apply)
                    {
                        switch (m_spellInfo->Id)
                        {
                            case 57819: FactionID = 1106; break; // Argent Crusade
                            case 57820: FactionID = 1098; break; // Knights of the Ebon Blade
                            case 57821: FactionID = 1090; break; // Kirin Tor
                            case 57822: FactionID = 1091; break; // The Wyrmrest Accord
                            case 93339: FactionID = 1135; break; // The Earthen Ring
                            case 94158: FactionID = 1172; break; // Dragonmaw Clan
                            case 93337: FactionID = 1173; break; // Ramkahen
                            case 93341: FactionID = 1158; break; // Guardians of Hyjal
                            case 93368: FactionID = 1174; break; // Wildhammer Clan
                            case 93347: FactionID = 1171; break; // Therazane
                            case 93830: FactionID = 1133; break; // Bilgewater Cartel
                            case 93827: FactionID = 530;  break; // Darkspear Trolls
                            case 93806: FactionID = 69;   break; // Darnassus
                            case 93811: FactionID = 930;  break; // Exodar
                            case 93816: FactionID = 1134; break; // Gilneas
                            case 93821: FactionID = 54;   break; // Gnomeregan
                            case 93805: FactionID = 47;   break; // Ironforge
                            case 93825: FactionID = 76;   break; // Orgrimmar
                            case 93795: FactionID = 72;   break; // Stormwind
                            case 94463: FactionID = 81;   break; // Thunder Bluff
                            case 94462: FactionID = 68;   break; // Undercity
                            case 93828: FactionID = 911;  break; // Silvermoon
                            case 126434: FactionID = 1353;break; // Tushui
                            case 126436: FactionID = 1352;break; // Huojin
                        }
                    }
                    caster->ToPlayer()->SetChampioningFaction(FactionID);
                    break;
                }
                // LK Intro VO (1)
                case 58204:
                    if (target->GetTypeId() == TYPEID_PLAYER)
                    {
                        // Play part 1
                        if (apply)
                            target->PlayDirectSound(14970, target->ToPlayer());
                        // continue in 58205
                        else
                            target->CastSpell(target, 58205, true);
                    }
                    break;
                // LK Intro VO (2)
                case 58205:
                    if (target->GetTypeId() == TYPEID_PLAYER)
                    {
                        // Play part 2
                        if (apply)
                            target->PlayDirectSound(14971, target->ToPlayer());
                        // Play part 3
                        else
                            target->PlayDirectSound(14972, target->ToPlayer());
                    }
                    break;
                case 62061: // Festive Holiday Mount
                    if (target->HasAuraType(SPELL_AURA_MOUNTED))
                    {
                        uint32 creatureEntry = 0;
                        if (apply)
                        {
                            if (target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                                creatureEntry = 24906;
                            else
                                creatureEntry = 15665;
                        }
                        else
                            creatureEntry = target->GetAuraEffectsByType(SPELL_AURA_MOUNTED).front()->GetMiscValue();

                        if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creatureEntry))
                        {
                            uint32 team = 0;
                            if (target->GetTypeId() == TYPEID_PLAYER)
                                team = target->ToPlayer()->GetTeam();

                            uint32 displayID = sObjectMgr->ChooseDisplayId(team, creatureInfo);
                            sObjectMgr->GetCreatureModelRandomGender(&displayID);

                            target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, displayID);
                        }
                    }
                    break;
            }

            break;
        }
        case SPELLFAMILY_MAGE:
        {
            //if (!(mode & AURA_EFFECT_HANDLE_REAL))
                //break;
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            //if (!(mode & AURA_EFFECT_HANDLE_REAL))
                //break;
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
                break;

            switch (GetId())
            {
                case 52610: // Savage Roar
                case 127538:// Savage Roar (Glyphed)
                {
                    if (apply)
                    {
                        if (target->GetShapeshiftForm() == FORM_CAT)
                            target->CastSpell(target, 62071, true, NULL, nullptr, GetCasterGUID());
                    }
                    else
                        target->RemoveAurasDueToSpell(62071);
                    break;
                }
                case 61336:                                 // Survival Instincts
                {
                    if (!(mode & AURA_EFFECT_HANDLE_REAL))
                        break;

                    if (apply)
                    {
                        if (!target->IsInFeralForm())
                            break;

                        target->CastSpell(target, 50322, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(50322);
                    break;
                }
                case 78675: // Solar Beam
                case 113286:// Solar Beam (symbiosis)
                {
                    if (apply && caster)
                    {
                        if (Aura* aur = caster->AddAura(81261, target))
                        {
                            aur->SetMaxDuration(GetBase()->GetDuration());
                            aur->SetDuration(GetBase()->GetDuration());
                        }
                    }
                    else
                        target->RemoveAura(81261);
                    break;
                }
            }
            // Predatory Strikes
            if (target->GetTypeId() == TYPEID_PLAYER && GetSpellInfo()->SpellIconID == 1563)
            {
                target->ToPlayer()->UpdateAttackPowerAndDamage();
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (!(mode & AURA_EFFECT_HANDLE_REAL))
                break;
            // Sentry Totem
            if (GetId() == 6495 && caster && caster->GetTypeId() == TYPEID_PLAYER)
            {
                if (apply)
                {
                    if (uint64 guid = caster->m_SummonSlot[4])
                    {
                        if (Creature* totem = caster->GetMap()->GetCreature(guid))
                            if (totem->isTotem())
                                caster->ToPlayer()->CastSpell(totem, 6277, true);
                    }
                }
                else
                    caster->ToPlayer()->StopCastingBindSight();
                return;
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (!(mode & AURA_EFFECT_HANDLE_REAL))
                break;

            switch (GetId())
            {
                // Guardian of Ancient Kings (Retribution spec)
                case 86698:
                {
                    if (!caster)
                        break;

                    if (apply)
                        caster->CastSpell(caster, 86701, true);
                    else
                    {
                        if (caster->HasAura(86700))
                            caster->CastSpell(caster, 86704, true);

                        caster->RemoveAurasDueToSpell(86700);
                        caster->RemoveAurasDueToSpell(86701);
                    }
                    break;
                }
                // Guardian of Ancient Kings (Holy spec)
                case 86669:
                {
                    if (!apply && caster)
                        caster->RemoveAurasDueToSpell(86674);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_DEATHKNIGHT:
        {
            if (!(mode & AURA_EFFECT_HANDLE_REAL))
                break;
            // Improved Frost Presence
            if (m_spellInfo->SpellIconID == 2636)
            {
                if (apply)
                {
                    if (!target->HasAura(48266) && !target->HasAura(63611))
                        target->CastSpell(target, 63611, true);
                }
                else
                    target->RemoveAurasDueToSpell(63611);
            break;
        }
        case SPELLFAMILY_WARLOCK:
            {
                switch(GetId())
                {
                    // Demonic Pact
                case 47236:
                    if (caster)
                        caster->CastSpell(caster, 53646, true);
                    break;
                }
                break;
            }
        }
    }
}

void AuraEffect::HandleChannelDeathItem(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (apply || aurApp->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
        return;

    Unit* caster = GetCaster();

    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* plCaster = caster->ToPlayer();
    Unit* target = aurApp->GetTarget();

    // Item amount
    if (GetAmount() <= 0)
        return;

    if (GetSpellInfo()->Effects[m_effIndex].ItemType == 0)
        return;

    //Adding items
    uint32 noSpaceForCount = 0;
    uint32 count = m_amount;

    ItemPosCountVec dest;
    InventoryResult msg = plCaster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, GetSpellInfo()->Effects[m_effIndex].ItemType, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)
    {
        count-=noSpaceForCount;
        plCaster->SendEquipError(msg, NULL, NULL, GetSpellInfo()->Effects[m_effIndex].ItemType);
        if (count == 0)
            return;
    }

    Item* newitem = plCaster->StoreNewItem(dest, GetSpellInfo()->Effects[m_effIndex].ItemType, true);
    if (!newitem)
    {
        plCaster->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL);
        return;
    }
    plCaster->SendNewItem(newitem, count, true, true);
}

void AuraEffect::HandleBindSight(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    Unit* caster = GetCaster();

    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    caster->ToPlayer()->SetViewpoint(target, apply);
}

void AuraEffect::HandleForceReaction(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)target;

    uint32 faction_id = GetMiscValue();
    ReputationRank faction_rank = ReputationRank(m_amount);

    player->GetReputationMgr().ApplyForceReaction(faction_id, faction_rank, apply);
    player->GetReputationMgr().SendForceReactions();

    // stop fighting if at apply forced rank friendly or at remove real rank friendly
    if ((apply && faction_rank >= REP_FRIENDLY) || (!apply && player->GetReputationRank(faction_id) >= REP_FRIENDLY))
        player->StopAttackFaction(faction_id);
}

void AuraEffect::HandleAuraEmpathy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_UNIT)
        return;

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (target->HasAuraType(GetAuraType()))
            return;
    }

    CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(target->GetEntry());
    if (ci && ci->type == CREATURE_TYPE_BEAST)
        target->ApplyModUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
}

void AuraEffect::HandleAuraModFaction(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->setFaction(GetMiscValue());
        if (target->GetTypeId() == TYPEID_PLAYER)
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    }
    else
    {
        target->RestoreFaction();
        if (target->GetTypeId() == TYPEID_PLAYER)
            target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    }
}

void AuraEffect::HandleComprehendLanguage(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else
    {
        if (target->HasAuraType(GetAuraType()))
            return;

        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    }
}

void AuraEffect::HandleAuraConvertRune(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)target;

    if (player->getClass() != CLASS_DEATH_KNIGHT)
        return;

    uint32 runes = m_amount;
    RuneType rune = RuneType(GetMiscValueB());
    bool permanently = GetId() == 54637; // Find the better way

    // convert number of runes specified in aura amount of rune type in miscvalue to runetype in miscvalueb
    if (apply)
    {
        for (uint32 i = 0; i < MAX_RUNES && runes; ++i)
        {
            if (GetMiscValue() != player->GetCurrentRune(i))
                continue;

            if (permanently || !player->GetRuneCooldown(i))
            {
                player->AddRuneBySpell(i, rune, GetId());
                if (permanently)
                    player->SetRuneConvertType(i, permanently);

                --runes;
            }
        }
    }
    else
        player->RemoveRunesBySpell(GetId());
}

void AuraEffect::HandleAuraLinked(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    Unit* target = aurApp->GetTarget();

    uint32 triggeredSpellId = m_spellInfo->Effects[m_effIndex].TriggerSpell;
    SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggeredSpellId);
    if (!triggeredSpellInfo)
        return;

    if (mode & AURA_EFFECT_HANDLE_REAL)
    {
        if (apply)
        {
            Unit* caster = triggeredSpellInfo->NeedsToBeTriggeredByCaster() ? GetCaster() : target;

            if (!caster)
                return;
            // If amount avalible cast with basepoints (Crypt Fever for example)
            if (GetAmount())
                caster->CastCustomSpell(target, triggeredSpellId, &m_amount, NULL, NULL, true, NULL, this);
            else
                caster->CastSpell(target, triggeredSpellId, true, NULL, this);
        }
        else
        {
            uint64 casterGUID = triggeredSpellInfo->NeedsToBeTriggeredByCaster() ? GetCasterGUID() : target->GetGUID();
            target->RemoveAura(triggeredSpellId, casterGUID, 0, aurApp->GetRemoveMode());
        }
    }
    else if (mode & AURA_EFFECT_HANDLE_REAPPLY && apply)
    {
        uint64 casterGUID = triggeredSpellInfo->NeedsToBeTriggeredByCaster() ? GetCasterGUID() : target->GetGUID();
        // change the stack amount to be equal to stack amount of our aura
        Aura* triggeredAura = target->GetAura(triggeredSpellId, casterGUID);
        if (triggeredAura != nullptr)
            triggeredAura->ModStackAmount(GetBase()->GetStackAmount() - triggeredAura->GetStackAmount());
    }
}

void AuraEffect::HandleAuraOpenStable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER || !target->IsInWorld())
        return;

    if (apply)
        target->ToPlayer()->GetSession()->SendStablePet(target->GetGUID());

     // client auto close stable dialog at !apply aura
}

// Aura used by Throw Totem, Siege Explosive ,Sonic Divebomb, Mantid Barrel Interact and I has a bomb! spells 5.0.5
void AuraEffect::HandleAuraModNextSpell(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 triggerSpellId = GetSpellInfo()->Effects[EFFECT_0].TriggerSpell;
    if (apply)
        target->ToPlayer()->AddTemporarySpell(triggerSpellId);
    else
        target->ToPlayer()->RemoveTemporarySpell(triggerSpellId);
}

void AuraEffect::HandleAuraModFakeInebriation(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();

    if (apply)
    {
        target->m_invisibilityDetect.AddFlag(INVISIBILITY_DRUNK);
        target->m_invisibilityDetect.AddValue(INVISIBILITY_DRUNK, GetAmount());

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            int32 oldval = target->ToPlayer()->GetInt32Value(PLAYER_FAKE_INEBRIATION);
            target->ToPlayer()->SetInt32Value(PLAYER_FAKE_INEBRIATION, oldval + GetAmount());
        }
    }
    else
    {
        bool removeDetect = !target->HasAuraType(SPELL_AURA_MOD_FAKE_INEBRIATE);

        target->m_invisibilityDetect.AddValue(INVISIBILITY_DRUNK, -GetAmount());

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            int32 oldval = target->ToPlayer()->GetInt32Value(PLAYER_FAKE_INEBRIATION);
            target->ToPlayer()->SetInt32Value(PLAYER_FAKE_INEBRIATION, oldval - GetAmount());

            if (removeDetect)
                removeDetect = !target->ToPlayer()->GetDrunkValue();
        }

        if (removeDetect)
            target->m_invisibilityDetect.DelFlag(INVISIBILITY_DRUNK);
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleAuraOverrideSpells(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();

    if (!target || !target->IsInWorld())
        return;

    uint32 overrideId = uint32(GetMiscValue());

    if (apply)
    {
        target->SetUInt16Value(PLAYER_FIELD_OVERRIDE_SPELLS_ID, 1, overrideId);
        if (OverrideSpellDataEntry const* overrideSpells = sOverrideSpellDataStore.LookupEntry(overrideId))
            for (uint8 i = 0; i < MAX_OVERRIDE_SPELL; ++i)
                if (uint32 spellId = overrideSpells->spellId[i])
                    target->AddTemporarySpell(spellId);
    }
    else
    {
        target->SetUInt16Value(PLAYER_FIELD_OVERRIDE_SPELLS_ID, 1, 0);
        if (OverrideSpellDataEntry const* overrideSpells = sOverrideSpellDataStore.LookupEntry(overrideId))
            for (uint8 i = 0; i < MAX_OVERRIDE_SPELL; ++i)
                if (uint32 spellId = overrideSpells->spellId[i])
                    target->RemoveTemporarySpell(spellId);
    }
}

void AuraEffect::HandleAuraSetVehicle(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target->IsInWorld())
        return;

    uint32 vehicleId = GetMiscValue();

    if (apply)
    {
        if (!target->CreateVehicleKit(vehicleId, 0))
            return;
    }
    else if (target->GetVehicleKit())
        target->RemoveVehicleKit();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    ObjectGuid guid = target->GetGUID();

    target->ToPlayer()->SendPlayerVehicleData(apply ? vehicleId : 0);

    if (apply)
    {
        WorldPacket data(SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA, 0);
        target->ToPlayer()->GetSession()->SendPacket(&data);
    }
}

void AuraEffect::HandlePreventResurrection(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (aurApp->GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        aurApp->GetTarget()->RemoveByteFlag(PLAYER_FIELD_LIFETIME_MAX_RANK, 0, PLAYER_FIELD_BYTE_RELEASE_TIMER);
    else if (!aurApp->GetTarget()->GetBaseMap()->Instanceable())
        aurApp->GetTarget()->SetByteFlag(PLAYER_FIELD_LIFETIME_MAX_RANK, 0, PLAYER_FIELD_BYTE_RELEASE_TIMER);
}

void AuraEffect::HandlePeriodicDummyAuraTick(Unit* target, Unit* caster) const
{
    switch (GetSpellInfo()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (GetId())
            {
                case 138786:
                    if (caster && caster->ToPlayer() && GetBase())
                        caster->CastSpell(caster, 138788, true, caster->ToPlayer()->GetItemByGuid(GetBase()->GetCastItemGUID()), nullptr, caster->GetGUID());
                    break;
                case 138759:
                    if (caster && caster->ToPlayer() && GetBase())
                        caster->CastSpell(caster, 138760, true, caster->ToPlayer()->GetItemByGuid(GetBase()->GetCastItemGUID()), nullptr, caster->GetGUID());
                    break;
                case 146193: // 148958
                case 146195: // 148957
                case 146197: // 148954
                case 146199: // 148956
                {
                    const static int32 s_VisualSpells[4] = { 148958, 148957, 148954, 148956 };
                    
                    int32 index = (GetId() - 146193) / 2;
                    ASSERT(index >= 0 && index <= 3);
                    
                    if (caster)
                        caster->CastSpell(target, s_VisualSpells[index]);
                    break;
                }
                case 146198:
                {
                    if (!caster || !GetBase() || !GetBase()->GetCaster())
                        return;

                    caster->CastSpell(target, 148008, true);
                    break;
                }
                case 110310: // Dampening
                {
                    if (!caster)
                        return;

                    auto aura = GetBase();
                    if (auto effect = aura->GetEffect(EFFECT_0))
                    {
                        if (!caster->GetMap()->IsBattleArena())
                            caster->RemoveAura(110310);

                        auto amount = std::min(effect->GetAmount() + 1, 100);
                        if (amount == effect->GetAmount())
                            return;

                        effect->SetAmount(amount);
                    }
                    break;
                }
                case 121164: // PURPLE
                case 121175: // ORANGE
                case 121176: // GREEN
                case 121177: // YELLOW
                {
                    const static int32 max_amount_of_effects[3] = { -100, 600, 100 };
                    
                    auto aura = GetBase();

                    for (uint8 i = 0; i < 3; ++i)
                    {
                        if (auto effect = aura->GetEffect(i))
                        {
                            auto amount = effect->GetAmount() + effect->GetBaseAmount();
                            if (i == 0)
                                amount = std::max(amount, max_amount_of_effects[i]);
                            else
                                amount = std::min(amount, max_amount_of_effects[i]);

                            if (amount == effect->GetAmount())
                                return;

                            effect->ChangeAmount(amount);
                        }
                    }
                    break;
                }
                case 98971: // Smoldering Rune, Item - Death Knight T12 DPS 2P Bonus
                    if (Unit* l_Caster = GetCaster())
                        l_Caster->CastSpell(l_Caster, 99055, true);
                    break;
                case 91296: // Egg Shell, Corrupted Egg Shell
                    if (Unit* l_Caster = GetCaster())
                        l_Caster->CastSpell(l_Caster, 91306, true);
                    break;
                case 91308: // Egg Shell, Corrupted Egg Shell (H)
                    if (Unit* l_Caster = GetCaster())
                        l_Caster->CastSpell(l_Caster, 91311, true);
                    break;
                case 66149: // Bullet Controller Periodic - 10 Man
                case 68396: // Bullet Controller Periodic - 25 Man
                {
                    if (!caster)
                        break;

                    caster->CastCustomSpell(66152, SPELLVALUE_MAX_TARGETS, urand(1, 6), target, true);
                    caster->CastCustomSpell(66153, SPELLVALUE_MAX_TARGETS, urand(1, 6), target, true);
                    break;
                }
                case 62292: // Blaze (Pool of Tar)
                    // should we use custom damage?
                    target->CastSpell((Unit*)NULL, m_spellInfo->Effects[m_effIndex].TriggerSpell, true);
                    break;
                case 62399: // Overload Circuit
                {
                    if (target->GetMap()->IsDungeon() && int(target->GetAppliedAuras().count(62399)) >= (target->GetMap()->IsHeroic() ? 4 : 2))
                    {
                         target->CastSpell(target, 62475, true); // System Shutdown
                         if (Unit* veh = target->GetVehicleBase())
                             veh->CastSpell(target, 62475, true);
                    }
                    break;
                }
                case 64821: // Fuse Armor (Razorscale)
                {
                    if (GetBase()->GetStackAmount() == GetSpellInfo()->StackAmount)
                    {
                        target->CastSpell(target, 64774, true, NULL, nullptr, GetCasterGUID());
                        target->RemoveAura(64821);
                    }
                    break;
                }
                case 102522:// Fan the Flames
                {
                    if (!caster)
                        break;

                    switch (rand()%4)
                    {
                        case 1:
                            caster->CastSpell(caster, 109090, true);
                            break;
                        case 2:
                            caster->CastSpell(caster, 109105, true);
                            break;
                    }

                    if (GetBase()->GetDuration() < 1000)
                        caster->CastSpell(caster, 109107, true);

                    break;
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Mirror Image
            if (GetId() == 55342)
                // Set name of summons to name of caster
                target->CastSpell((Unit*)NULL, m_spellInfo->Effects[m_effIndex].TriggerSpell, true);
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            switch (GetSpellInfo()->Id)
            {
                // Master of Subtlety
                case 31666:
                    if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                        target->RemoveAurasDueToSpell(31665);
                    break;
                // Killing Spree
                case 51690:
                {
                    // TODO: this should use effect[1] of 51690
                    UnitList targets;
                    {
                        // eff_radius == 0
                        float radius = GetSpellInfo()->GetMaxRange(false);

                        CellCoord p(JadeCore::ComputeCellCoord(target->GetPositionX(), target->GetPositionY()));
                        Cell cell(p);

                        JadeCore::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck u_check(target, radius);
                        JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck> checker(target, targets, u_check);

                        TypeContainerVisitor<JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck>, GridTypeMapContainer > grid_object_checker(checker);
                        TypeContainerVisitor<JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck>, WorldTypeMapContainer > world_object_checker(checker);

                        cell.Visit(p, grid_object_checker,  *GetBase()->GetOwner()->GetMap(), *target, radius);
                        cell.Visit(p, world_object_checker, *GetBase()->GetOwner()->GetMap(), *target, radius);
                    }

                    std::vector<uint64> validTargets;

                    if (Unit* targetEnemy = target->getVictim())
                        validTargets.push_back(targetEnemy->GetGUID());
                    else
                    {
                        for (auto itr : targets)
                        {
                             if (itr->HasAuraType(SPELL_AURA_MOD_CONFUSE) || itr->HasAuraType(SPELL_AURA_MOD_CHARM) || itr->HasAuraType(SPELL_AURA_MOD_FEAR) || itr->HasAuraType(SPELL_AURA_MOD_STUN))
                                 continue;

                             validTargets.push_back(itr->GetGUID());

                             // If we don't have Blade Flurry we should attack one target
                             if (!target->HasAura(13877))
                                 break;
                        }
                    }

                    if (validTargets.empty())
                        return;

                    Unit* spellTarget = sObjectAccessor->GetUnit(*target, JadeCore::Containers::SelectRandomContainerElement(validTargets));

                    target->CastSpell(spellTarget, 57840, true);
                    target->CastSpell(spellTarget, 57841, true);
                    break;
                }
                // Overkill
                case 58428:
                    if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                        target->RemoveAurasDueToSpell(58427);
                    break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            switch (GetSpellInfo()->Id)
            {
                // Camouflage
                case 80326:
                {
                    if (!caster)
                        break;

                    if ((caster->isMoving() && !caster->HasAura(119449)) || caster->HasAura(80325))
                        return;

                    if (caster->HasAura(119449) || (caster->GetOwner() && caster->GetOwner()->HasAura(119449)))
                        caster->CastSpell(caster, 119450, true);
                    else
                        caster->CastSpell(caster, 80325, true);
                    break;
                }
                // Feeding Frenzy Rank 1
                case 53511:
                    if (target->getVictim() && target->getVictim()->HealthBelowPct(35))
                        target->CastSpell(target, 60096, true, 0, this);
                    return;
                // Feeding Frenzy Rank 2
                case 53512:
                    if (target->getVictim() && target->getVictim()->HealthBelowPct(35))
                        target->CastSpell(target, 60097, true, 0, this);
                    return;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
            if (!caster || !target)
                break;

            switch (GetId())
            {
                // Astral Shift
                case 52179:
                    // Periodic need for remove visual on stun/fear/silence lost
                    if (!(target->GetUInt32Value(UNIT_FIELD_FLAGS)&(UNIT_FLAG_STUNNED|UNIT_FLAG_FLEEING|UNIT_FLAG_SILENCED)))
                        target->RemoveAurasDueToSpell(52179);
                    break;
            }
            break;
        case SPELLFAMILY_DEATHKNIGHT:
            if (!caster)
                break;

            switch (GetId())
            {
                // Hysteria
                case 49016:
                {
                    uint32 damage = uint32(target->CountPctFromMaxHealth(1));
                    target->DealDamage(target, damage, NULL, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, GetSpellInfo(), false);
                    break;
                }
                // Blood of the North
                // Reaping
                // Blood Rites
                case 50034:
                case 56835:
                {
                    // do not remove death runes while in combat
                    if (target->isInCombat())
                        break;
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        break;
                    if (target->ToPlayer()->getClass() != CLASS_DEATH_KNIGHT)
                        break;

                    // timer expired - remove death runes
                    target->ToPlayer()->RemoveRunesBySpell(GetId());
                    break;
                }
                // Death's Advance
                case 96268:
                {
                    if (caster->ToPlayer()->GetRuneCooldown(RUNE_UNHOLY*2) && caster->ToPlayer()->GetRuneCooldown(RUNE_UNHOLY*2+1))
                        GetBase()->RefreshDuration();
                    else
                        caster->RemoveAurasDueToSpell(96268);
                    break;
                }
                default:
                    break;
            }
            break;
        case SPELLFAMILY_PALADIN:
            if (!caster || !target)
                break;

            // Holy Radiance
            // Spell 86452 doesn't exist, i don't know what it should do, maybe later will find.
            //if (GetId() == 82327)
            //    caster->CastSpell(target, 86452, true);
            break;
        case SPELLFAMILY_WARLOCK:
            switch (GetId())
            {
                // Curse of Elements - Jinx
                case 1490:
                {
                    Unit * caster = GetCaster();
                    if (!caster)
                        break;
                    if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 5002, 0))
                    {
                        if (aurEff->GetId() == 85479)
                            caster->CastSpell(target, 86105, true);
                        else caster->CastSpell(target, 85547, true);
                    }
                    break;
                }
            }
            break;
        default:
            break;
    }
}

void AuraEffect::HandlePeriodicTriggerSpellAuraTick(Unit* target, Unit* caster) const
{
    // generic casting code with custom spells and target/caster customs
    uint32 triggerSpellId = GetSpellInfo()->Effects[GetEffIndex()].TriggerSpell;

    SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId);
    SpellInfo const* auraSpellInfo = GetSpellInfo();
    uint32 auraId = auraSpellInfo->Id;

    // specific code for cases with no trigger spell provided in field
    if (triggeredSpellInfo == NULL)
    {
        switch (auraSpellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (auraId)
                {
                    // Earth's Vengeance, Morchok, Dragon Soul
                    case 103176:
                    {
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            Position pos;
                            caster->GetNearPosition(pos, 40.0f, frand(0.0f, 2 * M_PI));
                            if (caster->GetDistance(pos) >= 20.0f)
                                caster->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 103177, true);
                        }
                        return;
                    }
                    // Shadow Cloak, Illidan, Well of Eternity
                    case 103004:
                        if (target->isInCombat())
                            return;

                        if (!target->HasAura(102994))
                            target->CastSpell(target, 102994, true);

                        target->CastSpell(target, 103020, true);
                        return;
                    // Thaumaturgy Channel
                    case 9712:
                        triggerSpellId = 21029;
                        break;
                    // Brood Affliction: Bronze
                    case 23170:
                        triggerSpellId = 23171;
                        break;
                    // Restoration
                    case 24379:
                    case 23493:
                    {
                        if (caster)
                        {
                            int32 heal = caster->CountPctFromMaxHealth(10);
                            caster->HealBySpell(target, auraSpellInfo, heal);

                            if (int32 mana = caster->GetMaxPower(POWER_MANA))
                            {
                                mana /= 10;
                                caster->EnergizeBySpell(caster, 23493, mana, POWER_MANA);
                            }
                        }
                        return;
                    }
                    // Nitrous Boost
                    case 27746:
                        if (caster && target->GetPower(POWER_MANA) >= 10)
                        {
                            target->ModifyPower(POWER_MANA, -10);
                            target->SendEnergizeSpellLog(caster, 27746, 10, POWER_MANA);
                        }
                        else
                            target->RemoveAurasDueToSpell(27746);
                        return;
                    // Frost Blast
                    case 27808:
                        if (caster)
                            caster->CastCustomSpell(29879, SPELLVALUE_BASE_POINT0, int32(target->CountPctFromMaxHealth(21)), target, true, NULL, this);
                        return;
                    // Inoculate Nestlewood Owlkin
                    case 29528:
                        if (target->GetTypeId() != TYPEID_UNIT) // prevent error reports in case ignored player target
                            return;
                        break;
                    // Feed Captured Animal
                    case 29917:
                        triggerSpellId = 29916;
                        break;
                    // Extract Gas
                    case 30427:
                    {
                        // move loot to player inventory and despawn target
                        if (caster && caster->GetTypeId() == TYPEID_PLAYER &&
                                target->GetTypeId() == TYPEID_UNIT &&
                                target->ToCreature()->GetCreatureTemplate()->type == CREATURE_TYPE_GAS_CLOUD)
                        {
                            Player* player = caster->ToPlayer();
                            Creature* creature = target->ToCreature();
                            // missing lootid has been reported on startup - just return
                            if (!creature->GetCreatureTemplate()->SkinLootId)
                                return;

                            player->AutoStoreLoot(creature->GetCreatureTemplate()->SkinLootId, LootTemplates_Skinning, true);

                            creature->DespawnOrUnsummon();
                        }
                        return;
                    }
                    // Quake
                    case 30576:
                        triggerSpellId = 30571;
                        break;
                    // Doom
                    // TODO: effect trigger spell may be independant on spell targets, and executed in spell finish phase
                    // so instakill will be naturally done before trigger spell
                    case 31347:
                    {
                        target->CastSpell(target, 31350, true, NULL, this);
                        target->Kill(target);
                        return;
                    }
                    // Spellcloth
                    case 31373:
                    {
                        // Summon Elemental after create item
                        target->SummonCreature(17870, 0, 0, 0, target->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                        return;
                    }
                    // Flame Quills
                    case 34229:
                    {
                        // cast 24 spells 34269-34289, 34314-34316
                        for (uint32 spell_id = 34269; spell_id != 34290; ++spell_id)
                            target->CastSpell(target, spell_id, true, NULL, this);
                        for (uint32 spell_id = 34314; spell_id != 34317; ++spell_id)
                            target->CastSpell(target, spell_id, true, NULL, this);
                        return;
                    }
                    // Remote Toy
                    case 37027:
                        triggerSpellId = 37029;
                        break;
                    // Eye of Grillok
                    case 38495:
                        triggerSpellId = 38530;
                        break;
                    // Absorb Eye of Grillok (Zezzak's Shard)
                    case 38554:
                    {
                        if (!caster || target->GetTypeId() != TYPEID_UNIT)
                            return;

                        caster->CastSpell(caster, 38495, true, NULL, this);

                        Creature* creatureTarget = target->ToCreature();

                        creatureTarget->DespawnOrUnsummon();
                        return;
                    }
                    // Tear of Azzinoth Summon Channel - it's not really supposed to do anything, and this only prevents the console spam
                    case 39857:
                        triggerSpellId = 39856;
                        break;
                    // Personalized Weather
                    case 46736:
                        triggerSpellId = 46737;
                        break;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                switch (auraId)
                {
                    // Totemic Mastery (Skyshatter Regalia (Shaman Tier 6) - bonus)
                    case 38443:
                    {
                        bool all = true;
                        for (int i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
                        {
                            if (!target->m_SummonSlot[i])
                            {
                                all = false;
                                break;
                            }
                        }

                        if (all)
                            target->CastSpell(target, 38437, true, NULL, this);
                        else
                            target->RemoveAurasDueToSpell(38437);
                        return;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        // Spell exist but require custom code
        switch (auraId)
        {
            case 104287:
            {
                if (!caster || !caster->ToPlayer())
                    return;

                switch (caster->ToPlayer()->GetHighestStat())
                {
                    case STAT_AGILITY:
                        triggerSpellId = 104274;
                        break;
                    case STAT_INTELLECT:
                        triggerSpellId = 104276;
                        break;
                    case STAT_SPIRIT:
                        triggerSpellId = 104279;
                        break;
                }
                break;
            }
            case 86082: // Permafrost, Nezir, Throne of the Four Winds
            case 93231:
            case 93232:
            case 93233:
                if (caster)
                    caster->CastSpell(target, triggerSpellId, true);
                return;
            // Hour of Twilight, Ultraxion, Dragon Soul
            case 106371:
            case 109415:
            case 109416:
            case 109417:
                if (caster)
                    if (Creature* pUltraxion = caster->ToCreature())
                        if (Unit* pTarget = pUltraxion->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0.0f, true))
                            pUltraxion->CastSpell(pTarget, 105925, true);
                        return;
            case 107851: // Focused Assault, Hagara, Dragon Soul
            case 110900:
            case 110899:
            case 110898:
                if (caster)
                    if (Creature* pHagara = caster->ToCreature())
                        if (Unit* pTarget = pHagara->getVictim())
                        {
                            if (!pHagara->GetMap()->IsHeroic())
                                if (!pHagara->isInFront(pTarget) || !pHagara->IsWithinMeleeRange(pTarget))
                                {
                                    GetBase()->Remove();
                                    return;
                                }

                            pHagara->CastSpell(pHagara->getVictim(), 107850, true);
                        }
                return;
            case 105285: // Target, Hagara, Dragon Soul
                if (caster)
                    caster->CastSpell(target, triggerSpellId, true);
                return;
            // Flood, Ancient Water Lord, Dragon Soul
            case 107797:
                if (caster)
                    caster->CastSpell(caster, 107791, true);
                return;
            // Meltdown, Fragment of Rhyolith
            case 98646:
            {
                if (!caster || !caster->ToCreature())
                    return;

                float mod = 1.0f;
                if (triggerSpellId == 101646 || triggerSpellId == 101648)
                    mod = 0.5f;

                int32 bp0 = mod * caster->GetHealth();
                if (Unit* pTarget = caster->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    caster->CastCustomSpell(pTarget, triggerSpellId, &bp0, NULL, NULL, true);
                return;
            }
            // Pursuing Spikes (Anub'arak)
            case 65920:
            case 65922:
            case 65923:
            {
                Unit* permafrostCaster = NULL;
                Aura* permafrostAura = target->GetAura(66193);
                if (!permafrostAura)
                    permafrostAura = target->GetAura(67855);
                if (!permafrostAura)
                    permafrostAura = target->GetAura(67856);
                if (!permafrostAura)
                    permafrostAura = target->GetAura(67857);

                if (permafrostAura)
                    permafrostCaster = permafrostAura->GetCaster();

                if (permafrostCaster)
                {
                    if (Creature* permafrostCasterCreature = permafrostCaster->ToCreature())
                        permafrostCasterCreature->DespawnOrUnsummon(3000);

                    target->CastSpell(target, 66181, false);
                    target->RemoveAllAuras();
                    if (Creature* targetCreature = target->ToCreature())
                        targetCreature->DisappearAndDie();
                }
                break;
            }
            // Mana Tide
            case 16191:
                target->CastCustomSpell(target, triggerSpellId, &m_amount, NULL, NULL, true, NULL, this);
                return;
            // Negative Energy Periodic
            case 46284:
                target->CastCustomSpell(triggerSpellId, SPELLVALUE_MAX_TARGETS, m_tickNumber / 10 + 1, NULL, true, NULL, this);
                return;
            // Poison (Grobbulus)
            case 28158:
            case 54362:
            // Slime Pool (Dreadscale & Acidmaw)
            case 66882:
                target->CastCustomSpell(triggerSpellId, SPELLVALUE_RADIUS_MOD, (int32)((((float)m_tickNumber / 60) * 0.9f + 0.1f) * 10000 * 2 / 3), NULL, true, NULL, this);
                return;
            // Beacon of Light
            case 53563:
            {
                // area aura owner casts the spell
                GetBase()->GetUnitOwner()->CastSpell(target, triggeredSpellInfo, true, 0, this, GetBase()->GetUnitOwner()->GetGUID());
                return;
            }
            // Slime Spray - temporary here until preventing default effect works again
            // added on 9.10.2010
            case 69508:
            {
                if (caster)
                    caster->CastSpell(target, triggerSpellId, true, NULL, nullptr, caster->GetGUID());
                return;
            }
            case 24745: // Summon Templar, Trigger
            case 24747: // Summon Templar Fire, Trigger
            case 24757: // Summon Templar Air, Trigger
            case 24759: // Summon Templar Earth, Trigger
            case 24761: // Summon Templar Water, Trigger
            case 24762: // Summon Duke, Trigger
            case 24766: // Summon Duke Fire, Trigger
            case 24769: // Summon Duke Air, Trigger
            case 24771: // Summon Duke Earth, Trigger
            case 24773: // Summon Duke Water, Trigger
            case 24785: // Summon Royal, Trigger
            case 24787: // Summon Royal Fire, Trigger
            case 24791: // Summon Royal Air, Trigger
            case 24792: // Summon Royal Earth, Trigger
            case 24793: // Summon Royal Water, Trigger
            {
                // All this spells trigger a spell that requires reagents; if the
                // triggered spell is cast as "triggered", reagents are not consumed
                if (caster)
                    caster->CastSpell(target, triggerSpellId, false);
                return;
            }
            case 106768:
                return;
            case 10:  // Blizzard
            case 5740:  // Rain of Fire
            {
                if (caster && target)
                {
                    if (caster != target)
                        return;
                }
                break;
            }
            default:
                break;
        }
    }

    // Reget trigger spell proto
    triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId);

    if (triggeredSpellInfo)
    {
        if (Unit* triggerCaster = triggeredSpellInfo->NeedsToBeTriggeredByCaster() ? caster : target)
        {
            triggerCaster->CastSpell(target, triggeredSpellInfo, true, NULL, this);
        }
    }
    else
    {
        Creature* c = target->ToCreature();
        if (!c || !caster || !sScriptMgr->OnDummyEffect(caster, GetId(), SpellEffIndex(GetEffIndex()), target->ToCreature()) ||
            !c->AI()->sOnDummyEffect(caster, GetId(), SpellEffIndex(GetEffIndex())))
            sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandlePeriodicTriggerSpellAuraTick: Spell %u has non-existent spell %u in EffectTriggered[%d] and is therefor not triggered.", GetId(), triggerSpellId, GetEffIndex());
    }
}

void AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick(Unit* target, Unit* caster) const
{
    uint32 triggerSpellId = GetSpellInfo()->Effects[m_effIndex].TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        if (Unit* triggerCaster = triggeredSpellInfo->NeedsToBeTriggeredByCaster() ? caster : target)
        {
            int32 basepoints0 = GetAmount();

            switch (triggerSpellId)
            {
                // Face Rage, Rageface, Shannox
                case 99948:
                    basepoints0 += 5000 * m_tickNumber; 
                    break;
            }

            // Hack fix for Protectors of the Endless - Defiled Ground
            if (triggerSpellId == 117988)
                triggerCaster->CastCustomSpell(target, triggerSpellId, 0, &basepoints0, 0, true, 0, this);
            else
                triggerCaster->CastCustomSpell(target, triggerSpellId, &basepoints0, 0, 0, true, 0, this);
        }
    }
}

void AuraEffect::HandlePeriodicDamageAurasTick(Unit* target, Unit* caster) const
{
    if (!caster || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    // Consecrate ticks can miss and will not show up in the combat log
    if (GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false) != SPELL_MISS_NONE)
        return;

    // some auras remove at specific health level or more
    if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
    {
        switch (GetSpellInfo()->Id)
        {
            case 124255:
            {
                GetBase()->GetEffect(EFFECT_1)->SetAmount(GetBase()->GetEffect(EFFECT_1)->GetAmount() - GetAmount());

                const static uint32 staggerVisualEffects[3] = { LIGHT_STAGGER_EFFECT, MODERATE_STAGGER_EFFECT, HEAVY_STAGGER_EFFECT };

                for (uint32 spellId : staggerVisualEffects)
                {
                    if (auto aura = target->GetAura(spellId))
                    {
                        if (auto effect_0 = aura->GetEffect(EFFECT_0))
                            effect_0->SetAmount(GetAmount());
                        if (auto effect_1 = aura->GetEffect(EFFECT_1))
                            effect_1->SetAmount(GetBase()->GetEffect(EFFECT_1)->GetAmount());     
                        break;
                    }
                }

                if (GetBase()->GetEffect(EFFECT_1)->GetAmount() <= 0)
                    GetBase()->Remove();

                break;
            }
            case 13812: // Explosive Trap
            {
                // Glyph of Explosive Trap
                if (caster->HasAura(119403))
                    return;
            }
            case 1120:  // Soul Drain
            {
                caster->CombatStart(target, true);
                break;
            }
            // Custom MoP Script
            case 172:   // Corruption
                caster->EnergizeBySpell(caster, 172, 4, POWER_DEMONIC_FURY);
                break;
            case 43093: case 31956: case 38801:  // Grievous Wound
            case 35321: case 38363: case 39215:  // Gushing Wound
                if (target->IsFullHealth())
                {
                    target->RemoveAurasDueToSpell(GetSpellInfo()->Id);
                    return;
                }
                break;
            case 121447: // quick-dry
                if (target->GetPositionZ() > 131.170273)
                {
                    if (target->HasAura(121447))
                    {
                        if (!target->HasAura(122063))
                            target->AddAura(122063, target);
                        else
                        {
                            if (Aura* auracheck = target->GetAura(122063))
                                auracheck->SetStackAmount(auracheck->GetStackAmount() + 1);
                        }
                    }
                }
                break;
            case 38772: // Grievous Wound
            {
                uint32 percent = GetSpellInfo()->Effects[EFFECT_1].CalcValue(caster);
                if (!target->HealthBelowPct(percent))
                {
                    target->RemoveAurasDueToSpell(GetSpellInfo()->Id);
                    return;
                }
                break;
            }
        }
    }
    else
    {
        switch (GetSpellInfo()->Id)
        {
            case 49016:
            {
                if (caster->HasAura(58616))
                    return;
                break;
            }
        }
    }

    uint32 absorb = 0;
    uint32 resist = 0;
    CleanDamage cleanDamage = CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL);

    // ignore non positive values (can be result apply spellmods to aura damage
    uint32 damage = std::max(GetAmount(), 0);

    if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
    {
        if (m_fixed_periodic.HasDamage())
            damage = m_fixed_periodic.GetFixedDamage();
        else
            damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), damage, DOT, GetBase()->GetStackAmount());

        if (GetId() == 980)
            damage = (damage * GetBase()->GetStackAmount()) / 10; // Agony store its damage as if it was full stack

        damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage, DOT, GetBase()->GetStackAmount());

        // Calculate armor mitigation
        if (Unit::IsDamageReducedByArmor(GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), GetEffIndex()))
        {
            uint32 damageReductedArmor = caster->CalcArmorReducedDamage(target, damage, GetSpellInfo());
            cleanDamage.mitigated_damage += damage - damageReductedArmor;
            damage = damageReductedArmor;
        }

        // 77486 - Mastery : Shadowy Recall
        if (GetSpellInfo()->SchoolMask == SPELL_SCHOOL_MASK_SHADOW && caster->HasAura(77486))
        {
            float Mastery = caster->ToPlayer()->GetFloatValue(PLAYER_MASTERY) * 1.8f;

            if (roll_chance_f(Mastery))
            {
                int32 bp = damage;

                switch (GetSpellInfo()->Id)
                {
                    case 589:   // Shadow Word: Pain
                        caster->CastCustomSpell(target, 124464, &bp, NULL, NULL, true);
                        break;
                    case 2944:  // Devouring Plague
                        caster->CastCustomSpell(target, 124467, &bp, NULL, NULL, true);
                        break;
                    case 15407: // Mind Flay
                        caster->CastCustomSpell(target, 124468, &bp, NULL, NULL, true);
                        break;
                    case 34914: // Vampiric Touch
                        caster->CastCustomSpell(target, 124465, &bp, NULL, NULL, true);
                        break;
                    default:
                        break;
                }
            }
        }

        // Nether Tempest and Living Bomb deal 85% of damage if used on player
        if (GetSpellInfo()->Id == 44457 || GetSpellInfo()->Id == 114923)
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                damage *= 0.85f;
        }

        // Hack fix damage of Drain Life with Grimoire of Sacrifice
        if (GetSpellInfo()->Id == 689)
        {
            if (Player* _player = GetCaster()->ToPlayer())
            {
                if (_player->HasAura(108503))
                    damage *= 0.714f;
            }
        }

        // Black Arrow (3674), Explosive Shot (53301) and Serpent String (118253) periodic damage increased by Hunter Mark
        if (GetSpellInfo()->Id == 3674 || GetSpellInfo()->Id == 53301 || GetSpellInfo()->Id == 118253)
        {
            if (target->HasAura(1130, GetCaster()->GetGUID()))
                damage *= 1.05f;
        }

        // Mind flay with Solace and Insanity
        if (GetSpellInfo()->Id == 15407 && GetCaster()->HasSpell(139139))
        {
            if (Player* _player = GetCaster()->ToPlayer())
            {
                if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PRIEST_SHADOW && target->HasAura(2944))
                {
                    if (Aura* devouringPlague = target->GetAura(2944))
                    {
                        uint8 powerUsed = devouringPlague->GetEffect(2)->GetAmount();
                        damage *= (powerUsed * 0.33f + 1);
                    }
                }
            }
        }

        // Chaos Bolt with Grimoire
        if (GetSpellInfo()->Id == 116858)
        {
            if (Player* _player = GetCaster()->ToPlayer())
                if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_WARLOCK_DESTRUCTION && _player->HasAura(108503))
                {
                    SpellInfo const* chaosBolt;
                    int32 chaosBoltDamage;
                    chaosBolt = sSpellMgr->GetSpellInfo(116858);
                    chaosBoltDamage = _player->CalculateSpellDamage(target, chaosBolt, 0);
                    chaosBoltDamage = caster->SpellDamageBonusDone(target, chaosBolt, chaosBoltDamage, SPELL_DIRECT_DAMAGE);
                    chaosBoltDamage = CalculatePct(chaosBoltDamage, 15);
                    chaosBoltDamage /= 3;
                    damage = chaosBoltDamage;
                }
        }

        // Malefic Grasp
        if (GetSpellInfo()->Id == 103103)
        {
            int32 dmgPct =  GetSpellInfo()->Effects[2].BasePoints;
            int32 afflictionDamage;

            caster->SendPlaySpellVisual(25955, target, 20.f);

            // Grimoire of Sacrifice - damage increased 20%
            if (caster->HasAura(108503))
                dmgPct = CalculatePct(dmgPct, 120);

            // Every tick, Malefic Grasp deals instantly 30% of tick-damage for each affliction effects on the target
            // Corruption ...
            if (Aura* corruption = target->GetAura(146739, caster->GetGUID()))
            {
                afflictionDamage = corruption->GetEffect(0)->GetAmount();
                afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                caster->CastCustomSpell(target, 131740, &afflictionDamage, NULL, NULL, true);
            }
            // Unstable Affliction ...
            if (Aura* unstableAffliction = target->GetAura(30108, caster->GetGUID()))
            {
                afflictionDamage = unstableAffliction->GetEffect(0)->GetAmount();
                afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                caster->CastCustomSpell(target, 131736, &afflictionDamage, NULL, NULL, true);
            }
            // Seed of Corruption ...
            if (Aura* seedOfCorruption = target->GetAura(27243, caster->GetGUID()))
            {
                afflictionDamage = seedOfCorruption->GetEffect(0)->GetAmount();
                afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                caster->CastCustomSpell(target, 132566, &afflictionDamage, NULL, NULL, true);
            }
            // Agony ...
            if (Aura* agony = target->GetAura(980, caster->GetGUID()))
            {
                afflictionDamage = agony->GetEffect(0)->GetFixedDamageInfo().GetFixedDamage();
                afflictionDamage = (afflictionDamage * agony->GetStackAmount()) / 10; // Agony store its damage as if it was full stack

                afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                agony->SetScriptData(0, 1); // Prevent recalculation of damage (snapshot)
                agony->ModStackAmount(1);
                agony->SetScriptData(0, 0); // Reenable

                caster->CastCustomSpell(target, 131737, &afflictionDamage, NULL, NULL, true);
            }
        }
        // Soul Drain
        if (GetSpellInfo()->Id == 1120)
        {
            // Energize one soul shard every 2 ticks
            if (m_tickNumber == 2 || m_tickNumber == 4 || m_tickNumber == 6)
                caster->SetPower(POWER_SOUL_SHARDS, caster->GetPower(POWER_SOUL_SHARDS) + 100);

            // if target is below 20% of life ...
            if (target->GetHealthPct() <= 20)
            {
                // ... drain soul deal 100% more damage ...
                damage *= 2;

                int32 dmgPct = GetSpellInfo()->Effects[4].BasePoints;
                int32 afflictionDamage;

                // Grimoire of Sacrifice - damage increased 20%
                if (caster->HasAura(108503))
                    dmgPct = CalculatePct(dmgPct, 120);

                // Every tick, Soul Drain deals instantly 60% of tick-damage for each affliction effects on the target
                // Corruption ...
                if (Aura* corruption = target->GetAura(146739, caster->GetGUID()))
                {
                    afflictionDamage = corruption->GetEffect(0)->GetAmount();
                    afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                    caster->CastCustomSpell(target, 131740, &afflictionDamage, NULL, NULL, true);
                }
                // Unstable Affliction ...
                if (Aura* unstableAffliction = target->GetAura(30108, caster->GetGUID()))
                {
                    afflictionDamage = unstableAffliction->GetEffect(0)->GetAmount();
                    afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                    caster->CastCustomSpell(target, 131736, &afflictionDamage, NULL, NULL, true);
                }
                // Seed of Corruption ...
                if (Aura* seedOfCorruption = target->GetAura(27243, caster->GetGUID()))
                {
                    afflictionDamage = seedOfCorruption->GetEffect(0)->GetAmount();
                    afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                    caster->CastCustomSpell(target, 132566, &afflictionDamage, NULL, NULL, true);
                }
                // Agony ...
                if (Aura* agony = target->GetAura(980, caster->GetGUID()))
                {
                    afflictionDamage = agony->GetEffect(0)->GetFixedDamageInfo().GetFixedDamage();
                    afflictionDamage = (afflictionDamage * agony->GetStackAmount()) / 10; // Agony store its damage as if it was full stack

                    afflictionDamage = CalculatePct(afflictionDamage, dmgPct);

                    agony->SetScriptData(0, 1); // Prevent recalculation of damage (snapshot)
                    agony->ModStackAmount(1);
                    agony->SetScriptData(0, 0); // Reenable

                    caster->CastCustomSpell(target, 131737, &afflictionDamage, NULL, NULL, true);
                }
            }
        }
        // Execution Sentence damage-per-tick calculation
        if (GetSpellInfo()->Id == 114916)
        {
            int32 spellPowerBonus = int32(5.936 * caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY));
            damage = 12989 + spellPowerBonus;

            float PvPPower = (1 + GetCaster()->GetFloatValue(PLAYER_FIELD_PVP_POWER_DAMAGE) / 100);
            // We need to calculate damage with PvPPower
            if (PvPPower > 1.0f)
                damage *= PvPPower;
            // Inquisition
            if (caster->HasAura(84963))
                damage *= 1.3f;
            // Avenging Wrath
            if (caster->HasAura(31884))
                damage *= 1.2f;
            // Berserking (bg buff)
            if (caster->HasAura(23505))
                damage *= 1.3f;
            uint32 totalTick = GetTotalTicks();
            if (m_tickNumber == 1)
                damage = int32(damage * 0.04f); // First:   4.0%
            else if (m_tickNumber == 2)
                damage = int32(damage * 0.044f); // Second:  4.4%
            else if (m_tickNumber == 3)
                damage = int32(damage * 0.05f); // Third:   5.0%
            else if (m_tickNumber == 4)
                damage = int32(damage * 0.055f); // Fourth:  5.5%
            else if (m_tickNumber == 5)
                damage = int32(damage * 0.06f); // Fifth:   6.0%
            else if (m_tickNumber == 6)
                damage = int32(damage * 0.066f); // Sixth:   6.6%
            else if (m_tickNumber == 7)
                damage = int32(damage * 0.073f); // Seventh: 7.3%
            else if (m_tickNumber == 8)
                damage = int32(damage * 0.08f); // Eight:   8.0%
            else if (m_tickNumber == 9)
                damage = int32(damage * 0.088f); // Ninth:   8.8%
            else
                damage = int32(damage * 0.444f); // Final:   44.4%
        }
        if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_GENERIC)
        {
            switch (GetId())
            {
                case 70911: // Unbound Plague
                case 72854: // Unbound Plague
                case 72855: // Unbound Plague
                case 72856: // Unbound Plague
                    damage *= uint32(pow(1.25f, int32(m_tickNumber)));
                    break;
                default:
                    break;
            }
        }
    }
    else
        damage = uint32(target->CountPctFromMaxHealth(damage));

    bool crit = false;

    if (m_fixed_periodic.HasCritChance())
        crit = roll_chance_f(m_fixed_periodic.GetCriticalChance());
    else
        crit = IsPeriodicTickCrit(target, caster);

    if (crit)
        damage = caster->SpellCriticalDamageBonus(m_spellInfo, damage, target);

    // If Doom critical tick, a Wild Imp will appear to fight with the Warlock
    if (m_spellInfo->Id == 603 && crit)
        caster->CastSpell(caster, 104317, true);

    // Item - Warlock T16 2P Bonus - 145072
    // If Unstable Affliction tick is crit
    if (m_spellInfo->Id == 30108 && crit && roll_chance_i(50) && caster->HasAura(145072))
        caster->AddAura(145082, caster);

    // Lynx Rush damage per tick, not in the whole duration
    if (m_spellInfo->Id == 120699)
    {
        if (Pet* hunterPet = caster->ToPet())
        {
            if (Player* hunter = hunterPet->GetOwner())
            {
                int32 hunterAttackPower = hunter->GetTotalAttackPowerValue(RANGED_ATTACK);
                damage = (86 + (hunterAttackPower * 0.038f)) * 5;
            }
        }

        // if already has an aura
        if (Aura* lynxRush = target->GetAura(120699, caster->GetGUID()))
        {
            uint32 amountOfStack = lynxRush->GetStackAmount();
            damage = damage * amountOfStack;
        }

        damage = int32(damage / 5.0f);
    }
    
    if (m_spellInfo->Id == 124255)
        damage = GetAmount();

    int32 dmg = damage;
    if (m_spellInfo->IsAffectedByResilience())
        caster->ApplyResilience(target, &dmg);
    damage = dmg;

    SpellSchoolMask schoolMask = caster->CalcAbsorbResist(target, GetSpellInfo()->GetSchoolMask(), DOT, damage, &absorb, &resist, GetSpellInfo());

    caster->DealDamageMods(target, damage, &absorb);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_DOT;
    if (absorb)
        procEx |= PROC_EX_ABSORB;

    damage = (damage <= absorb+resist) ? 0 : (damage-absorb-resist);
    if (damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

    int32 overkill = damage - target->GetHealth();
    if (overkill < 0)
        overkill = 0;

    SpellPeriodicAuraLogInfo pInfo(this, damage, overkill, absorb, resist, 0.0f, crit);
    target->SendPeriodicAuraLog(&pInfo);

    caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, damage, absorb, BASE_ATTACK, GetSpellInfo(), nullptr, GetBase());

    caster->DealDamage(target, damage, &cleanDamage, DOT, schoolMask, GetSpellInfo(), true);
}

void AuraEffect::HandlePeriodicHealthLeechAuraTick(Unit* target, Unit* caster) const
{
    if (!caster || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    if (GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false) != SPELL_MISS_NONE)
        return;

    uint32 absorb = 0;
    uint32 resist = 0;
    CleanDamage cleanDamage = CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL);

    uint32 damage = std::max(GetAmount(), 0);

    if (m_fixed_periodic.HasDamage())
        damage = m_fixed_periodic.GetFixedDamage();
    else
        damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), damage, DOT, GetBase()->GetStackAmount());
    damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage, DOT, GetBase()->GetStackAmount());

    bool crit = false;
    if (m_fixed_periodic.HasCritChance())
        crit = roll_chance_f(m_fixed_periodic.GetCriticalChance());
    else
        crit = IsPeriodicTickCrit(target, caster);

    if (crit)
        damage = caster->SpellCriticalDamageBonus(m_spellInfo, damage, target);

    // Calculate armor mitigation
    if (Unit::IsDamageReducedByArmor(GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), m_effIndex))
    {
        uint32 damageReductedArmor = caster->CalcArmorReducedDamage(target, damage, GetSpellInfo());
        cleanDamage.mitigated_damage += damage - damageReductedArmor;
        damage = damageReductedArmor;
    }

    int32 dmg = damage;
    caster->ApplyResilience(target, &dmg);
    damage = dmg;

    SpellSchoolMask schoolMask = caster->CalcAbsorbResist(target, GetSpellInfo()->GetSchoolMask(), DOT, damage, &absorb, &resist, m_spellInfo);

    caster->SendSpellNonMeleeDamageLog(target, GetId(), damage, schoolMask, absorb, resist, false, 0, crit);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_DOT;
    damage = (damage <= absorb+resist) ? 0 : (damage-absorb-resist);
    if (damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;
    if (caster->isAlive())
        caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, damage, absorb, BASE_ATTACK, GetSpellInfo());
    int32 new_damage = caster->DealDamage(target, damage, &cleanDamage, DOT, schoolMask, GetSpellInfo(), false);
    if (caster->isAlive())
    {
        float gainMultiplier = GetSpellInfo()->Effects[GetEffIndex()].CalcValueMultiplier(caster);

        uint32 heal = uint32(caster->SpellHealingBonusDone(caster, GetSpellInfo(), uint32(new_damage * gainMultiplier), DOT, GetBase()->GetStackAmount()));
        heal = uint32(caster->SpellHealingBonusTaken(caster, GetSpellInfo(), GetEffIndex(), heal, DOT, GetBase()->GetStackAmount()));

        int32 gain = caster->HealBySpell(caster, GetSpellInfo(), heal);
        caster->getHostileRefManager().threatAssist(caster, gain * 0.5f, GetSpellInfo());
    }
}

void AuraEffect::HandlePeriodicHealthFunnelAuraTick(Unit* target, Unit* caster) const
{
    if (!caster || !caster->isAlive() || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    uint32 damage = std::max(GetAmount(), 0);
    // do not kill health donator
    if (caster->GetHealth() < damage)
        damage = caster->GetHealth() - 1;
    if (!damage)
        return;

    caster->ModifyHealth(-(int32)damage);

    float gainMultiplier = GetSpellInfo()->Effects[GetEffIndex()].CalcValueMultiplier(caster);

    damage = int32(damage * gainMultiplier);

    caster->HealBySpell(target, GetSpellInfo(), damage);
}

void AuraEffect::HandlePeriodicHealAurasTick(Unit* target, Unit* caster) const
{
    if (!caster || !target->isAlive())
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // heal for caster damage (must be alive)
    if (target != caster && GetSpellInfo()->AttributesEx2 & SPELL_ATTR2_HEALTH_FUNNEL && !caster->isAlive())
        return;

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->IsFullHealth())
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    int32 damage = std::max(m_amount, 0);

    // Fix Second Wind only in AURA_STATE_HEALTHLESS_35_PERCENT
    if (m_spellInfo->Id == 16491)
        if (caster->GetHealthPct() > 35.0f)
            return;

    if (GetAuraType() == SPELL_AURA_OBS_MOD_HEALTH)
    {
        // Taken mods
        float TakenTotalMod = 1.0f;

        // Tenacity increase healing % taken
        if (AuraEffect const* Tenacity = target->GetAuraEffect(58549, 0))
            AddPct(TakenTotalMod, Tenacity->GetAmount());

        TakenTotalMod = std::max(TakenTotalMod, 0.0f);

        // Soulburn: Health Funnel
        if (m_spellInfo->Id == 755 && caster->HasAura(74434))
        {
            damage = 14;
            caster->RemoveAura(74434);
        }

        // Mend Pet
        if (m_spellInfo->Id == 136)
            if (caster->HasAura(19573)) // Glyph of Mend Pet
                if (roll_chance_i(50))
                    caster->CastSpell(target, 24406, true); // Dispel

        damage = uint32(target->CountPctFromMaxHealth(damage));
        damage = uint32(damage * TakenTotalMod);

        damage = caster->SpellHealingBonusDone(target, GetSpellInfo(), damage, DOT, GetBase()->GetStackAmount());
        damage = target->SpellHealingBonusTaken(caster, GetSpellInfo(), GetEffIndex(), damage, DOT, GetBase()->GetStackAmount());

        // Spirit Bond
        if (m_spellInfo->Id == 118694)
        {
            if (caster->GetOwner() && caster->GetOwner()->GetTypeId() == TYPEID_PLAYER)
            {
                float PvPPower = caster->GetOwner()->ToPlayer()->GetPvpHealingBonus();
                damage = int32((damage * PvPPower) * PvPPower);
            }            
        }
    }
    else
    {
        if (m_fixed_periodic.HasDamage())
            damage = m_fixed_periodic.GetFixedDamage();
        else
            damage = caster->SpellHealingBonusDone(target, GetSpellInfo(), damage, DOT, GetBase()->GetStackAmount());

        // Wild Growth = amount + (6 - 2*doneTicks) * ticks* amount / 100
        if (m_spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && m_spellInfo->SpellIconID == 2864)
        {
            int32 addition = int32(float(damage * GetTotalTicks()) * ((6-float(2*(GetTickNumber()-1)))/100));

            // Item - Druid T10 Restoration 2P Bonus
            if (AuraEffect* aurEff = caster->GetAuraEffect(70658, 0))
                // divided by 50 instead of 100 because calculated as for every 2 tick
                addition += abs(int32((addition * aurEff->GetAmount()) / 50));

            damage += addition;
        }

        damage = target->SpellHealingBonusTaken(caster, GetSpellInfo(), GetEffIndex(), damage, DOT, GetBase()->GetStackAmount());
    }

    // Stay of Execution heal-per-tick calculation
    if (GetSpellInfo()->Id == 114917 && GetCaster()->GetTypeId() == TYPEID_PLAYER)
    {
        int32 spellPowerBonus = int32(5.936 * caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY));
        damage = 12989 + spellPowerBonus;
        float PvPPower = GetCaster()->ToPlayer()->GetPvpHealingBonus();
        // We need to calculate damage with PvPPower
        if (PvPPower > 1.0f)
            damage *= PvPPower;
        // Seal of Insight
        if (caster->HasAura(20165))
            damage *= 1.05f;
        // Avenging Wrath
        if (caster->HasAura(31884))
            damage *= 1.2f;
        // Holy Insight
        if (caster->HasAura(112859))
            damage *= 1.25f;

        damage = caster->SpellHealingBonusTaken(caster, GetSpellInfo(), GetEffIndex(), damage, HEAL);

        uint32 totalTick = GetTotalTicks();
        if (m_tickNumber == 1)
            damage = int32(damage * 0.04f); // First:   4.0%
        else if (m_tickNumber == 2)
            damage = int32(damage * 0.044f); // Second:  4.4%
        else if (m_tickNumber == 3)
            damage = int32(damage * 0.05f); // Third:   5.0%
        else if (m_tickNumber == 4)
            damage = int32(damage * 0.055f); // Fourth:  5.5%
        else if (m_tickNumber == 5)
            damage = int32(damage * 0.06f); // Fifth:   6.0%
        else if (m_tickNumber == 6)
            damage = int32(damage * 0.066f); // Sixth:   6.6%
        else if (m_tickNumber == 7)
            damage = int32(damage * 0.073f); // Seventh: 7.3%
        else if (m_tickNumber == 8)
            damage = int32(damage * 0.08f); // Eight:   8.0%
        else if (m_tickNumber == 9)
            damage = int32(damage * 0.088f); // Ninth:   8.8%
        else
            damage = int32(damage * 0.444f); // Final:   44.4%

        // Mastery: Illuminated Healing - 76669
        if (caster->ToPlayer()->GetSpecializationId() == SPEC_PALADIN_HOLY)
        {
            int32 Mastery = int32(caster->GetFloatValue(PLAYER_MASTERY) * 1.25f);

            int32 basepoints0 = CalculatePct(damage, Mastery);
            int32 triggered_spell_id = 86273;

            if (AuraEffect* aurShield = target->GetAuraEffect(triggered_spell_id, EFFECT_0, caster->GetGUID()))
                basepoints0 += aurShield->GetAmount();

            int32 maxHealth = caster->CountPctFromMaxHealth(33);
            basepoints0 = std::min(basepoints0, maxHealth);

            if (!target->HasAura(triggered_spell_id, caster->GetGUID()))
                caster->AddAura(triggered_spell_id, target);

            if (AuraEffect* aurShield = target->GetAuraEffect(triggered_spell_id, EFFECT_0, caster->GetGUID()))
                aurShield->SetAmount(basepoints0);
        }
    }

    // Apply PvPPower for Second Wind and Dark Regeneration
    if (GetCaster()->GetTypeId() == TYPEID_PLAYER && (m_spellInfo->Id == 16491 || m_spellInfo->Id == 108359))
    {
        float PvPPower = GetCaster()->ToPlayer()->GetPvpHealingBonus();
        if (PvPPower > 1.0f)
            damage *= PvPPower;
    }

    bool crit = false;
    if (m_fixed_periodic.HasCritChance())
        crit = roll_chance_f(m_fixed_periodic.GetCriticalChance());
    else
        crit = IsPeriodicTickCrit(target, caster);
    
    if (crit)
        damage = caster->SpellCriticalHealingBonus(m_spellInfo, damage, target);

    uint32 absorb = 0;
    uint32 heal = uint32(damage);
    caster->CalcHealAbsorb(target, GetSpellInfo(), heal, absorb);
    int32 gain = caster->DealHeal(target, heal, GetSpellInfo());

    SpellPeriodicAuraLogInfo pInfo(this, heal, heal - gain, absorb, 0, 0.0f, crit);
    target->SendPeriodicAuraLog(&pInfo);

    target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellInfo());

    bool haveCastItem = GetBase()->GetCastItemGUID() != 0;

    // Health Funnel
    // damage caster for heal amount
    if (target != caster && GetSpellInfo()->Id == 755)
    {
        uint32 funnelDamage = caster->CountPctFromMaxHealth(2);
        caster->DealDamage(caster, funnelDamage, NULL, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
    }

    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx = (crit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT) | PROC_EX_INTERNAL_HOT;
    // ignore item heals
    if (!haveCastItem)
        caster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, damage, absorb, BASE_ATTACK, GetSpellInfo());
}

void AuraEffect::HandlePeriodicManaLeechAuraTick(Unit* target, Unit* caster) const
{
    Powers powerType = Powers(GetMiscValue());

    if (!caster || !caster->isAlive() || !target->isAlive() || target->getPowerType() != powerType)
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    if (GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false) != SPELL_MISS_NONE)
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    int32 drainAmount = std::max(m_amount, 0);

    // Special case: draining x% of mana (up to a maximum of 2*x% of the caster's maximum mana)
    // It's mana percent cost spells, m_amount is percent drain from target
    for (auto itr : GetSpellInfo()->SpellPowers)
    {
        if (itr->powerType == POWER_MANA && itr->CostBasePercentage)
        {
            // max value
            int32 maxmana = CalculatePct(caster->GetMaxPower(powerType), drainAmount * 2.0f);
            ApplyPct(drainAmount, target->GetMaxPower(powerType));
            if (drainAmount > maxmana)
                drainAmount = maxmana;
            break;
        }
    }

    int32 drainedAmount = -target->ModifyPower(powerType, -drainAmount);

    float gainMultiplier = GetSpellInfo()->Effects[GetEffIndex()].CalcValueMultiplier(caster);

    SpellPeriodicAuraLogInfo pInfo(this, drainedAmount, 0, 0, 0, gainMultiplier, false);
    target->SendPeriodicAuraLog(&pInfo);

    int32 gainAmount = int32(drainedAmount * gainMultiplier);
    int32 gainedAmount = 0;
    if (gainAmount)
    {
        gainedAmount = caster->ModifyPower(powerType, gainAmount);
        target->AddThreat(caster, float(gainedAmount) * 0.5f, GetSpellInfo()->GetSchoolMask(), GetSpellInfo());
    }

    // Drain Mana
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK
        && m_spellInfo->SpellFamilyFlags[0] & 0x00000010)
    {
        int32 manaFeedVal = 0;
        if (AuraEffect const* aurEff = GetBase()->GetEffect(1))
            manaFeedVal = aurEff->GetAmount();
        // Mana Feed - Drain Mana
        if (manaFeedVal > 0)
        {
            int32 feedAmount = CalculatePct(gainedAmount, manaFeedVal);
            caster->CastCustomSpell(caster, 32554, &feedAmount, NULL, NULL, true, NULL, this);
        }
    }
}

void AuraEffect::HandleObsModPowerAuraTick(Unit* target, Unit* caster) const
{
    Powers powerType;
    if (GetMiscValue() == POWER_ALL)
        powerType = target->getPowerType();
    else
        powerType = Powers(GetMiscValue());

    if (!target->isAlive() || !target->GetMaxPower(powerType))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->GetPower(powerType) == target->GetMaxPower(powerType))
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    uint32 amount = std::max(m_amount, 0) * target->GetMaxPower(powerType) /100;
    SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0, 0.0f, false);
    target->SendPeriodicAuraLog(&pInfo);

    // Hack Fix Glyph of Evocation - 56380
    if (GetBase()->GetId() == 12051 && GetTickNumber() > 1)
    {
        if (caster->GetTypeId() == TYPEID_PLAYER && target->HasAura(56380) && !target->HasAura(114003))
        {
            int32 bp = caster->CountPctFromMaxHealth(20);

            // pvp-power affects on heal
            float PvPPower = caster->ToPlayer()->GetPvpHealingBonus();
            bp *= PvPPower;
                        
            bp = caster->SpellHealingBonusDone(caster, sSpellMgr->GetSpellInfo(12051), bp, HEAL);
            bp = caster->SpellHealingBonusTaken(caster, sSpellMgr->GetSpellInfo(12051), GetEffIndex(), bp, HEAL);
            target->HealBySpell(target, sSpellMgr->GetSpellInfo(12051), bp, false);
        }
    }

    int32 gain = target->ModifyPower(powerType, amount);

    if (caster)
        target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellInfo());
}

void AuraEffect::HandlePeriodicEnergizeAuraTick(Unit* target, Unit* caster) const
{
    Powers powerType = Powers(GetMiscValue());

    if (!target->isAlive() || !target->GetMaxPower(powerType))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->GetPower(powerType) == target->GetMaxPower(powerType))
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    int32 amount = std::max(m_amount, 0);

    SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0, 0.0f, false);
    target->SendPeriodicAuraLog(&pInfo);

    int32 gain = target->ModifyPower(powerType, amount);

    if (caster)
        target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellInfo());
}

void AuraEffect::HandlePeriodicPowerBurnAuraTick(Unit* target, Unit* caster) const
{
    Powers powerType = Powers(GetMiscValue());

    if (!caster || !target->isAlive() || target->getPowerType() != powerType)
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    // ignore negative values (can be result apply spellmods to aura damage
    int32 damage = std::max(m_amount, 0);

    uint32 gain = uint32(-target->ModifyPower(powerType, -damage));

    float dmgMultiplier = GetSpellInfo()->Effects[GetEffIndex()].CalcValueMultiplier(caster);

    SpellInfo const* spellProto = GetSpellInfo();
    // maybe has to be sent different to client, but not by SMSG_PERIODICAURALOG
    SpellNonMeleeDamage damageInfo(caster, target, spellProto->Id, spellProto->SchoolMask);
    // no SpellDamageBonus for burn mana
    caster->CalculateSpellDamageTaken(&damageInfo, int32(gain * dmgMultiplier), spellProto);

    caster->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb);

    caster->SendSpellNonMeleeDamageLog(&damageInfo);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    uint32 procEx       = createProcExtendMask(&damageInfo, SPELL_MISS_NONE) | PROC_EX_INTERNAL_DOT;
    if (damageInfo.damage)
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

    caster->ProcDamageAndSpell(damageInfo.target, procAttacker, procVictim, procEx, damageInfo.damage, damageInfo.absorb, BASE_ATTACK, spellProto);

    caster->DealSpellDamage(&damageInfo, true);
}

void AuraEffect::HandleProcTriggerSpellAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    Unit* triggerCaster = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();

    uint32 triggerSpellId = GetSpellInfo()->Effects[GetEffIndex()].TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        triggerCaster->CastSpell(triggerTarget, triggeredSpellInfo, true, NULL, this);
    }
}

void AuraEffect::HandleProcTriggerSpellWithValueAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    Unit* triggerCaster = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();

    uint32 triggerSpellId = GetSpellInfo()->Effects[m_effIndex].TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        int32 basepoints0 = GetAmount();
        triggerCaster->CastCustomSpell(triggerTarget, triggerSpellId, &basepoints0, NULL, NULL, true, NULL, this);
    }
}

void AuraEffect::HandleProcTriggerDamageAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    Unit* target = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();
    SpellNonMeleeDamage damageInfo(target, triggerTarget, GetId(), GetSpellInfo()->SchoolMask);
    uint32 damage = target->SpellDamageBonusDone(triggerTarget, GetSpellInfo(), GetAmount(), SPELL_DIRECT_DAMAGE);
    damage = triggerTarget->SpellDamageBonusTaken(target, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE);
    target->CalculateSpellDamageTaken(&damageInfo, damage, GetSpellInfo());
    target->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb);
    target->SendSpellNonMeleeDamageLog(&damageInfo);
    target->DealSpellDamage(&damageInfo, true);
}

void AuraEffect::HandleRaidProcFromChargeAuraProc(AuraApplication* aurApp, ProcEventInfo& /*eventInfo*/)
{
    Unit* target = aurApp->GetTarget();

    uint32 triggerSpellId;
    switch (GetId())
    {
        case 57949:            // Shiver
            triggerSpellId = 57952;
            //animationSpellId = 57951; dummy effects for jump spell have unknown use (see also 41637)
            break;
        case 59978:            // Shiver
            triggerSpellId = 59979;
            break;
        case 43593:            // Cold Stare
            triggerSpellId = 43594;
            break;
        default:
            sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandleRaidProcFromChargeAuraProc: received not handled spell: %u", GetId());
            return;
    }

    int32 jumps = GetBase()->GetCharges();

    // current aura expire on proc finish
    GetBase()->SetCharges(0);
    GetBase()->SetUsingCharges(true);

    // next target selection
    if (jumps > 0)
    {
        if (Unit* caster = GetCaster())
        {
            float radius = GetSpellInfo()->Effects[GetEffIndex()].CalcRadius(caster);

            if (Unit* triggerTarget = target->GetNextRandomRaidMemberOrPet(radius))
            {
                target->CastSpell(triggerTarget, GetSpellInfo(), true, NULL, this, GetCasterGUID());
                Aura* aura = triggerTarget->GetAura(GetId(), GetCasterGUID());
                if (aura != nullptr)
                    aura->SetCharges(jumps);
            }
        }
    }

    target->CastSpell(target, triggerSpellId, true, NULL, this, GetCasterGUID());
}


void AuraEffect::HandleRaidProcFromChargeWithValueAuraProc(AuraApplication* aurApp, ProcEventInfo& /*eventInfo*/)
{
    Unit* target = aurApp->GetTarget();

    // Currently only Prayer of Mending
    if (!(GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PRIEST && GetSpellInfo()->SpellFamilyFlags[1] & 0x20))
    {
        sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "AuraEffect::HandleRaidProcFromChargeWithValueAuraProc: received not handled spell: %u", GetId());
        return;
    }
    uint32 triggerSpellId = 33110;

    int32 value = GetAmount();

    int32 jumps = GetBase()->GetCharges();

    // current aura expire on proc finish
    GetBase()->SetCharges(0);
    GetBase()->SetUsingCharges(true);

    // next target selection
    if (jumps > 0)
    {
        if (Unit* caster = GetCaster())
        {
            float radius = GetSpellInfo()->Effects[GetEffIndex()].CalcRadius(caster);

            if (Unit* triggerTarget = target->GetNextRandomRaidMemberOrPet(radius))
            {
                target->CastCustomSpell(triggerTarget, GetId(), &value, NULL, NULL, true, NULL, this, GetCasterGUID());
                Aura* aura = triggerTarget->GetAura(GetId(), GetCasterGUID());
                if (aura != nullptr)
                    aura->SetCharges(jumps);
            }
        }
    }

    target->CastCustomSpell(target, triggerSpellId, &value, NULL, NULL, true, NULL, this, GetCasterGUID());
}

void AuraEffect::HandleAuraForceWeather(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Player* target = aurApp->GetTarget()->ToPlayer();

    if (!target)
        return;

   if (apply)
    {
        WorldPacket data(SMSG_WEATHER, (4 + 4 + 1));
        data << float(1.0f);
        data << uint32(GetMiscValue());
        data.WriteBit(0);
        target->GetSession()->SendPacket(&data);
    }
    else
    {
        // send weather for current zone
        if (Weather* weather = WeatherMgr::FindWeather(target->GetZoneId()))
            weather->SendWeatherUpdateToPlayer(target);
        else
        {
            if (!WeatherMgr::AddWeather(target->GetZoneId()))
            {
                // send fine weather packet to remove old weather
                WeatherMgr::SendFineWeatherUpdateToPlayer(target);
            }
        }
    }
}

void AuraEffect::HandleProgressBar(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();

    if (!target)
        return;

    uint32 entryId = GetMiscValue();

    if (apply)
    {
        UnitPowerBarEntry const* powerBarEntry = sUnitPowerBarStore.LookupEntry(entryId);
        if (powerBarEntry)
        {
            target->SetMaxPower(POWER_ALTERNATE_POWER, powerBarEntry->m_MaxValue);
            target->SetPower(POWER_ALTERNATE_POWER, powerBarEntry->m_DefaultValue);
        }
        else
        {
            target->SetMaxPower(POWER_ALTERNATE_POWER, 0);
            target->SetPower(POWER_ALTERNATE_POWER, 0);
        }
    }
    else
    {
        target->SetMaxPower(POWER_ALTERNATE_POWER, 0);
        target->SetPower(POWER_ALTERNATE_POWER, 0);
        return;
    }

    /*if (apply)
    {
        switch (GetSpellInfo()->Id)
        {
            case 88824: // Sound Bar Atramedes (HandlePeriodicDummyAuraTick for movement add?).
            case 93103: //Cho'gall Corruption
            case 78949: // Onyxia Discharge - target is same as caster.
            case 98229: // Majordomo Heroic Concentration
                target->SetPower(POWER_ALTERNATE_POWER, 0);
                target->SetMaxPower(POWER_ALTERNATE_POWER, 100);
                break;
            case 98226: // Rhyolith Balance - Orientation: - power goes to left, + power goes to right.
                target->SetPower(POWER_ALTERNATE_POWER, 25);
                target->SetMaxPower(POWER_ALTERNATE_POWER, 50);
                break;
            case 101410: // Molten Feather Bar - Alysrazor - 98734 is spell to give 1 power, 97128 needs script and triggers it. 99464 (Molting) is Alys aura.
                target->SetPower(POWER_ALTERNATE_POWER, 0);
                target->SetMaxPower(POWER_ALTERNATE_POWER, 3);
                break;
            case 102668: // Sands of the Hourglass, Murozond, End Time
                target->SetPower(POWER_ALTERNATE_POWER, 5);
                target->SetMaxPower(POWER_ALTERNATE_POWER, 5);
                break;
            case 121447: // Quick-Dry Resin, Wing Leader Ner'onok
                target->SetPower(POWER_ALTERNATE_POWER, 50);
                target->SetMaxPower(POWER_ALTERNATE_POWER, 100);
                break;
            default:
                target->SetPower(POWER_ALTERNATE_POWER, 0);
                target->SetMaxPower(POWER_ALTERNATE_POWER, 100);
                break;

            //Deathwing Corrupted Blood meter 106843 - 100 power
            //Hideous Amalgamation Absorb Blood bar - 109329. starts 25 - max 50 power.
            //Thrall Bar - 100439, Elemental Bonds - Destroy totems to free Thrall. 100 power...Firelands.
            //Thrall Bar - 99961, Elemental Bonds - Destroy Forces.100 power...Deepholm.
            //Thrall Bar - 99642, Elemental Bonds - Destroy Elementals.100 power...Vasjh'ir.
            //Thrall Bar - 99358, Elemental Bonds - Destroy Elementals.100 power...Uldum.
            //Warden Stillwater - 35000 power, 92598.
            //Waves like map treasure, 7 power - 92597.
            //Waves like map treasure, 5 power - 91651.
            //Heroic Will Ultraxion - Needs no stuff. No power, puts you in shadow realm 5 sec.
            //Enter the Dream - Ysera - 106464 - Needs no stuff. No power, decreases damage by 50%.
            //DeathWingBack? (Roll Control?)
        }
    }

    uint32 startPower = 0;
    uint32 maxPower = 0;

    // Unknow max misc : 116

    switch (GetMiscValue())
    {
        case 80:
            maxPower = 3;
            break;
        case 32:
            maxPower = 4;
            break;
        case 89:
            startPower = 4;
            maxPower = 4;
            break;
        case 30:
        case 34:
        case 90:
            maxPower = 5;
            break;
        case 33:
        case 35:
            maxPower = 7;
            break;
        case 88:
            maxPower = 10;
            break;
        case 117:
            maxPower = 25;
            break;
        case 129:
        case 133:
            maxPower = 30;
            break;
        case 29:
            maxPower = 34;
            break;
        case 114:
        case 203:
            maxPower = 40;
            break;
        case 64:
            maxPower = 50;
            break;
        case 84:
            maxPower = 60;
            break;
        case 137:
        case 149:
        case 195:
            maxPower = 90;
            break;
        case 23:
        case 37:
        case 39:
        case 61:
        case 65:
        case 68:
        case 69:
        case 72:
        case 78:
        case 91:
        case 93:
        case 94:
        case 103:
        case 107:
        case 110:
        case 113:
        case 134:
        case 148:
        case 151:
        case 165:
        case 176:
        case 178:
        case 179:
        case 183:
        case 199:
        case 204:
        case 205:
        case 206:
        case 207:
        default:
            maxPower = 100;
            break;
        case 63:
            maxPower = 105;
            break;
        case 87:
            maxPower = 120;
            break;
        case 66:
        case 67:
            maxPower = 180;
            break;
        case 24:
            maxPower = 250;
            break;
        case 26:
            maxPower = 300;
            break;
        case 158:
            maxPower = 700;
            break;
        case 36:
            maxPower = 35000;
            break;
        case 238: // 137607, Conduit Level, Lei Shen, Throne of Thunder (for players)
            startPower = 0;
            maxPower = 100;
            break;
        case 237: // 137581, Conduit Level, Lei Shen, Throne of Thunder
        case 240: // 139271, Conduit Level, Lei Shen, Throne of Thunder
        case 241: // 139272, Conduit Level, Lei Shen, Throne of Thunder
        case 242: // 139273, Conduit Level, Lei Shen, Throne of Thunder
            startPower = 1;
            maxPower = 3;
            break;
    }

    switch (GetMiscValue())
    {
        case 34:
        case 90:
        case 204:
        case 205:
        //case 206:
        case 207:
            startPower = 5;
            break;
        case 103:
            startPower = 10;
            break;
        case 64:
            startPower = 25;
            break;
        case 87:
        case 93:
        case 148:
        case 151:
        case 176:
        case 183:
            startPower = 50;
            break;
        case 178:
            startPower = 100;
            break;
        default:
            startPower = 0;
            break;
    }

    target->SetMaxPower(POWER_ALTERNATE_POWER, maxPower);
    target->SetPower(POWER_ALTERNATE_POWER, startPower);*/
}

void AuraEffect::HandleAreaTrigger(AuraApplication const* p_AurApp, uint8 p_Mode, bool p_Apply) const
{
    if (!(p_Mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* l_Target = p_AurApp->GetTarget();
    if (!l_Target)
        return;

    // AreaTrigger is removed at the end of his own duration
    if (!p_Apply)
        return;

    uint32 l_MiscValue = GetMiscValue();
    Position l_Position;
    l_Target->GetPosition(&l_Position);

    AreaTrigger* l_AreaTrigger = new AreaTrigger;
    if (!l_AreaTrigger->CreateAreaTriggerFromSpell(l_Target->GetMap()->GenerateLowGuid<HighGuid::AreaTrigger>(), l_Target, m_spellInfo, 0, l_Position, l_Position))
        delete l_AreaTrigger;
}

void AuraEffect::HandleAuraStrangulate(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    // Asphyxiate
    if (m_spellInfo->Id == 108194)
    {
        int32 newZ = 5;
        target->SetControlled(apply, UNIT_STATE_STUNNED);

        if (apply)
            target->UpdateHeight(target->GetPositionZ() + newZ);
        else
            target->UpdateHeight(target->GetPositionZ() - newZ);
    }
}

void AuraEffect::HandleModManaRegenByHaste(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateManaRegen();
}

void AuraEffect::HandleChangeSpellVisualEffect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    Player* player = target->ToPlayer();
    if (player == NULL)
        return;

    uint32 spellToReplace = apply ? GetMiscValue() : 0;
    uint32 replacer = apply ? m_spellInfo->Id : 0;

    player->SetDynamicUInt32Value(PLAYER_DYNAMIC_UNK_0, 0, spellToReplace);
    player->SetDynamicUInt32Value(PLAYER_DYNAMIC_UNK_0, 1, replacer);
}

void AuraEffect::DoUniqueStackAura(AuraApplication const * aurApp, uint8 mode, bool apply) const
{
    AuraEffect* first_mod = NULL;
    AuraEffect* second_mod = NULL;
    AuraApplication const *first_app = NULL;
    AuraApplication const *second_app = NULL;
    auto target = aurApp->GetTarget();
    auto visibleAuras = target->GetVisibleAuras();
    for (auto itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        auto _base = itr->second->GetBase();
        if (_base->IsUniqueVisibleAuraBuff())
        {
            for (uint8 i = 0; i < 32; ++i)
            {
                if (!(_base->GetEffectMask() & (1 << i)))
                    continue;

                AuraEffect* effect = _base->GetEffect(i);
                if (effect->GetAuraType() != GetAuraType())
                    continue;

                if (!first_mod)
                {
                    first_mod = effect;
                    first_app = itr->second;
                }
                else if (first_mod->GetAmount() < effect->GetAmount())
                {
                    second_mod = first_mod;
                    first_mod = effect;
                    second_app = first_app;
                    first_app = itr->second;
                }
                else if (!second_mod || second_mod->GetAmount() < effect->GetAmount())
                {
                    second_mod = effect;
                    second_app = itr->second;
                }
            }
        }
    }

    AuraEffect* d_this =  const_cast<AuraEffect*>(this);
    if (!first_mod)
    {
        first_mod = d_this;
        first_app = aurApp;
    }
    else if (first_mod->GetAmount() < GetAmount())
    {
        second_mod = first_mod;
        first_mod = d_this;
        first_app = aurApp;
        second_app = first_app;
    }
    if (first_mod != d_this)
        return;

    if (!second_mod)
    {
        (*this.*AuraEffectHandler[GetAuraType()])(aurApp, mode, apply);
        return;
    }
    (*(apply ? second_mod : first_mod).*AuraEffectHandler[GetAuraType()])(apply ? second_app : first_app, mode, false);
    (*(apply ? first_mod : second_mod).*AuraEffectHandler[GetAuraType()])(apply ? second_app : first_app, mode, true);
}

mod_pair AuraEffect::GetUniqueVisibleAuraBuff(Unit* target, int8 x) 
{
    AuraEffect* first_mod = NULL;
    AuraEffect* second_mod = NULL;
    auto visibleAuras = target->GetVisibleAuras();
    for (auto itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        auto _base = itr->second->GetBase();
        if (_base->IsUniqueVisibleAuraBuff())
        {
            for (uint8 i = 0; i < 32; ++i)
            {
                if (!(_base->GetEffectMask() & (1 << i)))
                    continue;

                AuraEffect* effect = _base->GetEffect(i);
                if (effect->GetAuraType() != GetAuraType() || (x && !(effect->GetMiscValue() & int32(1 << x))))
                    continue;

                if (!first_mod)
                    first_mod = effect;

                else if (first_mod->GetAmount() < effect->GetAmount())
                {
                    second_mod = first_mod;
                    first_mod = effect;
                }
                else if (!second_mod || second_mod->GetAmount() < effect->GetAmount())
                    second_mod = effect;
            }
        }

        AuraEffect* d_this = const_cast<AuraEffect*>(this);
        if (!first_mod)
            first_mod = d_this;

        else if (first_mod->GetAmount() < GetAmount())
        {
            second_mod = first_mod;
            first_mod = d_this;
        }
        if (first_mod != d_this)
        {
            first_mod = NULL;
            second_mod = NULL;
        }

        mod_pair pair = { first_mod, second_mod };
        return pair;
    }

    return mod_pair();
}

bool AuraEffect::IsUniqueStackAuraType()
{
    // BOO code
    pAuraEffectHandler pAEH_value = AuraEffectHandler[GetAuraType()];
    return (pAEH_value == &AuraEffect::HandleAuraModWeaponCritPercent && GetSpellInfo()->Effects[GetEffIndex()].HasRadius()) ||
        pAEH_value == &AuraEffect::HandleAuraModAttackPowerPercent || pAEH_value == &AuraEffect::HandleModPowerRegen ||
        pAEH_value == &AuraEffect::HandleAuraModRangedAttackPowerPercent || pAEH_value == &AuraEffect::HandleModDamagePercentDone ||
        pAEH_value == &AuraEffect::HandleAuraModCritPct || pAEH_value == &AuraEffect::HandleModMeleeSpeedPct || 
        pAEH_value == &AuraEffect::HandleModAttackSpeed || pAEH_value == &AuraEffect::HandleAuraModRangedHaste || 
        pAEH_value == &AuraEffect::HandleAuraModIncreaseEnergy || pAEH_value == &AuraEffect::HandleModMeleeRangedSpeedPct || 
        pAEH_value == &AuraEffect::HandleModTotalPercentStat || pAEH_value == &AuraEffect::HandleAuraModSpellPowerPercent;
}

void AuraEffect::HandleTriggerAtApply(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    if (apply)
        target->CastSpell(target, m_spellInfo->Effects[GetEffIndex()].TriggerSpell, true);
}

void AuraEffect::HandleAuraModPetStatPctFromMax(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target || target->GetTypeId() != TYPEID_PLAYER || !target->ToPlayer()->GetPet())
        return;

    Pet* pet = target->ToPlayer()->GetPet();
    if (GetMiscValueB() > 0 && GetMiscValueB() != pet->GetEntry())
        return;

    switch (GetMiscValue())
    {
        case 1: // max health
            pet->UpdateMaxHealth();
            break;
        case 2: // damage
            break;
        case 13: // armor
            pet->UpdateArmor();
            break;
    }
}

void AuraEffect::HandleAuraModPvPPowerHealing(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    if (!target || !target->ToPlayer())
        return;

    target->ToPlayer()->UpdatePvPPowerPercentage();
}