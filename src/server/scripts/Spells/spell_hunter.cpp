/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/*
 * Scripts for spells with SPELLFAMILY_HUNTER, SPELLFAMILY_PET and SPELLFAMILY_GENERIC spells used by hunter players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_hun_".
 */

#include "ScriptMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum HunterSpells
{
    HUNTER_SPELL_BESTIAL_WRATH                      = 19574,
    HUNTER_PET_SPELL_LAST_STAND_TRIGGERED           = 53479,
    HUNTER_PET_HEART_OF_THE_PHOENIX                 = 55709,
    HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED       = 54114,
    HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF          = 55711,
    HUNTER_PET_SPELL_CARRION_FEEDER_TRIGGERED       = 54045,
    HUNTER_SPELL_POSTHASTE                          = 109215,
    HUNTER_SPELL_POSTHASTE_INCREASE_SPEED           = 118922,
    HUNTER_SPELL_NARROW_ESCAPE                      = 109298,
    HUNTER_SPELL_NARROW_ESCAPE_RETS                 = 128405,
    HUNTER_SPELL_SERPENT_STING                      = 118253,
    HUNTER_SPELL_SERPENT_SPREAD                     = 87935,
    HUNTER_SPELL_CHIMERA_SHOT_HEAL                  = 53353,
    HUNTER_SPELL_RAPID_INTENSITY                    = 131564,
    HUNTER_SPELL_RAPID_FIRE                         = 3045,
    HUNTER_SPELL_STEADY_SHOT_ENERGIZE               = 77443,
    HUNTER_SPELL_COBRA_SHOT_ENERGIZE                = 91954,
    HUNTER_SPELL_KILL_COMMAND                       = 34026,
    HUNTER_SPELL_KILL_COMMAND_TRIGGER               = 83381,
    SPELL_MAGE_TEMPORAL_DISPLACEMENT                = 80354,
    HUNTER_SPELL_INSANITY                           = 95809,
    SPELL_SHAMAN_SATED                              = 57724,
    SPELL_SHAMAN_EXHAUSTED                          = 57723,
    HUNTER_SPELL_POWERSHOT                          = 109259,
    HUNTER_SPELL_IMPROVED_SERPENT_STING_AURA        = 82834,
    HUNTER_SPELL_IMPROVED_SERPENT_STING             = 83077,
    HUNTER_SPELL_GLAIVE_TOSS_DAMAGES                = 121414,
    HUNTER_SPELL_GLAIVE_TOSS                        = 117050,
    HUNTER_SPELL_BINDING_SHOT_AREA                  = 109248,
    HUNTER_SPELL_BINDING_SHOT_LINK                  = 117405,
    HUNTER_SPELL_BINDING_SHOT_STUN                  = 117526,
    HUNTER_SPELL_BINDING_SHOT_IMMUNE                = 117553,
    HUNTER_SPELL_PIERCIG_SHOTS                      = 53238,
    HUNTER_SPELL_PIERCIG_SHOTS_EFFECT               = 63468,
    HUNTER_SPELL_MASTERS_CALL                       = 62305,
    HUNTER_SPELL_MASTERS_CALL_TRIGGERED             = 54216,
    HUNTER_SPELL_COBRA_STRIKES_AURA                 = 53260,
    HUNTER_SPELL_COBRA_STRIKES_STACKS               = 53257,
    HUNTER_SPELL_BEAST_CLEAVE_AURA                  = 115939,
    HUNTER_SPELL_BEAST_CLEAVE_PROC                  = 118455,
    HUNTER_SPELL_BEAST_CLEAVE_DAMAGE                = 118459,
    HUNTER_SPELL_LYNX_RUSH_AURA                     = 120697,
    HUNTER_SPELL_LYNX_CRUSH_DAMAGE                  = 120699,
    HUNTER_SPELL_FRENZY_STACKS                      = 19615,
    HUNTER_SPELL_FOCUS_FIRE_READY                   = 88843,
    HUNTER_SPELL_FOCUS_FIRE_AURA                    = 82692,
    HUNTER_SPELL_A_MURDER_OF_CROWS_SUMMON           = 131900,
    HUNTER_NPC_MURDER_OF_CROWS                      = 61994,
    HUNTER_SPELL_DIRE_BEAST                         = 120679,
    DIRE_BEAST_JADE_FOREST                          = 121118,
    DIRE_BEAST_KALIMDOR                             = 122802,
    DIRE_BEAST_EASTERN_KINGDOMS                     = 122804,
    DIRE_BEAST_OUTLAND                              = 122806,
    DIRE_BEAST_NORTHREND                            = 122807,
    DIRE_BEAST_KRASARANG_WILDS                      = 122809,
    DIRE_BEAST_VALLEY_OF_THE_FOUR_WINDS             = 122811,
    DIRE_BEAST_VALE_OF_THE_ETERNAL_BLOSSOM          = 126213,
    DIRE_BEAST_KUN_LAI_SUMMIT                       = 126214,
    DIRE_BEAST_TOWNLONG_STEPPES                     = 126215,
    DIRE_BEAST_DREAD_WASTES                         = 126216,
    DIRE_BEAST_DUNGEONS                             = 132764,
    DIRE_BEAST_ARENA_OR_BG                          = 138574,
    HUNTER_SPELL_STAMPEDE_DAMAGE_REDUCTION          = 130201,
    HUNTER_SPELL_GLYPH_OF_STAMPEDE                  = 57902,
    HUNTER_SPELL_GLYPH_OF_COLLAPSE                  = 126095,
    HUNTER_SPELL_MARKED_FOR_DIE                     = 132106,
    HUNTER_SPELL_HUNTERS_MARK                       = 1130,
    HUNTER_SPELL_GLYPH_OF_MISDIRECTION              = 56829,
    HUNTER_SPELL_MISDIRECTION                       = 34477,
    HUNTER_SPELL_MISDIRECTION_PROC                  = 35079,
    HUNTER_SPELL_TRACK_BEASTS                       = 1494,
    HUNTER_SPELL_TRACK_DEMONS                       = 19878,
    HUNTER_SPELL_TRACK_DRAGONKIN                    = 19879,
    HUNTER_SPELL_TRACK_ELEMENTALS                   = 19880,
    HUNTER_SPELL_TRACK_GIANTS                       = 19882,
    HUNTER_SPELL_TRACK_HUMANOIDS                    = 19883,
    HUNTER_SPELL_TRACK_UNDEAD                       = 19884,
    HUNTER_SPELL_TRACK_HIDDEN                       = 19885,
    HUNTER_SPELL_VIPER_VENOM                        = 118976,
    HUNTER_SPELL_FETCH                              = 125050,
    HUNTER_SPELL_GLAIVE_TOSS_AURA                   = 117050,
    HUNTER_SPELL_GLAIVE_TOSS_RIGHT                  = 120755,
    HUNTER_SPELL_GLAIVE_TOSS_LEFT                   = 120756,
    HUNTER_SPELL_GLAIVE_TOSS_DAMAGE_AND_SNARE_LEFT  = 120761,
    HUNTER_SPELL_GLAIVE_TOSS_DAMAGE_AND_SNARE_RIGHT = 121414,
    HUNTER_SPELL_ASPECT_OF_THE_BEAST                = 61648,
    HUNTER_SPELL_EXPLOSIVE_SHOT                     = 53301,
    HUNTER_SPELL_GLYPH_OF_LIBERATION                = 132106,
    HUNTER_SPELL_GLYPH_OF_LIBERATION_HEAL           = 115927,
    HUNTER_SPELL_MASTER_MARKSMAN                    = 34487,
    HUNTER_SPELL_MASTER_MARKSMAN_TRIGGERED          = 82925,
    HUNTER_SPELL_FIRE                               = 82926,
    HUNTER_SPELL_STEADY_FOCUS                       = 53224,
    HUNTER_SPELL_STEADY_FOCUS_TRIGGERED             = 53220,
    HUNTER_SPELL_ROAR_OF_SACRIFICE_DAMAGE           = 67481,
    HUNTER_T16_2P_BONUS                             = 144637,
    HUNTER_T16_4P_BONUS                             = 144641,
    HUNTER_T16_4P_EFFECT_AIMED_SHOT                 = 144659,
    HUNTER_SPELL_ICE_TRAP_EFFECT                    = 135299
};

// Glyph of Aspect of the Beast - 125042
class spell_hun_glyph_of_aspect_of_the_beast : public SpellScriptLoader
{
    public:
        spell_hun_glyph_of_aspect_of_the_beast() : SpellScriptLoader("spell_hun_glyph_of_aspect_of_the_beast") { }

