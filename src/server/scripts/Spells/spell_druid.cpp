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
 * Scripts for spells with SPELLFAMILY_DRUID and SPELLFAMILY_GENERIC spells used by druid players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dru_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Containers.h"

enum DruidSpells
{
    DRUID_INCREASED_MOONFIRE_DURATION       = 38414,
    DRUID_NATURES_SPLENDOR                  = 57865,
    DRUID_SURVIVAL_INSTINCTS                = 50322,
    DRUID_SAVAGE_ROAR                       = 62071,
    DRUID_SAVAGE_ROAR_AURA                  = 52610,
    DRUID_SAVAGE_ROAR_GLYPH_AURA            = 127538,
    SPELL_DRUID_ITEM_T8_BALANCE_RELIC       = 64950,
    SPELL_DRUID_WRATH                       = 5176,
    SPELL_DRUID_STARFIRE                    = 2912,
    SPELL_DRUID_STARSURGE                   = 78674,
    SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE    = 81070,
    SPELL_DRUID_STARSURGE_ENERGIZE          = 86605,
    SPELL_DRUID_SOLAR_ECLIPSE               = 48517,
    SPELL_DRUID_LUNAR_ECLIPSE               = 48518,
    SPELL_DRUID_LUNAR_ECLIPSE_OVERRIDE      = 107095,
    SPELL_DRUID_STARFALL                    = 48505,
    SPELL_DRUID_NATURES_GRACE               = 16886,
    SPELL_DRUID_EUPHORIA                    = 81062,
    SPELL_DRUID_PROWL                       = 5215,
    SPELL_DRUID_WEAKENED_ARMOR              = 113746,
    SPELL_DRUID_GLYPH_OF_FRENZIED_REGEN     = 54810,
    SPELL_DRUID_FRENZIED_REGEN_HEAL_TAKE    = 124769,
    SPELL_DRUID_CELESTIAL_ALIGNMENT         = 112071,
    SPELL_DRUID_ASTRAL_COMMUNION            = 127663,
    SPELL_DRUID_SUNFIRE                     = 93402,
    SPELL_DRUID_MOONFIRE                    = 8921,
    SPELL_DRUID_SWIFTMEND                   = 81262,
    SPELL_DRUID_SWIFTMEND_TICK              = 81269,
    DRUID_NPC_WILD_MUSHROOM                 = 47649,
    DRUID_SPELL_FUNGAL_GROWTH_SUMMON        = 81283,
    DRUID_SPELL_MUSHROOM_BIRTH_VISUAL       = 94081,
    DRUID_SPELL_WILD_MUSHROOM_DEATH_VISUAL  = 92701,
    DRUID_SPELL_WILD_MUSHROOM_SUICIDE       = 92853,
    DRUID_SPELL_WILD_MUSHROOM_DAMAGE        = 78777,
    SPELL_DRUID_WILD_MUSHROOM_HEAL          = 102792,
    SPELL_DRUID_FAERIE_DECREASE_SPEED       = 102354,
    SPELL_DRUID_SKULL_BASH_MANA_COST        = 82365,
    SPELL_DRUID_SKULL_BASH_INTERUPT         = 93985,
    SPELL_DRUID_SKULL_BASH_CHARGE           = 93983,
    SPELL_DRUID_FORM_CAT_INCREASE_SPEED     = 113636,
    SPELL_DRUID_GLYPH_OF_REGROWTH           = 116218,
    SPELL_DRUID_REGROWTH                    = 8936,
    SPELL_DRUID_MARK_OF_THE_WILD            = 1126,
    SPELL_DRUID_OMEN_OF_CLARITY             = 113043,
    SPELL_DRUID_CLEARCASTING                = 16870,
    SPELL_DRUID_LIFEBLOOM                   = 33763,
    SPELL_DRUID_LIFEBLOOM_FINAL_HEAL        = 33778,
    SPELL_DRUID_KILLER_INSTINCT             = 108299,
    SPELL_DRUID_KILLER_INSTINCT_MOD_STAT    = 108300,
    SPELL_DRUID_CAT_FORM                    = 768,
    SPELL_DRUID_BEAR_FORM                   = 5487,
    SPELL_DRUID_BEAR_FORM_RAGE_GAIN         = 17057,
    SPELL_DRUID_INFECTED_WOUNDS             = 58180,
    SPELL_DRUID_BEAR_HUG                    = 102795,
    SPELL_DRUID_RIP                         = 1079,
    SPELL_DRUID_SAVAGE_DEFENSE_DODGE_PCT    = 132402,
    SPELL_DRUID_DASH                        = 1850,
    SPELL_DRUID_BERSERK_BEAR                = 50334,
    SPELL_DRUID_BERSERK_CAT                 = 106951,
    SPELL_DRUID_STAMPEDING_ROAR             = 106898,
    SPELL_DRUID_SOLAR_BEAM                  = 78675,
    SPELL_DRUID_SOLAR_BEAM_SILENCE          = 81261,
    SPELL_DRUID_URSOLS_VORTEX_AREA_TRIGGER  = 102793,
    SPELL_DRUID_URSOLS_VORTEX_SNARE         = 127797,
    SPELL_DRUID_URSOLS_VORTEX_JUMP_DEST     = 118283,
    SPELL_DRUID_CENARION_WARD               = 102352,
    SPELL_DRUID_SYMBIOSIS_FOR_CASTER        = 110309,
    SPELL_DRUID_SYMBIOSIS_DEATH_KNIGHT      = 110478,
    SPELL_DRUID_SYMBIOSIS_HUNTER            = 110479,
    SPELL_DRUID_SYMBIOSIS_MAGE              = 110482,
    SPELL_DRUID_SYMBIOSIS_MONK              = 110483,
    SPELL_DRUID_SYMBIOSIS_PALADIN           = 110484,
    SPELL_DRUID_SYMBIOSIS_PRIEST            = 110485,
    SPELL_DRUID_SYMBIOSIS_ROGUE             = 110486,
    SPELL_DRUID_SYMBIOSIS_SHAMAN            = 110488,
    SPELL_DRUID_SYMBIOSIS_WARLOCK           = 110490,
    SPELL_DRUID_SYMBIOSIS_WARRIOR           = 110491,
    SPELL_DRUID_SHATTERING_BLOW             = 112997,
    WARLOCK_DEMONIC_CIRCLE_SUMMON           = 48018,
    SPELL_DRUID_RAKE                        = 1822,
    SPELL_DRUID_CONSECRATION_DUMMY          = 81298,
    SPELL_DRUID_CONSECRATION_DAMAGE         = 110705,
    SPELL_DRUID_SHOOTING_STARS              = 93400,
    SPELL_DRUID_TIGERS_FURY                 = 5217,
    SPELL_DRUID_SOUL_OF_THE_FOREST          = 114107,
    SPELL_DRUID_SOUL_OF_THE_FOREST_HASTE    = 114108,
    SPELL_DRUID_SWIPE_CAT                   = 62078,
    SPELL_DRUID_MANGLE_BEAR                 = 33878,
    SPELL_DRUID_STAMPEDE                    = 81022,
    SPELL_DRUID_INCARNATION_KING_OF_JUNGLE  = 102543,
    SPELL_DRUID_GLYPH_OF_SHRED              = 114234,
    SPELL_DRUID_GLYPH_OF_SHRED_OVERRIDE     = 114235,
    SPELL_DRUID_INCARNATION_CHOSEN_OF_ELUNE = 122114,
    SPELL_DRUID_GLYPH_OF_BLOOMING           = 121840,
    SPELL_DRUID_GLYPH_OF_THE_TREANT         = 114282,
    SPELL_DRUID_REJUVENATION                = 774,
    SPELL_DRUID_GENESIS                     = 145518,
    SPELL_DRUID_ASTRAL_INSIGH               = 145138,
    SPELL_DRUID_YSERAS_GIFT                 = 145109,
    SPELL_CHARM_WOODLAND_CREATURE           = 127757,
    SPELL_DRUID_ASTRAL_INSIGHT              = 145138,
    SPELL_DRUID_TOOTH_AND_CLAW_AURA         = 135286,
    SPELL_DRUID_TOOTH_AND_CLAW_ABSORB       = 135597,
    SPELL_DRUID_TOOTH_AND_CLAW_VISUAL_AURA  = 135601,
    SPELL_DRUID_GLYPH_OF_EFFLORESCENCE      = 145529,
    SPELL_DRUID_WILD_MUSHROOM_GROWING       = 138611,
    SPELL_DRUID_WILD_MUSHROOM_MOD_SCALE     = 138616,
    SPELL_DRUID_WILD_MUSHROOM_VISUAL        = 138664,
    SPELL_DRUID_MIGHT_OF_URSOC              = 106922,
    SPELL_DRUID_DREAM_OF_CENARIUS           = 108373,
    SPELL_DRUID_DREAM_OF_CENARIUS_BALANCE   = 145151,
    SPELL_DRUID_DREAM_OF_CENARIUS_CAT       = 145152,
    SPELL_DRUID_DREAM_OF_CENARIUS_BEAR      = 145162,
    SPELL_DRUID_DREAM_OF_CENARIUS_RESTOR    = 145153,
    SPELL_TREANT_RESTORATION_SPELL          = 113828,
    SPELL_TREANT_BALANCE_SPELL              = 113769,
    DRUID_T16_RESTORATION_2P_BONUS          = 144869,
    SPELL_DRUID_SAGE_MENDER                 = 144871,
    SPELL_DRUID_GLYPH_OF_GUIDED_STARS       = 146655
};

// Tooth and Claw - 135597
class spell_dru_tooth_and_claw_absorb : public SpellScriptLoader
{
    public:
        spell_dru_tooth_and_claw_absorb() : SpellScriptLoader("spell_dru_tooth_and_claw_absorb") { }

        class spell_dru_tooth_and_claw_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_tooth_and_claw_absorb_AuraScript);

            void OnAbsorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                if (Unit* attacker = dmgInfo.GetAttacker())
                {
                    if (!attacker->HasAura(SPELL_DRUID_TOOTH_AND_CLAW_VISUAL_AURA))
                    {
                        absorbAmount = 0;
                        return;
                    }
                    
                    attacker->RemoveAurasDueToSpell(SPELL_DRUID_TOOTH_AND_CLAW_VISUAL_AURA);
                }
            }

            void Register()
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dru_tooth_and_claw_absorb_AuraScript::OnAbsorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_tooth_and_claw_absorb_AuraScript();
        }
};

// Glyph of the Treant - 125047
class spell_dru_glyph_of_the_treant : public SpellScriptLoader
{
    public:
        spell_dru_glyph_of_the_treant() : SpellScriptLoader("spell_dru_glyph_of_the_treant") { }

        class spell_dru_glyph_of_the_treant_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_glyph_of_the_treant_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(SPELL_DRUID_GLYPH_OF_THE_TREANT, false);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(SPELL_DRUID_GLYPH_OF_THE_TREANT))
                        _player->removeSpell(SPELL_DRUID_GLYPH_OF_THE_TREANT, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_glyph_of_the_treant_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_dru_glyph_of_the_treant_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_glyph_of_the_treant_AuraScript();
        }
};

// Incarnation : Chosen of Elune - 102560
class spell_dru_incarnation_chosen_of_elune : public SpellScriptLoader
{
    public:
        spell_dru_incarnation_chosen_of_elune() : SpellScriptLoader("spell_dru_incarnation_chosen_of_elune") { }

        class spell_dru_incarnation_chosen_of_elune_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_incarnation_chosen_of_elune_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                {
                    _player->CastSpell(_player, SPELL_DRUID_INCARNATION_CHOSEN_OF_ELUNE, true);
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->RemoveAura(SPELL_DRUID_INCARNATION_CHOSEN_OF_ELUNE);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_incarnation_chosen_of_elune_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_chosen_of_elune_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_incarnation_chosen_of_elune_AuraScript();
        }
};

// Called by Incarnation : Chosen of Elune - 102560, Incarnation : Son of Ursoc - 102558 and Incarnation : King of the Jungle - 102543
// Incarnation - Skins
class spell_dru_incarnation_skins : public SpellScriptLoader
{
    public:
        spell_dru_incarnation_skins() : SpellScriptLoader("spell_dru_incarnation_skins") { }

