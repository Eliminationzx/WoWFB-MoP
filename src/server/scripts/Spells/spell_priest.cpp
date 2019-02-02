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
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

enum PriestSpells
{
    PRIEST_SPELL_GUARDIAN_SPIRIT_HEAL               = 48153,
    PRIEST_SPELL_PENANCE                            = 47540,
    PRIEST_SPELL_PENANCE_DAMAGE                     = 47758,
    PRIEST_SPELL_PENANCE_HEAL                       = 47757,
    PRIEST_SPELL_REFLECTIVE_SHIELD_TRIGGERED        = 33619,
    PRIEST_SPELL_REFLECTIVE_SHIELD_R1               = 33202,
    PRIEST_SHADOW_WORD_DEATH                        = 32409,
    PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH          = 107903,
    PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH             = 107904,
    PRIEST_GLYPH_OF_SHADOW                          = 107906,
    PRIEST_VOID_SHIFT                               = 108968,
    PRIEST_LEAP_OF_FAITH                            = 73325,
    PRIEST_LEAP_OF_FAITH_JUMP                       = 110726,
    PRIEST_INNER_WILL                               = 73413,
    PRIEST_INNER_FIRE                               = 588,
    PRIEST_SPELL_HALO_HEAL_SHADOW                   = 120696,
    PRIEST_SPELL_HALO_HEAL_HOLY                     = 120692,

    // Cascade
    PRIEST_CASCADE_HOLY_DAMAGE                      = 120785,
    PRIEST_CASCADE_HOLY_TRIGGER                     = 120786,
    PRIEST_CASCADE_INVISIBLE_AURA                   = 120840,
    PRIEST_CASCADE_HOLY_TALENT                      = 121135,
    PRIEST_CASCADE_HOLY_MISSILE                     = 121146,
    PRIEST_CASCADE_HOLY_HEAL                        = 121148,
    PRIEST_CASCADE_SHADOW_MISSILE                   = 127627,
    PRIEST_CASCADE_SHADOW_DAMAGE                    = 127628,
    PRIEST_CASCADE_SHADOW_HEAL                      = 127629,
    PRIEST_CASCADE_DAMAGE_TRIGGER                   = 127630,
    PRIEST_CASCADE_INVISIBLE_AURA_2                 = 127631,
    PRIEST_CASCADE_SHADOW_TALENT                    = 127632,

    PRIEST_SHADOWFORM_STANCE                        = 15473,
    PRIEST_SHADOW_WORD_PAIN                         = 589,
    PRIEST_DEVOURING_PLAGUE                         = 2944,
    PRIEST_DEVOURING_PLAGUE_HEAL                    = 127626,
    PRIEST_VAMPIRIC_TOUCH                           = 34914,
    PRIEST_PHANTASM_AURA                            = 108942,
    PRIEST_PHANTASM_PROC                            = 114239,
    PRIEST_SPIRIT_SHELL_AURA                        = 109964,
    PRIEST_SPIRIT_SHELL_ABSORPTION                  = 114908,
    PRIEST_ATONEMENT_AURA                           = 81749,
    PRIEST_ATONEMENT_HEAL                           = 81751,
    PRIEST_RAPTURE_ENERGIZE                         = 47755,
    PRIEST_TRAIN_OF_THOUGHT                         = 92297,
    PRIEST_INNER_FOCUS                              = 89485,
    PRIEST_GRACE_AURA                               = 47517,
    PRIEST_GRACE_PROC                               = 77613,
    PRIEST_STRENGTH_OF_SOUL_AURA                    = 89488,
    PRIEST_STRENGTH_OF_SOUL_REDUCE_TIME             = 89490,
    PRIEST_WEAKENED_SOUL                            = 6788,
    PRIEST_STRENGTH_OF_SOUL                         = 89488,
    PRIEST_EVANGELISM_AURA                          = 81662,
    PRIEST_EVANGELISM_STACK                         = 81661,
    PRIEST_ARCHANGEL                                = 81700,
    LIGHTWELL_CHARGES                               = 126150,
    LIGHTSPRING_RENEW                               = 126154,
    PRIEST_SMITE                                    = 585,
    PRIEST_HOLY_WORD_CHASTISE                       = 88625,
    PRIEST_HOLY_WORD_SANCTUARY_AREA                 = 88685,
    PRIEST_HOLY_WORD_SANCTUARY_HEAL                 = 88686,
    PRIEST_RAPID_RENEWAL_AURA                       = 95649,
    PRIEST_SPELL_EMPOWERED_RENEW                    = 63544,
    PRIEST_SPELL_DIVINE_INSIGHT_TALENT              = 109175,
    PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE          = 123266,
    PRIEST_SPELL_DIVINE_INSIGHT_HOLY                = 123267,
    PRIEST_SPELL_DIVINE_INSIGHT_SHADOW              = 124430,
    PRIEST_PRAYER_OF_MENDING                        = 33076,
    PRIEST_PRAYER_OF_MENDING_HEAL                   = 33110,
    PRIEST_PRAYER_OF_MENDING_RADIUS                 = 123262,
    PRIEST_BODY_AND_SOUL_AURA                       = 64129,
    PRIEST_BODY_AND_SOUL_INCREASE_SPEED             = 65081,
    PRIEST_SURGE_OF_LIGHT                           = 114255,
    PRIEST_SURGE_OF_DARKNESS                        = 87160,
    PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST       = 130733,
    PRIEST_SHADOW_WORD_INSANITY_DAMAGE              = 129249,
    PRIEST_SPELL_MIND_BLAST                         = 8092,
    PRIEST_SPELL_2P_S12_SHADOW                      = 92711,
    PRIEST_SPELL_DISPERSION_SPRINT                  = 129960,
    PRIEST_SPELL_4P_S12_SHADOW                      = 131556,
    PRIEST_SPELL_SIN_AND_PUNISHMENT                 = 87204,
    PRIEST_SPELL_2P_S12_HEAL                        = 33333,
    PRIEST_SPELL_2P_S12_PVPPOWER                    = 129895,
    PRIEST_SPELL_SOUL_OF_DIAMOND                    = 96219,
    PRIEST_SPELL_4P_S12_HEAL                        = 131566,
    PRIEST_SPELL_HOLY_SPARK                         = 131567,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY      = 62371,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM          = 27795,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_TALENT        = 20711,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT          = 27792,
    PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT    = 27827,
    PRIEST_SPELL_LEVITATE                           = 111758,
    PRIEST_SPELL_VOID_TENDRILS_SUMMON               = 127665,
    PRIEST_NPC_VOID_TENDRILS                        = 65282,
    PRIEST_NPC_PSYFIEND                             = 59190,
    PRIEST_SPELL_SPECTRAL_GUISE_CHARGES             = 119030,
    PRIEST_SPELL_POWER_WORD_SHIELD                  = 17,
    PRIEST_SPELL_POWER_WORD_SHIELD_DIVINE_INSIGHT   = 123258,
    PRIEST_SPELL_POWER_WORD_FORTITUDE               = 21562,
    PRIEST_SPELL_HOLY_NOVA                          = 132157,
    PRIEST_GLYPH_OF_MIND_FLAY                       = 120585,
    PRIEST_GLYPH_OF_MIND_FLAY_EFFECT                = 120587,
    PRIEST_SPELL_MIND_FLAY                          = 15407,
    PRIEST_GLYPH_OF_SHADOW_WORD_DEATH               = 120583,
    PRIEST_SPELL_GLYPH_OF_SHADOW_WORD_DEATH         = 129176,
    PRIEST_SPELL_SPELL_WARDING                      = 91724,
    PRIEST_GLYPH_OF_INNER_SANCTUM                   = 14771,
    PRIEST_GLYPH_OF_PRAYER_OF_MENDING               = 55685,
    PRIEST_SPELL_PRAYER_OF_MENDING                  = 41635,
    PRIEST_SPELL_4P_T16_SHADOW                      = 145179,
    PRIEST_SPELL_EMPOWERED_SHADOWS                  = 145180
};

// Power Word : Fortitude - 21562
class spell_pri_power_word_fortitude : public SpellScriptLoader
{
    public:
        spell_pri_power_word_fortitude() : SpellScriptLoader("spell_pri_power_word_fortitude") { }

        class spell_pri_power_word_fortitude_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_fortitude_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->IsInRaidWith(_player))
                        {
                            std::list<Unit*> playerList;
                            _player->GetPartyMembers(playerList, true);

                            for (auto itr : playerList)
                                _player->AddAura(PRIEST_SPELL_POWER_WORD_FORTITUDE, itr);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_power_word_fortitude_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_power_word_fortitude_SpellScript();
        }
};

// Spectral Guise Charges - 119030
class spell_pri_spectral_guise_charges : public SpellScriptLoader
{
    public:
        spell_pri_spectral_guise_charges() : SpellScriptLoader("spell_pri_spectral_guise_charges") { }

        class spell_pri_spectral_guise_charges_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_spectral_guise_charges_AuraScript);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Unit* attacker = eventInfo.GetActor();
                if (!attacker)
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetCaster()->GetGUID())
                    return;

                if (Unit* spectralGuise = GetCaster())
                    if (eventInfo.GetDamageInfo()->GetDamageType() == SPELL_DIRECT_DAMAGE || eventInfo.GetDamageInfo()->GetDamageType() == DIRECT_DAMAGE)
                        if (Aura* spectralGuiseCharges = spectralGuise->GetAura(PRIEST_SPELL_SPECTRAL_GUISE_CHARGES))
                            spectralGuiseCharges->DropCharge();
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->ToCreature())
                        caster->ToCreature()->DespawnOrUnsummon();
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_spectral_guise_charges_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_spectral_guise_charges_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_spectral_guise_charges_AuraScript();
        }
};

// Psyfiend Hit Me Driver - 114164
class spell_pri_psyfiend_hit_me_driver : public SpellScriptLoader
{
    public:
        spell_pri_psyfiend_hit_me_driver() : SpellScriptLoader("spell_pri_psyfiend_hit_me_driver") { }