        class spell_hun_glyph_of_aspect_of_the_beast_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_glyph_of_aspect_of_the_beast_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(HUNTER_SPELL_ASPECT_OF_THE_BEAST, false);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(HUNTER_SPELL_ASPECT_OF_THE_BEAST))
                        _player->removeSpell(HUNTER_SPELL_ASPECT_OF_THE_BEAST, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_hun_glyph_of_aspect_of_the_beast_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_hun_glyph_of_aspect_of_the_beast_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_glyph_of_aspect_of_the_beast_AuraScript();
        }
};

// Glaive Toss (damage) - 120761 and 121414
class spell_hun_glaive_toss_damage : public SpellScriptLoader
{
    public:
        spell_hun_glaive_toss_damage() : SpellScriptLoader("spell_hun_glaive_toss_damage") { }

        class spell_hun_glaive_toss_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_glaive_toss_damage_SpellScript);

            uint64 mainTargetGUID;

            bool Load()
            {
                mainTargetGUID = 0;
                return true;
            }

            void CorrectTargets(std::list<WorldObject*>& targets)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                targets.clear();

                std::list<Unit*> targetList;
                float radius = 50.0f;

                JadeCore::AnyUnfriendlyUnitInObjectRangeCheck u_check(caster, caster, radius);
                JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, targetList, u_check);
                caster->VisitNearbyObject(radius, searcher);

                for (Unit* target : targetList)
                {
                    if (target->HasAura(HUNTER_SPELL_GLAIVE_TOSS_AURA, caster->GetGUID()))
                    {
                        mainTargetGUID = target->GetGUID();
                        break;
                    }
                }

                if (!mainTargetGUID)
                    return;

                Unit* mainTarget = ObjectAccessor::GetUnit(*caster, mainTargetGUID);
                if (!mainTarget)
                    return;

                targets.push_back(mainTarget);

                for (Unit* target : targetList)
                    if (caster->IsValidAttackTarget(target) && target->IsInBetween(GetCaster(), mainTarget, 5.0f))
                        targets.push_back(target);
            }

            void OnDamage()
            {
                if (!mainTargetGUID)
                    return;

                Unit* target = ObjectAccessor::GetUnit(*GetCaster(), mainTargetGUID);
                if (!target)
                    return;

                if (GetHitUnit())
                    if (GetHitUnit() == target)
                        SetHitDamage(GetHitDamage() * 4);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_glaive_toss_damage_SpellScript::CorrectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_glaive_toss_damage_SpellScript::CorrectTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
                OnHit += SpellHitFn(spell_hun_glaive_toss_damage_SpellScript::OnDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_glaive_toss_damage_SpellScript();
        }
};

// Glaive Toss (Missile data) - 120755 and 120756
class spell_hun_glaive_toss_missile : public SpellScriptLoader
{
    public:
        spell_hun_glaive_toss_missile() : SpellScriptLoader("spell_hun_glaive_toss_missile") { }

        class spell_hun_glaive_toss_missile_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_glaive_toss_missile_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (GetCaster() == GetOriginalCaster())
                        GetCaster()->AddAura(HUNTER_SPELL_GLAIVE_TOSS_AURA, target);

                if (GetSpellInfo()->Id == HUNTER_SPELL_GLAIVE_TOSS_RIGHT)
                {
                    if (Player* plr = GetCaster()->ToPlayer())
                        plr->CastSpell(plr, HUNTER_SPELL_GLAIVE_TOSS_DAMAGE_AND_SNARE_RIGHT, true);
                    else if (GetOriginalCaster())
                    {
                        if (Player* caster = GetOriginalCaster()->ToPlayer())
                            caster->CastSpell(caster, HUNTER_SPELL_GLAIVE_TOSS_DAMAGE_AND_SNARE_RIGHT, true);
                    }
                }
                else
                {
                    if (Player* plr = GetCaster()->ToPlayer())
                        plr->CastSpell(plr, HUNTER_SPELL_GLAIVE_TOSS_DAMAGE_AND_SNARE_LEFT, true);
                    else if (GetOriginalCaster())
                    {
                        if (Player* caster = GetOriginalCaster()->ToPlayer())
                            caster->CastSpell(caster, HUNTER_SPELL_GLAIVE_TOSS_DAMAGE_AND_SNARE_LEFT, true);
                    }
                }
            }

            void HandleOnHit()
            {
                if (GetSpellInfo()->Id == HUNTER_SPELL_GLAIVE_TOSS_RIGHT)
                {
                    if (Unit* caster = GetCaster())
                        if (Unit* target = GetHitUnit())
                            if (caster == GetOriginalCaster())
                                target->CastSpell(caster, HUNTER_SPELL_GLAIVE_TOSS_LEFT, true, NULL, nullptr, caster->GetGUID());
                }
                else
                {
                    if (Unit* caster = GetCaster())
                        if (Unit* target = GetHitUnit())
                            if (caster == GetOriginalCaster())
                                target->CastSpell(caster, HUNTER_SPELL_GLAIVE_TOSS_RIGHT, true, NULL, nullptr, caster->GetGUID());
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_hun_glaive_toss_missile_SpellScript::HandleAfterCast);
                OnHit += SpellHitFn(spell_hun_glaive_toss_missile_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_glaive_toss_missile_SpellScript();
        }
};

// Glyph of Fetch - 126746
class spell_hun_glyph_of_fetch : public SpellScriptLoader
{
    public:
        spell_hun_glyph_of_fetch() : SpellScriptLoader("spell_hun_glyph_of_fetch") { }

        class spell_hun_glyph_of_fetch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_glyph_of_fetch_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(HUNTER_SPELL_FETCH, false);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(HUNTER_SPELL_FETCH))
                        _player->removeSpell(HUNTER_SPELL_FETCH, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_hun_glyph_of_fetch_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_hun_glyph_of_fetch_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_glyph_of_fetch_AuraScript();
        }
};

// Tracking - 118424
class spell_hun_tracking : public SpellScriptLoader
{
    public:
        spell_hun_tracking() : SpellScriptLoader("spell_hun_tracking") { }

        class spell_hun_tracking_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_tracking_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    _player->learnSpell(HUNTER_SPELL_TRACK_BEASTS,      false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_DEMONS,      false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_DRAGONKIN,   false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_ELEMENTALS,  false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_GIANTS,      false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_HUMANOIDS,   false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_UNDEAD,      false);
                    _player->learnSpell(HUNTER_SPELL_TRACK_HIDDEN,      false);
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_hun_tracking_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_tracking_AuraScript();
        }
};

// Dash - 113073
class spell_hun_dash : public SpellScriptLoader
{
    public:
        spell_hun_dash() : SpellScriptLoader("spell_hun_dash") { }

        class spell_hun_dash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_dash_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_dash_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_dash_SpellScript();
        }
};

// Dire Beast - 120679
class spell_hun_dire_beast : public SpellScriptLoader
{
    public:
        spell_hun_dire_beast() : SpellScriptLoader("spell_hun_dire_beast") { }

        class spell_hun_dire_beast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_dire_beast_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // Summon's skin is different function of Map or Zone ID
                        switch (_player->GetZoneId())
                        {
                            case 5785: // The Jade Forest
                                _player->CastSpell(target, DIRE_BEAST_JADE_FOREST, true);
                                break;
                            case 5805: // Valley of the Four Winds
                                _player->CastSpell(target, DIRE_BEAST_VALLEY_OF_THE_FOUR_WINDS, true);
                                break;
                            case 5840: // Vale of Eternal Blossoms
                                _player->CastSpell(target, DIRE_BEAST_VALE_OF_THE_ETERNAL_BLOSSOM, true);
                                break;
                            case 5841: // Kun-Lai Summit
                                _player->CastSpell(target, DIRE_BEAST_KUN_LAI_SUMMIT, true);
                                break;
                            case 5842: // Townlong Steppes
                                _player->CastSpell(target, DIRE_BEAST_TOWNLONG_STEPPES, true);
                                break;
                            case 6134: // Krasarang Wilds
                                _player->CastSpell(target, DIRE_BEAST_KRASARANG_WILDS, true);
                                break;
                            case 6138: // Dread Wastes
                                _player->CastSpell(target, DIRE_BEAST_DREAD_WASTES, true);
                                break;
                            default:
                            {
                                switch (_player->GetMapId())
                                {
                                    case 0: // Eastern Kingdoms
                                        _player->CastSpell(target, DIRE_BEAST_EASTERN_KINGDOMS, true);
                                        break;
                                    case 1: // Kalimdor
                                        _player->CastSpell(target, DIRE_BEAST_KALIMDOR, true);
                                        break;
                                    case 530: // Outland
                                        _player->CastSpell(target, DIRE_BEAST_OUTLAND, true);
                                        break;
                                    case 571: // Northrend
                                    case 646: // Deepholm - temporary use northrend spell
                                        _player->CastSpell(target, DIRE_BEAST_NORTHREND, true);
                                        break;
                                    default:
                                        if (_player->GetMap()->IsDungeon())
                                            _player->CastSpell(target, DIRE_BEAST_DUNGEONS, true);
                                        else if (_player->GetMap()->IsBattlegroundOrArena())
                                            _player->CastSpell(target, DIRE_BEAST_ARENA_OR_BG, true);
                                        break;
                                }
                                break;
                            }
                        }
                    }
                }
            }

            void Register()
            {
               OnHit += SpellHitFn(spell_hun_dire_beast_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_dire_beast_SpellScript();
        }
};