        class spell_dru_incarnation_skins_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_incarnation_skins_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->SetDisplayId(_player->GetModelForForm(_player->GetShapeshiftForm()));
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->SetDisplayId(_player->GetModelForForm(_player->GetShapeshiftForm()));
            }

            void Register()
            {
                switch (m_scriptSpellId)
                {
                    case 102543:// King of the Jungle
                        OnEffectApply += AuraEffectApplyFn(spell_dru_incarnation_skins_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
                        OnEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_skins_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
                        break;
                    case 102558:// Son of Ursoc
                        OnEffectApply += AuraEffectApplyFn(spell_dru_incarnation_skins_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                        OnEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_skins_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                        break;
                    case 102560:// Chosen of Elune
                        OnEffectApply += AuraEffectApplyFn(spell_dru_incarnation_skins_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                        OnEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_skins_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                        break;
                    default:
                        break;
                }
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_incarnation_skins_AuraScript();
        }
};

// Called by Berserk (cat) - 106951 and Tiger's Fury - 5217
// Glyph of Shred - 114234
class spell_dru_glyph_of_shred : public SpellScriptLoader
{
    public:
        spell_dru_glyph_of_shred() : SpellScriptLoader("spell_dru_glyph_of_shred") { }

        class spell_dru_glyph_of_shred_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_glyph_of_shred_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell(GetTarget(), SPELL_DRUID_GLYPH_OF_SHRED_OVERRIDE, true);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (aurEff->GetSpellInfo()->Id == SPELL_DRUID_BERSERK_CAT && !GetTarget()->HasAura(SPELL_DRUID_TIGERS_FURY))
                    GetTarget()->RemoveAurasDueToSpell(SPELL_DRUID_GLYPH_OF_SHRED_OVERRIDE);
                else if (aurEff->GetSpellInfo()->Id == SPELL_DRUID_TIGERS_FURY && !GetTarget()->HasAura(SPELL_DRUID_BERSERK_CAT))
                    GetTarget()->RemoveAurasDueToSpell(SPELL_DRUID_GLYPH_OF_SHRED_OVERRIDE);
            }

            void Register()
            {
                switch (m_scriptSpellId)
                {
                    case SPELL_DRUID_BERSERK_CAT:
                        OnEffectApply += AuraEffectApplyFn(spell_dru_glyph_of_shred_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                        AfterEffectRemove += AuraEffectRemoveFn(spell_dru_glyph_of_shred_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                        break;
                    case SPELL_DRUID_TIGERS_FURY:
                        OnEffectApply += AuraEffectApplyFn(spell_dru_glyph_of_shred_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                        AfterEffectRemove += AuraEffectRemoveFn(spell_dru_glyph_of_shred_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                        break;
                    default:
                        break;
                }
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_glyph_of_shred_AuraScript();
        }
};

 // Item - PvP Feral 4P Bonus - 131537
class spell_dru_item_pvp_feral_4p : public SpellScriptLoader
{
    public:
        spell_dru_item_pvp_feral_4p() : SpellScriptLoader("spell_dru_item_pvp_feral_4p") { }

        class spell_dru_item_pvp_feral_4p_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_item_pvp_feral_4p_AuraScript);

            uint32 update;

            bool Validate(SpellInfo const* /*spell*/)
            {
                update = 0;

                if (!sSpellMgr->GetSpellInfo(101976))
                    return false;
                return true;
            }

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                if (!GetCaster())
                    return;

                if (GetCaster()->HasAura(SPELL_DRUID_STAMPEDE))
                    return;

                update += diff;

                if (GetCaster()->HasAura(SPELL_DRUID_INCARNATION_KING_OF_JUNGLE))
                    return;

                if (update >= 30000)
                {
                    if (Player* _player = GetCaster()->ToPlayer())
                        _player->CastSpell(_player, SPELL_DRUID_STAMPEDE, true);

                    update = 0;
                }
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_dru_item_pvp_feral_4p_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_item_pvp_feral_4p_AuraScript();
        }
};

// Wild Charge (Moonkin) - 102383
class spell_dru_wild_charge_moonkin : public SpellScriptLoader
{
    public:
        spell_dru_wild_charge_moonkin() : SpellScriptLoader("spell_dru_wild_charge_moonkin") { }

        class spell_dru_wild_charge_moonkin_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_charge_moonkin_SpellScript);

            SpellCastResult CheckFight()
            {
                if (GetCaster())
                {
                    if (!GetCaster()->isInCombat())
                        return SPELL_FAILED_DONT_REPORT;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_wild_charge_moonkin_SpellScript::CheckFight);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_charge_moonkin_SpellScript();
        }
};

// Thrash (bear) - 77758
class spell_dru_thrash_bear : public SpellScriptLoader
{
    public:
        spell_dru_thrash_bear() : SpellScriptLoader("spell_dru_thrash_bear") { }

        class spell_dru_thrash_bear_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_thrash_bear_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                // Each tick has 25% chance to remove cooldown on Mangle
                if (Player* _plr = GetCaster()->ToPlayer())
                    if (roll_chance_i(25))
                        if (_plr->HasSpellCooldown(SPELL_DRUID_MANGLE_BEAR))
                            _plr->RemoveSpellCooldown(SPELL_DRUID_MANGLE_BEAR, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_thrash_bear_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_thrash_bear_AuraScript();
        }
};

// Swipe (cat) - 62078, Swipe (bear) - 779 and Maul - 6807
class spell_dru_swipe_and_maul : public SpellScriptLoader
{
    public:
        spell_dru_swipe_and_maul() : SpellScriptLoader("spell_dru_swipe_and_maul") { }

        class spell_dru_swipe_and_maul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_swipe_and_maul_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 damage = GetHitDamage();

                        // Swipe and Maul deals 20% more damage if target is bleeding
                        if (target->HasAuraState(AURA_STATE_BLEEDING))
                        {
                            AddPct(damage, 20);
                            SetHitDamage(damage);
                        }

                        if (GetSpellInfo()->Id == 6807 && caster->HasAura(SPELL_DRUID_TOOTH_AND_CLAW_AURA))
                        {
                            int32 bp = CalculatePct(caster->GetTotalAttackPowerValue(BASE_ATTACK), 88);
                            int32 agi = CalculatePct(caster->GetStat(STAT_AGILITY), 176);
                            if (agi > bp)
                                bp = agi;
                            if (caster->GetStat(STAT_STAMINA) > bp)
                                bp = caster->GetStat(STAT_STAMINA);

                            caster->RemoveAura(SPELL_DRUID_TOOTH_AND_CLAW_AURA);
                            caster->CastCustomSpell(caster, SPELL_DRUID_TOOTH_AND_CLAW_ABSORB, &bp, NULL, NULL, true);
                            caster->CastCustomSpell(target, SPELL_DRUID_TOOTH_AND_CLAW_VISUAL_AURA, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_swipe_and_maul_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_swipe_and_maul_SpellScript();
        }
};

// Called by Lunar Eclipse - 48518 and Solar Eclipse - 48517
// Soul of the Forest - 114107
class spell_dru_soul_of_the_forest_eclipse : public SpellScriptLoader
{
    public:
        spell_dru_soul_of_the_forest_eclipse() : SpellScriptLoader("spell_dru_soul_of_the_forest_eclipse") { }

        class spell_dru_soul_of_the_forest_eclipse_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_soul_of_the_forest_eclipse_AuraScript);

            void HandleEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                if (Player* _player = GetTarget()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_DRUID_SOUL_OF_THE_FOREST_HASTE))
                    {
                        if (aurEff->GetSpellInfo()->Id == SPELL_DRUID_SOLAR_ECLIPSE)
                            _player->SetEclipsePower(int32(_player->GetEclipsePower() - 20));
                        else if (aurEff->GetSpellInfo()->Id == SPELL_DRUID_LUNAR_ECLIPSE)
                            _player->SetEclipsePower(int32(_player->GetEclipsePower() + 20));
                    }
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_soul_of_the_forest_eclipse_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_soul_of_the_forest_eclipse_AuraScript();
        }
};

// Called by Swiftmend - 18562 and Mangle (Bear) - 33878
// Soul of the Forest - 114107
class spell_dru_soul_of_the_forest : public SpellScriptLoader
{
    public:
        spell_dru_soul_of_the_forest() : SpellScriptLoader("spell_dru_soul_of_the_forest") { }

        class spell_dru_soul_of_the_forest_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_soul_of_the_forest_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (GetSpellInfo()->Id == 18562)
                    {
                        if (!caster->HasAura(SPELL_DRUID_GLYPH_OF_EFFLORESCENCE))
                            caster->CastSpell(GetHitUnit(), SPELL_DRUID_SWIFTMEND, true);
                    }

                    if (GetSpellInfo()->Id == 33878)
                    {
                        // Dream of Cenarius
                        if (caster->ToPlayer()->GetSpecializationId(caster->ToPlayer()->GetActiveSpec()) == SPEC_DRUID_BEAR && caster->HasAura(SPELL_DRUID_DREAM_OF_CENARIUS) && GetSpell()->IsCritForTarget(GetHitUnit()))
                        {
                            if (roll_chance_i(40))
                                caster->AddAura(SPELL_DRUID_DREAM_OF_CENARIUS_BEAR, caster);
                        }
                    }

                    if (caster->HasAura(SPELL_DRUID_SOUL_OF_THE_FOREST))
                    {
                        if (GetSpellInfo()->Id == 18562)
                            caster->CastSpell(caster, SPELL_DRUID_SOUL_OF_THE_FOREST_HASTE, true);
                        else
                            caster->EnergizeBySpell(caster, SPELL_DRUID_SOUL_OF_THE_FOREST, 40, POWER_RAGE);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_soul_of_the_forest_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_soul_of_the_forest_SpellScript();
        }
};

// Tiger's Fury - 5217
class spell_dru_tigers_fury : public SpellScriptLoader
{
    public:
        spell_dru_tigers_fury() : SpellScriptLoader("spell_dru_tigers_fury") { }

        class spell_dru_tigers_fury_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_tigers_fury_SpellScript);

            SpellCastResult CheckBerzerk()
            {
                if (GetCaster())
                {
                    if (GetCaster()->HasAura(SPELL_DRUID_BERSERK_CAT))
                        return SPELL_FAILED_DONT_REPORT;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_tigers_fury_SpellScript::CheckBerzerk);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_tigers_fury_SpellScript();
        }
};

// Play Death - 110597
class spell_dru_play_death : public SpellScriptLoader
{
    public:
        spell_dru_play_death() : SpellScriptLoader("spell_dru_play_death") { }

        class spell_dru_play_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_play_death_AuraScript);

            int32 health;
            int32 mana;

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                health = GetTarget()->GetHealth();
                mana = GetTarget()->GetPower(POWER_MANA);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (health && mana)
                {
                    GetTarget()->SetHealth(health);
                    GetTarget()->SetPower(POWER_MANA, mana);
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_play_death_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_FEIGN_DEATH, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_play_death_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_FEIGN_DEATH, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_play_death_AuraScript();
        }
};

// Consecration - 110701 (periodic dummy)
class spell_dru_consecration : public SpellScriptLoader
{
    public:
        spell_dru_consecration() : SpellScriptLoader("spell_dru_consecration") { }

        class spell_dru_consecration_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_consecration_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_DRUID_CONSECRATION_DUMMY))
                    GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), SPELL_DRUID_CONSECRATION_DAMAGE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_consecration_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_consecration_AuraScript();
        }
};

// Consecration - 110701 (Symbiosis)
class spell_dru_consecration_area : public SpellScriptLoader
{
    public:
        spell_dru_consecration_area() : SpellScriptLoader("spell_dru_consecration_area") { }

        class spell_dru_consecration_area_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_consecration_area_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, SPELL_DRUID_CONSECRATION_DUMMY, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_dru_consecration_area_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_consecration_area_SpellScript();
        }
};

// Life Tap - 122290
class spell_dru_life_tap : public SpellScriptLoader
{
    public:
        spell_dru_life_tap() : SpellScriptLoader("spell_dru_life_tap") { }

        class spell_dru_life_tap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_life_tap_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->ModifyHealth(-1 * int32(_player->CountPctFromMaxHealth(20)));
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_life_tap_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_life_tap_SpellScript();
        }
};

// Binary predicate for sorting Units based on value of duration of an Aura
class AuraDurationCompareOrderPred
{
    public:
        AuraDurationCompareOrderPred(uint64 caster, uint32 auraId, bool ascending = true) : m_caster(caster), m_aura(auraId), m_ascending(ascending) {}
        bool operator() (const Unit* a, const Unit* b) const
        {
            return m_ascending ? a->GetAura(m_aura, m_caster)->GetDuration() < b->GetAura(m_aura, m_caster)->GetDuration() :
                                    a->GetAura(m_aura, m_caster)->GetDuration() > b->GetAura(m_aura, m_caster)->GetDuration();
        }
    private:
        uint64 m_caster;
        uint32 m_aura;
        const bool m_ascending;
};

// Soul Swap - 110810
class spell_dru_soul_swap : public SpellScriptLoader
{
    public:
        spell_dru_soul_swap() : SpellScriptLoader("spell_dru_soul_swap") { }

        class spell_dru_soul_swap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_soul_swap_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> tempList;
                        std::list<Unit*> targetList;

                        _player->GetAttackableUnitListInRange(tempList, 15.0f);

                        for (auto itr : tempList)
                        {
                            if (itr->GetGUID() == target->GetGUID())
                                continue;

                            if (itr->GetGUID() == _player->GetGUID())
                                continue;

                            if (!_player->IsValidAttackTarget(itr))
                                continue;

                            if (itr->HasAura(SPELL_DRUID_RIP, _player->GetGUID()) && itr->HasAura(SPELL_DRUID_RAKE, _player->GetGUID()))
                                targetList.push_back(itr);
                        }

                        if (!targetList.empty())
                        {
                            targetList.sort(AuraDurationCompareOrderPred(_player->GetGUID(), SPELL_DRUID_RIP));

                            for (auto itr : targetList)
                            {
                                int32 ripDuration;
                                int32 ripMaxDuration;
                                int32 ripAmount;
                                int32 rakeDuration;
                                int32 rakeMaxDuration;
                                int32 rakeAmount;

                                if (Aura* rip = itr->GetAura(SPELL_DRUID_RIP, _player->GetGUID()))
                                {
                                    ripDuration = rip->GetDuration();
                                    ripMaxDuration = rip->GetMaxDuration();
                                    ripAmount = rip->GetEffect(0)->GetAmount();
                                }
                                if (Aura* rake = itr->GetAura(SPELL_DRUID_RAKE, _player->GetGUID()))
                                {
                                    rakeDuration = rake->GetDuration();
                                    rakeMaxDuration = rake->GetMaxDuration();
                                    rakeAmount = rake->GetEffect(1)->GetAmount();
                                }

                                itr->RemoveAura(SPELL_DRUID_RIP, _player->GetGUID());
                                itr->RemoveAura(SPELL_DRUID_RAKE, _player->GetGUID());

                                _player->AddAura(SPELL_DRUID_RIP, target);
                                _player->AddAura(SPELL_DRUID_RAKE, target);

                                if (Aura* rip = target->GetAura(SPELL_DRUID_RIP, _player->GetGUID()))
                                {
                                    rip->SetDuration(ripDuration);
                                    rip->SetMaxDuration(ripMaxDuration);
                                    rip->GetEffect(0)->SetAmount(ripAmount);
                                }
                                if (Aura* rake = target->GetAura(SPELL_DRUID_RAKE, _player->GetGUID()))
                                {
                                    rake->SetDuration(rakeDuration);
                                    rake->SetMaxDuration(rakeMaxDuration);
                                    rake->GetEffect(1)->SetAmount(rakeAmount);
                                }

                                break;
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_soul_swap_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_soul_swap_SpellScript();
        }
};

// Demonic Circle : Teleport - 112970
class spell_dru_demonic_circle_teleport : public SpellScriptLoader
{
    public:
        spell_dru_demonic_circle_teleport() : SpellScriptLoader("spell_dru_demonic_circle_teleport") { }

        class spell_dru_demonic_circle_teleport_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_demonic_circle_teleport_SpellScript);