        class spell_pri_psyfiend_hit_me_driver_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_psyfiend_hit_me_driver_AuraScript);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Unit* attacker = eventInfo.GetActor();
                if (!attacker)
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetCaster()->GetGUID())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    std::list<Creature*> tempList;
                    std::list<Creature*> psyfiendList;

                    _player->GetCreatureListWithEntryInGrid(tempList, PRIEST_NPC_PSYFIEND, 100.0f); // TODO:

                    if (tempList.empty())
                        return;

                    for (auto itr : tempList)
                    {
                        if (!itr->isAlive())
                            continue;

                        if (!itr->GetOwner())
                            continue;

                        if (itr->GetOwner() == _player->GetOwner())
                        {
                            psyfiendList.push_back(itr);
                            break;
                        }
                    }

                    if (psyfiendList.empty())
                        return;

                    if (psyfiendList.size() > 1)
                        JadeCore::Containers::RandomResizeList(psyfiendList, 1);

                    for (auto itr : psyfiendList)
                        if (itr->AI())
                            itr->AI()->SetGUID(attacker->GetGUID());
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_psyfiend_hit_me_driver_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_psyfiend_hit_me_driver_AuraScript();
        }
};

// Void Tendrils - 108920
class spell_pri_void_tendrils : public SpellScriptLoader
{
    public:
        spell_pri_void_tendrils() : SpellScriptLoader("spell_pri_void_tendrils") { }

        class spell_pri_void_tendrils_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_void_tendrils_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        _player->CastSpell(target, PRIEST_SPELL_VOID_TENDRILS_SUMMON, true);

                        if (Creature* voidTendrils = target->FindNearestCreature(PRIEST_NPC_VOID_TENDRILS, 10.0f))
                            if (voidTendrils->AI())
                                voidTendrils->AI()->SetGUID(target->GetGUID());

                        if (Aura* voidTendrils = target->GetAura(GetSpellInfo()->Id, _player->GetGUID()))
                        {
                            if (target->GetTypeId() == TYPEID_PLAYER)
                                voidTendrils->SetMaxDuration(8000);
                            else
                                voidTendrils->SetMaxDuration(20000);
                            voidTendrils->SetDuration(voidTendrils->GetMaxDuration());
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_void_tendrils_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_void_tendrils_SpellScript();
        }
};

// Phantasm (proc) - 114239
class spell_pri_phantasm_proc : public SpellScriptLoader
{
    public:
        spell_pri_phantasm_proc() : SpellScriptLoader("spell_pri_phantasm_proc") { }

        class spell_pri_phantasm_proc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_phantasm_proc_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    _player->RemoveMovementImpairingAuras();
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_phantasm_proc_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_phantasm_proc_SpellScript();
        }
};

// Spirit of Redemption (Shapeshift) - 27827
class spell_pri_spirit_of_redemption_form : public SpellScriptLoader
{
    public:
        spell_pri_spirit_of_redemption_form() : SpellScriptLoader("spell_pri_spirit_of_redemption_form") { }

        class spell_pri_spirit_of_redemption_form_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_spirit_of_redemption_form_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY);
                    caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM);
                    caster->RemoveAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT);
                    caster->setDeathState(JUST_DIED);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_spirit_of_redemption_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_WATER_BREATHING, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_spirit_of_redemption_form_AuraScript();
        }
};

// Spirit of Redemption - 20711
class spell_pri_spirit_of_redemption : public SpellScriptLoader
{
    public:
        spell_pri_spirit_of_redemption() : SpellScriptLoader("spell_pri_spirit_of_redemption") { }

        class spell_pri_spirit_of_redemption_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_spirit_of_redemption_AuraScript);

            bool CalculateAmount(AuraEffect const* /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
                return true;
            }

            void Absorb(AuraEffect* /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                if (Unit* caster = GetCaster())
                {
                    if (dmgInfo.GetDamage() < caster->GetHealth())
                        return;

                    if (caster->HasAura(PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT))
                        return;

                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_FORM, true);
                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_IMMUNITY, true);
                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_ROOT, true);
                    caster->CastSpell(caster, PRIEST_SPELL_SPIRIT_OF_REDEMPTION_SHAPESHIFT, true);

                    absorbAmount = caster->GetHealth() - 1;
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_spirit_of_redemption_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_spirit_of_redemption_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_spirit_of_redemption_AuraScript();
        }
};

// Called by Prayer of Mending - 33076
// Item : S12 2P bonus - Heal
class spell_pri_item_s12_4p_heal : public SpellScriptLoader
{
    public:
        spell_pri_item_s12_4p_heal() : SpellScriptLoader("spell_pri_item_s12_4p_heal") { }

        class spell_pri_item_s12_4p_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_item_s12_4p_heal_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PRIEST_SPELL_2P_S12_HEAL) || _player->HasAura(PRIEST_SPELL_2P_S12_PVPPOWER))
                            _player->CastSpell(target, PRIEST_SPELL_HOLY_SPARK, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_item_s12_4p_heal_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_item_s12_4p_heal_SpellScript();
        }
};

// Called by Dispersion - 47585
// Item : S12 2P bonus - Shadow
class spell_pri_item_s12_2p_shadow : public SpellScriptLoader
{
    public:
        spell_pri_item_s12_2p_shadow() : SpellScriptLoader("spell_pri_item_s12_2p_shadow") { }

        class spell_pri_item_s12_2p_shadow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_item_s12_2p_shadow_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_SPELL_2P_S12_SHADOW))
                        _player->CastSpell(_player, PRIEST_SPELL_DISPERSION_SPRINT, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_item_s12_2p_shadow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_item_s12_2p_shadow_SpellScript();
        }
};

// Divine Insight - 124430
class spell_pri_divine_insight_shadow : public SpellScriptLoader
{
    public:
        spell_pri_divine_insight_shadow() : SpellScriptLoader("spell_pri_divine_insight_shadow") { }

        class spell_pri_divine_insight_shadow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_insight_shadow_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasSpellCooldown(PRIEST_SPELL_MIND_BLAST))
                        _player->RemoveSpellCooldown(PRIEST_SPELL_MIND_BLAST, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_divine_insight_shadow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_divine_insight_shadow_SpellScript();
        }
};

// Power Word - Insanity - 129249
class spell_pri_power_word_insanity : public SpellScriptLoader
{
    public:
        spell_pri_power_word_insanity() : SpellScriptLoader("spell_pri_power_word_insanity") { }

        class spell_pri_power_word_insanity_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_insanity_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID()))
                            target->RemoveAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID());

                        if (target->HasAura(PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST, _player->GetGUID()))
                            target->RemoveAura(PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST, _player->GetGUID());
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_power_word_insanity_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_power_word_insanity_SpellScript();
        }
};

// Power Word - Solace - 129250
class spell_pri_power_word_solace : public SpellScriptLoader
{
    public:
        spell_pri_power_word_solace() : SpellScriptLoader("spell_pri_power_word_solace") { }

        class spell_pri_power_word_solace_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_solace_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->EnergizeBySpell(_player, GetSpellInfo()->Id, int32(_player->GetMaxPower(POWER_MANA) * 0.007f), POWER_MANA);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_power_word_solace_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_power_word_solace_SpellScript();
        }
};

// Called by Shadow Word : Pain - 589
// Shadow Word : Insanity (allowing cast) - 130733
class spell_pri_shadow_word_insanity_allowing : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_insanity_allowing() : SpellScriptLoader("spell_pri_shadow_word_insanity_allowing") { }

        class spell_pri_shadow_word_insanity_allowing_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadow_word_insanity_allowing_AuraScript);

            std::list<Unit*> targetList;

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                aurEff->GetTargetList(targetList);

                for (auto itr : targetList)
                {
                    if (Unit* caster = GetCaster())
                    {
                        if (Aura* shadowWordPain = itr->GetAura(PRIEST_SHADOW_WORD_PAIN, caster->GetGUID()))
                        {
                            if (shadowWordPain->GetDuration() <= (shadowWordPain->GetEffect(0)->GetAmplitude() * 2))
                                caster->CastSpell(itr, PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST, true);
                            else
                                itr->RemoveAura(PRIEST_SHADOW_WORD_INSANITY_ALLOWING_CAST);
                        }
                    }
                }

                targetList.clear();
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_pri_shadow_word_insanity_allowing_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_shadow_word_insanity_allowing_AuraScript();
        }
};

// Shadowfiend - 34433 or Mindbender - 123040
class spell_pri_shadowfiend : public SpellScriptLoader
{
    public:
        spell_pri_shadowfiend() : SpellScriptLoader("spell_pri_shadowfiend") { }

        class spell_pri_shadowfiend_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadowfiend_SpellScript);

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        if (Guardian* pet = _player->GetGuardianPet())
                        {
                            pet->InitCharmInfo();
                            pet->SetReactState(REACT_AGGRESSIVE);

                            if (pet->IsValidAttackTarget(target))
                                pet->ToCreature()->AI()->AttackStart(target);
                            else
                            {
                                Unit* victim = _player->GetSelectedUnit();
                                if (victim && pet->IsValidAttackTarget(target))
                                    pet->ToCreature()->AI()->AttackStart(target);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pri_shadowfiend_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_shadowfiend_SpellScript();
        }
};

// Called by Flash Heal - 2061
// Surge of Light - 114255
class spell_pri_surge_of_light : public SpellScriptLoader
{
    public:
        spell_pri_surge_of_light() : SpellScriptLoader("spell_pri_surge_of_light") { }

        class spell_pri_surge_of_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_surge_of_light_SpellScript);

            void HandleOnCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Aura* surgeOfLight = _player->GetAura(PRIEST_SURGE_OF_LIGHT))
                    {
                        int32 stacks = surgeOfLight->GetStackAmount();
                        if (stacks <= 1)
                                _player->RemoveAura(PRIEST_SURGE_OF_LIGHT);
                            else
                                surgeOfLight->SetStackAmount(stacks - 1);
                    }
                }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_pri_surge_of_light_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_surge_of_light_SpellScript();
        }
};