// A Murder of Crows - 131894
class spell_hun_a_murder_of_crows : public SpellScriptLoader
{
    public:
        spell_hun_a_murder_of_crows() : SpellScriptLoader("spell_hun_a_murder_of_crows") { }

        class spell_hun_a_murder_of_crows_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_a_murder_of_crows_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* target = GetTarget())
                {
                    if (!GetCaster())
                        return;

                    if (aurEff->GetTickNumber() > 30)
                        return;

                    if (Player* _player = GetCaster()->ToPlayer())
                    {
                        // if target has less then 20% hp - reduce spell cooldown for 1 min
                        if (aurEff->GetTickNumber() == 1 && target->CountPctFromMaxHealth(20) > target->GetHealth())
                            _player->ReduceSpellCooldown(131894, 60000);

                        _player->CastSpell(target, HUNTER_SPELL_A_MURDER_OF_CROWS_SUMMON, true);

                        std::list<Creature*> tempList;

                        _player->GetCreatureListWithEntryInGrid(tempList, HUNTER_NPC_MURDER_OF_CROWS, 100.0f);

                        // Remove other players Crows
                        for (std::list<Creature*>::iterator i = tempList.begin(); i != tempList.end();)
                        {
                            Unit* owner = (*i)->GetOwner();
                            if (owner && owner == _player && (*i)->isSummon())
                            {
                                ++i;
                                continue;
                            }

                            i = tempList.erase(i);
                        }

                        for (auto itr : tempList)
                            itr->AI()->AttackStart(target);
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_a_murder_of_crows_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_a_murder_of_crows_AuraScript();
        }
};

// Focus Fire - 82692
class spell_hun_focus_fire : public SpellScriptLoader
{
    public:
        spell_hun_focus_fire() : SpellScriptLoader("spell_hun_focus_fire") { }

        class spell_hun_focus_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_focus_fire_SpellScript);

            SpellCastResult CheckFrenzy()
            {
                if (!GetCaster()->HasAura(HUNTER_SPELL_FRENZY_STACKS))
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Aura* focusFire = _player->GetAura(HUNTER_SPELL_FOCUS_FIRE_AURA))
                    {
                        if (Aura* frenzy = _player->GetAura(HUNTER_SPELL_FRENZY_STACKS))
                        {
                            if (Pet* pet = _player->GetPet())
                            {
                                int32 stackAmount = frenzy->GetStackAmount();

                                focusFire->GetEffect(0)->ChangeAmount(focusFire->GetEffect(0)->GetAmount() * stackAmount);

                                if (Aura* petFrenzy = pet->GetAura(HUNTER_SPELL_FRENZY_STACKS))
                                {
                                    pet->RemoveAura(HUNTER_SPELL_FRENZY_STACKS);
                                    pet->EnergizeBySpell(pet, GetSpellInfo()->Id, petFrenzy->GetStackAmount()*6, POWER_FOCUS);
                                }

                                _player->RemoveAura(HUNTER_SPELL_FRENZY_STACKS);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_focus_fire_SpellScript::CheckFrenzy);
                OnHit += SpellHitFn(spell_hun_focus_fire_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_focus_fire_SpellScript();
        }
};

// Frenzy - 19615
class spell_hun_frenzy : public SpellScriptLoader
{
    public:
        spell_hun_frenzy() : SpellScriptLoader("spell_hun_frenzy") { }

        class spell_hun_frenzy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_frenzy_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetOwner())
                        if (Aura* frenzy = caster->GetAura(HUNTER_SPELL_FRENZY_STACKS))
                            if (frenzy->GetStackAmount() >= 5)
                                caster->GetOwner()->CastSpell(caster->GetOwner(), HUNTER_SPELL_FOCUS_FIRE_READY, true);
            }

            void Register()
            {
               OnHit += SpellHitFn(spell_hun_frenzy_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_frenzy_SpellScript();
        }

        class spell_hun_frenzy_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_frenzy_AuraScript);

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (GetTarget()->GetOwner())
                    if (GetTarget()->GetOwner()->HasAura(HUNTER_SPELL_FOCUS_FIRE_READY))
                        GetTarget()->GetOwner()->RemoveAura(HUNTER_SPELL_FOCUS_FIRE_READY);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectApplyFn(spell_hun_frenzy_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_ATTACKSPEED_2, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_frenzy_AuraScript();
        }
};

// Lynx Rush - 120697
class spell_hun_lynx_rush : public SpellScriptLoader
{
    public:
        spell_hun_lynx_rush() : SpellScriptLoader("spell_hun_lynx_rush") { }

        class spell_hun_lynx_rush_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_lynx_rush_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                std::list<Unit*> tempList;
                std::list<Unit*> targetList;
                Unit* unitTarget = NULL;

                GetTarget()->GetAttackableUnitListInRange(tempList, 10.0f);

                for (auto itr : tempList)
                {
                    if (itr->GetGUID() == GetTarget()->GetGUID())
                        continue;

                    if (GetTarget()->GetOwner() && GetTarget()->GetOwner()->GetGUID() == itr->GetGUID())
                        continue;

                    if (!GetTarget()->IsValidAttackTarget(itr))
                        continue;

                    targetList.push_back(itr);
                }

                tempList.clear();

                if (targetList.empty())
                    return;

                JadeCore::Containers::RandomResizeList(targetList, 1);

                for (auto itr : targetList)
                {
                    unitTarget = itr;
                    break;
                }

                if (!unitTarget)
                    return;

                float angle = unitTarget->GetRelativeAngle(GetTarget());
                Position pos;

                unitTarget->GetContactPoint(GetTarget(), pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                unitTarget->GetFirstCollisionPosition(pos, unitTarget->GetObjectSize(), angle);
                GetTarget()->GetMotionMaster()->MoveCharge(pos.m_positionX, pos.m_positionY, pos.m_positionZ + unitTarget->GetObjectSize());

                GetTarget()->CastSpell(unitTarget, HUNTER_SPELL_LYNX_CRUSH_DAMAGE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_lynx_rush_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_lynx_rush_AuraScript();
        }

        class spell_hun_lynx_rush_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_lynx_rush_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (GetHitUnit())
                    {
                        if (Pet* pet = _player->GetPet())
                        {
                            if (pet->GetGUID() == GetHitUnit()->GetGUID())
                            {
                                std::list<Unit*> tempList;
                                std::list<Unit*> targetList;
                                Unit* unitTarget = NULL;

                                pet->GetAttackableUnitListInRange(tempList, 10.0f);

                                for (auto itr : tempList)
                                {
                                    if (itr->GetGUID() == pet->GetGUID())
                                        continue;

                                    if (_player->GetGUID() == itr->GetGUID())
                                        continue;

                                    if (!pet->IsValidAttackTarget(itr))
                                        continue;

                                    targetList.push_back(itr);
                                }

                                tempList.clear();

                                if (targetList.empty())
                                    return;

                                JadeCore::Containers::RandomResizeList(targetList, 1);

                                for (auto itr : targetList)
                                {
                                    unitTarget = itr;
                                    break;
                                }

                                if (!unitTarget)
                                    return;

                                float angle = unitTarget->GetRelativeAngle(pet);
                                Position pos;

                                unitTarget->GetContactPoint(pet, pos.m_positionX, pos.m_positionY, pos.m_positionZ);
                                unitTarget->GetFirstCollisionPosition(pos, unitTarget->GetObjectSize(), angle);
                                pet->GetMotionMaster()->MoveCharge(pos.m_positionX, pos.m_positionY, pos.m_positionZ + unitTarget->GetObjectSize());

                                pet->CastSpell(unitTarget, HUNTER_SPELL_LYNX_CRUSH_DAMAGE, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_lynx_rush_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_lynx_rush_SpellScript();
        }
};

// Beast Cleave - 118455
class spell_hun_beast_cleave_proc : public SpellScriptLoader
{
    public:
        spell_hun_beast_cleave_proc() : SpellScriptLoader("spell_hun_beast_cleave_proc") { }

        class spell_hun_beast_cleave_proc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_beast_cleave_proc_AuraScript);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetTarget()->GetGUID())
                    return;

                if (eventInfo.GetDamageInfo()->GetSpellInfo() && eventInfo.GetDamageInfo()->GetSpellInfo()->Id == 118459)
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (GetTarget()->HasAura(aurEff->GetSpellInfo()->Id, _player->GetGUID()))
                    {
                        int32 bp = int32(eventInfo.GetDamageInfo()->GetDamage() * 0.75f);

                        GetTarget()->CastCustomSpell(GetTarget(), HUNTER_SPELL_BEAST_CLEAVE_DAMAGE, &bp, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_hun_beast_cleave_proc_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_beast_cleave_proc_AuraScript();
        }
};

// Called by Multi Shot - 2643
// Beast Cleave - 115939
class spell_hun_beast_cleave : public SpellScriptLoader
{
    public:
        spell_hun_beast_cleave() : SpellScriptLoader("spell_hun_beast_cleave") { }

        class spell_hun_beast_cleave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_beast_cleave_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(HUNTER_SPELL_BEAST_CLEAVE_AURA))
                        if (Pet* pet = _player->GetPet())
                            _player->CastSpell(pet, HUNTER_SPELL_BEAST_CLEAVE_PROC, true);
            }

            void Register()
            {
               AfterCast += SpellCastFn(spell_hun_beast_cleave_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_beast_cleave_SpellScript();
        }
};

// Called by Arcane Shot - 3044
// Cobra Strikes - 53260
class spell_hun_cobra_strikes : public SpellScriptLoader
{
    public:
        spell_hun_cobra_strikes() : SpellScriptLoader("spell_hun_cobra_strikes") { }

        class spell_hun_cobra_strikes_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_cobra_strikes_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (!target->HasAura(HUNTER_SPELL_HUNTERS_MARK))
                            _player->CastSpell(target, HUNTER_SPELL_HUNTERS_MARK, true);
                    }
                }
            }

            void Register()
            {
               OnHit += SpellHitFn(spell_hun_cobra_strikes_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_cobra_strikes_SpellScript();
        }
};