            SpellCastResult CheckTarget()
            {
                if (!GetCaster())
                    return SPELL_FAILED_DONT_REPORT;

                std::list<Unit*> groupList;

                GetCaster()->GetPartyMembers(groupList);

                // Demonic Circle: Teleport
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(48020);

                if (!groupList.empty())
                {
                    for (auto itr : groupList)
                    {
                        if (itr->HasAura(SPELL_DRUID_SYMBIOSIS_WARLOCK, GetCaster()->GetGUID()))
                        {
                            if (GameObject* circle = itr->GetGameObject(WARLOCK_DEMONIC_CIRCLE_SUMMON))
                            {
                                if (GetCaster()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
                                    return SPELL_CAST_OK;
                                else
                                    return SPELL_FAILED_OUT_OF_RANGE;
                            }
                        }
                    }
                }
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    std::list<Unit*> groupList;

                    _player->GetPartyMembers(groupList);

                    if (!groupList.empty())
                    {
                        for (auto itr : groupList)
                        {
                            if (itr->HasAura(SPELL_DRUID_SYMBIOSIS_WARLOCK, _player->GetGUID()))
                            {
                                if (GameObject* circle = itr->GetGameObject(WARLOCK_DEMONIC_CIRCLE_SUMMON))
                                {
                                    _player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                                    _player->RemoveMovementImpairingAuras();
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_demonic_circle_teleport_SpellScript::CheckTarget);
                OnHit += SpellHitFn(spell_dru_demonic_circle_teleport_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_demonic_circle_teleport_SpellScript();
        }
};

// Shattering Blow - 112997
class spell_dru_shattering_blow : public SpellScriptLoader
{
    public:
        spell_dru_shattering_blow() : SpellScriptLoader("spell_dru_shattering_blow") { }

        class spell_dru_shattering_blow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_shattering_blow_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAuraWithMechanic(1<<MECHANIC_IMMUNE_SHIELD))
                        {
                            target->RemoveAura(SPELL_DRUID_SHATTERING_BLOW);
                            target->RemoveAurasWithMechanic(1<<MECHANIC_IMMUNE_SHIELD, AURA_REMOVE_BY_ENEMY_SPELL);
                        }

                        _player->CastSpell(_player, SPELL_DRUID_CAT_FORM, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_shattering_blow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_shattering_blow_SpellScript();
        }
};

// Symbiosis Aura - 110478/110479/110482/110483/110484/110485/110486/110488/110490/110491
 class spell_dru_symbiosis_aura : public SpellScriptLoader
{
    public:
        spell_dru_symbiosis_aura() : SpellScriptLoader("spell_dru_symbiosis_aura") { }

        class spell_dru_symbiosis_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_symbiosis_aura_AuraScript);

            ObjectGuid m_casterGuid;

            void OnUpdate(uint32 diff)
            {

                if (!GetCaster() && GetUnitOwner())
                {
                    if (Player* target = GetUnitOwner()->ToPlayer())
                        target->RemoveAura(GetSpellInfo()->Id);
                    if (Player* caster = Caster())
                        caster->RemoveAura(SPELL_DRUID_SYMBIOSIS_FOR_CASTER);
                }
                else if (GetCaster() && !GetUnitOwner())
                {
                    if (Player* caster = GetCaster()->ToPlayer())
                        caster->RemoveAura(SPELL_DRUID_SYMBIOSIS_FOR_CASTER);
                }
                else if (GetCaster() && GetUnitOwner())
                {
                    Player* caster = GetCaster()->ToPlayer();
                    Player* target = GetUnitOwner()->ToPlayer();
                    if (!target || !caster)
                        return;

                    if (target->GetMapId() != caster->GetMapId() 
                        || !target->IsInSameRaidWith(caster) 
                        || !caster->HasAura(SPELL_DRUID_SYMBIOSIS_FOR_CASTER))
                    {
                        target->RemoveAura(GetSpellInfo()->Id);
                        caster->RemoveAura(SPELL_DRUID_SYMBIOSIS_FOR_CASTER);
                    }
                }
            }

            Player* Caster()
            {
                if (m_casterGuid)
                    return sObjectAccessor->FindPlayer(m_casterGuid);

                return nullptr;
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    m_casterGuid = GetCaster()->GetGUID();
                else
                    ObjectGuid(0LL);
            }

            void Register()
            {
                OnAuraUpdate += AuraUpdateFn(spell_dru_symbiosis_aura_AuraScript::OnUpdate);
                AfterEffectApply += AuraEffectApplyFn(spell_dru_symbiosis_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_symbiosis_aura_AuraScript();
        }
};

 const static uint32 s_TargetSpells[MAX_CLASSES][2] =
 {
     { 0, 0 },                                  // CLASS_NONE
     { SPELL_DRUID_SYMBIOSIS_WARRIOR, 0 },      // CLASS_WARRIOR
     { SPELL_DRUID_SYMBIOSIS_PALADIN, 0 },      // CLASS_PALADIN
     { SPELL_DRUID_SYMBIOSIS_HUNTER, 113073 },  // CLASS_HUNTER
     { SPELL_DRUID_SYMBIOSIS_ROGUE, 113613 },   // CLASS_ROGUE
     { SPELL_DRUID_SYMBIOSIS_PRIEST, 0 },       // CLASS_PRIEST
     { SPELL_DRUID_SYMBIOSIS_DEATH_KNIGHT, 0 }, // CLASS_DEATH_KNIGHT
     { SPELL_DRUID_SYMBIOSIS_SHAMAN, 0 },       // CLASS_SHAMAN
     { SPELL_DRUID_SYMBIOSIS_MAGE, 113074 },    // CLASS_MAGE
     { SPELL_DRUID_SYMBIOSIS_WARLOCK, 113295 }, // CLASS_WARLOCK
     { SPELL_DRUID_SYMBIOSIS_MONK, 0 },         // CLASS_MONK
     { 0, 0 },                                  // CLASS_DRUID
 };


 // default, healer, dps, tank
 const static uint32 s_TargetsBPSpells[MAX_CLASSES][ROLES_TANK + 1] =
 {
     { 0, 0, 0, 0 },                // CLASS_NONE
     { 0, 0, 122294, 122286 },      // CLASS_WARRIOR
     { 0, 113269, 122287, 113075 }, // CLASS_PALADIN
     { 0, 0, 0, 0 },                // CLASS_HUNTER
     { 0, 0, 0, 0 },                // CLASS_ROGUE
     { 0, 113506, 113277, 0 },      // CLASS_PRIEST
     { 0, 0, 113516, 113072 },      // CLASS_DEATH_KNIGHT
     { 0, 113289, 113286, 0 },      // CLASS_SHAMAN
     { 0, 0, 0, 0 },                // CLASS_MAGE
     { 0, 0, 0, 0 },                // CLASS_WARLOCK
     { 0, 113275, 127361, 113306 }, // CLASS_MONK
     { 0, 0, 0, 0 },                // CLASS_DRUID
 };

 // balance, bear, cat, restoration
 const static uint32 s_CasterSpells[MAX_CLASSES][4] =
 {
     { 0, 0, 0, 0 },                     // CLASS_NONE
     { 122292, 113002, 112997, 113004 }, // CLASS_WARRIOR
     { 110698, 110701, 110700, 122288 }, // CLASS_PALADIN
     { 110588, 110600, 110597, 110617 }, // CLASS_HUNTER
     { 110788, 122289, 110730, 110791 }, // CLASS_ROGUE
     { 110707, 110717, 110715, 110718 }, // CLASS_PRIEST
     { 110570, 122285, 122282, 110575 }, // CLASS_DEATH_KNIGHT
     { 110802, 110803, 110807, 110806 }, // CLASS_SHAMAN
     { 110621, 110694, 110693, 110696 }, // CLASS_MAGE
     { 122291, 122290, 110810, 112970 }, // CLASS_WARLOCK
     { 126458, 126453, 126449, 126456 }, // CLASS_MONK
     { 0, 0, 0, 0 },                     // CLASS_DRUID
 };

 const static uint32 s_CasterSpecs[4] =
 {
     SPEC_DRUID_BALANCE,
     SPEC_DRUID_BEAR,
     SPEC_DRUID_CAT,
     SPEC_DRUID_RESTORATION,
 };

// Symbiosis - 110309
class spell_dru_symbiosis : public SpellScriptLoader
{
    public:
        spell_dru_symbiosis() : SpellScriptLoader("spell_dru_symbiosis") { }

        class spell_dru_symbiosis_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_symbiosis_SpellScript);

            uint8 GetIndex(uint32 spec) const
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (s_CasterSpecs[i] == spec)
                        return i;
                }

                return 0xFF;
            }

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    Player* target = GetHitUnit() ? GetHitUnit()->ToPlayer() : nullptr;
                    if (!target || target == _player || target->getClass() == CLASS_DRUID || !target->IsInSameRaidWith(_player))
                    {
                        // should newer happend
                        return;
                    }

                    if (target)
                    {
                        if (AuraEffect* _effect = _player->GetAuraEffect(SPELL_DRUID_SYMBIOSIS_FOR_CASTER, EFFECT_0))
                        {
                            uint32 index = GetIndex(_player->GetSpecializationId(_player->GetActiveSpec()));
                            uint32 targetRole = target->GetRoleForGroup(target->GetSpecializationId(target->GetActiveSpec()));

                            if (targetRole == ROLES_DEFAULT || index == 0xFF)
                                return;

                            uint32 spellCaster = s_CasterSpells[target->getClass()][index];
                            if (spellCaster)
                                _effect->ChangeAmount(spellCaster);

                            if (uint32 spellTarget = s_TargetSpells[target->getClass()][0])
                            {
                                int32 bpTarget = s_TargetSpells[target->getClass()][1];
                                if (!bpTarget)
                                    bpTarget = s_TargetsBPSpells[target->getClass()][targetRole];

                                if (bpTarget)
                                    _player->CastCustomSpell(target, spellTarget, &bpTarget, NULL, NULL, true);
                            }
                        }
                    }
                }
            }

            SpellCastResult Check()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    Player* target = GetExplTargetUnit() ? GetExplTargetUnit()->ToPlayer() : nullptr;
                    if (!target || target == _player || target->getClass() == CLASS_DRUID || !target->IsInSameRaidWith(_player))
                        return SPELL_FAILED_UNKNOWN;
                }
                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_symbiosis_SpellScript::Check);
                OnEffectHitTarget += SpellEffectFn(spell_dru_symbiosis_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_symbiosis_SpellScript();
        }
};

// Moonfire - 8921
class spell_dru_moonfire : public SpellScriptLoader
{
    public:
        spell_dru_moonfire() : SpellScriptLoader("spell_dru_moonfire") { }

        class spell_dru_moonfire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_moonfire_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(SPELL_DRUID_CELESTIAL_ALIGNMENT))
                            _player->CastSpell(target, SPELL_DRUID_SUNFIRE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_moonfire_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_moonfire_SpellScript();
        }
};

// Cenarion Ward - 102351
class spell_dru_cenarion_ward : public SpellScriptLoader
{
    public:
        spell_dru_cenarion_ward() : SpellScriptLoader("spell_dru_cenarion_ward") { }

        class spell_dru_cenarion_ward_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_cenarion_ward_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetTarget())
                    {
                        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                        if (removeMode == AURA_REMOVE_BY_DEFAULT)
                            caster->CastSpell(target, SPELL_DRUID_CENARION_WARD, true);

                        if (caster->HasAura(SPELL_DRUID_CENARION_WARD) && caster->GetGUID() != target->GetGUID())
                            caster->RemoveAura(SPELL_DRUID_CENARION_WARD);
                    }
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_cenarion_ward_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_cenarion_ward_AuraScript();
        }
};

// Ursol's Vortex (snare) - 127797
class spell_dru_ursols_vortex_snare : public SpellScriptLoader
{
    public:
        spell_dru_ursols_vortex_snare() : SpellScriptLoader("spell_dru_ursols_vortex_snare") { }

        class spell_dru_ursols_vortex_snare_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_ursols_vortex_snare_AuraScript);

            std::list<Unit*> targetList;

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                aurEff->GetTargetList(targetList);

                for (auto itr : targetList)
                {
                    if (Unit* caster = GetCaster())
                        if (AreaTrigger* areaTrigger = caster->GetAreaTrigger(SPELL_DRUID_URSOLS_VORTEX_AREA_TRIGGER))
                            if (itr->GetDistance(areaTrigger) > 8.0f && !itr->HasAura(SPELL_DRUID_URSOLS_VORTEX_JUMP_DEST))
                                itr->CastSpell(areaTrigger->GetPositionX(), areaTrigger->GetPositionY(), areaTrigger->GetPositionZ(), SPELL_DRUID_URSOLS_VORTEX_JUMP_DEST, true);
                }

                targetList.clear();
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_dru_ursols_vortex_snare_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_ursols_vortex_snare_AuraScript();
        }
};

// Ursol's Vortex - 102793
class spell_dru_ursols_vortex : public SpellScriptLoader
{
    public:
        spell_dru_ursols_vortex() : SpellScriptLoader("spell_dru_ursols_vortex") { }

        class spell_dru_ursols_vortex_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_ursols_vortex_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (!target->HasAura(SPELL_DRUID_URSOLS_VORTEX_AREA_TRIGGER))
                            _player->CastSpell(target, SPELL_DRUID_URSOLS_VORTEX_SNARE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_ursols_vortex_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_ursols_vortex_SpellScript();
        }

        class spell_dru_ursols_vortex_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_ursols_vortex_AuraScript);

            std::list<Unit*> targetList;

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                aurEff->GetTargetList(targetList);

                for (auto itr : targetList)
                {
                    if (Unit* caster = GetCaster())
                        if (DynamicObject* dynObj = caster->GetDynObject(SPELL_DRUID_URSOLS_VORTEX_AREA_TRIGGER))
                            if (itr->GetDistance(dynObj) > 8.0f && !itr->HasAura(SPELL_DRUID_URSOLS_VORTEX_JUMP_DEST))
                                itr->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), SPELL_DRUID_URSOLS_VORTEX_JUMP_DEST, true);
                }

                targetList.clear();
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_dru_ursols_vortex_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_ursols_vortex_AuraScript();
        }
};

// Solar beam - 78675
class spell_dru_solar_beam : public SpellScriptLoader
{
    public:
        spell_dru_solar_beam() : SpellScriptLoader("spell_dru_solar_beam") { }

        class spell_dru_solar_beam_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_solar_beam_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_DRUID_SOLAR_BEAM))
                    GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), SPELL_DRUID_SOLAR_BEAM_SILENCE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_solar_beam_AuraScript::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_solar_beam_AuraScript();
        }
};

// Dash - 1850
class spell_dru_dash : public SpellScriptLoader
{
    public:
        spell_dru_dash() : SpellScriptLoader("spell_dru_dash") { }

        class spell_dru_dash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_dash_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_DRUID_STAMPEDING_ROAR))
                        _player->RemoveAura(SPELL_DRUID_STAMPEDING_ROAR);

                    _player->RemoveMovementImpairingAuras();
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_dash_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_dash_SpellScript();
        }
};

// Called by Mangle (bear) - 33878, Mangle (cat) - 33876, Ravage - 6785 and Shred - 5221 (Shred - 114236)
// Rip - 1079
class spell_dru_rip_duration : public SpellScriptLoader
{
    public:
        spell_dru_rip_duration() : SpellScriptLoader("spell_dru_rip_duration") { }

        class spell_dru_rip_duration_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_rip_duration_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // Each time you Shred, Ravage, or Mangle the target while in Cat Form ...
                        if (_player->GetShapeshiftForm() == FORM_CAT)
                        {
                            if (Aura* rip = target->GetAura(SPELL_DRUID_RIP, _player->GetGUID()))
                            {
                                int32 duration = rip->GetDuration();
                                int32 maxDuration = rip->GetMaxDuration();

                                int32 countMin = maxDuration;
                                int32 countMax = sSpellMgr->GetSpellInfo(SPELL_DRUID_RIP)->GetMaxDuration() + 6000;

                                // ... the duration of your Rip on that target is extended by 2 sec, up to a maximum of 6 sec.
                                if ((countMin + 2000) < countMax)
                                {
                                    rip->SetDuration(duration + 2000);
                                    rip->SetMaxDuration(countMin + 2000);
                                }
                                else if (countMin < countMax)
                                {
                                    rip->SetDuration(duration + 2000);
                                    rip->SetMaxDuration(countMax);
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_rip_duration_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_rip_duration_SpellScript();
        }
};

// Savage Defense - 62606
class spell_dru_savage_defense : public SpellScriptLoader
{
    public:
        spell_dru_savage_defense() : SpellScriptLoader("spell_dru_savage_defense") { }

        class spell_dru_savage_defense_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_savage_defense_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, SPELL_DRUID_SAVAGE_DEFENSE_DODGE_PCT, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_savage_defense_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_savage_defense_SpellScript();
        }
};

// Bear Form - 5487
class spell_dru_bear_form : public SpellScriptLoader
{
    public:
        spell_dru_bear_form() : SpellScriptLoader("spell_dru_bear_form") { }