// Called by Leap of Faith - 73325, Power Word : Shield - 17 and Power Word : Shield - 123258
// Body and Soul - 64129
class spell_pri_body_and_soul : public SpellScriptLoader
{
    public:
        spell_pri_body_and_soul() : SpellScriptLoader("spell_pri_body_and_soul") { }

        class spell_pri_body_and_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_body_and_soul_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PRIEST_BODY_AND_SOUL_AURA))
                            _player->CastSpell(target, PRIEST_BODY_AND_SOUL_INCREASE_SPEED, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_body_and_soul_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_body_and_soul_SpellScript();
        }
};

// Prayer of Mending (Divine Insight) - 123259
class spell_pri_prayer_of_mending_divine_insight : public SpellScriptLoader
{
    public:
        spell_pri_prayer_of_mending_divine_insight() : SpellScriptLoader("spell_pri_prayer_of_mending_divine_insight") { }

        class spell_pri_prayer_of_mending_divine_insight_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_prayer_of_mending_divine_insight_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (Aura* prayerOfMending = target->GetAura(PRIEST_PRAYER_OF_MENDING_RADIUS, _player->GetGUID()))
                        {
                            int32 value = prayerOfMending->GetEffect(0)->GetAmount();

                            if (_player->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_HOLY))
                                _player->RemoveAura(PRIEST_SPELL_DIVINE_INSIGHT_HOLY);

                            target->CastCustomSpell(target, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());
                            if (target->HasAura(GetSpellInfo()->Id))
                                target->RemoveAura(GetSpellInfo()->Id);

                            float radius = sSpellMgr->GetSpellInfo(PRIEST_PRAYER_OF_MENDING_RADIUS)->Effects[0].CalcRadius(_player);

                            if (Unit* secondTarget = target->GetNextRandomRaidMemberOrPet(radius))
                            {
                                target->CastCustomSpell(secondTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());
                                if (secondTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                    secondTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                secondTarget->CastCustomSpell(secondTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());

                                if (Unit* thirdTarget = target->GetNextRandomRaidMemberOrPet(radius))
                                {
                                    secondTarget->CastCustomSpell(thirdTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());
                                    if (thirdTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                        thirdTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                    thirdTarget->CastCustomSpell(thirdTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());

                                    if (Unit* fourthTarget = target->GetNextRandomRaidMemberOrPet(radius))
                                    {
                                        thirdTarget->CastCustomSpell(fourthTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());
                                        if (fourthTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                            fourthTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                        fourthTarget->CastCustomSpell(fourthTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());

                                        if (Unit* fifthTarget = target->GetNextRandomRaidMemberOrPet(radius))
                                        {
                                            fourthTarget->CastCustomSpell(fifthTarget, PRIEST_PRAYER_OF_MENDING, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());
                                            if (fifthTarget->HasAura(PRIEST_PRAYER_OF_MENDING))
                                                fifthTarget->RemoveAura(PRIEST_PRAYER_OF_MENDING);

                                            fifthTarget->CastCustomSpell(fifthTarget, PRIEST_PRAYER_OF_MENDING_HEAL, &value, NULL, NULL, true, NULL, nullptr, _player->GetGUID());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_prayer_of_mending_divine_insight_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_prayer_of_mending_divine_insight_SpellScript();
        }
};

// Called by Greater Heal - 2060 and Prayer of Healing - 596
// Divine Insight (Holy) - 109175
class spell_pri_divine_insight_holy : public SpellScriptLoader
{
    public:
        spell_pri_divine_insight_holy() : SpellScriptLoader("spell_pri_divine_insight_holy") { }

        class spell_pri_divine_insight_holy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_insight_holy_SpellScript);

            bool m_effective;

            bool Load() override
            {
                m_effective = true;
                return true;
            }

            void HandleOnHit()
            {
                if (!m_effective)
                    return;

                if (!GetCaster())
                    return;

                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_TALENT) && caster->GetSpecializationId(caster->GetActiveSpec()) == SPEC_PRIEST_HOLY)
                        if (roll_chance_i(40))
                            caster->CastSpell(caster, PRIEST_SPELL_DIVINE_INSIGHT_HOLY, true);

                m_effective = false;
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_divine_insight_holy_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_divine_insight_holy_SpellScript();
        }
};

// Called by Power Word : Shield (Divine Insight) - 123258
// Divine Insight (Discipline) - 123266
class spell_pri_divine_insight_discipline : public SpellScriptLoader
{
    public:
        spell_pri_divine_insight_discipline() : SpellScriptLoader("spell_pri_divine_insight_discipline") { }

        class spell_pri_divine_insight_discipline_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_insight_discipline_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE))
                        caster->RemoveAura(PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE);

                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAura(PRIEST_SPELL_POWER_WORD_SHIELD))
                            target->RemoveAura(PRIEST_SPELL_POWER_WORD_SHIELD);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_divine_insight_discipline_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_divine_insight_discipline_SpellScript();
        }
};

// Holy Word : Sanctuary - 88685
class spell_pri_holy_word_sanctuary : public SpellScriptLoader
{
    public:
        spell_pri_holy_word_sanctuary() : SpellScriptLoader("spell_pri_holy_word_sanctuary") { }

        class spell_pri_holy_word_sanctuary_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_holy_word_sanctuary_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (GetCaster() == nullptr)
                    return;

                if (DynamicObject* dynObj = GetCaster()->GetDynObject(PRIEST_HOLY_WORD_SANCTUARY_AREA))
                    GetCaster()->CastSpell(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ(), PRIEST_HOLY_WORD_SANCTUARY_HEAL, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_holy_word_sanctuary_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_holy_word_sanctuary_AuraScript();
        }
};

// Called by Smite - 585
// Chakra : Chastise - 81209
class spell_pri_chakra_chastise : public SpellScriptLoader
{
    public:
        spell_pri_chakra_chastise() : SpellScriptLoader("spell_pri_chakra_chastise") { }

        class spell_pri_chakra_chastise_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_chakra_chastise_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (roll_chance_i(10))
                            if (_player->HasSpellCooldown(PRIEST_HOLY_WORD_CHASTISE))
                                _player->RemoveSpellCooldown(PRIEST_HOLY_WORD_CHASTISE, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_chakra_chastise_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_chakra_chastise_SpellScript();
        }
};

// Lightwell Renew - 60123
class spell_pri_lightwell_renew : public SpellScriptLoader
{
    public:
        spell_pri_lightwell_renew() : SpellScriptLoader("spell_pri_lightwell_renew") { }

        class spell_pri_lightwell_renew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_lightwell_renew_SpellScript);

            void HandleOnHit()
            {
                if (Unit* m_caster = GetCaster())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if (m_caster->GetTypeId() != TYPEID_UNIT || !m_caster->ToCreature()->isSummon())
                            return;

                        // proc a spellcast
                        if (Aura* chargesAura = m_caster->GetAura(LIGHTWELL_CHARGES))
                        {
                            m_caster->CastSpell(unitTarget, LIGHTSPRING_RENEW, true, NULL, nullptr, m_caster->ToTempSummon()->GetSummonerGUID());
                            if (chargesAura->ModStackAmount(-1))
                                m_caster->ToTempSummon()->UnSummon();
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_lightwell_renew_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_lightwell_renew_SpellScript();
        }
};

// Called by Heal - 2050, Greater Heal - 2060 and Flash Heal - 2061
// Strength of Soul - 89488
class spell_pri_strength_of_soul : public SpellScriptLoader
{
    public:
        spell_pri_strength_of_soul() : SpellScriptLoader("spell_pri_strength_of_soul") { }

        class spell_pri_strength_of_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_strength_of_soul_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(PRIEST_STRENGTH_OF_SOUL))
                        {
                            if (Aura* weakenedSoul = target->GetAura(PRIEST_WEAKENED_SOUL, _player->GetGUID()))
                            {
                                if (weakenedSoul->GetDuration() > 2000)
                                    weakenedSoul->SetDuration(weakenedSoul->GetDuration() - 2000);
                                else
                                    target->RemoveAura(PRIEST_WEAKENED_SOUL);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_strength_of_soul_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_strength_of_soul_SpellScript();
        }
};

// Called by Heal - 2050
// Grace - 47517
class spell_pri_grace : public SpellScriptLoader
{
    public:
        spell_pri_grace() : SpellScriptLoader("spell_pri_grace") { }

        class spell_pri_grace_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_grace_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        if (_player->HasAura(PRIEST_GRACE_AURA))
                            _player->CastSpell(target, PRIEST_GRACE_PROC, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_grace_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_grace_SpellScript();
        }
};

// Called by Smite - 585 and Greater Heal - 2060
// Train of Thought - 92297
class spell_pri_train_of_thought : public SpellScriptLoader
{
    public:
        spell_pri_train_of_thought() : SpellScriptLoader("spell_pri_train_of_thought") { }

        class spell_pri_train_of_thought_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_train_of_thought_SpellScript);

            void HandleAfterHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        ObjectGuid playerGuid = _player->GetGUID();

                        if (_player->HasAura(PRIEST_TRAIN_OF_THOUGHT))
                        {
                            if (GetSpellInfo()->Id == 585)
                            {
                                if (_player->HasSpellCooldown(PRIEST_SPELL_PENANCE))
                                    _player->ReduceSpellCooldown(PRIEST_SPELL_PENANCE, -500);
                            }
                            else if (GetSpellInfo()->Id == 2060)
                            {
                                if (_player->HasSpellCooldown(PRIEST_INNER_FOCUS))
                                    _player->ReduceSpellCooldown(PRIEST_SPELL_PENANCE, -5000);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pri_train_of_thought_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_train_of_thought_SpellScript();
        }
};

// Called by Power Word : Shield - 17
// Rapture - 47536
class spell_pri_rapture : public SpellScriptLoader
{
    public:
        spell_pri_rapture() : SpellScriptLoader("spell_pri_rapture") { }

        class spell_pri_rapture_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_rapture_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
                    {
                        int32 bp = int32(caster->GetStat(STAT_SPIRIT) * 1.5f);