// Barrage damage - 120361
class spell_hun_barrage : public SpellScriptLoader
{
    public:
        spell_hun_barrage() : SpellScriptLoader("spell_hun_barrage") { }

        class spell_hun_barrage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_barrage_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (!target->HasAura(120360))
                            SetHitDamage(GetHitDamage() / 2);
            }

            void Register()
            {
               OnHit += SpellHitFn(spell_hun_barrage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_barrage_SpellScript();
        }
};

// Binding Shot - 117405
class spell_hun_binding_shot : public SpellScriptLoader
{
    public:
        spell_hun_binding_shot() : SpellScriptLoader("spell_hun_binding_shot") { }

        class spell_hun_binding_shot_zone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_binding_shot_zone_AuraScript);

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    DynamicObject* dynObj = caster->GetDynObject(HUNTER_SPELL_BINDING_SHOT_AREA);

                    if (!dynObj)
                        return;

                    std::list<Unit*> bindedList;

                    CellCoord p(JadeCore::ComputeCellCoord(dynObj->GetPositionX(), dynObj->GetPositionY()));
                    Cell cell(p);
                    cell.SetNoCreate();

                    JadeCore::AnyUnitInObjectRangeCheck u_check(dynObj, 15.0f);
                    JadeCore::UnitListSearcher<JadeCore::AnyUnitInObjectRangeCheck> searcher(dynObj, bindedList, u_check);

                    TypeContainerVisitor<JadeCore::UnitListSearcher<JadeCore::AnyUnitInObjectRangeCheck>, WorldTypeMapContainer > world_unit_searcher(searcher);
                    TypeContainerVisitor<JadeCore::UnitListSearcher<JadeCore::AnyUnitInObjectRangeCheck>, GridTypeMapContainer >  grid_unit_searcher(searcher);

                    cell.Visit(p, world_unit_searcher, *dynObj->GetMap(), *dynObj, 15.0f);
                    cell.Visit(p, grid_unit_searcher, *dynObj->GetMap(), *dynObj, 15.0f);

                    bindedList.remove_if(JadeCore::UnitAuraCheck(false, GetSpellInfo()->Id, caster->GetGUID()));

                    for (auto itr : bindedList)
                    {
                        Unit* target = itr->ToUnit();
                        if (!target)
                            continue;

                        if (target->GetDistance(dynObj) > 5.0f)
                        {
                            if (!target->HasAura(HUNTER_SPELL_BINDING_SHOT_IMMUNE))
                            {
                                target->CastSpell(target, HUNTER_SPELL_BINDING_SHOT_STUN, true);
                                target->CastSpell(target, HUNTER_SPELL_BINDING_SHOT_IMMUNE, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_hun_binding_shot_zone_AuraScript::OnUpdate, EFFECT_1, SPELL_AURA_MOD_DAMAGE_FROM_CASTER);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_binding_shot_zone_AuraScript();
        }
};

// Binding Shot - 109248
class spell_hun_binding_shot_zone : public SpellScriptLoader
{
    public:
        spell_hun_binding_shot_zone() : SpellScriptLoader("spell_hun_binding_shot_zone") { }

        class spell_hun_binding_shot_zone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_binding_shot_zone_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    if (DynamicObject* dynObj = caster->GetDynObject(HUNTER_SPELL_BINDING_SHOT_AREA))
                        caster->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), HUNTER_SPELL_BINDING_SHOT_LINK, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_binding_shot_zone_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_binding_shot_zone_AuraScript();
        }
};

// Called by Serpent Sting - 118253
// Improved Serpent Sting - 82834
class spell_hun_improved_serpent_sting : public SpellScriptLoader
{
    public:
        spell_hun_improved_serpent_sting() : SpellScriptLoader("spell_hun_improved_serpent_sting") { }

        class spell_hun_improved_serpent_sting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_improved_serpent_sting_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(HUNTER_SPELL_IMPROVED_SERPENT_STING_AURA))
                        {
                            if (Aura* serpentSting = target->GetAura(HUNTER_SPELL_SERPENT_STING, _player->GetGUID()))
                            {
                                if (serpentSting->GetEffect(0))
                                {
                                    int32 bp = target->SpellDamageBonusTaken(_player, GetSpellInfo(), serpentSting->GetEffect(0)->GetAmount(), DOT);
                                    bp *= serpentSting->GetMaxDuration() / serpentSting->GetEffect(0)->GetAmplitude();
                                    bp = CalculatePct(bp, 15);

                                    // Hunter's Mark
                                    if (target->HasAura(1130, _player->GetGUID()))
                                        bp = int32(bp * 1.05f);

                                    _player->CastCustomSpell(target, HUNTER_SPELL_IMPROVED_SERPENT_STING, &bp, NULL, NULL, true);
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_improved_serpent_sting_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_improved_serpent_sting_SpellScript();
        }
};

// Powershot - 109259
class spell_hun_powershot : public SpellScriptLoader
{
    public:
        spell_hun_powershot() : SpellScriptLoader("spell_hun_powershot") { }

        class spell_hun_powershot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_powershot_SpellScript);

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> tempUnitMap;
                        _player->GetAttackableUnitListInRange(tempUnitMap, _player->GetDistance(target));

                        for (auto itr : tempUnitMap)
                        {
                            if (!itr->IsValidAttackTarget(_player))
                                continue;

                            if (itr->GetGUID() == _player->GetGUID())
                                continue;

                            if (!itr->IsInBetween(_player, target, 1.0f))
                                continue;

                            /*SpellNonMeleeDamage damageInfo(_player, itr, GetSpellInfo()->Id, GetSpellInfo()->SchoolMask);
                            damageInfo.damage = int32(GetHitDamage() / 2);
                            _player->SendSpellNonMeleeDamageLog(&damageInfo);
                            _player->DealSpellDamage(&damageInfo, true);*/

                            if (Creature* creatureTarget = itr->ToCreature())
                                if (creatureTarget->isWorldBoss() || creatureTarget->IsDungeonBoss())
                                    continue;

                            if (itr->GetTypeId() == TYPEID_PLAYER)
                                if (itr->ToPlayer()->GetKnockBackTime())
                                    continue;

                            // Instantly interrupt non melee spells being casted
                            if (itr->IsNonMeleeSpellCasted(true))
                                itr->InterruptNonMeleeSpells(true);

                            float ratio = 0.1f;
                            float speedxy = float(GetSpellInfo()->Effects[EFFECT_1].MiscValue) * ratio;
                            float speedz = float(GetSpellInfo()->Effects[EFFECT_1].BasePoints) * ratio;
                            if (speedxy < 0.1f && speedz < 0.1f)
                                return;

                            float x, y;
                            _player->GetPosition(x, y);

                            itr->KnockbackFrom(x, y, speedxy, speedz);

                            if (itr->GetTypeId() == TYPEID_PLAYER)
                                itr->ToPlayer()->SetKnockBackTime(getMSTime());
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_hun_powershot_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_powershot_SpellScript();
        }
};