        class spell_dru_bear_form_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_bear_form_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    _player->SetPower(POWER_RAGE, 0);
                    _player->CastSpell(_player, SPELL_DRUID_BEAR_FORM_RAGE_GAIN, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_bear_form_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_bear_form_SpellScript();
        }

        class spell_dru_bear_form_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_bear_form_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(SPELL_DRUID_MIGHT_OF_URSOC))
                        caster->RemoveAura(SPELL_DRUID_MIGHT_OF_URSOC);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_bear_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_bear_form_AuraScript();
        }
};

// Ferocious Bite - 22568
class spell_dru_ferocious_bite : public SpellScriptLoader
{
    public:
        spell_dru_ferocious_bite() : SpellScriptLoader("spell_dru_ferocious_bite") { }

        class spell_dru_ferocious_bite_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_ferocious_bite_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (target->GetHealthPct() < 25.0f)
                            if (Aura* rip = target->GetAura(SPELL_DRUID_RIP, _player->GetGUID()))
                                rip->RefreshDuration();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_ferocious_bite_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_ferocious_bite_SpellScript();
        }
};

// Bear Hug - 102795
class spell_dru_bear_hug : public SpellScriptLoader
{
    public:
        spell_dru_bear_hug() : SpellScriptLoader("spell_dru_bear_hug") { }

        class spell_dru_bear_hug_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_bear_hug_AuraScript);

            bool CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetShapeshiftForm() != FORM_BEAR)
                        caster->CastSpell(caster, SPELL_DRUID_BEAR_FORM, true);

                    amount = caster->CountPctFromMaxHealth(amount);
                    return true;
                }
                return false;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_bear_hug_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_bear_hug_AuraScript();
        }
};

// Ravage - 6785, Ravage (feral) - 102545
class spell_dru_ravage : public SpellScriptLoader
{
    public:
        spell_dru_ravage() : SpellScriptLoader("spell_dru_ravage") { }

        class spell_dru_ravage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_ravage_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                    {
                        // If target doesn't have Deterrence
                        if (!target->HasAura(19263))
                            _player->CastSpell(target, SPELL_DRUID_INFECTED_WOUNDS, true);

                        if (_player->HasAura(SPELL_DRUID_STAMPEDE))
                            _player->RemoveAura(SPELL_DRUID_STAMPEDE);
                    }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_ravage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_ravage_SpellScript();
        }
};

// Lifebloom - 33763 : Final heal
class spell_dru_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_lifebloom() : SpellScriptLoader("spell_dru_lifebloom") { }

        class spell_dru_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_lifebloom_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                MakeFinalHeal();
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // Final heal only on duration end
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                MakeFinalHeal();
            }

            void MakeFinalHeal()
            {
                auto aura = GetAura();
                if (!aura || !aura->GetOwner() || !GetCaster() ||  !GetCaster()->ToPlayer())
                    return;

                Unit* target = aura->GetOwner()->ToUnit();
                if (GetCaster()->ToPlayer()->HasSpellCooldown(SPELL_DRUID_LIFEBLOOM_FINAL_HEAL))
                    return;

                AuraEffect const* aurEff = aura->GetEffect(EFFECT_1);
                if (!aurEff)
                    return;

                int32 healAmount = aurEff->GetAmount();

                // Increase final heal by 50%
                if (GetCaster()->HasAura(SPELL_DRUID_GLYPH_OF_BLOOMING))
                    AddPct(healAmount, 50);

                target->CastCustomSpell(target, SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, &healAmount, nullptr, nullptr, true, nullptr, aurEff, GetCasterGUID());
                GetCaster()->ToPlayer()->AddSpellCooldown(SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, 0, 1 * IN_MILLISECONDS);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_lifebloom_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterDispel += AuraDispelFn(spell_dru_lifebloom_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_lifebloom_AuraScript();
        }
};

// Called by Cat Form - 768 and Bear Form - 5487
// Killer Instinct - 108299
class spell_dru_killer_instinct : public SpellScriptLoader
{
    public:
        spell_dru_killer_instinct() : SpellScriptLoader("spell_dru_killer_instinct") { }

        class spell_dru_killer_instinct_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_killer_instinct_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->HasAura(SPELL_DRUID_KILLER_INSTINCT))
                        _player->CastSpell(_player, SPELL_DRUID_KILLER_INSTINCT_MOD_STAT, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_killer_instinct_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_killer_instinct_SpellScript();
        }
};

// Called by Regrowth - 8936, Nourish - 50464 and Healing Touch - 5185
// Lifebloom - 33763 : Refresh duration
class spell_dru_lifebloom_refresh : public SpellScriptLoader
{
    public:
        spell_dru_lifebloom_refresh() : SpellScriptLoader("spell_dru_lifebloom_refresh") { }

        class spell_dru_lifebloom_refresh_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_lifebloom_refresh_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (!target->HasAura(SPELL_DRUID_GLYPH_OF_BLOOMING))
                            if (Aura* lifebloom = target->GetAura(SPELL_DRUID_LIFEBLOOM, _player->GetGUID()))
                                lifebloom->RefreshDuration();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_lifebloom_refresh_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_lifebloom_refresh_SpellScript();
        }
};

// Called by Lifebloom - 33763
// Omen of Clarity - 113043
class spell_dru_omen_of_clarity : public SpellScriptLoader
{
    public:
        spell_dru_omen_of_clarity() : SpellScriptLoader("spell_dru_omen_of_clarity") { }

        class spell_dru_omen_of_clarity_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_omen_of_clarity_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_DRUID_OMEN_OF_CLARITY))
                        if (roll_chance_i(4))
                            caster->CastSpell(caster, SPELL_DRUID_CLEARCASTING, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_omen_of_clarity_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_omen_of_clarity_AuraScript();
        }
};

// Mark of the Wild - 1126
class spell_dru_mark_of_the_wild : public SpellScriptLoader
{
    public:
        spell_dru_mark_of_the_wild() : SpellScriptLoader("spell_dru_mark_of_the_wild") { }

        class spell_dru_mark_of_the_wild_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_mark_of_the_wild_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                {
                    caster->AddAura(SPELL_DRUID_MARK_OF_THE_WILD, caster);

                    std::list<Unit*> memberList;
                    Player* plr = caster->ToPlayer();
                    plr->GetPartyMembers(memberList);

                    for (auto itr : memberList)
                        caster->AddAura(SPELL_DRUID_MARK_OF_THE_WILD, (itr));
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_mark_of_the_wild_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_mark_of_the_wild_SpellScript();
        }
};

// Nature's Cure - 88423 and Remove Corruption - 2782
class spell_dru_natures_cure : public SpellScriptLoader
{
    public:
        spell_dru_natures_cure() : SpellScriptLoader("spell_dru_natures_cure") { }

        class spell_dru_natures_cure_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_natures_cure_SpellScript);

            SpellCastResult CheckCleansing()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        DispelChargesList dispelList[MAX_SPELL_EFFECTS];

                        // Create dispel mask by dispel type
                        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        {
                            uint32 dispel_type = GetSpellInfo()->Effects[i].MiscValue;
                            uint32 dispelMask  = GetSpellInfo()->GetDispelMask(DispelType(dispel_type));

                            // Nature's Cure can dispell all Magic, Curse and poison
                            if (GetSpellInfo()->Id == 88423)
                                dispelMask = ((1<<DISPEL_MAGIC) | (1<<DISPEL_CURSE) | (1<<DISPEL_POISON));

                            // Remove Corruption can dispell all curse and poison
                            if (GetSpellInfo()->Id == 2782)
                                dispelMask = ((1<<DISPEL_CURSE) | (1<<DISPEL_POISON));

                            target->GetDispellableAuraList(caster, dispelMask, dispelList[i]);
                        }

                        bool empty = true;
                        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        {
                            if (dispelList[i].empty())
                                continue;

                            empty = false;
                            break;
                        }

                        if (empty)
                            return SPELL_FAILED_NOTHING_TO_DISPEL;

                        return SPELL_CAST_OK;
                    }
                }

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_natures_cure_SpellScript::CheckCleansing);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_natures_cure_SpellScript();
        }
};

// Called by Regrowth - 8936
// Glyph of Regrowth - 116218
class spell_dru_glyph_of_regrowth : public SpellScriptLoader
{
    public:
        spell_dru_glyph_of_regrowth() : SpellScriptLoader("spell_dru_glyph_of_regrowth") { }

        class spell_dru_glyph_of_regrowth_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_glyph_of_regrowth_AuraScript);

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                // Increases the critical strike chance of your Regrowth by 40%, but removes the periodic component of the spell.
                if (GetCaster())
                    if (GetCaster()->HasAura(SPELL_DRUID_GLYPH_OF_REGROWTH))
                        GetTarget()->RemoveAura(SPELL_DRUID_REGROWTH, GetCaster()->GetGUID());
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                // Duration automatically refreshes to 6 sec each time Regrowth heals targets at or below 50% health
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetTarget())
                        if (target->GetHealthPct() < 50)
                            if (Aura* regrowth = target->GetAura(SPELL_DRUID_REGROWTH, caster->GetGUID()))
                                regrowth->RefreshDuration();
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_glyph_of_regrowth_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_glyph_of_regrowth_AuraScript::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_glyph_of_regrowth_AuraScript();
        }
};

// Cat Form - 768
class spell_dru_cat_form : public SpellScriptLoader
{
    public:
        spell_dru_cat_form() : SpellScriptLoader("spell_dru_cat_form") { }

        class spell_dru_cat_form_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_cat_form_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (!_player->HasAura(SPELL_DRUID_FORM_CAT_INCREASE_SPEED))
                    {
                        _player->CastSpell(_player, SPELL_DRUID_FORM_CAT_INCREASE_SPEED, true);
                        _player->RemoveMovementImpairingAuras();
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_cat_form_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_cat_form_SpellScript();
        }

        class spell_dru_cat_form_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_cat_form_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Aura* dash = caster->GetAura(SPELL_DRUID_DASH))
                        if (dash->GetEffect(0))
                            if (dash->GetEffect(0)->GetAmount() == 0)
                                dash->GetEffect(0)->ChangeAmount(70);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* dash = caster->GetAura(SPELL_DRUID_DASH))
                        dash->GetEffect(0)->ChangeAmount(0);

                    if (caster->HasAura(SPELL_DRUID_PROWL))
                        caster->RemoveAura(SPELL_DRUID_PROWL);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_cat_form_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_cat_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_cat_form_AuraScript();
        }
};

// Skull Bash (cat) - 80965 and Skull Bash (bear) - 80964
class spell_dru_skull_bash : public SpellScriptLoader
{
    public:
        spell_dru_skull_bash() : SpellScriptLoader("spell_dru_skull_bash") { }

        class spell_dru_skull_bash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_skull_bash_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->CastSpell(target, SPELL_DRUID_SKULL_BASH_MANA_COST, true);
                        _player->CastSpell(target, SPELL_DRUID_SKULL_BASH_INTERUPT, true);
                        _player->CastSpell(target, SPELL_DRUID_SKULL_BASH_CHARGE, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_skull_bash_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_skull_bash_SpellScript();
        }
};

// Faerie Swarm - 102355
class spell_dru_faerie_swarm : public SpellScriptLoader
{
    public:
        spell_dru_faerie_swarm() : SpellScriptLoader("spell_dru_faerie_swarm") { }

        class spell_dru_faerie_swarm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_faerie_swarm_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->CastSpell(target, SPELL_DRUID_FAERIE_DECREASE_SPEED, true);
                        _player->CastSpell(target, SPELL_DRUID_WEAKENED_ARMOR, true);
                        _player->CastSpell(target, SPELL_DRUID_WEAKENED_ARMOR, true);
                        _player->CastSpell(target, SPELL_DRUID_WEAKENED_ARMOR, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_faerie_swarm_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_faerie_swarm_SpellScript();
        }
};

// Wild Mushroom (Heal effect with growing) - 102792
class spell_dru_wild_mushroom_heal : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_heal() : SpellScriptLoader("spell_dru_wild_mushroom_heal") { }

        class spell_dru_wild_mushroom_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_heal_SpellScript);

            uint8 count;

            bool Load()
            {
                count = 0;
                return true;
            }

            void HandleTargets(std::list<WorldObject*>& targets)
            {
                Unit* caster = GetCaster();

                targets.remove_if([caster](WorldObject* obj) -> bool { return obj == caster; });
                count = targets.size();
            }

            void HandleHeal()
            {
                if (!count)
                    return;

                if (Unit* mushroom = GetCaster())
                {
                    if (AuraEffect* growing = mushroom->GetAuraEffect(SPELL_DRUID_WILD_MUSHROOM_MOD_SCALE, EFFECT_1))
                    {
                        int32 bonus = growing->GetAmount() / count;
                        SetHitHeal(GetHitHeal() + bonus);
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_wild_mushroom_heal_SpellScript::HandleTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
                OnHit += SpellHitFn(spell_dru_wild_mushroom_heal_SpellScript::HandleHeal);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_heal_SpellScript();
        }
};

// Wild Mushroom (Growing effect) - 138611
class spell_dru_wild_mushroom_growing : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_growing() : SpellScriptLoader("spell_dru_wild_mushroom_growing") { }

        class spell_dru_wild_mushroom_growing_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_wild_mushroom_growing_AuraScript);

            uint32 currAmount;

            bool Load()
            {
                currAmount = 0;
                return true;
            }

            bool CalculateAmount(AuraEffect const* /*aurEff*/, int32 &amount, bool & /*canBeRecalculated*/)
            {
                // Max amount : 200% of caster's health
                amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
                return true;
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* target = eventInfo.GetActor();
                if (!target)
                    return;

                uint32 overHeal = eventInfo.GetHealInfo()->GetHeal();
                uint32 maxAmount = aurEff->GetAmount();
                
                float mod = 1.0f;
                if (GetUnitOwner()->HasAura(134735))
                    AddPct(mod, -60);

                if (auto auraEffect = GetUnitOwner()->GetAuraEffect(110310, EFFECT_0))
                {
                    if (auto effAmount = auraEffect->GetAmount())
                        AddPct(mod, -effAmount);
                }

                maxAmount *= mod;

                currAmount += overHeal;
                currAmount = std::min(currAmount, maxAmount);
                if (currAmount == maxAmount && GetUnitOwner())
                    GetUnitOwner()->CastSpell(GetUnitOwner(), SPELL_DRUID_WILD_MUSHROOM_VISUAL, true);

                int32 newPct = float(currAmount) / float(maxAmount) * 100.0f;
                int32 bp2 = currAmount;

                std::list<Creature*> tempList;

                target->GetCreatureListWithEntryInGrid(tempList, DRUID_NPC_WILD_MUSHROOM, 200.0f);

                // Remove other players mushrooms
                for (std::list<Creature*>::iterator i = tempList.begin(); i != tempList.end();)
                {
                    Unit* owner = (*i)->GetOwner();
                    if (owner && owner == target && (*i)->isSummon())
                    {
                        ++i;
                        continue;
                    }

                    i = tempList.erase(i);
                }

                if (tempList.empty() || tempList.size() > 1)
                    return;

                Creature* mushroom = tempList.back();
                mushroom->RemoveAura(SPELL_DRUID_WILD_MUSHROOM_MOD_SCALE);
                target->CastCustomSpell(mushroom, SPELL_DRUID_WILD_MUSHROOM_MOD_SCALE, &newPct, &bp2, NULL, true);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_wild_mushroom_growing_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_DUMMY);
                OnEffectProc += AuraEffectProcFn(spell_dru_wild_mushroom_growing_AuraScript::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_wild_mushroom_growing_AuraScript();
        }
};