                        if (caster->ToPlayer() && !caster->ToPlayer()->HasSpellCooldown(PRIEST_RAPTURE_ENERGIZE))
                        {
                            caster->EnergizeBySpell(caster, PRIEST_RAPTURE_ENERGIZE, bp, POWER_MANA);
                            caster->ToPlayer()->AddSpellCooldown(PRIEST_RAPTURE_ENERGIZE, 0, 12 * IN_MILLISECONDS);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_rapture_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_rapture_AuraScript();
        }
};

// Called by Heal - 2050, Flash Heal - 2061, Greater Heal - 2060 and Prayer of Healing - 596
// Spirit Shell - 109964
class spell_pri_spirit_shell : public SpellScriptLoader
{
    public:
        spell_pri_spirit_shell() : SpellScriptLoader("spell_pri_spirit_shell") { }

        class spell_pri_spirit_shell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_spirit_shell_SpellScript);

            void HandleOnHit()
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->HasAura(PRIEST_SPIRIT_SHELL_AURA))
                        {
                            int32 bp = GetHitHeal();

                            SetHitHeal(0);
                            
                            if (AuraEffect* shellAbsorption = target->GetAuraEffect(PRIEST_SPIRIT_SHELL_ABSORPTION, 0))
                            {
                                AddPct(bp, int32(_player->GetFloatValue(PLAYER_MASTERY) * 1.6f));

                                int32 existedbp = shellAbsorption->GetAmount();
                                int32 maxAuraAmount = int32(target->CountPctFromMaxHealth(60));
                                if (existedbp + bp < maxAuraAmount)
                                    shellAbsorption->SetAmount(existedbp + bp);
                                else
                                    shellAbsorption->SetAmount(maxAuraAmount);

                                if (Aura* aur = shellAbsorption->GetBase())
                                    aur->SetDuration(aur->GetMaxDuration());
                            }
                            else
                                _player->CastCustomSpell(target, PRIEST_SPIRIT_SHELL_ABSORPTION, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_spirit_shell_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_spirit_shell_SpellScript();
        }
};

// Devouring Plague - 2944
class spell_pri_devouring_plague : public SpellScriptLoader
{
    public:
        spell_pri_devouring_plague() : SpellScriptLoader("spell_pri_devouring_plague") { }

        class spell_pri_devouring_plague_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_devouring_plague_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PRIEST_SHADOW)
                    {
                        if (Aura* devouringPlague = GetHitUnit()->GetAura(2944))
                        {
                            uint8 powerUsed = devouringPlague->GetEffect(2)->GetAmount();

                            // Shadow Orb visual
                            if (_player->HasAura(77487))
                                _player->RemoveAura(77487);
                            // Glyph of Shadow Ravens
                            else if (_player->HasAura(127850))
                                _player->RemoveAura(127850);

                            // Instant damage equal to amount of shadow orb
                            SetHitDamage(int32(GetHitDamage() * powerUsed));

                            // Item - Priest T16 Shadow 4P Bonus - 145179
                            if (_player->HasAura(PRIEST_SPELL_4P_T16_SHADOW))
                            {
                                int32 bp = 20 * powerUsed;
                                if (Unit* target = GetHitUnit())
                                    GetCaster()->CastCustomSpell(target, PRIEST_SPELL_EMPOWERED_SHADOWS, &bp, NULL, NULL, true);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_devouring_plague_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_devouring_plague_SpellScript;
        }

        class spell_pri_devouring_plague_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_devouring_plague_AuraScript);

            uint8 powerUsed;

            bool Load()
            {
                powerUsed = 0;
                return true;
            }

            bool CalculateAmount(AuraEffect const* /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (!GetCaster())
                    return false;

                powerUsed = GetCaster()->GetPower(POWER_SHADOW_ORB) + 1;
                GetCaster()->SetPower(POWER_SHADOW_ORB, 0);

                // Calculate SpellPower from Intellect.
                int32 spellPower = GetCaster()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);

                amount = int32((amount + (spellPower * 0.131f)) * powerUsed);

                return true;
            }

            bool CalculateSecondAmount(AuraEffect const* /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (!GetCaster())
                    return false;

                amount = powerUsed;
                return true;
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                int32 bp = 1;

                if (AuraEffect const* aurEff2 = aurEff->GetBase()->GetEffect(2))
                    bp *= aurEff2->GetAmount();

                GetCaster()->CastCustomSpell(GetCaster(), PRIEST_DEVOURING_PLAGUE_HEAL, &bp, NULL, NULL, true);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_devouring_plague_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_devouring_plague_AuraScript::CalculateSecondAmount, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_devouring_plague_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_devouring_plague_AuraScript();
        }
};

// Called by Fade - 586
// Phantasm - 108942
class spell_pri_phantasm : public SpellScriptLoader
{
    public:
        spell_pri_phantasm() : SpellScriptLoader("spell_pri_phantasm") { }

        class spell_pri_phantasm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_phantasm_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_PHANTASM_AURA))
                        _player->CastSpell(_player, PRIEST_PHANTASM_PROC, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_phantasm_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_phantasm_SpellScript;
        }
};

// Mind Spike - 73510
class spell_pri_mind_spike : public SpellScriptLoader
{
    public:
        spell_pri_mind_spike() : SpellScriptLoader("spell_pri_mind_spike") { }

        class spell_pri_mind_spike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mind_spike_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // Surge of Darkness - Your next Mind Spike will not consume your damage-over-time effects ...
                        if (!_player->HasAura(PRIEST_SURGE_OF_DARKNESS))
                        {
                            // Mind Spike remove all DoT on the target's
                            if (target->HasAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID()))
                                target->RemoveAura(PRIEST_SHADOW_WORD_PAIN, _player->GetGUID());
                            if (target->HasAura(PRIEST_DEVOURING_PLAGUE, _player->GetGUID()))
                                target->RemoveAura(PRIEST_DEVOURING_PLAGUE, _player->GetGUID());
                            if (target->HasAura(PRIEST_VAMPIRIC_TOUCH, _player->GetGUID()))
                                target->RemoveAura(PRIEST_VAMPIRIC_TOUCH, _player->GetGUID());
                        }
                        // ... and deals 50% additional damage.
                        else if (Aura* surgeOfDarkness = _player->GetAura(PRIEST_SURGE_OF_DARKNESS))
                        {
                            SetHitDamage(int32(GetHitDamage() * 1.5f));

                            int32 stacks = surgeOfDarkness->GetStackAmount();
                            if (stacks <= 1)
                                _player->RemoveAura(PRIEST_SURGE_OF_DARKNESS);
                            else
                                surgeOfDarkness->SetStackAmount(stacks - 1);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_mind_spike_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mind_spike_SpellScript;
        }
};

// Called by Holy Fire - 14914, Smite - 585 and Penance - 47666
// Evangelism - 81662
class spell_pri_evangelism : public SpellScriptLoader
{
    public:
        spell_pri_evangelism() : SpellScriptLoader("spell_pri_evangelism") { }

        class spell_pri_evangelism_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_evangelism_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(PRIEST_EVANGELISM_AURA))
                        if (GetHitDamage())
                            _player->CastSpell(_player, PRIEST_EVANGELISM_STACK, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_evangelism_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_evangelism_SpellScript;
        }
};

// Archangel - 81700
class spell_pri_archangel : public SpellScriptLoader
{
    public:
        spell_pri_archangel() : SpellScriptLoader("spell_pri_archangel") { }

        class spell_pri_archangel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_archangel_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    int stackNumber = _player->GetAura(PRIEST_EVANGELISM_STACK)->GetStackAmount();
                    if (!(stackNumber > 0))
                        return;

                    if (Aura* archangel = _player->GetAura(GetSpellInfo()->Id))
                    {
                        if (archangel->GetEffect(0))
                        {
                            archangel->GetEffect(0)->ChangeAmount(archangel->GetEffect(0)->GetAmount() * stackNumber);
                            _player->RemoveAura(PRIEST_EVANGELISM_STACK);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_archangel_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_archangel_SpellScript;
        }
};

// Cascade - 127630 (damage trigger) or Cascade - 120786 (heal trigger)
class spell_pri_cascade_second : public SpellScriptLoader
{
    public:
        spell_pri_cascade_second() : SpellScriptLoader("spell_pri_cascade_second") { }

        class spell_pri_cascade_second_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cascade_second_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        std::list<Unit*> checkAuras;
                        std::list<Unit*> targetList;
                        int32 affectedUnits = 0;

                        _player->GetAttackableUnitListInRange(targetList, 40.0f);

                        for (auto itr : targetList)
                        {
                            if (itr->HasAura(PRIEST_CASCADE_INVISIBLE_AURA))
                                if (Unit* caster = itr->GetAura(PRIEST_CASCADE_INVISIBLE_AURA)->GetCaster())
                                    if (caster->GetGUID() == _player->GetGUID())
                                        affectedUnits++;
                        }

                        // Stop the script if the max targets is reached ...
                        if (affectedUnits >= 15)
                            return;

                        if (Aura* boundNumber = _player->GetAura(PRIEST_CASCADE_INVISIBLE_AURA_2))
                            if (boundNumber->GetCharges() >= 3)
                                return;

                        for (auto itr : targetList)
                            checkAuras.push_back(itr);

                        for (auto itr : checkAuras)
                        {
                            if (itr->HasAura(PRIEST_CASCADE_INVISIBLE_AURA))
                                if (Unit* caster = itr->GetAura(PRIEST_CASCADE_INVISIBLE_AURA)->GetCaster())
                                    if (caster->GetGUID() == _player->GetGUID())
                                        targetList.remove(itr);

                            if (!itr->IsWithinLOSInMap(_player))
                                targetList.remove(itr);

                            if (!itr->isInFront(_player))
                                targetList.remove(itr);

                            if (itr->GetGUID() == _player->GetGUID())
                                targetList.remove(itr);

                            // damage
                            if (GetSpellInfo()->Id == 127630)
                                if (!_player->IsValidAttackTarget(itr))
                                    targetList.remove(itr);

                            // heal
                            if (GetSpellInfo()->Id == 120786)
                                if (_player->IsValidAttackTarget(itr))
                                    targetList.remove(itr);
                        }

                        // ... or if there are no targets reachable
                        if (targetList.size() == 0)
                            return;