// Ferveur - 82726
class spell_hun_ferveur : public SpellScriptLoader
{
    public:
        spell_hun_ferveur() : SpellScriptLoader("spell_hun_ferveur") { }

        class spell_hun_ferveur_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_ferveur_SpellScript);

            void HandleAfterHit()
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Pet* pet = _player->GetPet())
                    {
                        _player->CastSpell(pet, 82728, true);
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_hun_ferveur_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_ferveur_SpellScript();
        }
};

// Feign Death - 5384
class spell_hun_feign_death : public SpellScriptLoader
{
    public:
        spell_hun_feign_death() : SpellScriptLoader("spell_hun_feign_death") { }

        class spell_hun_feign_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_feign_death_AuraScript);

            int32 focus;

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                focus = GetTarget()->GetPower(POWER_FOCUS);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (focus)
                    GetTarget()->SetPower(POWER_FOCUS, focus);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_hun_feign_death_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_FEIGN_DEATH, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_feign_death_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_FEIGN_DEATH, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_feign_death_AuraScript();
        }
};

// Called by Multi Shot - 2643
// Serpent Spread - 87935
class spell_hun_serpent_spread : public SpellScriptLoader
{
    public:
        spell_hun_serpent_spread() : SpellScriptLoader("spell_hun_serpent_spread") { }

        class spell_hun_serpent_spread_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_serpent_spread_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(HUNTER_SPELL_SERPENT_SPREAD))
                            _player->CastSpell(target, HUNTER_SPELL_SERPENT_STING, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_serpent_spread_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_serpent_spread_SpellScript();
        }
};

// Ancient Hysteria - 90355
class spell_hun_ancient_hysteria : public SpellScriptLoader
{
    public:
        spell_hun_ancient_hysteria() : SpellScriptLoader("spell_hun_ancient_hysteria") { }

        class spell_hun_ancient_hysteria_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_ancient_hysteria_SpellScript);

            SpellCastResult CheckMap()
            {
                if (GetCaster())
                {
                    if (GetCaster()->GetMap() && GetCaster()->GetMap()->IsBattleArena())
                        return SPELL_FAILED_DONT_REPORT;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(JadeCore::UnitAuraCheck(true, HUNTER_SPELL_INSANITY));
                targets.remove_if(JadeCore::UnitAuraCheck(true, SPELL_SHAMAN_EXHAUSTED));
                targets.remove_if(JadeCore::UnitAuraCheck(true, SPELL_SHAMAN_SATED));
                targets.remove_if(JadeCore::UnitAuraCheck(true, SPELL_MAGE_TEMPORAL_DISPLACEMENT));
            }

            void ApplyDebuff()
            {
                if (Unit* target = GetHitUnit())
                {
                    /// We must add a check here too, players can use a bug allowing them to receive twice a buff like this one
                    /// If they cast the spells exactly at the same time
                    if (target->HasAura(SPELL_SHAMAN_EXHAUSTED) || target->HasAura(HUNTER_SPELL_INSANITY) ||
                        target->HasAura(SPELL_MAGE_TEMPORAL_DISPLACEMENT) || target->HasAura(SPELL_SHAMAN_SATED))
                    {
                        target->RemoveAura(GetSpellInfo()->Id);
                        return;
                    }

                    target->CastSpell(target, HUNTER_SPELL_INSANITY, true);
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_ancient_hysteria_SpellScript::CheckMap);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_ancient_hysteria_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_ancient_hysteria_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_ancient_hysteria_SpellScript::RemoveInvalidTargets, EFFECT_2, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_hun_ancient_hysteria_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_ancient_hysteria_SpellScript();
        }
};

// Kill Command - 34026
class spell_hun_kill_command : public SpellScriptLoader
{
    public:
        spell_hun_kill_command() : SpellScriptLoader("spell_hun_kill_command") { }

        class spell_hun_kill_command_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_kill_command_SpellScript);

            bool Validate(SpellInfo const* /*SpellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(HUNTER_SPELL_KILL_COMMAND))
                    return false;
                return true;
            }

            SpellCastResult CheckCastMeet()
            {
                Unit* pet = GetCaster()->GetGuardianPet();
                Unit* petTarget = GetExplTargetUnit();

                if (!pet || pet->isDead())
                    return SPELL_FAILED_NO_PET;

                if (pet->isInStun())
                    return SPELL_FAILED_STUNNED;

                if (pet->isInRoots())
                    return SPELL_FAILED_ROOTED;

                if (pet->HasCrowdControlAura() || pet->HasAuraType(SPELL_AURA_MOD_CONFUSE))
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                // pet has a target and target is within 5 yards
                if (!petTarget || !pet->IsWithinDist(petTarget, 25.0f, true))
                    return SPELL_FAILED_OUT_OF_RANGE;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* pet = GetCaster()->GetGuardianPet())
                {
                    if (!pet)
                        return;

                    if (!GetExplTargetUnit())
                        return;

                    pet->CastSpell(GetExplTargetUnit(), HUNTER_SPELL_KILL_COMMAND_TRIGGER, false);

                    if (pet->getVictim())
                    {
                        pet->AttackStop();
                        pet->ToCreature()->AI()->AttackStart(GetExplTargetUnit());
                    }
                    else
                        pet->ToCreature()->AI()->AttackStart(GetExplTargetUnit());
                }
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (Unit* pet = GetCaster()->GetGuardianPet())
                        {
                            if (!pet)
                                return;

                            if (target->GetGUID() == pet->GetGUID())
                                return;
                        }
 
                        if (!target->HasAura(HUNTER_SPELL_HUNTERS_MARK))
                            _player->CastSpell(target, HUNTER_SPELL_HUNTERS_MARK, true);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_kill_command_SpellScript::CheckCastMeet);
                OnEffectHit += SpellEffectFn(spell_hun_kill_command_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnHit += SpellHitFn(spell_hun_kill_command_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_kill_command_SpellScript();
        }
};

// Rapid Fire - 3045
class spell_hun_rapid_fire : public SpellScriptLoader
{
    public:
        spell_hun_rapid_fire() : SpellScriptLoader("spell_hun_rapid_fire") { }

        class spell_hun_rapid_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_rapid_fire_SpellScript);

            // CRASHFIX: crash in 5.4.0 due to GetEffect), effect 1 doesn't exist in DBC in 5.4
            void HandleOnHit()
            {
                /*if (Player* _player = GetCaster()->ToPlayer())
                {
                    // Item - Bonus season 12 PvP
                    if (_player->HasAura(HUNTER_SPELL_RAPID_INTENSITY))
                    {
                        if (AuraApplication* aura = _player->GetAuraApplication(HUNTER_SPELL_RAPID_FIRE))
                        {
                            Aura* rapidFire = aura->GetBase();

                            rapidFire->GetEffect(1)->ChangeAmount(3200);
                        }
                    }
                    else
                    {
                        if (AuraApplication* aura = _player->GetAuraApplication(HUNTER_SPELL_RAPID_FIRE))
                        {
                            Aura* rapidFire = aura->GetBase();

                            rapidFire->GetEffect(1)->ChangeAmount(0);
                        }
                    }
                }*/
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_rapid_fire_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_rapid_fire_SpellScript();
        }
};

// Cobra Shot - 77767
class spell_hun_cobra_shot : public SpellScriptLoader
{
    public:
        spell_hun_cobra_shot() : SpellScriptLoader("spell_hun_cobra_shot") { }

        class spell_hun_cobra_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_cobra_shot_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        _player->CastSpell(_player, HUNTER_SPELL_COBRA_SHOT_ENERGIZE, true);
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (AuraEffect* aurEff = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_HUNTER, 16384, 0, 0, GetCaster()->GetGUID()))
                        {
                            Aura* serpentSting = aurEff->GetBase();
                            serpentSting->SetDuration(serpentSting->GetDuration() + (GetSpellInfo()->Effects[EFFECT_2].BasePoints * 1000));

                            if (serpentSting->GetMaxDuration() < serpentSting->GetDuration())
                                serpentSting->SetMaxDuration(serpentSting->GetDuration());
                        }
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_hun_cobra_shot_SpellScript::HandleAfterCast);
                OnEffectHitTarget += SpellEffectFn(spell_hun_cobra_shot_SpellScript::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_cobra_shot_SpellScript();
        }
};

// Steady Shot - 56641
class spell_hun_steady_shot : public SpellScriptLoader
{
    public:
        spell_hun_steady_shot() : SpellScriptLoader("spell_hun_steady_shot") { }