// Wild Mushroom (Restoration) - 145205, Wild Mushroom (Restoration, with Glyph Glyph of the Sprouting Mushroom - 146654) - 147349
class spell_dru_wild_mushroom_resto : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_resto() : SpellScriptLoader("spell_dru_wild_mushroom_resto") { }

        class spell_dru_wild_mushroom_resto_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_resto_SpellScript)

            void HandleSummon(SpellEffIndex effIndex)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    PreventHitDefaultEffect(effIndex);

                    const SpellInfo* spell = GetSpellInfo();
                    std::list<Creature*> tempList;

                    player->GetCreatureListWithEntryInGrid(tempList, DRUID_NPC_WILD_MUSHROOM, 200.0f);

                    // Remove other players mushrooms
                    for (std::list<Creature*>::iterator i = tempList.begin(); i != tempList.end();)
                    {
                        Unit* owner = (*i)->GetOwner();
                        if (owner && owner == player && (*i)->isSummon())
                        {
                            ++i;
                            continue;
                        }

                        i = tempList.erase(i);
                    }

                    // 1 mushrooms max
                    if ((int32)tempList.size() >= spell->Effects[effIndex].BasePoints)
                    {
                        Creature* mushroom = tempList.back();

                        // Recasting Wild Mushroom will move the Mushroom without losing this accumulated healing.
                        if (WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest()))
                        {
                            mushroom->CastSpell(mushroom, DRUID_SPELL_WILD_MUSHROOM_DEATH_VISUAL, true);
                            mushroom->NearTeleportTo(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ(), mushroom->GetOrientation());
                            mushroom->CastSpell(mushroom, DRUID_SPELL_MUSHROOM_BIRTH_VISUAL, true);

                            if (player->HasAura(SPELL_DRUID_GLYPH_OF_EFFLORESCENCE))
                            {
                                if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_DRUID_SWIFTMEND))
                                    dynObj->Remove();

                                player->CastSpell(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ(), 81262, true);
                            }

                            return;
                        }
                    }

                    Position pos;
                    GetExplTargetDest()->GetPosition(&pos);
                    const SummonPropertiesEntry* properties = sSummonPropertiesStore.LookupEntry(spell->Effects[effIndex].MiscValueB);
                    TempSummon* summon = player->SummonCreature(spell->Effects[effIndex].MiscValue, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, spell->GetDuration());
                    if (!summon)
                        return;

                    summon->SetByteValue(UNIT_FIELD_BYTES_2, 1, player->GetByteValue(UNIT_FIELD_BYTES_2, 1));
                    summon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, player->GetGUID());
                    summon->setFaction(player->getFaction());
                    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellInfo()->Id);
                    summon->SetMaxHealth(5);
                    summon->SetFullHealth();
                    summon->CastSpell(summon, DRUID_SPELL_MUSHROOM_BIRTH_VISUAL, true); // Wild Mushroom : Detonate Birth Visual
                    player->CastSpell(player, SPELL_DRUID_WILD_MUSHROOM_GROWING, true);

                    if (player->HasAura(SPELL_DRUID_GLYPH_OF_EFFLORESCENCE))
                        player->CastSpell(summon, SPELL_DRUID_SWIFTMEND, true);
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_dru_wild_mushroom_resto_SpellScript::HandleSummon, EFFECT_1, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_resto_SpellScript();
        }
};

// Wild Mushroom - 88747
class spell_dru_wild_mushroom : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom() : SpellScriptLoader("spell_dru_wild_mushroom") { }

        class spell_dru_wild_mushroom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_SpellScript)

            void HandleSummon(SpellEffIndex effIndex)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    PreventHitDefaultEffect(effIndex);

                    const SpellInfo* spell = GetSpellInfo();
                    std::list<Creature*> tempList;

                    player->GetCreatureListWithEntryInGrid(tempList, DRUID_NPC_WILD_MUSHROOM, 200.0f);

                    // Remove other players mushrooms
                    for (std::list<Creature*>::iterator i = tempList.begin(); i != tempList.end();)
                    {
                        Unit* owner = (*i)->GetOwner();
                        if (owner && owner == player && (*i)->isSummon())
                        {
                            ++i;
                            continue;
                        }

                        i = tempList.erase(i);
                    }

                    // 3 mushrooms max
                    if ((int32)tempList.size() >= spell->Effects[effIndex].BasePoints)
                        tempList.back()->ToTempSummon()->UnSummon();

                    Position pos;
                    GetExplTargetDest()->GetPosition(&pos);
                    const SummonPropertiesEntry* properties = sSummonPropertiesStore.LookupEntry(spell->Effects[effIndex].MiscValueB);
                    TempSummon* summon = player->SummonCreature(spell->Effects[effIndex].MiscValue, pos, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, spell->GetDuration());
                    if (!summon)
                        return;

                    summon->SetByteValue(UNIT_FIELD_BYTES_2, 1, player->GetByteValue(UNIT_FIELD_BYTES_2, 1));
                    summon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, player->GetGUID());
                    summon->setFaction(player->getFaction());
                    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellInfo()->Id);
                    summon->SetMaxHealth(5);
                    summon->CastSpell(summon, DRUID_SPELL_MUSHROOM_BIRTH_VISUAL, true); // Wild Mushroom : Detonate Birth Visual
                }
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_dru_wild_mushroom_SpellScript::HandleSummon, EFFECT_1, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_SpellScript();
        }
};

// Wild Mushroom : Detonate - 88751
class spell_dru_wild_mushroom_detonate : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_detonate() : SpellScriptLoader("spell_dru_wild_mushroom_detonate") { }

        class spell_dru_wild_mushroom_detonate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_detonate_SpellScript)

            // Globals variables
            float spellRange;
            std::list<uint64> mushroomList;

            bool Load()
            {
                spellRange = GetSpellInfo()->GetMaxRange(true);

                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return false;

                std::list<Creature*> list;
                player->GetCreatureListWithEntryInGrid(list, DRUID_NPC_WILD_MUSHROOM, 50.0f);

                for (std::list<Creature*>::const_iterator i = list.begin(); i != list.end(); ++i)
                {
                    Unit* owner = (*i)->GetOwner();
                    if (owner && owner == player && (*i)->isSummon())
                    {
                        mushroomList.push_back((*i)->GetGUID());
                        continue;
                    }
                }

                if (!spellRange)
                    return false;

                return true;
            }

            SpellCastResult CheckCast()
            {
                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return SPELL_FAILED_CASTER_DEAD;

                if (mushroomList.empty())
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                bool inRange = false;

                for (std::list<uint64>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); ++i)
                {
                    Creature* mushroom = player->GetMap()->GetCreature(*i);
                    if (!mushroom)
                        continue;
                    Position shroomPos;
                    mushroom->GetPosition(&shroomPos);
                    if (player->IsWithinDist3d(&shroomPos, spellRange)) // Must have at least one mushroom within 40 yards
                    {
                        inRange = true;
                        break;
                    }
                }

                if (!inRange)
                    return SPELL_FAILED_DONT_REPORT;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    for (std::list<uint64>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); ++i)
                    {
                        Creature* mushroom = player->GetMap()->GetCreature(*i);
                        if (!mushroom)
                            continue;
                        Position shroomPos;
                        mushroom->GetPosition(&shroomPos);
                        if (!player->IsWithinDist3d(&shroomPos, spellRange))
                            continue;

                        mushroom->SetVisible(true);

                        player->CastSpell(mushroom, DRUID_SPELL_WILD_MUSHROOM_DAMAGE, true);    // Damage

                        player->CastSpell(mushroom, DRUID_SPELL_FUNGAL_GROWTH_SUMMON, true);    // Fungal Growth

                        mushroom->CastSpell(mushroom, DRUID_SPELL_WILD_MUSHROOM_DEATH_VISUAL, true);// Explosion visual
                        mushroom->CastSpell(mushroom, DRUID_SPELL_WILD_MUSHROOM_SUICIDE, true);     // Suicide
                        mushroom->DespawnOrUnsummon(500);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_wild_mushroom_detonate_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_dru_wild_mushroom_detonate_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_detonate_SpellScript();
        }
};

// Wild Mushroom : Bloom - 102791
class spell_dru_wild_mushroom_bloom : public SpellScriptLoader
{
    public:
        spell_dru_wild_mushroom_bloom() : SpellScriptLoader("spell_dru_wild_mushroom_bloom") { }

        class spell_dru_wild_mushroom_bloom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_wild_mushroom_bloom_SpellScript)

            // Globals variables
            float spellRange;
            std::list<uint64> mushroomList;

            bool Load()
            {
                mushroomList.clear();
                spellRange = GetSpellInfo()->GetMaxRange(true);

                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return false;

                std::list<Creature*> list;
                player->GetCreatureListWithEntryInGrid(list, DRUID_NPC_WILD_MUSHROOM, spellRange*2.0f);

                for (std::list<Creature*>::const_iterator i = list.begin(); i != list.end(); ++i)
                {
                    Unit* owner = (*i)->GetOwner();
                    if (owner && owner == player && (*i)->isSummon())
                    {
                        mushroomList.push_back((*i)->GetGUID());
                        continue;
                    }
                }

                if (!spellRange)
                    return false;

                return true;
            }

            SpellCastResult CheckCast()
            {
                Player* player = GetCaster()->ToPlayer();
                if (!player)
                    return SPELL_FAILED_CASTER_DEAD;

                if (mushroomList.empty())
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                bool inRange = false;

                for (std::list<uint64>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); ++i)
                {
                    Creature* mushroom = player->GetMap()->GetCreature(*i);
                    if (!mushroom)
                        continue;
                    Position shroomPos;
                    mushroom->GetPosition(&shroomPos);
                    if (player->IsWithinDist3d(&shroomPos, spellRange)) // Must have at least one mushroom within 40 yards
                    {
                        inRange = true;
                        break;
                    }
                }

                if (!inRange)
                    return SPELL_FAILED_CUSTOM_ERROR;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    player->RemoveAura(SPELL_DRUID_WILD_MUSHROOM_VISUAL);
                    for (std::list<uint64>::const_iterator i = mushroomList.begin(); i != mushroomList.end(); ++i)
                    {
                        Creature* mushroom = player->GetMap()->GetCreature(*i);
                        if (!mushroom)
                            continue;
                        Position shroomPos;
                        mushroom->GetPosition(&shroomPos);
                        if (!player->IsWithinDist3d(&shroomPos, spellRange))
                            continue;

                        if (player->HasAura(SPELL_DRUID_GLYPH_OF_EFFLORESCENCE))
                        {
                            if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_DRUID_SWIFTMEND))
                                dynObj->Remove();
                            mushroom->RemoveAura(SPELL_DRUID_SWIFTMEND);
                        }

                        mushroom->CastSpell(mushroom, DRUID_SPELL_WILD_MUSHROOM_SUICIDE, true); // Explosion visual and suicide
                        mushroom->CastSpell(mushroom, SPELL_DRUID_WILD_MUSHROOM_HEAL, true, NULL, nullptr, player->GetGUID()); // heal
                        mushroom->RemoveDynObject(SPELL_DRUID_SWIFTMEND);
                        mushroom->RemoveAura(SPELL_DRUID_SWIFTMEND);
                        player->RemoveAura(SPELL_DRUID_WILD_MUSHROOM_GROWING);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_wild_mushroom_bloom_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_dru_wild_mushroom_bloom_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_wild_mushroom_bloom_SpellScript();
        }
};

// Swiftmend (heal) - 81269
class spell_dru_swiftmend_heal : public SpellScriptLoader
{
    public:
        spell_dru_swiftmend_heal() : SpellScriptLoader("spell_dru_swiftmend_heal") { }

        class spell_dru_swiftmend_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_swiftmend_heal_SpellScript);

            void CorrectTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;
                
                Unit* caster = GetCaster();
                targets.remove_if([caster](WorldObject* obj) -> bool { return obj == caster; });

                std::list<Unit*> unitList;

                for (auto itr : targets)
                    if (itr->ToUnit())
                        unitList.push_back(itr->ToUnit());

                targets.clear();

                unitList.sort(JadeCore::HealthPctOrderPred());
                unitList.resize(3);

                for (auto itr : unitList)
                    targets.push_back(itr);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_swiftmend_heal_SpellScript::CorrectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_swiftmend_heal_SpellScript();
        }
};

// Dream of cenarius (heal) - 145153 
class spell_dru_dream_of_cenarius_heal : public SpellScriptLoader
{
    public:
        spell_dru_dream_of_cenarius_heal() : SpellScriptLoader("spell_dru_dream_of_cenarius_heal") { }

        class spell_dru_dream_of_cenarius_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_dream_of_cenarius_heal_SpellScript);

            void CorrectTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                std::list<Unit*> unitList;

                for (auto itr : targets)
                    if (itr->ToPlayer())
                        unitList.push_back(itr->ToUnit());

                targets.clear();

                unitList.sort(JadeCore::HealthPctOrderPred());
                unitList.resize(1);

                for (auto itr : unitList)
                    targets.push_back(itr);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_dream_of_cenarius_heal_SpellScript::CorrectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_dream_of_cenarius_heal_SpellScript();
        }
};

// Swiftmend - 81262
class spell_dru_swiftmend : public SpellScriptLoader
{
    public:
        spell_dru_swiftmend() : SpellScriptLoader("spell_dru_swiftmend") { }

        class spell_dru_swiftmend_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_swiftmend_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_DRUID_SWIFTMEND))
                {
                    GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), SPELL_DRUID_SWIFTMEND_TICK, true);

                    Aura* swiftmendAura = GetAura();
                    if (swiftmendAura && GetCaster()->HasAura(145529))
                        swiftmendAura->SetDuration(8000);
                }
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                        return;

                    if (!caster->ToPlayer() && caster->IsInWorld() && caster->isAlive())
                        caster->CastSpell(caster, SPELL_DRUID_SWIFTMEND, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_swiftmend_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_swiftmend_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_swiftmend_AuraScript();
        }
};

// Astral Communion - 127663
class spell_dru_astral_communion : public SpellScriptLoader
{
    public:
        spell_dru_astral_communion() : SpellScriptLoader("spell_dru_astral_communion") { }