                        // Each bound hit twice more targets up to 8 for the same bound
                        JadeCore::Containers::RandomResizeList(targetList, (affectedUnits * 2));

                        for (auto itr : targetList)
                        {
                            if (_player->HasAura(PRIEST_SHADOWFORM_STANCE))
                            {
                                switch (GetSpellInfo()->Id)
                                {
                                    // damage
                                    case 127630:
                                        target->CastSpell(itr, PRIEST_CASCADE_SHADOW_DAMAGE, true, 0, nullptr, _player->GetGUID());
                                        break;
                                    // heal
                                    case 120786:
                                        target->CastSpell(itr, PRIEST_CASCADE_SHADOW_MISSILE, true, 0, nullptr, _player->GetGUID());
                                        target->CastSpell(itr, PRIEST_CASCADE_SHADOW_HEAL, true, 0, nullptr, _player->GetGUID());
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                switch (GetSpellInfo()->Id)
                                {
                                    // damage
                                    case 127630:
                                        target->CastSpell(itr, PRIEST_CASCADE_HOLY_DAMAGE, true, 0, nullptr, _player->GetGUID());
                                        break;
                                    // heal
                                    case 120786:
                                        target->CastSpell(itr, PRIEST_CASCADE_HOLY_MISSILE, true, 0, nullptr, _player->GetGUID());
                                        target->CastSpell(itr, PRIEST_CASCADE_HOLY_HEAL, true, 0, nullptr, _player->GetGUID());
                                        break;
                                    default:
                                        break;
                                }
                            }

                            _player->CastSpell(itr, PRIEST_CASCADE_INVISIBLE_AURA, true);
                        }

                        if (Aura* boundNumber = _player->GetAura(PRIEST_CASCADE_INVISIBLE_AURA_2))
                        {
                            boundNumber->RefreshDuration();
                            boundNumber->SetCharges(boundNumber->GetCharges() + 1);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cascade_second_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cascade_second_SpellScript;
        }
};

// Cascade - 120785 (holy damage) or Cascade - 127628 (shadow damage) or Cascade - 127627 (shadow missile) or Cascade - 121146 (holy missile)
class spell_pri_cascade_trigger : public SpellScriptLoader
{
    public:
        spell_pri_cascade_trigger() : SpellScriptLoader("spell_pri_cascade_trigger") { }

        class spell_pri_cascade_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cascade_trigger_SpellScript);

            void HandleOnHit()
            {
                if (GetOriginalCaster())
                {
                    if (Player* _player = GetOriginalCaster()->ToPlayer())
                    {
                        if (Unit* target = GetHitUnit())
                        {
                            // Trigger for SpellScript
                            if (_player->IsValidAttackTarget(target))
                                _player->CastSpell(target, PRIEST_CASCADE_DAMAGE_TRIGGER, true); // Only damage
                            else if (_player->IsValidAssistTarget(target))
                                _player->CastSpell(target, PRIEST_CASCADE_HOLY_TRIGGER, true); // Only heal
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cascade_trigger_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cascade_trigger_SpellScript;
        }
};

// Cascade (shadow) - 127632 and Cascade - 121135
class spell_pri_cascade_first : public SpellScriptLoader
{
    public:
        spell_pri_cascade_first() : SpellScriptLoader("spell_pri_cascade_first") { }

        class spell_pri_cascade_first_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_cascade_first_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        switch (GetSpellInfo()->Id)
                        {
                            case 127632:
                            {
                                // First missile
                                if (_player->IsValidAttackTarget(target))
                                    _player->CastSpell(target, PRIEST_CASCADE_SHADOW_DAMAGE, true, 0, nullptr, _player->GetGUID());
                                else if (_player->IsValidAssistTarget(target))
                                {
                                    _player->CastSpell(target, PRIEST_CASCADE_SHADOW_MISSILE, true, 0, nullptr, _player->GetGUID());
                                    _player->CastSpell(target, PRIEST_CASCADE_SHADOW_HEAL, true, 0, nullptr, _player->GetGUID());
                                }

                                break;
                            }
                            case 121135:
                            {
                                // First missile
                                if (_player->IsValidAttackTarget(target))
                                    _player->CastSpell(target, PRIEST_CASCADE_HOLY_DAMAGE, true, 0, nullptr, _player->GetGUID());
                                else if (_player->IsValidAssistTarget(target))
                                {
                                    _player->CastSpell(target, PRIEST_CASCADE_HOLY_MISSILE, true, 0, nullptr, _player->GetGUID());
                                    _player->CastSpell(target, PRIEST_CASCADE_HOLY_HEAL, true, 0, nullptr, _player->GetGUID());
                                }

                                break;
                            }
                        }

                        // Invisible aura : Each target cannot be hit more than once time [...]
                        _player->CastSpell(target, PRIEST_CASCADE_INVISIBLE_AURA, true);
                        // Invisible aura 2 : [...] or Cascade can bound three times
                        _player->CastSpell(_player, PRIEST_CASCADE_INVISIBLE_AURA_2, true); // First bound

                        if (Aura* boundNumber = _player->GetAura(PRIEST_CASCADE_INVISIBLE_AURA_2))
                            boundNumber->SetCharges(1);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_cascade_first_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_cascade_first_SpellScript;
        }
};

// Halo (shadow) - 120696 and Halo - 120692 : Heal
class spell_pri_halo_heal : public SpellScriptLoader
{
    public:
        spell_pri_halo_heal() : SpellScriptLoader("spell_pri_halo_heal") { }

        class spell_pri_halo_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_halo_heal_SpellScript);

            void HandleHeal(SpellEffIndex eff)
            {
                if (Unit* _player = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 heal = GetHitHeal();
                        heal += int32(_player->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * 3.25f);

                        float Distance = _player->GetDistance(target);
                        float pct = Distance / 25.0f;
                        heal = int32(heal * pct);

                        SetHitHeal(heal);
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_halo_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_halo_heal_SpellScript;
        }
};

// Halo (shadow) - 120517 and Halo - 120644 : Damage
class spell_pri_halo_damage : public SpellScriptLoader
{
    public:
        spell_pri_halo_damage() : SpellScriptLoader("spell_pri_halo_damage") { }

        class spell_pri_halo_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_halo_damage_SpellScript);

            void HandleDamage(SpellEffIndex eff)
            {
                if (Unit* _player = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 damage = GetHitDamage();
                        damage += int32(_player->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * 1.95f);

                        float Distance = _player->GetDistance(target);
                        float pct = Distance / 25.0f;
                        damage = int32(damage * pct);

                        SetHitDamage(damage);
                    }
                }
            }

            void HandleScript(SpellEffIndex eff)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (GetSpellInfo()->Id == 120517)
                        {
                            _player->CastSpell(target, PRIEST_SPELL_HALO_HEAL_HOLY, true);
                            // visual effect - to enable it we should fix a speed of animation
                            //_player->CastSpell(_player, 120630, true);
                        }
                        else
                        {
                            _player->CastSpell(target, PRIEST_SPELL_HALO_HEAL_SHADOW, true);
                            // visual effect
                            //_player->CastSpell(_player, 120643, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_halo_damage_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnEffectHitTarget += SpellEffectFn(spell_pri_halo_damage_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_halo_damage_SpellScript;
        }
};

// Inner Fire - 588 or Inner Will - 73413
class spell_pri_inner_fire_or_will : public SpellScriptLoader
{
    public:
        spell_pri_inner_fire_or_will() : SpellScriptLoader("spell_pri_inner_fire_or_will") { }

        class spell_pri_inner_fire_or_will_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_inner_fire_or_will_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_INNER_FIRE) || !sSpellMgr->GetSpellInfo(PRIEST_INNER_WILL))
                    return false;
                return true;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (GetSpellInfo()->Id == PRIEST_INNER_FIRE)
                    {
                        if (_player->HasAura(PRIEST_INNER_WILL))
                            _player->RemoveAura(PRIEST_INNER_WILL);
                        if (_player->HasAura(PRIEST_GLYPH_OF_INNER_SANCTUM))
                            _player->CastSpell(_player, PRIEST_SPELL_SPELL_WARDING, true);
                    }
                    else if (GetSpellInfo()->Id == PRIEST_INNER_WILL)
                    {
                        if (_player->HasAura(PRIEST_INNER_FIRE))
                            _player->RemoveAura(PRIEST_INNER_FIRE);
                        if (_player->HasAura(PRIEST_SPELL_SPELL_WARDING))
                            _player->RemoveAura(PRIEST_SPELL_SPELL_WARDING);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_inner_fire_or_will_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_inner_fire_or_will_SpellScript;
        }
};

// Glyph of Inner Sanctum - 14771
// Inner Fire - 588
class spell_pri_inner_fire : public SpellScriptLoader
{
    public:
        spell_pri_inner_fire() : SpellScriptLoader("spell_pri_inner_fire") { }

        class spell_pri_inner_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_inner_fire_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* _player = GetCaster())
                    _player->RemoveAura(PRIEST_SPELL_SPELL_WARDING); /// don't bother checking if aura is there 'cause remove aura does anyway
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_inner_fire_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_RESISTANCE_PCT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_inner_fire_AuraScript();
        }
};

// Leap of Faith - 73325 and Leap of Faith - 110718 (Symbiosis)
class spell_pri_leap_of_faith : public SpellScriptLoader
{
    public:
        spell_pri_leap_of_faith() : SpellScriptLoader("spell_pri_leap_of_faith") { }

        class spell_pri_leap_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_leap_of_faith_SpellScript);

            void HandleOnHit()
            {
                if (Unit* _caster = GetCaster())
                    if (Unit* target = GetHitUnit())
                        target->CastSpell(_caster, PRIEST_LEAP_OF_FAITH_JUMP, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_leap_of_faith_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_leap_of_faith_SpellScript;
        }
};

// Void Shift - 108968
class spell_pri_void_shift : public SpellScriptLoader
{
    public:
        spell_pri_void_shift() : SpellScriptLoader("spell_pri_void_shift") { }