        class spell_hun_steady_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_steady_shot_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        // Master Marksman
                        if (roll_chance_i(50))
                        {
                            if (_player->HasAura(HUNTER_SPELL_MASTER_MARKSMAN))
                            {
                                if (Aura* aimed = _player->GetAura(HUNTER_SPELL_MASTER_MARKSMAN_TRIGGERED, _player->GetGUID()))
                                {
                                    //  After reaching 3 stacks, your next Aimed Shot's cast time and Focus cost are reduced by 100% for 10 sec
                                    if (aimed && aimed->GetStackAmount() >= 2)
                                    {
                                        _player->RemoveAura(HUNTER_SPELL_MASTER_MARKSMAN_TRIGGERED);
                                        _player->CastSpell(_player, HUNTER_SPELL_FIRE, true); // Fire !
                                    }
                                    else if (aimed && aimed->GetStackAmount() == 1)
                                        _player->CastSpell(_player, HUNTER_SPELL_MASTER_MARKSMAN_TRIGGERED, true);
                                }
                                else
                                    _player->CastSpell(_player, HUNTER_SPELL_MASTER_MARKSMAN_TRIGGERED, true);
                            }
                        }

                        // Improved Steady Shot
                        if (Aura* steadyFocus = _player->GetAura(HUNTER_SPELL_STEADY_FOCUS, _player->GetGUID()))
                        {
                            uint8 charges = steadyFocus->GetEffect(0)->GetAmount();

                            if (charges == 2)
                            {
                                steadyFocus->GetEffect(0)->SetAmount(charges + 1);
                                _player->CastSpell(_player, HUNTER_SPELL_STEADY_FOCUS_TRIGGERED, true);
                            }
                        }

                        int32 bp = sSpellMgr->GetSpellInfo(HUNTER_SPELL_STEADY_SHOT_ENERGIZE)->Effects[0].BasePoints;
                        if (_player->HasAura(HUNTER_SPELL_STEADY_FOCUS_TRIGGERED))
                            bp += 3;

                        _player->CastCustomSpell(_player, HUNTER_SPELL_STEADY_SHOT_ENERGIZE, &bp, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_hun_steady_shot_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_steady_shot_SpellScript();
        }
};

// Chimera Shot - 53209
class spell_hun_chimera_shot : public SpellScriptLoader
{
    public:
        spell_hun_chimera_shot() : SpellScriptLoader("spell_hun_chimera_shot") { }

        class spell_hun_chimera_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_chimera_shot_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (!target->HasAura(HUNTER_SPELL_HUNTERS_MARK))
                            _player->CastSpell(target, HUNTER_SPELL_HUNTERS_MARK, true);

                        AuraEffect const* serpentSting = target->GetAuraEffect(HUNTER_SPELL_SERPENT_STING, EFFECT_0, _player->GetGUID());

                        if (serpentSting)
                            serpentSting->GetBase()->RefreshDuration();

                        _player->CastSpell(_player, HUNTER_SPELL_CHIMERA_SHOT_HEAL, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_chimera_shot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_chimera_shot_SpellScript();
        }
};

class spell_hun_last_stand_pet : public SpellScriptLoader
{
    public:
        spell_hun_last_stand_pet() : SpellScriptLoader("spell_hun_last_stand_pet") { }

        class spell_hun_last_stand_pet_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_last_stand_pet_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(HUNTER_PET_SPELL_LAST_STAND_TRIGGERED))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                int32 healthModSpellBasePoints0 = int32(caster->CountPctFromMaxHealth(30));
                caster->CastCustomSpell(caster, HUNTER_PET_SPELL_LAST_STAND_TRIGGERED, &healthModSpellBasePoints0, NULL, NULL, true, NULL);
            }

            void Register()
            {
                // add dummy effect spell handler to pet's Last Stand
                OnEffectHitTarget += SpellEffectFn(spell_hun_last_stand_pet_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_last_stand_pet_SpellScript();
        }
};

// Master's Call - 53271
class spell_hun_masters_call : public SpellScriptLoader
{
    public:
        spell_hun_masters_call() : SpellScriptLoader("spell_hun_masters_call") { }

        class spell_hun_masters_call_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_masters_call_SpellScript);

            SpellCastResult Check()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Pet* pet = _player->GetPet())
                    {
                        if (pet->isDead())
                            return SPELL_FAILED_NO_PET;

                        if (pet->isInStun())
                            return SPELL_FAILED_STUNNED;

                        float x, y, z;
                        pet->GetPosition(x, y, z);

                        if (_player->IsWithinLOS(x, y, z))
                            return SPELL_CAST_OK;
                    }
                }
                return SPELL_FAILED_LINE_OF_SIGHT;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (Pet* pet = caster->GetPet())
                        {
                            pet->RemoveMovementImpairingAuras();
                            pet->CastSpell(target, HUNTER_SPELL_MASTERS_CALL_TRIGGERED, true);
                        }
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    target->RemoveMovementImpairingAuras();
                    target->CastSpell(target, HUNTER_SPELL_MASTERS_CALL, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_masters_call_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnEffectHitTarget += SpellEffectFn(spell_hun_masters_call_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
                OnCheckCast += SpellCheckCastFn(spell_hun_masters_call_SpellScript::Check);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_masters_call_SpellScript();
        }
};

// Scatter Shot - 37506
class spell_hun_scatter_shot : public SpellScriptLoader
{
    public:
        spell_hun_scatter_shot() : SpellScriptLoader("spell_hun_scatter_shot") { }

        class spell_hun_scatter_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_scatter_shot_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                // break Auto Shot and autohit
                caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
                caster->AttackStop();
                caster->SendAttackSwingCancelAttack();

                if (caster->HasAura(HUNTER_SPELL_GLYPH_OF_COLLAPSE))
                    if (Unit* target = GetHitUnit())
                        target->RemoveAllAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_scatter_shot_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_scatter_shot_SpellScript();
        }
};

// 53302, 53303, 53304 Sniper Training
enum eSniperTrainingSpells
{
    SPELL_SNIPER_TRAINING_R1        = 53302,
    SPELL_SNIPER_TRAINING_BUFF_R1   = 64418,
};

class spell_hun_sniper_training : public SpellScriptLoader
{
    public:
        spell_hun_sniper_training() : SpellScriptLoader("spell_hun_sniper_training") { }