        class spell_dru_astral_communion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_astral_communion_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetTarget()->ToPlayer())
                {
                    int32 eclipse = 25; // 25 Solar or Lunar energy

                    if (_player->HasAura(SPELL_DRUID_ASTRAL_INSIGHT))
                        eclipse = 100;
                    
                    // We can't reach eclipse power while we're with Celestial Alignment
                    if (_player->HasAura(SPELL_DRUID_CELESTIAL_ALIGNMENT))
                        eclipse = 0;

                    // Give Solar energy just if our last eclipse was lunar else, give Lunar energy
                    if (_player->GetLastEclipsePower() == SPELL_DRUID_LUNAR_ECLIPSE)
                        _player->SetEclipsePower(int32(_player->GetEclipsePower() + eclipse));
                    else
                        _player->SetEclipsePower(int32(_player->GetEclipsePower() - eclipse));

                    if (_player->HasAura(SPELL_DRUID_ASTRAL_INSIGHT))
                    {
                        _player->CastStop();
                        _player->RemoveAura(SPELL_DRUID_ASTRAL_INSIGHT);
                    }

                    if (_player->GetEclipsePower() == 100 && !_player->HasAura(SPELL_DRUID_SOLAR_ECLIPSE))
                    {
                        _player->CastSpell(_player, SPELL_DRUID_SOLAR_ECLIPSE, true, 0); // Cast Solar Eclipse
                        _player->CastSpell(_player, SPELL_DRUID_NATURES_GRACE, true); // Cast Nature's Grace
                        _player->CastSpell(_player, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, true); // Cast Eclipse - Give 35% of POWER_MANA
                        _player->CastStop();

                        // Now our last eclipse is Solar
                        _player->SetLastEclipsePower(SPELL_DRUID_SOLAR_ECLIPSE);
                    }
                    else if (_player->GetEclipsePower() == -100 && !_player->HasAura(SPELL_DRUID_LUNAR_ECLIPSE))
                    {
                        _player->CastSpell(_player, SPELL_DRUID_LUNAR_ECLIPSE, true, 0); // Cast Lunar Eclipse
                        _player->CastSpell(_player, SPELL_DRUID_NATURES_GRACE, true); // Cast Nature's Grace
                        _player->CastSpell(_player, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, true); // Cast Eclipse - Give 35% of POWER_MANA
                        _player->CastSpell(_player, SPELL_DRUID_LUNAR_ECLIPSE_OVERRIDE, true);
                        _player->CastStop();

                        // Now our last eclipse is Lunar
                        _player->SetLastEclipsePower(SPELL_DRUID_LUNAR_ECLIPSE);

                        if (_player->HasSpellCooldown(SPELL_DRUID_STARFALL))
                            _player->RemoveSpellCooldown(SPELL_DRUID_STARFALL, true);
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_astral_communion_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_astral_communion_AuraScript();
        }
};

// Celestial Alignment - 112071
class spell_dru_celestial_alignment : public SpellScriptLoader
{
    public:
        spell_dru_celestial_alignment() : SpellScriptLoader("spell_dru_celestial_alignment") { }

        class spell_dru_celestial_alignment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_celestial_alignment_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->SetEclipsePower(0);
                        _player->RemoveLastEclipsePower();

                        _player->m_lastEclipseState = ECLIPSE_NONE;
                        _player->CastSpell(_player, SPELL_DRUID_NATURES_GRACE, true); // Cast Nature's Grace
                        _player->CastSpell(_player, SPELL_DRUID_ECLIPSE_GENERAL_ENERGIZE, true); // Cast Eclipse - Give 35% of POWER_MANA
                        _player->RemoveAura(SPELL_DRUID_SOLAR_ECLIPSE);
                        _player->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE);
                        _player->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE_OVERRIDE);

                        if (_player->HasSpellCooldown(SPELL_DRUID_STARFALL))
                            _player->RemoveSpellCooldown(SPELL_DRUID_STARFALL, true);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_celestial_alignment_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_celestial_alignment_SpellScript();
        }

        class spell_dru_celestial_alignment_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_celestial_alignment_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAura(SPELL_DRUID_SOLAR_ECLIPSE);
                    caster->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE);
                    caster->RemoveAura(SPELL_DRUID_NATURES_GRACE);
                    caster->RemoveAura(SPELL_DRUID_LUNAR_ECLIPSE_OVERRIDE);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_celestial_alignment_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_celestial_alignment_AuraScript();
        }
};

// Shooting Stars - 93400
class spell_dru_shooting_stars : public SpellScriptLoader
{
    public:
        spell_dru_shooting_stars() : SpellScriptLoader("spell_dru_shooting_stars") { }

        class spell_dru_shooting_stars_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_shooting_stars_AuraScript);

            void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->RemoveSpellCooldown(SPELL_DRUID_STARSURGE, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_shooting_stars_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_shooting_stars_AuraScript();
        }
};

// Frenzied Regeneration - 22842
class spell_dru_frenzied_regeneration : public SpellScriptLoader
{
    public:
        spell_dru_frenzied_regeneration() : SpellScriptLoader("spell_dru_frenzied_regeneration") { }

        class spell_dru_frenzied_regeneration_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_frenzied_regeneration_SpellScript);

            void HandleOnHit()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (!caster->HasAura(SPELL_DRUID_GLYPH_OF_FRENZIED_REGEN))
                        {
                            int32 rageused = std::min(600, caster->GetPower(POWER_RAGE));
                            int32 AP = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                            int32 agility = caster->GetStat(STAT_AGILITY);
                            int32 stamina = caster->GetStat(STAT_STAMINA);

                            int32 healAmount = std::max(int32(2.2f * (AP - agility * 2)), int32(stamina * 2.5f));
                            healAmount = rageused * healAmount / 600;

                            if (Player* modOwner = caster->GetSpellModOwner())
                                modOwner->ApplySpellMod(GetSpellInfo()->Id, SPELLMOD_DAMAGE, healAmount);

                            healAmount = GetCaster()->SpellHealingBonusTaken(GetCaster(), GetSpellInfo(), EFFECT_0, healAmount, SPELL_DIRECT_DAMAGE);

                            SetHitHeal(healAmount);
                            caster->EnergizeBySpell(caster, 22842, -rageused, POWER_RAGE);
                        }
                        else
                        {
                            SetHitHeal(0);
                            caster->CastSpell(caster, SPELL_DRUID_FRENZIED_REGEN_HEAL_TAKE, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_frenzied_regeneration_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_frenzied_regeneration_SpellScript();
        }
};

// Stampeding Roar - 106898, Stampeding Roar (cat) - 77764 and Stampeding Roar(bear) - 77761
class spell_dru_stampeding_roar_speed : public SpellScriptLoader
{
    public:
        spell_dru_stampeding_roar_speed() : SpellScriptLoader("spell_dru_stampeding_roar_speed") { }

        class spell_dru_stampeding_roar_speed_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_stampeding_roar_speed_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(SPELL_DRUID_DASH))
                        if (_player->HasAura(GetSpellInfo()->Id))
                            _player->RemoveAura(GetSpellInfo()->Id);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_stampeding_roar_speed_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_stampeding_roar_speed_SpellScript();
        }
};

// Stampeding Roar - 97993, Stampeding Roar (cat) - 77764 and Stampeding Roar(bear) - 77761
class spell_dru_stampeding_roar : public SpellScriptLoader
{
    public:
        spell_dru_stampeding_roar() : SpellScriptLoader("spell_dru_stampeding_roar") { }

        class spell_dru_stampeding_roar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_stampeding_roar_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        target->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_stampeding_roar_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_stampeding_roar_SpellScript();
        }
};

// Innervate - 29166
class spell_dru_innervate : public SpellScriptLoader
{
    public:
        spell_dru_innervate() : SpellScriptLoader("spell_dru_innervate") { }

        class spell_dru_innervate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_innervate_AuraScript);

            uint32 regenManaFromSpirit;

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetTarget())
                    {
                        regenManaFromSpirit = uint32(target->GetStat(STAT_SPIRIT) / 2.0f);
                        // we should check how much we need to regen, we can't regen less than 8% of max mana
                        uint32 minimumForRegen = uint32((target->GetMaxPower(POWER_MANA) * 0.08f) / 10.0f); 
                        if (regenManaFromSpirit < minimumForRegen)
                            regenManaFromSpirit = minimumForRegen;
                    }
                }
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetTarget())
                    {
                        if (regenManaFromSpirit > 0)
                            _player->EnergizeBySpell(target, GetSpellInfo()->Id, regenManaFromSpirit, POWER_MANA);
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_innervate_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_innervate_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_innervate_AuraScript();
        }
};

// Lacerate - 33745
class spell_dru_lacerate : public SpellScriptLoader
{
    public:
        spell_dru_lacerate() : SpellScriptLoader("spell_dru_lacerate") { }

        class spell_dru_lacerate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_lacerate_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (roll_chance_i(25))
                            _player->RemoveSpellCooldown(SPELL_DRUID_MANGLE_BEAR, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_lacerate_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_lacerate_SpellScript();
        }
};

// Faerie Fire - 770
class spell_dru_faerie_fire : public SpellScriptLoader
{
    public:
        spell_dru_faerie_fire() : SpellScriptLoader("spell_dru_faerie_fire") { }

        class spell_dru_faerie_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_faerie_fire_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->CastSpell(target, SPELL_DRUID_WEAKENED_ARMOR, true);
                        _player->CastSpell(target, SPELL_DRUID_WEAKENED_ARMOR, true);
                        _player->CastSpell(target, SPELL_DRUID_WEAKENED_ARMOR, true);

                        if (_player->GetShapeshiftForm() == FORM_BEAR && roll_chance_i(25))
                        {
                            if (_player->HasSpellCooldown(SPELL_DRUID_MANGLE_BEAR))
                                _player->RemoveSpellCooldown(SPELL_DRUID_MANGLE_BEAR, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_faerie_fire_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_faerie_fire_SpellScript();
        }
};

// Teleport : Moonglade - 18960
class spell_dru_teleport_moonglade : public SpellScriptLoader
{
    public:
        spell_dru_teleport_moonglade() : SpellScriptLoader("spell_dru_teleport_moonglade") { }

        class spell_dru_teleport_moonglade_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_teleport_moonglade_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->TeleportTo(1, 7964.063f, -2491.099f, 487.83f, _player->GetOrientation());
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_dru_teleport_moonglade_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_teleport_moonglade_SpellScript();
        }
};

// Growl - 6795, Might of Ursoc - 106922, Stampeding Roar - 106898
class spell_dru_growl : public SpellScriptLoader
{
    public:
        spell_dru_growl() : SpellScriptLoader("spell_dru_growl") { }

        class spell_dru_growl_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_growl_SpellScript);            

            void HandleOnHit()
            {
                // This spell activate the bear form
                if (Unit* l_Caster = GetCaster())
                {
                    // Don't cast bear form if the caster is already in bear form
                    if (l_Caster->GetShapeshiftForm() == FORM_BEAR)
                        return;

                    // Fix Glyph of Stampede in this condition - 114300
                    if ((GetSpellInfo()->Id == 106898 && l_Caster->GetShapeshiftForm() != FORM_CAT && l_Caster->GetShapeshiftForm() != FORM_BEAR && !l_Caster->HasAura(114300)) || GetSpellInfo()->Id != 106898)
                        l_Caster->CastSpell(l_Caster, SPELL_DRUID_BEAR_FORM, true);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_growl_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_growl_SpellScript();
        }
};

// Prowl - 5212, Prowl - 102547 and Dash - 1850
class spell_dru_prowl : public SpellScriptLoader
{
    public:
        spell_dru_prowl() : SpellScriptLoader("spell_dru_prowl") { }

        class spell_dru_prowl_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_prowl_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_DRUID_PROWL))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                // This spell activate the cat form
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->CastSpell(_player, SPELL_DRUID_CAT_FORM, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_prowl_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_prowl_SpellScript();
        }
};

// 5176 - Wrath, 2912 - Starfire and 78674 - Starsurge
class spell_dru_eclipse : public SpellScriptLoader
{
    public:
        spell_dru_eclipse() : SpellScriptLoader("spell_dru_eclipse") { }

        class spell_dru_eclipse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_eclipse_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        if ((!_player->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) || !_player->HasAura(SPELL_DRUID_LUNAR_ECLIPSE)) && _player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_DRUID_BALANCE)
                        {
                            switch (GetSpellInfo()->Id)
                            {
                                case SPELL_DRUID_WRATH:
                                {
                                    int32 eclipse = 15; // 15 Lunar energy

                                    // The last eclipse was Lunar, so wrath shouldn't give energy
                                    if (_player->GetLastEclipsePower() == SPELL_DRUID_LUNAR_ECLIPSE || _player->HasAura(SPELL_DRUID_CELESTIAL_ALIGNMENT))
                                        eclipse = 0;

                                    if (_player->HasAura(SPELL_DRUID_EUPHORIA) && !_player->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && !_player->HasAura(SPELL_DRUID_LUNAR_ECLIPSE))
                                        eclipse *= 2;

                                    _player->SetEclipsePower(int32(_player->GetEclipsePower() - eclipse));

                                    // Your crits with Wrath also increase sunfire duration by 2s
                                    if (GetSpell()->IsCritForTarget(target))
                                    {
                                        if (Aura* aura = target->GetAura(SPELL_DRUID_SUNFIRE))
                                        {
                                            aura->SetDuration(aura->GetDuration() + 2000);
                                            if (aura->GetMaxDuration() < aura->GetDuration())
                                                aura->SetMaxDuration(aura->GetDuration());
                                        }
                                    }

                                    break;
                                }
                                case SPELL_DRUID_STARFIRE:
                                {
                                    int32 eclipse = 20; // 20 Solar energy

                                    // The last eclipse was Solar, so Starfire shouldn't give energy
                                    if (_player->GetLastEclipsePower() == SPELL_DRUID_SOLAR_ECLIPSE || _player->HasAura(SPELL_DRUID_CELESTIAL_ALIGNMENT))
                                        eclipse = 0;

                                    if (_player->HasAura(SPELL_DRUID_EUPHORIA) && !_player->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && !_player->HasAura(SPELL_DRUID_LUNAR_ECLIPSE))
                                        eclipse *= 2;

                                    _player->SetEclipsePower(int32(_player->GetEclipsePower() + eclipse));

                                    // Your crits with Starfire also increase moonfire duration by 2s
                                    if (GetSpell()->IsCritForTarget(target))
                                    {
                                        if (Aura* aura = target->GetAura(SPELL_DRUID_MOONFIRE))
                                        {
                                            aura->SetDuration(aura->GetDuration() + 2000);
                                            if (aura->GetMaxDuration() < aura->GetDuration())
                                                aura->SetMaxDuration(aura->GetDuration());
                                        }
                                    }

                                    break;
                                }
                                case SPELL_DRUID_STARSURGE:
                                {
                                    int32 eclipse = 20; // 20 Solar or Lunar energy

                                    if (_player->HasAura(SPELL_DRUID_CELESTIAL_ALIGNMENT))
                                        eclipse = 0;

                                    if (_player->HasAura(SPELL_DRUID_EUPHORIA) && !_player->HasAura(SPELL_DRUID_SOLAR_ECLIPSE) && !_player->HasAura(SPELL_DRUID_LUNAR_ECLIPSE))
                                        eclipse *= 2;

                                    // Give Solar energy just if our last eclipse was lunar else, give Lunar energy
                                    if (_player->GetLastEclipsePower() == SPELL_DRUID_LUNAR_ECLIPSE)
                                        _player->SetEclipsePower(int32(_player->GetEclipsePower() + eclipse));
                                    else
                                        _player->SetEclipsePower(int32(_player->GetEclipsePower() - eclipse));

                                    // Your crits with Starsurge also increase sunfire duration by 2s
                                    if (GetSpell()->IsCritForTarget(target))
                                    {
                                        if (Aura* aura = target->GetAura(SPELL_DRUID_SUNFIRE))
                                        {
                                            aura->SetDuration(aura->GetDuration() + 2000);
                                            if (aura->GetMaxDuration() < aura->GetDuration())
                                                aura->SetMaxDuration(aura->GetDuration());
                                        }
                                    }
                                    // Your crits with Starsurge also increase moonfire duration by 2s
                                    if (GetSpell()->IsCritForTarget(target))
                                    {
                                        if (Aura* aura = target->GetAura(SPELL_DRUID_MOONFIRE))
                                        {
                                            aura->SetDuration(aura->GetDuration() + 2000);
                                            if (aura->GetMaxDuration() < aura->GetDuration())
                                                aura->SetMaxDuration(aura->GetDuration());
                                        }
                                    }

                                    break;
                                }
                            }
                            // Now check if we have lunar or solar eclipse and give it with effects
                            _player->SendEclipse();
                        }
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_dru_eclipse_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_eclipse_SpellScript();
        }
};