        class spell_pri_void_shift_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_void_shift_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_VOID_SHIFT))
                    return false;
                return true;
            }

            SpellCastResult CheckTarget()
            {
                if (GetExplTargetUnit())
                    if (GetExplTargetUnit()->GetTypeId() != TYPEID_PLAYER)
                        return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        float playerPct;
                        float targetPct;

                        playerPct = _player->GetHealthPct();
                        targetPct = target->GetHealthPct();

                        if (playerPct < 25.0f)
                            playerPct = 25.0f;
                        if (targetPct < 25.0f)
                            targetPct = 25.0f;

                        playerPct /= 100.0f;
                        targetPct /= 100.0f;

                        _player->SetHealth(_player->GetMaxHealth() * targetPct);
                        target->SetHealth(target->GetMaxHealth() * playerPct);
                    }
                }
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pri_void_shift_SpellScript::CheckTarget);
                OnEffectHitTarget += SpellEffectFn(spell_pri_void_shift_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_void_shift_SpellScript;
        }
};

// Psychic Horror - 64044
class spell_pri_psychic_horror : public SpellScriptLoader
{
    public:
        spell_pri_psychic_horror() : SpellScriptLoader("spell_pri_psychic_horror") { }

        class spell_pri_psychic_horror_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_psychic_horror_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (caster->ToPlayer() && caster->ToPlayer()->GetSpecializationId(caster->ToPlayer()->GetActiveSpec()) == SPEC_PRIEST_SHADOW)
                        {
                            int32 currentPower = caster->GetPower(POWER_SHADOW_ORB);
                            caster->ModifyPower(POWER_SHADOW_ORB, -currentPower);

                            // +1s per Shadow Orb consumed
                            if (Aura* psychicHorror = target->GetAura(64044))
                            {
                                int32 maxDuration = psychicHorror->GetMaxDuration();
                                int32 newDuration = maxDuration + (currentPower + 1) * IN_MILLISECONDS;
                                psychicHorror->SetDuration(newDuration);

                                if (newDuration > maxDuration)
                                    psychicHorror->SetMaxDuration(newDuration);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_psychic_horror_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_psychic_horror_SpellScript();
        }
};

// Guardian Spirit - 47788
class spell_pri_guardian_spirit : public SpellScriptLoader
{
    public:
        spell_pri_guardian_spirit() : SpellScriptLoader("spell_pri_guardian_spirit") { }

        class spell_pri_guardian_spirit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_guardian_spirit_AuraScript);

            uint32 healPct;

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SPELL_GUARDIAN_SPIRIT_HEAL))
                    return false;
                return true;
            }

            bool Load()
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                return true;
            }

            bool CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
                return true;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                if (dmgInfo.GetDamage() < target->GetHealth())
                {
                    PreventDefaultAction();
                    return;
                }

                absorbAmount = dmgInfo.GetDamage();

                // remove the aura now, we don't want 40% healing bonus
                Remove(AURA_REMOVE_BY_ENEMY_SPELL);

                int32 healAmount = int32(target->CountPctFromMaxHealth(healPct));
                target->CastCustomSpell(target, PRIEST_SPELL_GUARDIAN_SPIRIT_HEAL, &healAmount, NULL, NULL, true);
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit_AuraScript::Absorb, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_guardian_spirit_AuraScript();
        }
};

// Penance - 47540
class spell_pri_penance : public SpellScriptLoader
{
    public:
        spell_pri_penance() : SpellScriptLoader("spell_pri_penance") { }

        class spell_pri_penance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_penance_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* spellEntry)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SPELL_PENANCE))
                    return false;
                // can't use other spell than this penance due to spell_ranks dependency
                if (sSpellMgr->GetFirstSpellInChain(PRIEST_SPELL_PENANCE) != sSpellMgr->GetFirstSpellInChain(spellEntry->Id))
                    return false;

                uint8 rank = sSpellMgr->GetSpellRank(spellEntry->Id);
                if (!sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_DAMAGE, rank, true))
                    return false;
                if (!sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_HEAL, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if (!unitTarget->isAlive())
                            return;

                        uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);

                        if (_player->IsFriendlyTo(unitTarget))
                            _player->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_HEAL, rank), false, 0);
                        else
                            _player->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_DAMAGE, rank), false, 0);

                        // Divine Insight (Discipline)
                        if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PRIEST_DISCIPLINE)
                            if (_player->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_TALENT))
                                    _player->CastSpell(_player, PRIEST_SPELL_DIVINE_INSIGHT_DISCIPLINE, true);
                    }
                }
            }

            SpellCastResult CheckCast()
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetExplTargetUnit())
                    if (!caster->IsFriendlyTo(target) && !caster->IsValidAttackTarget(target))
                        return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }

            void Register()
            {
                // add dummy effect spell handler to Penance
                OnEffectHitTarget += SpellEffectFn(spell_pri_penance_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_pri_penance_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_penance_SpellScript;
        }
};

// Reflective Shield
class spell_pri_reflective_shield_trigger : public SpellScriptLoader
{
    public:
        spell_pri_reflective_shield_trigger() : SpellScriptLoader("spell_pri_reflective_shield_trigger") { }

        class spell_pri_reflective_shield_trigger_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_reflective_shield_trigger_AuraScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SPELL_REFLECTIVE_SHIELD_TRIGGERED) || !sSpellMgr->GetSpellInfo(PRIEST_SPELL_REFLECTIVE_SHIELD_R1))
                    return false;
                return true;
            }

            void Trigger(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                if (!target || !dmgInfo.GetAttacker() || dmgInfo.GetAttacker() == target)
                    return;

                if (dmgInfo.GetSpellInfo() && ((dmgInfo.GetSpellInfo()->AttributesEx & SPELL_ATTR1_CANT_BE_REDIRECTED) || (dmgInfo.GetSpellInfo()->AttributesEx & SPELL_ATTR1_CANT_BE_REFLECTED)))
                    return;

                if (GetCaster())
                    if (AuraEffect* talentAurEff = target->GetAuraEffect(PRIEST_SPELL_REFLECTIVE_SHIELD_R1, EFFECT_0))
                    {
                        int32 bp = CalculatePct(absorbAmount+dmgInfo.GetDamage(), talentAurEff->GetAmount());
                        target->CastCustomSpell(dmgInfo.GetAttacker(), PRIEST_SPELL_REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
                    }
            }

            void Register()
            {
                AfterEffectAbsorb += AuraEffectAbsorbFn(spell_pri_reflective_shield_trigger_AuraScript::Trigger, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_reflective_shield_trigger_AuraScript();
        }
};

enum PrayerOfMending
{
    SPELL_T9_HEALING_2_PIECE = 67201,
};

// Prayer of Mending Heal
class spell_pri_prayer_of_mending_heal : public SpellScriptLoader
{
    public:
        spell_pri_prayer_of_mending_heal() : SpellScriptLoader("spell_pri_prayer_of_mending_heal") { }

        class spell_pri_prayer_of_mending_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_prayer_of_mending_heal_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetOriginalCaster())
                {
                    if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_T9_HEALING_2_PIECE, EFFECT_0))
                    {
                        int32 heal = GetHitHeal();
                        AddPct(heal, aurEff->GetAmount());
                        SetHitHeal(heal);
                    }
                }

            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_prayer_of_mending_heal_SpellScript();
        }
};

// Vampiric Touch - 34914
class spell_pri_vampiric_touch : public SpellScriptLoader
{
    public:
        spell_pri_vampiric_touch() : SpellScriptLoader("spell_pri_vampiric_touch") { }

        class spell_pri_vampiric_touch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_vampiric_touch_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (GetCaster())
                    GetCaster()->EnergizeBySpell(GetCaster(), GetSpellInfo()->Id, GetCaster()->CountPctFromMaxMana(2), POWER_MANA);
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* dispeller = dispelInfo->GetDispeller())
                        if (caster->HasAura(PRIEST_SPELL_4P_S12_SHADOW))
                            dispeller->CastSpell(dispeller, PRIEST_SPELL_SIN_AND_PUNISHMENT, true, 0, nullptr, caster->GetGUID());
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_vampiric_touch_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
                AfterDispel += AuraDispelFn(spell_pri_vampiric_touch_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_vampiric_touch_AuraScript();
        }
};

// Called by Renew - 139
// Rapid Renew - 95649
class spell_priest_renew : public SpellScriptLoader
{
    public:
        spell_priest_renew() : SpellScriptLoader("spell_priest_renew") { }

        class spell_priest_renew_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_priest_renew_AuraScript);

            void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    // Empowered Renew
                    if (Aura* empoweredRenew = caster->GetAura(PRIEST_RAPID_RENEWAL_AURA))
                    {
                        uint32 heal = caster->SpellHealingBonusDone(GetTarget(), GetSpellInfo(), GetEffect(EFFECT_0)->GetAmount(), DOT);
                        heal = GetTarget()->SpellHealingBonusTaken(caster, GetSpellInfo(), EFFECT_0, heal, DOT);

                        int32 basepoints0 = empoweredRenew->GetEffect(EFFECT_2)->GetAmount() * GetEffect(EFFECT_0)->GetTotalTicks() * int32(heal) / 100;
                        caster->CastCustomSpell(GetTarget(), PRIEST_SPELL_EMPOWERED_RENEW, &basepoints0, NULL, NULL, true, NULL, aurEff);
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_priest_renew_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_priest_renew_AuraScript();
        }
};

// Called by Shadow Form - 15473
// Glyph of Shadow - 107906
class spell_pri_shadowform : public SpellScriptLoader
{
    public:
        spell_pri_shadowform() : SpellScriptLoader("spell_pri_shadowform") { }

        class spell_pri_shadowform_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadowform_AuraScript);

            bool Validate(SpellInfo const* /*entry*/)
            {
                if (!sSpellMgr->GetSpellInfo(PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH) ||
                    !sSpellMgr->GetSpellInfo(PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH))
                    return false;
                return true;
            }

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell(GetTarget(), GetTarget()->HasAura(PRIEST_GLYPH_OF_SHADOW) ? PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH : PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH, true);
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->RemoveAurasDueToSpell(GetTarget()->HasAura(PRIEST_GLYPH_OF_SHADOW) ? PRIEST_SHADOWFORM_VISUAL_WITH_GLYPH : PRIEST_SHADOWFORM_VISUAL_WITHOUT_GLYPH);
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_shadowform_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadowform_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_shadowform_AuraScript();
        }
};