        class spell_hun_sniper_training_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_sniper_training_AuraScript);

            bool Validate(SpellInfo const* /*entry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SNIPER_TRAINING_R1) || !sSpellMgr->GetSpellInfo(SPELL_SNIPER_TRAINING_BUFF_R1))
                    return false;
                return true;
            }

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                if (aurEff->GetAmount() <= 0)
                {
                    Unit* caster = GetCaster();
                    uint32 spellId = SPELL_SNIPER_TRAINING_BUFF_R1 + GetId() - SPELL_SNIPER_TRAINING_R1;
                    if (Unit* target = GetTarget())
                    {
                        Aura* aur = target->GetAura(spellId);
                        if (!aur || aur->GetMaxDuration() - aur->GetDuration() > 5000)
                        {
                            SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(spellId);
                            Unit* triggerCaster = triggeredSpellInfo->NeedsToBeTriggeredByCaster() ? caster : target;
                            triggerCaster->CastSpell(target, triggeredSpellInfo, true, 0, aurEff);
                        }
                    }
                }
            }

            void HandleUpdatePeriodic(AuraEffect* aurEff)
            {
                if (Player* playerTarget = GetUnitOwner()->ToPlayer())
                {
                    int32 baseAmount = aurEff->GetBaseAmount();
                    int32 amount = playerTarget->isMoving() ?
                    playerTarget->CalculateSpellDamage(playerTarget, GetSpellInfo(), aurEff->GetEffIndex(), &baseAmount) :
                    aurEff->GetAmount() - 1;
                    aurEff->SetAmount(amount);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_sniper_training_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_hun_sniper_training_AuraScript::HandleUpdatePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_sniper_training_AuraScript();
        }
};

class spell_hun_pet_heart_of_the_phoenix : public SpellScriptLoader
{
    public:
        spell_hun_pet_heart_of_the_phoenix() : SpellScriptLoader("spell_hun_pet_heart_of_the_phoenix") { }

        class spell_hun_pet_heart_of_the_phoenix_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_pet_heart_of_the_phoenix_SpellScript);

            bool Load()
            {
                if (!GetCaster()->isPet())
                    return false;
                return true;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED) || !sSpellMgr->GetSpellInfo(HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* owner = caster->GetOwner())
                    if (!caster->HasAura(HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
                    {
                        owner->CastCustomSpell(HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED, SPELLVALUE_BASE_POINT0, 100, caster, true);
                        caster->CastSpell(caster, HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF, true);
                    }
            }

            void Register()
            {
                // add dummy effect spell handler to pet's Last Stand
                OnEffectHitTarget += SpellEffectFn(spell_hun_pet_heart_of_the_phoenix_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_pet_heart_of_the_phoenix_SpellScript();
        }
};

class spell_hun_pet_carrion_feeder : public SpellScriptLoader
{
    public:
        spell_hun_pet_carrion_feeder() : SpellScriptLoader("spell_hun_pet_carrion_feeder") { }

        class spell_hun_pet_carrion_feeder_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_pet_carrion_feeder_SpellScript);

            bool Load()
            {
                if (!GetCaster()->isPet())
                    return false;
                return true;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(HUNTER_PET_SPELL_CARRION_FEEDER_TRIGGERED))
                    return false;
                return true;
            }

            SpellCastResult CheckIfCorpseNear()
            {
                Unit* caster = GetCaster();
                float max_range = GetSpellInfo()->GetMaxRange(false);
                WorldObject* result = NULL;
                // search for nearby enemy corpse in range
                JadeCore::AnyDeadUnitSpellTargetInRangeCheck check(caster, max_range, GetSpellInfo(), TARGET_CHECK_ENEMY);
                JadeCore::WorldObjectSearcher<JadeCore::AnyDeadUnitSpellTargetInRangeCheck> searcher(caster, result, check);
                caster->GetMap()->VisitFirstFound(caster->m_positionX, caster->m_positionY, max_range, searcher);
                if (!result)
                    return SPELL_FAILED_NO_EDIBLE_CORPSES;
                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                caster->CastSpell(caster, HUNTER_PET_SPELL_CARRION_FEEDER_TRIGGERED, false);
            }

            void Register()
            {
                // add dummy effect spell handler to pet's Last Stand
                OnEffectHit += SpellEffectFn(spell_hun_pet_carrion_feeder_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_hun_pet_carrion_feeder_SpellScript::CheckIfCorpseNear);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_pet_carrion_feeder_SpellScript();
        }
};

// Misdirection - 34477 and Misdirection - 110588 (Symbiosis)
class spell_hun_misdirection : public SpellScriptLoader
{
    public:
        spell_hun_misdirection() : SpellScriptLoader("spell_hun_misdirection") { }

        class spell_hun_misdirection_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_misdirection_AuraScript);

            bool _hasGlyph;

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                {
                    _hasGlyph = false;
                    return;
                }

                _hasGlyph = false;

                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetTarget())
                        if (Pet* pet = _player->GetPet())
                            if (pet->GetGUID() == target->GetGUID())
                                if (_player->HasAura(HUNTER_SPELL_GLYPH_OF_MISDIRECTION))
                                    _hasGlyph = true;
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (!GetDuration())
                    {
                        _player->SetReducedThreatPercent(0, 0);

                        if (_hasGlyph)
                        {
                            if (_player->HasSpellCooldown(HUNTER_SPELL_MISDIRECTION))
                                _player->RemoveSpellCooldown(HUNTER_SPELL_MISDIRECTION, true);
                            if (_player->HasSpellCooldown(HUNTER_SPELL_MISDIRECTION_PROC))
                                _player->RemoveSpellCooldown(HUNTER_SPELL_MISDIRECTION, true);
                        }
                    }
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_hun_misdirection_AuraScript::OnApply, EFFECT_2, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_misdirection_AuraScript();
        }
};

// Misdirection (proc) - 35079
class spell_hun_misdirection_proc : public SpellScriptLoader
{
    public:
        spell_hun_misdirection_proc() : SpellScriptLoader("spell_hun_misdirection_proc") { }

        class spell_hun_misdirection_proc_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_misdirection_proc_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->SetReducedThreatPercent(0, 0);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection_proc_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_misdirection_proc_AuraScript();
        }
};

// Disengage - 781
class spell_hun_disengage : public SpellScriptLoader
{
    public:
        spell_hun_disengage() : SpellScriptLoader("spell_hun_disengage") { }

        class spell_hun_disengage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_disengage_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(HUNTER_SPELL_POSTHASTE))
                    {
                        _player->RemoveMovementImpairingAuras();
                        _player->CastSpell(_player, HUNTER_SPELL_POSTHASTE_INCREASE_SPEED, true);
                    }
                    else if (_player->HasAura(HUNTER_SPELL_NARROW_ESCAPE))
                    {
                        std::list<Unit*> unitList;
                        std::list<Unit*> retsList;

                        _player->GetAttackableUnitListInRange(unitList, 8.0f);

                        for (auto itr : unitList)
                            if (_player->IsValidAttackTarget(itr))
                                retsList.push_back(itr);

                        for (auto itr : retsList)
                            _player->CastSpell(itr, HUNTER_SPELL_NARROW_ESCAPE_RETS, true);
                    }

                    if (_player->HasAura(HUNTER_SPELL_GLYPH_OF_LIBERATION))
                        _player->CastSpell(_player, HUNTER_SPELL_GLYPH_OF_LIBERATION_HEAL, true);
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_hun_disengage_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_disengage_SpellScript();
        }
};

// Tame Beast - 1515
class spell_hun_tame_beast : public SpellScriptLoader
{
    public:
        spell_hun_tame_beast() : SpellScriptLoader("spell_hun_tame_beast") { }

        class spell_hun_tame_beast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_tame_beast_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (!GetExplTargetUnit())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    // If we have a full list we shoulden't be able to create a new one.
                    if (caster->getSlotForNewPet() == PET_SLOT_FULL_LIST)
                    {
                        caster->SendPetTameResult(PET_TAME_ERROR_TOO_MANY_PETS);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (Creature* target = GetExplTargetUnit()->ToCreature())
                    {
                        if (target->getLevel() > caster->getLevel())
                            return SPELL_FAILED_HIGHLEVEL;

                        if (!target->GetCreatureTemplate()->isTameable(caster->ToPlayer()->CanTameExoticPets()))
                            return SPELL_FAILED_BAD_TARGETS;

                        if (caster->GetPetGUID())
                            return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                        if (caster->GetCharmGUID())
                            return SPELL_FAILED_ALREADY_HAVE_CHARM;
                    }
                    else
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    return SPELL_CAST_OK;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_FAILED_DONT_REPORT;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_tame_beast_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_tame_beast_SpellScript();
        }
};

class spell_hun_explosive_shot : public SpellScriptLoader
{
    public:
        spell_hun_explosive_shot() : SpellScriptLoader("spell_hun_explosive_shot") 
        { 
        }

        class spell_hun_explosive_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_explosive_shot_SpellScript);

            bool Load()
            {
                amount = 0;

                return true;
            }

            void HandleDummy()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (!amount)
                    amount = (uint32)(GetHitUnit()->GetRemainingPeriodicAmount(GetCaster()->GetGUID(), 53301, SPELL_AURA_PERIODIC_DAMAGE, EFFECT_1) / 3.0f);

                GetSpell()->SetSpellValue(SPELLVALUE_BASE_POINT2, amount);
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_hun_explosive_shot_SpellScript::HandleDummy);
            }

        private:
            uint32 amount;
            
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_explosive_shot_SpellScript();
        }
};

class spell_area_hunter_ice_trap : public SpellAreaTriggerScript
{
    public:
        spell_area_hunter_ice_trap() : SpellAreaTriggerScript("spell_area_hunter_ice_trap") {}

        bool OnAddTarget(AreaTrigger* trigger, Unit* target)
        {
            if (Unit* l_Caster = trigger->GetCaster())
            {
                if (l_Caster->_IsValidAttackTarget(target, sSpellMgr->GetSpellInfo(HUNTER_SPELL_ICE_TRAP_EFFECT)))
                {
                    target->CastSpell(target, HUNTER_SPELL_ICE_TRAP_EFFECT, true);
                    return true;
                }
            }

            return false;
        }

        bool OnRemoveTarget(AreaTrigger* trigger, Unit* target)
        {
            target->RemoveAura(HUNTER_SPELL_ICE_TRAP_EFFECT);
            return true;
        }
};

// Explosive Shot - 53301
class spell_hun_explosive_shot2 : public SpellScriptLoader
{
    public:
        spell_hun_explosive_shot2() : SpellScriptLoader("spell_hun_explosive_shot2") { }

        class spell_hun_explosive_shot2_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_explosive_shot2_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 damage = GetHitDamage();
                        damage = int32(damage * 1.1f);
                        SetHitDamage(damage);

                        // Lock and Load
                        if (Aura* lockAndLoad = _player->GetAura(56453, _player->GetGUID()))
                        {
                            if (_player->HasAura(HUNTER_T16_4P_BONUS))
                            {
                                // With 40% chance Explosive shot won't take a charge of Lock and Load
                                if (roll_chance_i(60))
                                    lockAndLoad->DropCharge();
                            }
                            else
                                lockAndLoad->DropCharge();
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_hun_explosive_shot2_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_explosive_shot2_SpellScript();
        }
};

// Explosive trap - 13812, Explosive trap (knockback) - 149575
// Fix very bad bug with this trap. It doesn't damage (on launch) and knockback targets with the same faction with hunter.
// It's a global bug, because WorldObject's can't use spells on target TARGET_SRC_CASTER, it calles TempSummon that casts a spell.
class spell_explotion_trap_targets : public SpellScriptLoader
{
public:
    spell_explotion_trap_targets() : SpellScriptLoader("spell_explotion_trap_targets") { }

    class spell_explotion_trap_targets_spell_script : public SpellScript
    {
        PrepareSpellScript(spell_explotion_trap_targets_spell_script);

        void CorrectTargets(std::list<WorldObject*>& targets)
        {
            if (!GetCaster())
                return;

            if (Unit* caster = GetCaster())
            {
                if (TempSummon* trap = caster->ToTempSummon())
                {
                    uint64 ownerGUID = trap->GetOwnerGUID();
                    if (!ownerGUID)
                        return;

                    if (Unit* owner = trap->GetOwner())
                    {
                        // Glyph of Exlosive Trap
                        // For knockback effect, if we don't have a glyph we don't need to apply it
                        if (GetSpellInfo()->Id == 149575 && !owner->HasAura(119403))
                        {
                            targets.clear();
                            return;
                        }
                        // For damage on launch effect, if have a glyph we don't need to damage
                        else if (GetSpellInfo()->Id == 13812 && owner->HasAura(119403))
                        {
                            targets.clear();
                            return;
                        }

                        std::list<Unit*> targetsNew;

                        CellCoord p(JadeCore::ComputeCellCoord(trap->GetPositionX(), trap->GetPositionY()));
                        Cell cell(p);
                        cell.SetNoCreate();

                        JadeCore::AnyUnitInObjectRangeCheck u_check(trap, 10.0f);
                        JadeCore::UnitListSearcher<JadeCore::AnyUnitInObjectRangeCheck> searcher(trap, targetsNew, u_check);

                        TypeContainerVisitor<JadeCore::UnitListSearcher<JadeCore::AnyUnitInObjectRangeCheck>, WorldTypeMapContainer> world_unit_searcher(searcher);
                        TypeContainerVisitor<JadeCore::UnitListSearcher<JadeCore::AnyUnitInObjectRangeCheck>, GridTypeMapContainer>  grid_unit_searcher(searcher);

                        cell.Visit(p, world_unit_searcher, *trap->GetMap(), *trap, 10.0f);
                        cell.Visit(p, grid_unit_searcher, *trap->GetMap(), *trap, 10.0f);

                        targets.clear();

                        if (!targetsNew.empty())
                        {
                            targets.clear();

                            for (auto target : targetsNew)
                            {
                                if (target == owner || target == trap)
                                    continue;

                                if (!owner->_IsValidAttackTarget(target, GetSpellInfo()))
                                    continue;

                                targets.push_back(target);
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_explotion_trap_targets_spell_script::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_explotion_trap_targets_spell_script::CorrectTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_explotion_trap_targets_spell_script();
    }
};

// Lock and Load - 56453
class spell_hun_lock_and_load : public SpellScriptLoader
{
    public:
        spell_hun_lock_and_load() : SpellScriptLoader("spell_hun_lock_and_load") { }

        class spell_hun_lock_and_load_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_lock_and_load_AuraScript);

            void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->RemoveSpellCooldown(HUNTER_SPELL_EXPLOSIVE_SHOT, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_hun_lock_and_load_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_lock_and_load_AuraScript();
        }
};

// Called by Arcane Shot - 3044, Aimed Shot - 19434 and Multi-Shot - 2643
// Item - Hunter T16 2P Bonus - 144637
class spell_hun_t16_2p_bonus : public SpellScriptLoader
{
    public:
        spell_hun_t16_2p_bonus() : SpellScriptLoader("spell_hun_t16_2p_bonus") { }

        class spell_hun_t16_2p_bonus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_t16_2p_bonus_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->getLevel() == 90 && _player->HasAura(HUNTER_T16_2P_BONUS))
                        _player->ReduceSpellCooldown(HUNTER_SPELL_RAPID_FIRE, 4000);
                }
            }

            void Register()
            {
               AfterCast += SpellCastFn(spell_hun_t16_2p_bonus_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_t16_2p_bonus_SpellScript();
        }
};

// Called by Aimed Shot (instant with Fire! buff) - 82928
// Item - Hunter T16 4P Bonus - 144641
class spell_hun_t16_4p_bonus : public SpellScriptLoader
{
    public:
        spell_hun_t16_4p_bonus() : SpellScriptLoader("spell_hun_t16_4p_bonus") { }

        class spell_hun_t16_4p_bonus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_t16_4p_bonus_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(HUNTER_T16_4P_BONUS))
                        _player->AddAura(HUNTER_T16_4P_EFFECT_AIMED_SHOT, _player);
            }

            void HandleBeforeCast()
            {
                if (GetCaster())
                    GetCaster()->RemoveAura(HUNTER_SPELL_FIRE);
            }

            void Register()
            {
               OnHit += SpellHitFn(spell_hun_t16_4p_bonus_SpellScript::HandleOnHit);
               BeforeCast += SpellCastFn(spell_hun_t16_4p_bonus_SpellScript::HandleBeforeCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_t16_4p_bonus_SpellScript();
        }
};

// Dismiss Pet - 2641
class spell_hun_dismiss_pet : public SpellScriptLoader
{
    public:
        spell_hun_dismiss_pet() : SpellScriptLoader("spell_hun_dismiss_pet") { }

        class spell_hun_dismiss_pet_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_dismiss_pet_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (Battleground *bg = caster->GetBattleground())
                    {
                        // can't be used on arena when it's started
                        if (bg->isArena() && bg->GetStatus() == STATUS_IN_PROGRESS)
                            return SPELL_FAILED_NOT_IN_ARENA;
                    }

                    return SPELL_CAST_OK;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_FAILED_DONT_REPORT;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_dismiss_pet_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_hun_dismiss_pet_SpellScript();
        }
};

// Roar of Sacrifice - 53480
class spell_hun_pet_roar_of_sacrifice : public SpellScriptLoader
{
    public:
        spell_hun_pet_roar_of_sacrifice() : SpellScriptLoader("spell_hun_pet_roar_of_sacrifice") { }

        class spell_hun_pet_roar_of_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_pet_roar_of_sacrifice_AuraScript);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                if (Unit* l_Protector = GetCaster())
                {
                    if (!l_Protector->isAlive())
                    {
                        l_Protector->RemoveAura(GetId());
                        return;
                    }

                    if (Unit* l_ProtectedUnit = GetUnitOwner())
                    {
                        if (l_Protector->GetGUID() == l_ProtectedUnit->GetGUID())
                            return;

                        int32 l_Value = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount());
                        l_ProtectedUnit->CastCustomSpell(l_Protector, HUNTER_SPELL_ROAR_OF_SACRIFICE_DAMAGE, &l_Value, NULL, NULL, true);
                    }
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_hun_pet_roar_of_sacrifice_AuraScript::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hun_pet_roar_of_sacrifice_AuraScript();
        }
};

void AddSC_hunter_spell_scripts()
{
    new spell_hun_glyph_of_aspect_of_the_beast();
    new spell_hun_glaive_toss_damage();
    new spell_hun_glaive_toss_missile();
    new spell_hun_glyph_of_fetch();
    new spell_hun_tracking();
    new spell_hun_dash();
    new spell_hun_dire_beast();
    new spell_hun_a_murder_of_crows();
    new spell_hun_focus_fire();
    new spell_hun_frenzy();
    new spell_hun_lynx_rush();
    new spell_hun_beast_cleave_proc();
    new spell_hun_beast_cleave();
    new spell_hun_cobra_strikes();
    new spell_hun_barrage();
    new spell_hun_binding_shot();
    new spell_hun_binding_shot_zone();
    new spell_hun_improved_serpent_sting();
    new spell_hun_powershot();
    new spell_hun_feign_death();
    new spell_hun_serpent_spread();
    new spell_hun_ancient_hysteria();
    new spell_hun_kill_command();
    new spell_hun_rapid_fire();
    new spell_hun_cobra_shot();
    new spell_hun_steady_shot();
    new spell_hun_chimera_shot();
    new spell_hun_last_stand_pet();
    new spell_hun_masters_call();
    new spell_hun_scatter_shot();
    new spell_hun_sniper_training();
    new spell_hun_pet_heart_of_the_phoenix();
    new spell_hun_pet_carrion_feeder();
    new spell_hun_misdirection();
    new spell_hun_misdirection_proc();
    new spell_hun_disengage();
    new spell_hun_tame_beast();
    new spell_hun_ferveur();
    new spell_hun_explosive_shot();
    new spell_area_hunter_ice_trap();
    new spell_hun_explosive_shot2();
    new spell_explotion_trap_targets();
    new spell_hun_lock_and_load();
    new spell_hun_t16_2p_bonus();
    new spell_hun_t16_4p_bonus();
    new spell_hun_dismiss_pet();
    new spell_hun_pet_roar_of_sacrifice();
}