class spell_dru_t10_restoration_4p_bonus : public SpellScriptLoader
{
    public:
        spell_dru_t10_restoration_4p_bonus() : SpellScriptLoader("spell_dru_t10_restoration_4p_bonus") { }

        class spell_dru_t10_restoration_4p_bonus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_t10_restoration_4p_bonus_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster()->ToPlayer()->GetGroup())
                {
                    targets.clear();
                    targets.push_back(GetCaster());
                }
                else
                {
                    targets.remove(GetExplTargetUnit());
                    if (targets.empty())
                        return;

                    std::list<Unit*> tempTargets;
                    for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                        if ((*itr)->GetTypeId() == TYPEID_PLAYER && GetCaster()->IsInRaidWith((*itr)->ToUnit()))
                            tempTargets.push_back((*itr)->ToUnit());

                    if (tempTargets.empty())
                    {
                        targets.clear();
                        FinishCast(SPELL_FAILED_DONT_REPORT);
                        return;
                    }

                    Unit* target = JadeCore::Containers::SelectRandomContainerElement(tempTargets);
                    targets.clear();
                    targets.push_back(target);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_t10_restoration_4p_bonus_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_t10_restoration_4p_bonus_SpellScript();
        }
};

// 40121 - Swift Flight Form (Passive)
class spell_dru_swift_flight_passive : public SpellScriptLoader
{
    public:
        spell_dru_swift_flight_passive() : SpellScriptLoader("spell_dru_swift_flight_passive") { }

        class spell_dru_swift_flight_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_swift_flight_passive_AuraScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->GetSkillValue(SKILL_RIDING) >= 375)
                        amount = 310;
                return true;
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_swift_flight_passive_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_swift_flight_passive_AuraScript();
        }
};

class spell_dru_starfall_dummy : public SpellScriptLoader
{
    public:
        spell_dru_starfall_dummy() : SpellScriptLoader("spell_dru_starfall_dummy") { }

        class spell_dru_starfall_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_starfall_dummy_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (Unit* caster = GetCaster())
                {
                    targets.remove_if([caster](WorldObject* object) -> bool
                    {
                        Unit* target = object->ToUnit();
                        if (!target)
                            return true;

                        if (!caster->IsValidAttackTarget(target))
                            return true;

                        /// All targets within 40 yards of the caster that you're in combat with
                        if (!target->isInCombat())
                            return true;

                        // Glyph of Guided Stars - Starfall can only hit targets with Moonfire or Sunfire
                        if (caster->HasAura(SPELL_DRUID_GLYPH_OF_GUIDED_STARS))
                            if (!target->HasAura(SPELL_DRUID_MOONFIRE) && !target->HasAura(SPELL_DRUID_SUNFIRE))
                                return true;

                        return false; // Valid target
                    });
                }

                JadeCore::Containers::RandomResizeList(targets, 2);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                // Shapeshifting into an animal form or mounting cancels the effect
                if (caster->GetCreatureType() == CREATURE_TYPE_BEAST || caster->IsMounted())
                {
                    if (SpellInfo const* spellInfo = GetTriggeringSpell())
                        caster->RemoveAurasDueToSpell(spellInfo->Id);
                    return;
                }

                // Any effect which causes you to lose control of your character will supress the starfall effect.
                if (caster->HasUnitState(UNIT_STATE_CONTROLLED))
                    return;

                caster->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_starfall_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_dru_starfall_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_starfall_dummy_SpellScript();
        }
};

class spell_dru_savage_roar : public SpellScriptLoader
{
    public:
        spell_dru_savage_roar() : SpellScriptLoader("spell_dru_savage_roar") { }

        class spell_dru_savage_roar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_savage_roar_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetShapeshiftForm() != FORM_CAT)
                    return SPELL_FAILED_ONLY_SHAPESHIFT;

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_savage_roar_SpellScript::CheckCast);
            }
        };

        class spell_dru_savage_roar_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_savage_roar_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(DRUID_SAVAGE_ROAR))
                    return false;
                return true;
            }

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->CastSpell(target, DRUID_SAVAGE_ROAR, true, NULL, aurEff, GetCasterGUID());
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(DRUID_SAVAGE_ROAR);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_savage_roar_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_savage_roar_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_savage_roar_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_savage_roar_AuraScript();
        }
};

// Survival Instincts - 61336
class spell_dru_survival_instincts : public SpellScriptLoader
{
    public:
        spell_dru_survival_instincts() : SpellScriptLoader("spell_dru_survival_instincts") { }

        class spell_dru_survival_instincts_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_survival_instincts_AuraScript);

            void AfterApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell(GetTarget(), DRUID_SURVIVAL_INSTINCTS, true);
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(DRUID_SURVIVAL_INSTINCTS);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_survival_instincts_AuraScript::AfterApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_survival_instincts_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_survival_instincts_AuraScript();
        }
};

/// Genesis - 145518
class spell_dru_genesis : public SpellScriptLoader
{
    public:
        spell_dru_genesis() : SpellScriptLoader("spell_dru_genesis") { }

        class spell_dru_genesis_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_genesis_SpellScript);

            void FilterTargets(std::list<WorldObject*>& p_Targets)
            {
                if (Unit* l_Caster = GetCaster())
                {
                    p_Targets.remove_if([&](WorldObject* p_Target)
                    {
                        if (Unit* l_Unit = p_Target->ToUnit())
                        {
                            Aura const* aura = l_Unit->GetAura(DruidSpells::SPELL_DRUID_REJUVENATION);
                            if (!aura || aura->GetMaxDuration() == 1500)
                                return true;

                            if (l_Unit->IsWithinDistInMap(l_Caster, 40.f))
                            {
                                if (l_Unit->IsInRaidWith(l_Caster))
                                    return false;
                            }
                        }

                        return true;
                    });
                }
            }
            
            void HandleHit()
            {
                if (Unit* l_Caster = GetCaster())
                {
                    if (Unit* l_Target = GetHitUnit())
                    {
                        if (l_Target->HasAura(DruidSpells::SPELL_DRUID_REJUVENATION))
                        {
                            l_Target->RemoveAura(DruidSpells::SPELL_DRUID_REJUVENATION);
                            if (Aura* l_Rejuvenation = l_Caster->AddAura(DruidSpells::SPELL_DRUID_REJUVENATION, l_Target))
                            {
                                l_Rejuvenation->SetDuration(1500);
                                l_Rejuvenation->SetMaxDuration(1500);
                                if (AuraEffect* l_RejuvenationEffect = l_Rejuvenation->GetEffect(SpellEffIndex::EFFECT_0))
                                    l_RejuvenationEffect->SetAmplitude(500);
                            }
                        }
                    }
                }      
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_genesis_SpellScript::FilterTargets, SpellEffIndex::EFFECT_0, Targets::TARGET_UNIT_DEST_AREA_ALLY);
                OnHit += SpellHitFn(spell_dru_genesis_SpellScript::HandleHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_genesis_SpellScript();
        }
};

// Ysera's Gift - 145108
class spell_dru_yseras_gift : public SpellScriptLoader
{
    public:
        spell_dru_yseras_gift() : SpellScriptLoader("spell_dru_yseras_gift") { }

        class spell_dru_yseras_gift_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_yseras_gift_AuraScript);

            void OnPeriodic(AuraEffect const* p_AurEff)
            {
                if (Unit* l_Caster = GetCaster())
                {
                    Unit* l_HealTarget;
                    if (l_Caster->IsFullHealth())
                    {
                        l_HealTarget = nullptr;

                        std::list<Unit*> l_AlliesList;
                        l_Caster->GetPartyMembers(l_AlliesList);

                        for (Unit* l_Ally : l_AlliesList)
                        {
                            if (l_Ally->GetGUID() == l_Caster->GetGUID())
                                continue;

                            if (l_Ally->IsFullHealth())
                                continue;

                            if (l_HealTarget && l_Ally->GetHealthPct() > l_HealTarget->GetHealthPct())
                                continue;

                            if (l_Caster->IsValidAssistTarget(l_Ally))
                                continue;

                            l_HealTarget = l_Ally;
                            break;
                        }
                    }
                    else
                        l_HealTarget = l_Caster;

                    if (l_HealTarget)
                    {
                        int32 l_BP = l_HealTarget->CountPctFromMaxHealth(p_AurEff->GetAmount());
                        l_Caster->CastCustomSpell(l_HealTarget, DruidSpells::SPELL_DRUID_YSERAS_GIFT, &l_BP, nullptr, nullptr, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_yseras_gift_AuraScript::OnPeriodic, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_yseras_gift_AuraScript();
        }
};

// Charm Woodland Creature - 127757
// Glyph of Charm Woodland Creature - 57855
class spell_druid_glyph_of_charm_woodland_creature : public SpellScriptLoader
{
    public:
        spell_druid_glyph_of_charm_woodland_creature() : SpellScriptLoader("spell_druid_glyph_of_charm_woodland_creature") { }

        class spell_druid_glyph_of_charm_woodland_creature_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_druid_glyph_of_charm_woodland_creature_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(SPELL_CHARM_WOODLAND_CREATURE, false);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(SPELL_CHARM_WOODLAND_CREATURE))
                        _player->removeSpell(SPELL_CHARM_WOODLAND_CREATURE, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_druid_glyph_of_charm_woodland_creature_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_druid_glyph_of_charm_woodland_creature_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_druid_glyph_of_charm_woodland_creature_AuraScript();
        }
};

const static uint32 sHeartOfTheWildBonus[4][2] =
{
    { SPEC_DRUID_CAT, 108293 },
    { SPEC_DRUID_BEAR, 108294 },
    { SPEC_DRUID_BALANCE, 108292 },
    { SPEC_DRUID_RESTORATION, 108291 },
};

// Heart of the wild - 108288
class spell_dru_heart_wild : public SpellScriptLoader
{
    public:
        spell_dru_heart_wild() : SpellScriptLoader("spell_dru_heart_wild") { }

        class spell_dru_heart_wild_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_heart_wild_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget()->ToPlayer())
                    return;

                for (auto ptr : sHeartOfTheWildBonus)
                {
                    if (ptr[0] == GetTarget()->ToPlayer()->GetSpecializationId())
                        continue;

                    GetTarget()->CastSpell(GetTarget(), ptr[1], true);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_heart_wild_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_heart_wild_AuraScript();
        }
};

// Called by Healing Touch - 5185
// Dream of Cenarius - 108733
class spell_dru_dream_of_cenarius : public SpellScriptLoader
{
    public:
        spell_dru_dream_of_cenarius() : SpellScriptLoader("spell_dru_dream_of_cenarius") { }