// Levitate - 1706
class spell_pri_levitate : public SpellScriptLoader
{
    public:
        spell_pri_levitate() : SpellScriptLoader("spell_pri_levitate") { }

        class spell_pri_levitate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_levitate_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (GetCaster())
                    if (GetHitUnit())
                        GetCaster()->CastSpell(GetHitUnit(), PRIEST_SPELL_LEVITATE, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_levitate_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_levitate_SpellScript;
        }
};

class spell_pri_shadow_word_death : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_death() : SpellScriptLoader("spell_pri_shadow_word_death") { }

        class spell_pri_shadow_word_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadow_word_death_SpellScript);

            void HandleDamage()
            {
                if (!GetExplTargetUnit() || GetExplTargetUnit()->isDead())
                    return;

                int32 damage = GetHitDamage();

                // Pain and Suffering reduces damage
                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(47580, 1))
                    AddPct(damage, aurEff->GetAmount());
                else if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(47581, 1))
                    AddPct(damage, aurEff->GetAmount());

                // Item - Priest T13 Shadow 2P Bonus (Shadow Word: Death)
                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(105843, 1))
                    AddPct(damage, -aurEff->GetAmount());

                GetCaster()->CastCustomSpell(GetCaster(), PRIEST_SHADOW_WORD_DEATH, &damage, 0, 0, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_shadow_word_death_SpellScript::HandleDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_shadow_word_death_SpellScript();
        }
};

// Glyph of Holy Nova - 125045
class spell_pri_glyph_of_holy_nova : public SpellScriptLoader
{
    public:
        spell_pri_glyph_of_holy_nova() : SpellScriptLoader("spell_pri_glyph_of_holy_nova") { }

        class spell_pri_glyph_of_holy_nova_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_glyph_of_holy_nova_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(PRIEST_SPELL_HOLY_NOVA, false);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(PRIEST_SPELL_HOLY_NOVA))
                        _player->removeSpell(PRIEST_SPELL_HOLY_NOVA, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_pri_glyph_of_holy_nova_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_glyph_of_holy_nova_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_glyph_of_holy_nova_AuraScript();
        }
};

// Glyph of Mind Flay - 120585
// Mind Flay - 15407
class spell_pri_mind_flay : public SpellScriptLoader
{
    public:
        spell_pri_mind_flay() : SpellScriptLoader("spell_pri_mind_flay") { }

        class spell_pri_mind_flay_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_mind_flay_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetSpecializationId(_player->GetActiveSpec()) == SPEC_PRIEST_SHADOW && _player->HasAura(PRIEST_GLYPH_OF_MIND_FLAY))
                        _player->CastSpell(_player, PRIEST_GLYPH_OF_MIND_FLAY_EFFECT, true);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_mind_flay_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_mind_flay_AuraScript();
        }
};

// Power Word : Shield - 17
class spell_pri_power_word_shield : public SpellScriptLoader
{
    public:
        spell_pri_power_word_shield() : SpellScriptLoader("spell_pri_power_word_shield") { }

        class spell_pri_power_word_shield_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_shield_SpellScript);

            void HandleBeforeCast()
            {
                if (Unit* _caster = GetCaster())
                    if (_caster->HasAura(PRIEST_SPELL_POWER_WORD_SHIELD_DIVINE_INSIGHT) && !_caster->HasAura(PRIEST_WEAKENED_SOUL))
                        _caster->RemoveAura(PRIEST_SPELL_POWER_WORD_SHIELD_DIVINE_INSIGHT);
            }

            void HandleAfterHit()
            {
                if (Unit* _caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        float crit_chance = _caster->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_MASK_HOLY);

                        if (roll_chance_f(crit_chance) && _caster->HasAura(47515))
                        {
                            if (Aura* powerWordShield = target->GetAura(PRIEST_SPELL_POWER_WORD_SHIELD))
                            {
                                int32 bp = powerWordShield->GetEffect(0)->GetAmount();
                                powerWordShield->GetEffect(0)->SetAmount(bp * 2);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                BeforeCast += SpellCastFn(spell_pri_power_word_shield_SpellScript::HandleBeforeCast);
                AfterHit += SpellHitFn(spell_pri_power_word_shield_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_power_word_shield_SpellScript();
        }
};

// Glyph of Shadow Word : Death - 120583
// Shadow Word : Death (glyph spell) - 129176
class spell_pri_glyph_of_shadow_word_death : public SpellScriptLoader
{
    public:
        spell_pri_glyph_of_shadow_word_death() : SpellScriptLoader("spell_pri_glyph_of_shadow_word_death") { }

        class spell_pri_glyph_of_shadow_word_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_glyph_of_shadow_word_death_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetTarget()->ToPlayer())
                    _player->learnSpell(PRIEST_SPELL_GLYPH_OF_SHADOW_WORD_DEATH, false);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetTarget()->ToPlayer())
                    if (_player->HasSpell(PRIEST_SPELL_GLYPH_OF_SHADOW_WORD_DEATH))
                        _player->removeSpell(PRIEST_SPELL_GLYPH_OF_SHADOW_WORD_DEATH, false, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_pri_glyph_of_shadow_word_death_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_glyph_of_shadow_word_death_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pri_glyph_of_shadow_word_death_AuraScript();
        }
};

// Shadow Word: Death - 129176 (spell with glyph of Shadow Word : Death - 120583)
class spell_pri_of_shadow_word_death_spell_glyph : public SpellScriptLoader
{
    public:
        spell_pri_of_shadow_word_death_spell_glyph() : SpellScriptLoader("spell_pri_of_shadow_word_death_spell_glyph") { }

        class spell_pri_of_shadow_word_death_spell_glyph_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_of_shadow_word_death_spell_glyph_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->GetHealth() > CalculatePct(target->GetMaxHealth(), 20))
                        {
                            int32 bp = GetHitDamage();
                            caster->CastCustomSpell(target,  PRIEST_SHADOW_WORD_DEATH, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_of_shadow_word_death_spell_glyph_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_of_shadow_word_death_spell_glyph_SpellScript();
        }
};

// Called by Mind Blast - 8092
// Fix for Divine Insight (shadow) if proc while player is casting - 124430
class spell_pri_mind_blast : public SpellScriptLoader
{
    public:
        spell_pri_mind_blast() : SpellScriptLoader("spell_pri_mind_blast") { }

        class spell_pri_mind_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mind_blast_SpellScript);

            void HandleAfterCast()
            {
                if (!GetCaster())
                    return;

                if (Player* _caster = GetCaster()->ToPlayer())
                {
                    if (_caster->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_SHADOW))
                    {
                        if (_caster->HasSpellCooldown(PRIEST_SPELL_MIND_BLAST))
                            _caster->RemoveSpellCooldown(PRIEST_SPELL_MIND_BLAST, true);
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_pri_mind_blast_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mind_blast_SpellScript();
        }
};

// binding heal 32546
class spell_binding_heal : public SpellScriptLoader
{
    public:
        spell_binding_heal() : SpellScriptLoader("spell_binding_heal") { }

        class spell_binding_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_binding_heal_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                 unitList.clear();
                 unitList.push_back(GetExplTargetUnit());
                 if (Unit* _caster = GetCaster())
                 {
                     if (_caster->HasAura(63248))
                    {
                       std::list<Unit*> targets;
                       JadeCore::AnyFriendlyUnitInObjectRangeCheck u_check(_caster, _caster, 20.0f);
                       JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> searcher(_caster, targets, u_check);
                       _caster->VisitNearbyObject(20, searcher);

                       if (!targets.empty())
                       {
                           targets.remove(_caster);
                           targets.remove(GetExplTargetUnit());

                           // remove not LoS targets
                           for (std::list<Unit*>::iterator tIter = targets.begin(); tIter != targets.end();)
                           {
                               if (!_caster->IsWithinLOSInMap(*tIter) || (*tIter)->isTotem() || (*tIter)->isSpiritService() || (*tIter)->GetCreatureType() == CREATURE_TYPE_CRITTER)
                                   targets.erase(tIter++);
                               else
                                   ++tIter;
                           }

                           // no appropriate targets
                           if (targets.empty())
                               return;
                       }
                        unitList.push_back(JadeCore::Containers::SelectRandomContainerElement(targets));
                    }
                 }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_binding_heal_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_binding_heal_SpellScript();
        }
};

// Called by Spectral Guise - 112833
class spell_pri_spectral_guise : public SpellScriptLoader
{
    public:
        spell_pri_spectral_guise() : SpellScriptLoader("spell_pri_spectral_guise") { }

        class spell_pri_spectral_guise_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_spectral_guise_SpellScript);

            void HandleBeforeHit()
            {
                if (Unit* _caster = GetCaster())
                {
                    // Just hack for Shroud of Concealment, to prevent use bug
                    if (_caster->HasAura(115834))
                        _caster->RemoveAura(115834);
                }
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_pri_spectral_guise_SpellScript::HandleBeforeHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_spectral_guise_SpellScript();
        }
};

Position divinestarFirstPos;
//Position divinestarEndPos;
float divinestarDistance;

class spell_priest_divine_star : public SpellScriptLoader
{
    public:
        spell_priest_divine_star() : SpellScriptLoader("spell_priest_divine_star") { }

        class spell_priest_divine_star_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_priest_divine_star_SpellScript);

            void HandleBeforeCast()
            {
                if (!GetCaster())
                    return;

                Unit* caster = GetCaster();

                uint32 spellId = 58880;
                if (caster->HasAura(15473)) // shadow form
                    spellId = 122127;

                caster->GetPosition(&divinestarFirstPos);

                Position divinestarEndPos;
                caster->GetNearPosition(divinestarEndPos, 24.0f, 0.0f);
                divinestarDistance = divinestarFirstPos.GetExactDist(&divinestarEndPos);

                GetCaster()->CastSpell(divinestarEndPos.GetPositionX(), divinestarEndPos.GetPositionY(), divinestarEndPos.GetPositionZ(), spellId, true);
            }

            void Register()
            {
                BeforeCast += SpellCastFn(spell_priest_divine_star_SpellScript::HandleBeforeCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_priest_divine_star_SpellScript();
        }
};