        class spell_dru_dream_of_cenarius_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_dream_of_cenarius_SpellScript);

            void HandleOnHit()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // Druid T16 Restoration 2P Bonus - 144869
                        if (caster->HasAura(DRUID_T16_RESTORATION_2P_BONUS) && caster->HasAura(SPELL_DRUID_SAGE_MENDER))
                            caster->RemoveAura(SPELL_DRUID_SAGE_MENDER);

                        if (caster->getLevel() >= 90 && caster->HasAura(SPELL_DRUID_DREAM_OF_CENARIUS))
                        {
                            // Increases healing from Healing Touch by 20% for all specs except Restoration.
                            if (caster->GetActiveSpec() != SPEC_DRUID_RESTORATION)
                            {
                                int32 currentHeal = GetHitHeal();
                                currentHeal = int32(currentHeal * 1.2f);
                                SetHitHeal(currentHeal);

                                switch (caster->GetSpecializationId(caster->GetActiveSpec()))
                                {
                                    case SPEC_DRUID_BALANCE:
                                        caster->AddAura(SPELL_DRUID_DREAM_OF_CENARIUS_BALANCE, caster);
                                        break;
                                    case SPEC_DRUID_CAT:
                                        caster->AddAura(SPELL_DRUID_DREAM_OF_CENARIUS_CAT, caster);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_dream_of_cenarius_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_dream_of_cenarius_SpellScript();
        }
};

// Called by Wrath - 5176
// Dream of Cenarius - 108733
class spell_dru_dream_of_cenarius_restor : public SpellScriptLoader
{
    public:
        spell_dru_dream_of_cenarius_restor() : SpellScriptLoader("spell_dru_dream_of_cenarius_restor") { }

        class spell_dru_dream_of_cenarius_restor_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_dream_of_cenarius_restor_SpellScript);

            void HandleOnHit()
            {
                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (caster->getLevel() >= 90 && caster->GetSpecializationId(caster->GetActiveSpec()) == SPEC_DRUID_RESTORATION && caster->HasAura(SPELL_DRUID_DREAM_OF_CENARIUS))
                        {
                            int32 hitDamage = GetHitDamage();
                            caster->CastCustomSpell(target, SPELL_DRUID_DREAM_OF_CENARIUS_RESTOR, &hitDamage, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_dream_of_cenarius_restor_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_dream_of_cenarius_restor_SpellScript();
        }
};

// Force of Nature - 106737
// Cast the form appropriate summon spell
class spell_dru_force_of_nature : public SpellScriptLoader
{
    public:
        spell_dru_force_of_nature() : SpellScriptLoader("spell_dru_force_of_nature") { }

        enum eSpells
        {
            FERAL = 102703,
            GUARDIAN = 102706,
            BALANCE = 33831,
            RESTORATION = 102693
        };

        class spell_dru_force_of_nature_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_force_of_nature_SpellScript);

            void HandleOnCast()
            {
                if (Unit* l_Caster = GetCaster())
                {
                    const WorldLocation* l_TargetPos = GetExplTargetDest();

                    switch (l_Caster->GetShapeshiftForm())
                    {
                        case ShapeshiftForm::FORM_CAT:
                            l_Caster->CastSpell(*l_TargetPos, eSpells::FERAL, true);
                            break;
                        case ShapeshiftForm::FORM_BEAR:
                            l_Caster->CastSpell(*l_TargetPos, eSpells::GUARDIAN, true);
                            break;
                        case ShapeshiftForm::FORM_MOONKIN:
                            l_Caster->CastSpell(*l_TargetPos, eSpells::BALANCE, true);
                            break;
                        default:
                            l_Caster->CastSpell(*l_TargetPos, eSpells::RESTORATION, true);
                            break;
                    }
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_dru_force_of_nature_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_force_of_nature_SpellScript();
        }
};

// Force of Nature - 106737
// Called by Treant Restoration Spell (Healing Touch) - 113828
class spell_dru_pet_force_of_nature_heal : public SpellScriptLoader
{
public:
    spell_dru_pet_force_of_nature_heal() : SpellScriptLoader("spell_dru_pet_force_of_nature_heal") { }

    class spell_dru_pet_force_of_nature_heal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dru_pet_force_of_nature_heal_SpellScript);

        static bool compareHealth(const WorldObject* first, const WorldObject* second)
        {
            return first->ToUnit()->GetHealthPct() < second->ToUnit()->GetHealthPct();
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            struct is_not_valid_heal_target {

                Unit* caster;

                bool operator() (const WorldObject* target)
                {
                    if (!(target->ToUnit()))
                        return true;

                    const Unit* uTarget = target->ToUnit();

                    if (uTarget->isTotem())
                        return true;
                    if (!(caster->IsFriendlyTo(uTarget)))
                        return true;
                    if (!(caster->IsWithinLOSInMap(uTarget)))
                        return true;
                    if (!(caster->IsInRange(uTarget, 0.0f, 39.0f, true)))
                        return true;
                    if (!(uTarget->GetHealth()))
                        return true;
                    if (!(uTarget->GetMaxHealth()))
                        return true;

                    return false;
                }
            };

            is_not_valid_heal_target target_check;
            target_check.caster = GetCaster();
            targets.remove_if(target_check);

            if (targets.size() == 0)
                return;

            targets.sort(compareHealth);

            targets.resize(1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_pet_force_of_nature_heal_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dru_pet_force_of_nature_heal_SpellScript();
    }
};

// Force of Nature (106737) treant spells, for calculation
// Called by Treant Restoration Spell (Healing Touch) - 113828, Treant Balance Spell (Wrath) - 113769
class spell_dru_treants_spells_calculation : public SpellScriptLoader
{
    public:
        spell_dru_treants_spells_calculation() : SpellScriptLoader("spell_dru_treants_spells_calculation") { }

        class spell_dru_treants_spells_calculation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_treants_spells_calculation_SpellScript);

            void HandleOnHit()
            {
                if (!GetCaster())
                    return;

                Unit* owner = GetCaster()->ToTempSummon()->GetSummoner();
                if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (Player* player = owner->ToPlayer())
                {
                    // Healing Touch
                    if (GetSpellInfo()->Id == 113828)
                    {
                        uint32 heal = GetHitHeal();
                        uint32 spelldamage = int32(player->GetStat(STAT_INTELLECT)) - 10;
                        // Heals a nearby injured ally for 3201 to 3779 (+ 32.3% of Spell power) every 2.5 sec
                        spelldamage = uint32(spelldamage * 0.323f);
                        heal += spelldamage;
                        SetHitHeal(heal);

                        // Send information to combat log if treant heals other target, not druid
                        if (GetHitUnit()->GetGUID() != player->GetGUID())
                        {
                            const SpellInfo* healTouch = sSpellMgr->GetSpellInfo(113828);
                            owner->SendSpellNonMeleeDamageLog(GetHitUnit(), healTouch->Id, heal, healTouch->GetSchoolMask(), 0, 0, false, 0, false);
                        }
                    }
                    // Wrath
                    else if (GetSpellInfo()->Id == 113769)
                    {
                        uint32 damage = GetHitDamage();
                        uint32 spelldamage = int32(player->GetStat(STAT_INTELLECT)) - 10;
                        // Casts Wrath for 1930 to 2175 (+ 37.5% of Spell power) Nature damage every 2 sec
                        spelldamage = uint32(spelldamage * 0.375f);
                        damage += spelldamage;
                        SetHitDamage(damage);

                        // Send information to combat log
                        const SpellInfo* wrathSpell = sSpellMgr->GetSpellInfo(113769);
                        owner->SendSpellNonMeleeDamageLog(GetHitUnit(), wrathSpell->Id, damage, wrathSpell->GetSchoolMask(), 0, 0, false, 0, false);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_treants_spells_calculation_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_treants_spells_calculation_SpellScript();
        }
};

// Called by Starsurge - 78674
// Shooting Stars - 93399
class spell_dru_starsurge : public SpellScriptLoader
{
    public:
        spell_dru_starsurge() : SpellScriptLoader("spell_dru_starsurge") { }

        class spell_dru_starsurge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_starsurge_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetAuraBeforeInstantCast())
                    {
                        _player->RemoveAura(SPELL_DRUID_SHOOTING_STARS);
                        _player->SetAuraBeforeInstantCast(false);
                    }
                    else if (_player->HasAura(SPELL_DRUID_SHOOTING_STARS))
                        _player->RemoveSpellCooldown(SPELL_DRUID_STARSURGE, true);
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_dru_starsurge_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_starsurge_SpellScript();
        }
};

class spell_area_druid_ursols_vortex : public SpellAreaTriggerScript
{
public:
    spell_area_druid_ursols_vortex() : SpellAreaTriggerScript("spell_area_druid_ursols_vortex") {}

    bool OnAddTarget(AreaTrigger* trigger, Unit* target)
    {
        if (!trigger->GetCaster())
            return false;

        if (!target->isTargetableForAttack())
            return false;

        trigger->GetCaster()->CastSpell(target, 127797, true);

        return true;
    }
};

class spell_dru_astral_storm : public SpellScriptLoader
{
    public:
        spell_dru_astral_storm() : SpellScriptLoader("spell_dru_astral_storm") { }

        class spell_dru_astral_storm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_astral_storm_AuraScript);

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                // Prevent from triggering damage by DynamicObject
                if (GetOwner()->GetTypeId() != TYPEID_PLAYER)
                    PreventDefaultAction();
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_astral_storm_AuraScript::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_astral_storm_AuraScript();
        }
};

class spell_dru_hurricane : public SpellScriptLoader
{
    public:
        spell_dru_hurricane() : SpellScriptLoader("spell_dru_hurricane") { }

        class spell_dru_hurricane_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_hurricane_AuraScript);

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                // Prevent from triggering damage by DynamicObject
                if (GetOwner()->GetTypeId() != TYPEID_PLAYER)
                    PreventDefaultAction();
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_hurricane_AuraScript::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_hurricane_AuraScript();
        }
};

// Called by Rake - 1822
class spell_dru_rake : public SpellScriptLoader
{
    public:
        spell_dru_rake() : SpellScriptLoader("spell_dru_rake") { }

        class spell_dru_rake_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_rake_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    float Mastery = 1 + (_player->GetFloatValue(PLAYER_MASTERY) * 3.125f / 100.0f);
                    int32 damage = GetHitDamage();
                    damage = int32(damage * Mastery);
                    SetHitDamage(damage);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dru_rake_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_rake_SpellScript();
        }
};

// Called by Stampede - 81022, Incarnation: King of the Jungle - 102543
class spell_dru_stampede : public SpellScriptLoader
{
    public:
       spell_dru_stampede() : SpellScriptLoader("spell_dru_stampede") { }

        class spell_dru_stampede_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_stampede_AuraScript);

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    if (aurEff->GetSpellInfo()->Id == 81022 && caster->HasAura(102543))
                    {
                        if (Aura* incarnation = caster->GetAura(102543, caster->GetGUID()))
                        {
                            int32 duration = incarnation->GetDuration();
                            Aura::TryRefreshStackOrCreate(incarnation->GetSpellInfo(), MAX_EFFECT_MASK, caster, caster);
                            incarnation->SetDuration(duration);
                        }
                    }
                    else if (aurEff->GetSpellInfo()->Id == 102543 && caster->HasAura(81022))
                    {
                        if (Aura* stampede = caster->GetAura(81022, caster->GetGUID()))
                            Aura::TryRefreshStackOrCreate(stampede->GetSpellInfo(), MAX_EFFECT_MASK, caster, caster);
                    }
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_stampede_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dru_stampede_AuraScript();
        }
};


// Called by Savage Roar - 52610, Savage Roar Glyph - 127538
class spell_dru_savage_roar_powerful : public SpellScriptLoader
{
    public:
        spell_dru_savage_roar_powerful() : SpellScriptLoader("spell_dru_savage_roar_powerful") { }

        class spell_dru_savage_roar_powerful_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_savage_roar_powerful_SpellScript);

            SpellCastResult CheckMorePowerfulAura()
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    int32 comboPoints = player->GetComboPoints();
                    int32 newDuration;
                    int32 oldDuration;

                    if (Aura* savageRoar = player->GetAura(DRUID_SAVAGE_ROAR_AURA, player->GetGUID()))
                    {
                        if (comboPoints >= 1)
                        {
                            newDuration = (12 + (comboPoints * 6)) * 1000;
                            oldDuration = savageRoar->GetDuration();

                            if (newDuration < oldDuration)
                                return SPELL_FAILED_AURA_BOUNCED;  
                        }
                    }
                    else  if (Aura* savageRoarGlyph = player->GetAura(DRUID_SAVAGE_ROAR_GLYPH_AURA, player->GetGUID()))
                    {
                        newDuration = (12 + (comboPoints * 6)) * 1000;
                        oldDuration = savageRoarGlyph->GetDuration();

                        if (newDuration < oldDuration)
                            return SPELL_FAILED_AURA_BOUNCED;  
                    }
                }

                return SPELL_CAST_OK;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_savage_roar_powerful_SpellScript::CheckMorePowerfulAura);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dru_savage_roar_powerful_SpellScript();
        }
};

// Called by Nature's Swiftness - 132158
class spell_dru_natures_swiftness : public SpellScriptLoader
{
    public:
        spell_dru_natures_swiftness() : SpellScriptLoader("spell_dru_natures_swiftness") { }

        class spell_dru_natures_swiftness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_natures_swiftness_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Player* l_Caster = GetCaster()->ToPlayer())
                {
                    switch (l_Caster->GetSpecializationId(l_Caster->GetActiveSpec()))
                    {
                        case SPEC_DRUID_BALANCE:
                        case SPEC_DRUID_RESTORATION:
                            break;

                        default:
                            return SpellCastResult::SPELL_FAILED_SPELL_UNAVAILABLE;
                    }
                }

                return SpellCastResult::SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_natures_swiftness_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_natures_swiftness_SpellScript();
        }
};

void AddSC_druid_spell_scripts()
{
    new spell_dru_tooth_and_claw_absorb();
    new spell_dru_glyph_of_the_treant();
    new spell_dru_incarnation_chosen_of_elune();
    new spell_dru_incarnation_skins();
    new spell_dru_glyph_of_shred();
    new spell_dru_item_pvp_feral_4p();
    new spell_dru_wild_charge_moonkin();
    new spell_dru_thrash_bear();
    new spell_dru_swipe_and_maul();
    new spell_dru_soul_of_the_forest_eclipse();
    new spell_dru_soul_of_the_forest();
    new spell_dru_tigers_fury();
    new spell_dru_play_death();
    new spell_dru_consecration();
    new spell_dru_consecration_area();
    new spell_dru_life_tap();
    new spell_dru_soul_swap();
    new spell_dru_demonic_circle_teleport();
    new spell_dru_shattering_blow();
    new spell_dru_symbiosis_aura();
    new spell_dru_symbiosis();
    new spell_dru_moonfire();
    new spell_dru_cenarion_ward();
    new spell_dru_ursols_vortex_snare();
    new spell_dru_ursols_vortex();
    new spell_dru_solar_beam();
    new spell_dru_dash();
    new spell_dru_rip_duration();
    new spell_dru_savage_defense();
    new spell_dru_bear_form();
    new spell_dru_ferocious_bite();
    new spell_dru_bear_hug();
    new spell_dru_ravage();
    new spell_dru_lifebloom();
    new spell_dru_killer_instinct();
    new spell_dru_lifebloom_refresh();
    new spell_dru_omen_of_clarity();
    new spell_dru_mark_of_the_wild();
    new spell_dru_natures_cure();
    new spell_dru_glyph_of_regrowth();
    new spell_dru_cat_form();
    new spell_dru_skull_bash();
    new spell_dru_faerie_swarm();
    new spell_dru_wild_mushroom_heal();
    new spell_dru_wild_mushroom_growing();
    new spell_dru_wild_mushroom_resto();
    new spell_dru_wild_mushroom_bloom();
    new spell_dru_wild_mushroom_detonate();
    new spell_dru_wild_mushroom();
    new spell_dru_swiftmend_heal();
    new spell_dru_swiftmend();
    new spell_dru_astral_communion();
    new spell_dru_shooting_stars();
    new spell_dru_celestial_alignment();
    new spell_dru_frenzied_regeneration();
    new spell_dru_stampeding_roar_speed();
    new spell_dru_stampeding_roar();
    new spell_dru_innervate();
    new spell_dru_lacerate();
    new spell_dru_faerie_fire();
    new spell_dru_teleport_moonglade();
    new spell_dru_growl();
    new spell_dru_prowl();
    new spell_dru_eclipse();
    new spell_dru_t10_restoration_4p_bonus();
    new spell_dru_swift_flight_passive();
    new spell_dru_starfall_dummy();
    new spell_dru_savage_roar();
    new spell_dru_survival_instincts();
    new spell_dru_genesis();
    new spell_dru_yseras_gift();
    new spell_druid_glyph_of_charm_woodland_creature();
    new spell_dru_heart_wild();
    new spell_dru_dream_of_cenarius();
    new spell_dru_dream_of_cenarius_restor();
    new spell_dru_force_of_nature();
    new spell_dru_pet_force_of_nature_heal();
    new spell_dru_treants_spells_calculation();
    new spell_dru_starsurge();
    new spell_dru_astral_storm();
    new spell_dru_hurricane();
    new spell_dru_rake();
    new spell_dru_stampede();
    new spell_dru_savage_roar_powerful();
    new spell_dru_natures_swiftness();

    new spell_area_druid_ursols_vortex();
    new spell_dru_dream_of_cenarius_heal();
}