class spell_priest_divine_star_aoe : public SpellScriptLoader
{
public:
    spell_priest_divine_star_aoe() : SpellScriptLoader("spell_priest_divine_star_aoe") { }

    class spell_priest_divine_star_aoe_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_priest_divine_star_aoe_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if (!GetCaster())
            {
                unitList.clear();
                return;
            }

            Unit* caster = GetCaster();

            AuraEffect* aurEff = caster->GetAuraEffect(110744, EFFECT_0);
            if (aurEff == nullptr)
                aurEff = caster->GetAuraEffect(122121, EFFECT_0);
            if (aurEff == nullptr)
            {
                unitList.clear();
                return;
            }

            uint32 max_ticks = divinestarDistance * 9 / 24;
            if (max_ticks == 0) 
                max_ticks = 1;

            uint64 delay = (uint64)floor(divinestarDistance / 24 * 1000.0f);

            uint32 time = 15000 - aurEff->GetBase()->GetDuration();

            uint32 current_tick = aurEff->GetTickNumber();
            if (current_tick == 0)
                current_tick = 1;

            if (current_tick > max_ticks)
            {
                unitList.clear();
                aurEff->GetBase()->Remove();
                return;
            }

            float current_distance = 0.0f;
            if (current_tick <= 5)
                current_distance = current_tick * 6.0f - 4.0f;
            else
                current_distance = (10 - current_tick) * 6.0f - 4.0f;

            Position currentPos;
            currentPos.Relocate(divinestarFirstPos);

            divinestarFirstPos.MovePosition(currentPos, current_distance, 0.0f, caster);
            //caster->GetNearPosition(currentPos, current_distance, 0.0f);

            unitList.remove_if(PlayerCheck(currentPos));

            // Cast visual back to caster
            if (current_tick == 5)
            {
                if (Creature* trigger = caster->SummonCreature(WORLD_TRIGGER, currentPos.GetPositionX(), currentPos.GetPositionY(), currentPos.GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 1000))
                {
                    uint32 spellId = 58880;
                    if (caster->HasAura(15473)) // shadow form
                        spellId = 122127;

                    Position newPosition;
                    caster->GetPosition(&newPosition);

                    trigger->CastSpell(newPosition.GetPositionX(), newPosition.GetPositionY(), newPosition.GetPositionZ(), spellId, true);
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_priest_divine_star_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_priest_divine_star_aoe_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
        }

    private:

        class PlayerCheck
        {
            public:
                PlayerCheck(Position pos) : i_pos(pos) {}
                bool operator() (WorldObject* unit)
                {
                    if (unit->GetDistance(i_pos) <= 4.0f)
                        return false;
                    return true;
                }
            private:
                Position i_pos;
        };
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_priest_divine_star_aoe_SpellScript();
    }
};

// Called by Greater Heal - 2060, Holy Word: Serenity - 88684 and Flash Heal - 2061
// Removing of Item : S12 2P bonus - Heal
class spell_pri_holy_spark : public SpellScriptLoader
{
    public:
        spell_pri_holy_spark() : SpellScriptLoader("spell_pri_holy_spark") { }

        class spell_pri_holy_spark_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_holy_spark_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (target->HasAura(PRIEST_SPELL_2P_S12_HEAL, caster->GetGUID()))
                            target->RemoveAura(PRIEST_SPELL_2P_S12_HEAL);
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_holy_spark_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_holy_spark_SpellScript();
        }
};

// For spells triggered by Mastery: Shadowy Recall - 77486
// Called by - Shadow Word: Pain (124464), Devouring Plague (124467), Mind Flay (124468) and Vampiric Touch (124465).
class spell_pri_mastery_shadowy_recall : public SpellScriptLoader
{
    public:
        spell_pri_mastery_shadowy_recall() : SpellScriptLoader("spell_pri_mastery_shadowy_recall") { }

        class spell_pri_mastery_shadowy_recall_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mastery_shadowy_recall_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        switch (GetSpellInfo()->Id)
                        {
                            case 124464:
                                // Shadowy Apparitions
                                if (GetSpell()->IsCritForTarget(target))
                                {
                                    caster->SendPlaySpellVisual(33584, target, 6.f);
                                    caster->CastSpell(target, 147193, true);
                                }

                                // Divine Insight (shadow) - 109175
                                if (caster->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_TALENT) && roll_chance_i(5))
                                {
                                    if (!caster->HasAura(PRIEST_SPELL_DIVINE_INSIGHT_SHADOW))
                                        caster->CastSpell(caster, PRIEST_SPELL_DIVINE_INSIGHT_SHADOW, true);
                                }
                                break;
                            case 124467:
                                if (Aura* devouringPlague = target->GetAura(2944))
                                {
                                    int32 bp = 1;
                                    bp *= devouringPlague->GetEffect(2)->GetAmount();
                                    GetCaster()->CastCustomSpell(GetCaster(), PRIEST_DEVOURING_PLAGUE_HEAL, &bp, NULL, NULL, true);
                                }
                                break;
                            case 124468:
                                // Solace and Insanity - 139139
                                if (caster->HasSpell(139139))
                                {
                                    if (Aura* devouringPlague = target->GetAura(2944))
                                    {
                                        int32 damage = GetHitDamage();
                                        uint8 powerUsed = devouringPlague->GetEffect(2)->GetAmount();
                                        damage *= (powerUsed * 0.33f + 1);
                                        SetHitDamage(damage);
                                    }
                                }
                                break;
                            case 124465:
                                caster->EnergizeBySpell(caster, GetSpellInfo()->Id, caster->CountPctFromMaxMana(2), POWER_MANA);
                                break;      
                            default:
                                break;
                        }
                    }
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_pri_mastery_shadowy_recall_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_mastery_shadowy_recall_SpellScript();
        }
};

class spell_area_priest_angelic_feather : public SpellAreaTriggerScript
{
public:
    spell_area_priest_angelic_feather() : SpellAreaTriggerScript("spell_area_priest_angelic_feather") {}

    bool OnAddTarget(AreaTrigger* trigger, Unit* target)
    {
        if (!trigger->GetCaster())
            return false;

        if (trigger->GetDuration() > 0)
        {
            trigger->GetCaster()->CastSpell(target, 121557, true);
            trigger->SetDuration(0);
        }
        return false;
    }
};

class spell_area_priest_power_word_barrier: public SpellAreaTriggerScript
{
    public:
        spell_area_priest_power_word_barrier() : SpellAreaTriggerScript("spell_area_priest_power_word_barrier") {}

        bool OnAddTarget(AreaTrigger* trigger, Unit* target)
        {
            Unit* caster = trigger->GetCaster();
            if (!caster)
                return false;

            // skip not valid assist targets
            if (!caster->_IsValidAssistTarget(target, sSpellMgr->GetSpellInfo(81782)))
                return false;

            target->AddAura(81782, target);

            return true;
        }

        bool OnRemoveTarget(AreaTrigger* trigger, Unit* target)
        {
            target->RemoveAura(81782);

            return true;
        }
};

void AddSC_priest_spell_scripts()
{
    new spell_pri_power_word_fortitude();
    new spell_pri_spectral_guise_charges();
    new spell_pri_psyfiend_hit_me_driver();
    new spell_pri_void_tendrils();
    new spell_pri_phantasm_proc();
    new spell_pri_spirit_of_redemption_form();
    new spell_pri_spirit_of_redemption();
    new spell_pri_item_s12_4p_heal();
    new spell_pri_item_s12_2p_shadow();
    new spell_pri_divine_insight_shadow();
    new spell_pri_power_word_insanity();
    new spell_pri_power_word_solace();
    new spell_pri_shadow_word_insanity_allowing();
    new spell_pri_shadowfiend();
    new spell_pri_surge_of_light();
    new spell_pri_body_and_soul();
    new spell_pri_prayer_of_mending_divine_insight();
    new spell_pri_divine_insight_holy();
    new spell_pri_divine_insight_discipline();
    new spell_pri_holy_word_sanctuary();
    new spell_pri_chakra_chastise();
    new spell_pri_lightwell_renew();
    new spell_pri_strength_of_soul();
    new spell_pri_grace();
    new spell_pri_train_of_thought();
    new spell_pri_rapture();
    new spell_pri_spirit_shell();
    new spell_pri_devouring_plague();
    new spell_pri_phantasm();
    new spell_pri_mind_spike();
    new spell_pri_cascade_second();
    new spell_pri_cascade_trigger();
    new spell_pri_cascade_first();
    new spell_pri_halo_heal();
    new spell_pri_halo_damage();
    new spell_pri_inner_fire_or_will();
    new spell_pri_inner_fire();
    new spell_pri_leap_of_faith();
    new spell_pri_void_shift();
    new spell_pri_psychic_horror();
    new spell_pri_guardian_spirit();
    new spell_pri_penance();
    new spell_pri_reflective_shield_trigger();
    new spell_pri_prayer_of_mending_heal();
    new spell_pri_vampiric_touch();
    new spell_priest_renew();
    new spell_pri_shadowform();
    new spell_pri_evangelism();
    new spell_pri_archangel();
    new spell_pri_levitate();
    new spell_pri_shadow_word_death();
    new spell_pri_glyph_of_holy_nova();
    new spell_pri_mind_flay();
    new spell_pri_power_word_shield();
    new spell_pri_glyph_of_shadow_word_death();
    new spell_pri_of_shadow_word_death_spell_glyph();
    new spell_pri_mind_blast();
    new spell_binding_heal();
    new spell_pri_spectral_guise();
    new spell_priest_divine_star();
    new spell_priest_divine_star_aoe();
    new spell_pri_holy_spark();
    new spell_pri_mastery_shadowy_recall();

    new spell_area_priest_angelic_feather();
    new spell_area_priest_power_word_barrier();
}
