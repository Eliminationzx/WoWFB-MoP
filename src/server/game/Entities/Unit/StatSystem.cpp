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

#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "World.h"

inline bool _ModifyUInt32(bool apply, uint32& baseValue, int32& amount)
{
    // If amount is negative, change sign and value of apply.
    if (amount < 0)
    {
        apply = !apply;
        amount = -amount;
    }
    if (apply)
        baseValue += amount;
    else
    {
        // Make sure we do not get uint32 overflow.
        if (amount > int32(baseValue))
            amount = baseValue;
        baseValue -= amount;
    }
    return apply;
}

/*#######################################
########                         ########
########   PLAYERS STAT SYSTEM   ########
########                         ########
#######################################*/

bool Player::UpdateStats(Stats stat)
{
    if (stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    SetStat(stat, int32(value));

    if (stat == STAT_STAMINA || stat == STAT_INTELLECT || stat == STAT_STRENGTH)
    {
        Pet* pet = GetPet();
        if (pet)
            pet->UpdateStats(stat);
    }

    switch (stat)
    {
        case STAT_AGILITY:
            UpdateAllCritPercentages();
            UpdateDodgePercentage();
            break;
        case STAT_STAMINA:
            UpdateMaxHealth();
            break;
        case STAT_INTELLECT:
            UpdateMaxPower(POWER_MANA);
            UpdateAllSpellCritChances();
            UpdateArmor();                                  //SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT, only armor currently
            break;
        case STAT_SPIRIT:
            break;
        case STAT_STRENGTH:
            if (getLevel() == 90 && (getClass() == CLASS_DEATH_KNIGHT || getClass() == CLASS_WARRIOR || getClass() == CLASS_PALADIN))
                UpdateParryPercentage();
            break;
        default:
            break;
    }

    if (stat == STAT_STRENGTH)
        UpdateAttackPowerAndDamage(false);
    else if (stat == STAT_AGILITY)
    {
        UpdateAttackPowerAndDamage(false);
        UpdateAttackPowerAndDamage(true);
    }

    UpdateSpellDamageAndHealingBonus();
    UpdateManaRegen();

    // Update ratings in exist SPELL_AURA_MOD_RATING_FROM_STAT and only depends from stat
    uint32 mask = 0;
    AuraEffectList const& modRatingFromStat = GetAuraEffectsByType(SPELL_AURA_MOD_RATING_FROM_STAT);
    for (AuraEffectList::const_iterator i = modRatingFromStat.begin(); i != modRatingFromStat.end(); ++i)
        if (Stats((*i)->GetMiscValueB()) == stat)
            mask |= (*i)->GetMiscValue();
    if (mask)
    {
        for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
            if (mask & (1 << rating))
                ApplyRatingMod(CombatRating(rating), 0, true);
    }
    return true;
}

void Player::ApplySpellPowerBonus(int32 amount, bool apply)
{
    apply = _ModifyUInt32(apply, m_baseSpellPower, amount);

    // For speed just update for client
    ApplyModUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, amount, apply);
    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, amount, apply);
}

void Player::UpdateSpellDamageAndHealingBonus()
{
    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Get healing bonus for all schools
    SetStatInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL));
    // Get damage bonus for all schools
    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        SetStatInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i, SpellBaseDamageBonusDone(SpellSchoolMask(1 << i)));

    if (getClass() == CLASS_WARLOCK)
    {
        if (Pet* pet = GetPet())
            pet->UpdateAttackPowerAndDamage();
    }
        
}

bool Player::UpdateAllStats()
{
    for (int8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        float value = GetTotalStatValue(Stats(i));
        SetStat(Stats(i), int32(value));
    }

    UpdateArmor();
    // calls UpdateAttackPowerAndDamage() in UpdateArmor for SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR
    UpdateAttackPowerAndDamage(true);
    UpdateMaxHealth();

    for (uint8 i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    // Custom MoP script
    // Jab Override Driver
    if (GetTypeId() == TYPEID_PLAYER && getClass() == CLASS_MONK)
    {
        Item* mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

        if (mainItem && mainItem->GetTemplate()->Class == ITEM_CLASS_WEAPON && !HasAura(125660))
        {
            RemoveAura(108561); // 2H Staff Override
            RemoveAura(115697); // 2H Polearm Override
            RemoveAura(115689); // D/W Axes
            RemoveAura(115694); // D/W Maces
            RemoveAura(115696); // D/W Swords

            switch (mainItem->GetTemplate()->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_STAFF:
                    CastSpell(this, 108561, true);
                    break;
                case ITEM_SUBCLASS_WEAPON_POLEARM:
                    CastSpell(this, 115697, true);
                    break;
                case ITEM_SUBCLASS_WEAPON_AXE:
                    CastSpell(this, 115689, true);
                    break;
                case ITEM_SUBCLASS_WEAPON_MACE:
                    CastSpell(this, 115694, true);
                    break;
                case ITEM_SUBCLASS_WEAPON_SWORD:
                    CastSpell(this, 115696, true);
                    break;
                default:
                    break;
            }
        }
        else if (HasAura(125660))
        {
            RemoveAura(108561); // 2H Staff Override
            RemoveAura(115697); // 2H Polearm Override
            RemoveAura(115689); // D/W Axes
            RemoveAura(115694); // D/W Maces
            RemoveAura(115696); // D/W Swords
        }
    }
    // Way of the Monk - 120277
    if (getClass() == CLASS_MONK && HasAura(120277))
    {
        RemoveAurasDueToSpell(120275);
        RemoveAurasDueToSpell(108977);

        uint32 trigger = 0;
        if (IsTwoHandUsed())
        {
            trigger = 120275;
        }
        else
        {
            Item* mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            Item* offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (mainItem && mainItem->GetTemplate()->Class == ITEM_CLASS_WEAPON && offItem && offItem->GetTemplate()->Class == ITEM_CLASS_WEAPON)
                trigger = 108977;
        }

        if (trigger)
            CastSpell(this, trigger, true);
    }
    // Assassin's Resolve - 84601
    if (getClass() == CLASS_ROGUE && GetSpecializationId() == SPEC_ROGUE_ASSASSINATION)
    {
        Item* mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        if (((mainItem && mainItem->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER) || (offItem && offItem->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER)))
        {
            if (HasAura(84601))
                RemoveAura(84601);

            CastSpell(this, 84601, true);
        }
        else
            RemoveAura(84601);
    }

    UpdateAllRatings();
    UpdateAllCritPercentages();
    UpdateAllSpellCritChances();
    UpdateBlockPercentage();
    UpdateParryPercentage();
    UpdateDodgePercentage();
    UpdateSpellDamageAndHealingBonus();
    UpdateManaRegen();
    UpdateExpertise(BASE_ATTACK);
    UpdateExpertise(OFF_ATTACK);
    UpdateExpertise(RANGED_ATTACK);
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Player::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));

        Pet* pet = GetPet();
        if (pet)
            pet->UpdateResistances(school);
    }
    else
        UpdateArmor();
}

void Player::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    value  = GetModifierValue(unitMod, BASE_VALUE);         // base armor (from items)
    value *= GetModifierValue(unitMod, BASE_PCT);           // armor percent from items
    value += GetModifierValue(unitMod, TOTAL_VALUE);

    // Custom MoP Script
    // 77494 - Mastery : Nature's Guardian
    if (GetTypeId() == TYPEID_PLAYER && HasAura(77494))
    {
        float Mastery = 1.0f + GetFloatValue(PLAYER_MASTERY) * 1.25f / 100.0f;
        value *= Mastery;
    }

    //add dynamic flat mods
    AuraEffectList const& mResbyIntellect = GetAuraEffectsByType(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT);
    for (AuraEffectList::const_iterator i = mResbyIntellect.begin(); i != mResbyIntellect.end(); ++i)
    {
        if ((*i)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
            value += CalculatePct(GetStat(Stats((*i)->GetMiscValueB())), (*i)->GetAmount());
    }

    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));

    Pet* pet = GetPet();
    if (pet)
        pet->UpdateArmor();

    UpdateAttackPowerAndDamage();                           // armor dependent auras update for SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR
}

float Player::GetHealthBonusFromStamina()
{
    // Taken from PaperDollFrame.lua - 4.3.4.15595
    float ratio = 14.0f;
    if (gtOCTHpPerStaminaEntry const* hpBase = sGtOCTHpPerStaminaStore.LookupEntry((getClass() - 1) * GT_MAX_LEVEL + getLevel() - 1))
        ratio = hpBase->ratio;

    float stamina = GetStat(STAT_STAMINA);
    float baseStam = std::min(20.0f, stamina);
    float moreStam = stamina - baseStam;

    return baseStam + moreStam * ratio;
}

void Player::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;

    float value = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) + GetHealthBonusFromStamina();
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
    
    if (auto pet = GetPet())
        pet->UpdateMaxHealth();
}

void Player::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float value = GetModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE);
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    value = floor(value + 0.5f);
    SetMaxPower(power, uint32(value));
}

void Player::UpdateAttackPowerAndDamage(bool ranged)
{
    float val2 = 0.0f;
    float level = float(getLevel());

    ChrClassesEntry const* entry = sChrClassesStore.LookupEntry(getClass());
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mod_pos = UNIT_FIELD_ATTACK_POWER_MOD_POS;
    uint16 index_mod_neg = UNIT_FIELD_ATTACK_POWER_MOD_NEG;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mod_pos = UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS;
        index_mod_neg = UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
        val2 = (level + std::max(GetStat(STAT_AGILITY) - 10.0f, 0.0f)) * entry->RAPPerAgility;
    }
    else
    {
        float strengthValue = std::max((GetStat(STAT_STRENGTH) - 10.0f) * entry->APPerStrenth, 0.0f);
        float agilityValue = std::max((GetStat(STAT_AGILITY) - 10.0f) * entry->APPerAgility, 0.0f);

        if (GetShapeshiftForm() == FORM_CAT || GetShapeshiftForm() == FORM_BEAR)
            agilityValue += std::max((GetStat(STAT_AGILITY) - 10.0f) * 2, 0.0f);

        val2 = strengthValue + agilityValue;
    }

    SetModifierValue(unitMod, BASE_VALUE, val2);

    float base_attPower = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    // Fix base attack power for all classes, blizzlike - http://ru.wikipedia.org/wiki/World_of_Warcraft
    int32 bonusAp = 0;
    switch (getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
            bonusAp = getLevel() * 3;
            break;
        case CLASS_DRUID:
        case CLASS_HUNTER:
        case CLASS_ROGUE:
        case CLASS_SHAMAN:
            bonusAp = getLevel() * 2;
            break;
    }
    base_attPower += bonusAp;

    //add dynamic flat mods
    if (!ranged && HasAuraType(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR))
    {
        AuraEffectList const& mAPbyArmor = GetAuraEffectsByType(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR);
        for (AuraEffectList::const_iterator iter = mAPbyArmor.begin(); iter != mAPbyArmor.end(); ++iter)
        {
            // always: ((*i)->GetModifier()->m_miscvalue == 1 == SPELL_SCHOOL_MASK_NORMAL)
            int32 temp = int32(GetArmor() / (*iter)->GetAmount());
            if (temp > 0)
                attPowerMod += temp;
            else
                attPowerMod -= temp;
        }
    }

    if (HasAuraType(SPELL_AURA_OVERRIDE_AP_BY_SPELL_POWER_PCT))
    {
        int32 ApBySpellPct = 0;
        int32 spellPower = GetBaseSpellPowerBonus(); // SpellPower from Weapon
        spellPower += std::max(0, int32(GetStat(STAT_INTELLECT)) - 10); // SpellPower from intellect

        AuraEffectList const& mAPFromSpellPowerPct = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_AP_BY_SPELL_POWER_PCT);
        for (AuraEffectList::const_iterator i = mAPFromSpellPowerPct.begin(); i != mAPFromSpellPowerPct.end(); ++i)
            ApBySpellPct += CalculatePct(spellPower, (*i)->GetAmount());

        if (ApBySpellPct > 0)
        {
            SetInt32Value(index, uint32(ApBySpellPct));     //UNIT_FIELD_(RANGED)_ATTACK_POWER field
            SetFloatValue(index_mult, attPowerMultiplier);  //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
        }
        else
        {
            SetInt32Value(index, uint32(base_attPower + attPowerMod));  //UNIT_FIELD_(RANGED)_ATTACK_POWER field
            SetFloatValue(index_mult, attPowerMultiplier);              //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
        }
    }
    else
    {
        SetInt32Value(index, uint32(base_attPower + attPowerMod));  //UNIT_FIELD_(RANGED)_ATTACK_POWER field
        SetFloatValue(index_mult, attPowerMultiplier);              //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    }

    Pet* pet = GetPet();                                //update pet's AP
    //automatically update weapon damage after attack power modification
    if (ranged)
    {
        UpdateDamagePhysical(RANGED_ATTACK);
        if (pet && pet->isHunterPet()) // At ranged attack change for hunter pet
            pet->UpdateAttackPowerAndDamage();
    }
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        if (CanDualWield() && haveOffhandWeapon())           //allow update offhand damage only if player knows DualWield Spec and has equipped offhand weapon
            UpdateDamagePhysical(OFF_ATTACK);
        if (getClass() == CLASS_SHAMAN || getClass() == CLASS_PALADIN)                      // mental quickness
            UpdateSpellDamageAndHealingBonus();

        if (pet && pet->IsPetGhoul()) // At ranged attack change for hunter pet
            pet->UpdateAttackPowerAndDamage();
    }

    std::list<Creature*> l_Gargoyles;
    GetAllMinionsByEntry(l_Gargoyles, 27829);
    for (Creature* l_Gargoyle : l_Gargoyles)
        l_Gargoyle->UpdateAttackPowerAndDamage();
}

void Player::CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& min_damage, float& max_damage)
{
    UnitMods unitMod;

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    float att_speed = GetAPMultiplier(attType, normalized);

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) / 14.0f * att_speed;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = addTotalPct ? GetModifierValue(unitMod, TOTAL_PCT) : 1.0f;

    float weapon_mindamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(attType, MAXDAMAGE);

    if (IsInFeralForm())                                    //check if player is druid and in cat or bear forms
    {
        float weaponSpeed = BASE_ATTACK_TIME / 1000.f;
        if (Item* weapon = GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            if (weapon->GetTemplate()->Class == ITEM_CLASS_WEAPON)
                weaponSpeed = float(0.001f * weapon->GetTemplate()->Delay); 

        if (GetShapeshiftForm() == FORM_CAT)
        {
            weapon_mindamage = weapon_mindamage / weaponSpeed;
            weapon_maxdamage = weapon_maxdamage / weaponSpeed;
        }
        else if (GetShapeshiftForm() == FORM_BEAR)
        {
            weapon_mindamage = weapon_mindamage / weaponSpeed + weapon_mindamage / 2.5;
            weapon_maxdamage = weapon_mindamage / weaponSpeed + weapon_maxdamage / 2.5;
        }
    }
    else if (!CanUseAttackType(attType))      //check if player not in form but still can't use (disarm case)
    {
        //cannot use ranged/off attack, set values to 0
        if (attType != BASE_ATTACK)
        {
            min_damage = 0;
            max_damage = 0;
            return;
        }
        weapon_mindamage = BASE_MINDAMAGE;
        weapon_maxdamage = BASE_MAXDAMAGE;
    }
    /*
    TODO: Is this still needed after ammo has been removed?
    else if (attType == RANGED_ATTACK)                       //add ammo DPS to ranged damage
    {
        weapon_mindamage += ammo * att_speed;
        weapon_maxdamage += ammo * att_speed;
    }*/

    min_damage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct;
    max_damage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct;
}

void Player::UpdateDamagePhysical(WeaponAttackType attType)
{
    float mindamage;
    float maxdamage;

    CalculateMinMaxDamage(attType, false, true, mindamage, maxdamage);

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
            break;
        case OFF_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, maxdamage);
            break;
        case RANGED_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, maxdamage);
            break;
    }
}

void Player::UpdateBlockPercentage()
{
    // No block
    float value = 0.0f;
    if (CanBlock())
    {
        // Base value
        value = 5.0f;
        // Increase from SPELL_AURA_MOD_BLOCK_PERCENT aura
        value += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);

        // Custom MoP Script
        // 76671 - Mastery : Divine Bulwark - Block Percentage
        if (GetTypeId() == TYPEID_PLAYER && HasAura(76671))
            value += GetFloatValue(PLAYER_MASTERY);

        // Custom MoP Script
        // 76857 - Mastery : Critical Block - Block Percentage
        if (GetTypeId() == TYPEID_PLAYER && HasAura(76857))
            value += GetFloatValue(PLAYER_MASTERY) / 2.0f;

        // Increase from rating
        value += GetRatingBonusValue(CR_BLOCK);

         if (sWorld->getBoolConfig(CONFIG_STATS_LIMITS_ENABLE))
             value = value > sWorld->getFloatConfig(CONFIG_STATS_LIMITS_BLOCK) ? sWorld->getFloatConfig(CONFIG_STATS_LIMITS_BLOCK) : value;

        value = value < 0.0f ? 0.0f : value;
    }
    SetStatFloatValue(PLAYER_BLOCK_PERCENTAGE, value);
}

void Player::UpdateCritPercentage(WeaponAttackType attType)
{
    BaseModGroup modGroup;
    uint16 index;
    CombatRating cr;

    switch (attType)
    {
        case OFF_ATTACK:
            modGroup = OFFHAND_CRIT_PERCENTAGE;
            index = PLAYER_OFFHAND_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
        case RANGED_ATTACK:
            modGroup = RANGED_CRIT_PERCENTAGE;
            index = PLAYER_RANGED_CRIT_PERCENTAGE;
            cr = CR_CRIT_RANGED;
            break;
        case BASE_ATTACK:
        default:
            modGroup = CRIT_PERCENTAGE;
            index = PLAYER_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
    }

    float value = GetTotalPercentageModValue(modGroup) + GetRatingBonusValue(cr);
    // Modify crit from weapon skill and maximized defense skill of same level victim difference
    value += (int32(GetMaxSkillValueForLevel()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    if (sWorld->getBoolConfig(CONFIG_STATS_LIMITS_ENABLE))
        value = value > sWorld->getFloatConfig(CONFIG_STATS_LIMITS_CRIT) ? sWorld->getFloatConfig(CONFIG_STATS_LIMITS_CRIT) : value;

    value = value < 0.0f ? 0.0f : value;
    SetStatFloatValue(index, value);
}

void Player::UpdateAllCritPercentages()
{
    float value = GetMeleeCritFromAgility();

    SetBaseModValue(CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(OFFHAND_CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(RANGED_CRIT_PERCENTAGE, PCT_MOD, value);

    UpdateCritPercentage(BASE_ATTACK);
    UpdateCritPercentage(OFF_ATTACK);
    UpdateCritPercentage(RANGED_ATTACK);
}

const float m_diminishing_k[MAX_CLASSES] =
{
    0.9560f,  // Warrior
    0.8860f,  // Paladin
    0.9880f,  // Hunter
    0.9880f,  // Rogue
    0.9560f,  // Priest
    1.2220f,  // DK
    0.9880f,  // Shaman
    0.9830f,  // Mage
    0.9830f,  // Warlock
    1.4220f,  // Monk
    1.2220f   // Druid
};

void Player::UpdateParryPercentage()
{
    const float parry_cap[MAX_CLASSES] =
    {
        237.186f,       // Warrior
        237.186f,       // Paladin
        0.0f,           // Hunter
        145.560408f,    // Rogue
        0.0f,           // Priest
        237.186f,       // DK
        145.560408f,    // Shaman
        0.0f,           // Mage
        0.0f,           // Warlock
        90.6425f,       // Monk
        0.0f            // Druid
    };

    const float parry_coef[MAX_CLASSES] =
    {
        951.158596f / 1259.51807f,  // Warrior
        951.158596f / 1259.51807f,  // Paladin
        0.0f,                       // Hunter
        0.0f,                       // Rogue
        0.0f,                       // Priest
        951.158596f / 1259.51807f,  // DK
        0.0f,                       // Shaman
        0.0f,                       // Mage
        0.0f,                       // Warlock
        0.0f,                       // Monk
        0.0f                        // Druid
    };

    uint8 level = std::min<uint8>(getLevel(), GT_MAX_LEVEL);
    uint32 pclass = getClass();

    float value = 0.0f;
    if (CanParry() && parry_cap[pclass - 1] > 0.0f)
    {
        GtChanceToMeleeCritEntry const* entry = sGtChanceToMeleeCritStore.LookupEntry(/*hardcore this (pclass - 1) * GT_MAX_LEVEL*/300 + level - 1);

        float base_str = GetCreateStat(STAT_STRENGTH) * m_auraModifiersGroup[UNIT_MOD_STAT_START + STAT_STRENGTH][BASE_PCT];
        float bonus_str = GetTotalStatValue(STAT_STRENGTH) - base_str;

        const float baseParry = 3.0f;

        value = baseParry + GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
        if (parry_coef[pclass-1] > 0.0f)
        {
            float Q = entry->ratio * parry_coef[pclass-1];

            float subValue = (1 / parry_cap[pclass-1]) + (m_diminishing_k[pclass-1] / ((bonus_str / Q) + GetRatingBonusValue(CR_PARRY)));
            value += (base_str / Q) + (1.0f / subValue);
        }
        else
        {
            float subValue = (1 / parry_cap[pclass-1]) + (m_diminishing_k[pclass-1] / GetRatingBonusValue(CR_PARRY));
            value += (1.0f / subValue);
        }
    }

    if (sWorld->getBoolConfig(CONFIG_STATS_LIMITS_ENABLE))
        value = value > sWorld->getFloatConfig(CONFIG_STATS_LIMITS_PARRY) ? sWorld->getFloatConfig(CONFIG_STATS_LIMITS_PARRY) : value;

    value = value < 0.0f ? 0.0f : value;

    SetStatFloatValue(PLAYER_PARRY_PERCENTAGE, value);
}

void Player::UpdateDodgePercentage()
{
    const float dodge_cap[MAX_CLASSES] =
    {
        90.6425f,       // Warrior
        66.5674f,       // Paladin
        145.560408f,    // Hunter
        145.560408f,    // Rogue
        66.5674f,       // Priest
        90.6425f,       // DK
        145.560408f,    // Shaman
        150.375940f,    // Mage
        150.375940f,    // Warlock
        501.25f,        // Monk
        150.375940f     // Druid
    };
    
    const float dodge_coef[MAX_CLASSES] =
    {
        0.0f,                       // Warrior
        0.0f,                       // Paladin
        951.158596f / 1259.51807f,  // Hunter
        951.158596f / 1259.51807f,  // Rogue
        0.0f,                       // Priest
        0.0f,                       // DK
        951.158596f / 1259.51807f,  // Shaman
        0.0f,                       // Mage
        0.0f,                       // Warlock
        951.158596f / 1259.51807f,  // Monk
        951.158596f / 1259.51807f   // Druid
    };

    uint8 level = std::min<uint8>(getLevel(), GT_MAX_LEVEL);
    uint32 pclass = getClass();

    GtChanceToMeleeCritEntry const* entry = sGtChanceToMeleeCritStore.LookupEntry((pclass - 1) * GT_MAX_LEVEL + level - 1);
    
    float base_agility = GetCreateStat(STAT_AGILITY) * m_auraModifiersGroup[UNIT_MOD_STAT_START + STAT_AGILITY][BASE_PCT];
    float bonus_agility = GetTotalStatValue(STAT_AGILITY) - base_agility;

    const float baseDodge = 3.0f;

    float value = baseDodge + GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);
    if (dodge_coef[pclass-1] > 0.0f)
    {
        float A = entry->ratio * dodge_coef[pclass-1];

        float subValue = (1 / dodge_cap[pclass-1]) + (m_diminishing_k[pclass-1] / ((bonus_agility / A) + GetRatingBonusValue(CR_DODGE)));
        value += (base_agility / A) + (1.0f / subValue);
    }
    else
    {
        float subValue = (1 / dodge_cap[pclass-1]) + (m_diminishing_k[pclass-1] / GetRatingBonusValue(CR_DODGE));
        value += (1.0f / subValue);
    }

    if (sWorld->getBoolConfig(CONFIG_STATS_LIMITS_ENABLE))
        value = value > sWorld->getFloatConfig(CONFIG_STATS_LIMITS_DODGE) ? sWorld->getFloatConfig(CONFIG_STATS_LIMITS_DODGE) : value;

    value = value < 0.0f ? 0.0f : value;

    SetStatFloatValue(PLAYER_DODGE_PERCENTAGE, value);
}

void Player::UpdateSpellCritChance(uint32 school)
{
    // For normal school set zero crit chance
    if (school == SPELL_SCHOOL_NORMAL)
    {
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1, 0.0f);
        return;
    }
    // For others recalculate it from:
    float crit = 0.0f;
    // Crit from Intellect
    crit += GetSpellCritFromIntellect();
    // Increase crit from SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
    // Increase crit from SPELL_AURA_MOD_CRIT_PCT
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
    // Increase crit by school from SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    crit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, 1<<school);
    // Increase crit from spell crit ratings
    crit += GetRatingBonusValue(CR_CRIT_SPELL);

    // Store crit value
    SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + school, crit);

    if (Pet* pet = GetPet())
        pet->m_baseSpellCritChance = crit;
}

void Player::UpdateMasteryPercentage()
{
    // No mastery
    float value = 0.0f;
    if (CanMastery() && getLevel() >= 80)
    {
        // Mastery from SPELL_AURA_MASTERY aura
        value += GetTotalAuraModifier(SPELL_AURA_MASTERY);
        // Mastery from rating
        value += GetRatingBonusValue(CR_MASTERY);
        value = value < 0.0f ? 0.0f : value;
    }
    // Custom MoP Script
    // 113861 - Dark Soul: Knowledge
    if (getClass() == CLASS_WARLOCK && HasAura(113861))
        value += 30.0f;
    SetFloatValue(PLAYER_MASTERY, value);
    // Custom MoP Script
    // 76671 - Mastery : Divine Bulwark - Update Block Percentage
    // 76857 - Mastery : Critical Block - Update Block Percentage
    if ((getClass() == CLASS_PALADIN && HasAura(76671)) || (getClass() == CLASS_WARRIOR && HasAura(76857)))
        UpdateBlockPercentage();
    // 77494 - Mastery : Nature's Guardian - Update Armor
    if (getClass() == CLASS_DRUID && HasAura(77494))
        UpdateArmor();
}

void Player::UpdatePvPPowerPercentage()
{
    float value = GetRatingBonusValue(CR_PVP_POWER);
    value = value < 0.0f ? 0.0f : value;

    float damage_value = value;
    float heal_value = value;

    float healMultiplier = 1.0f;
    switch (GetSpecializationId(GetActiveSpec()))
    {
        // All other specializations and classes (including tanking) receive a 40% bonus to healing from PvP Power.
        case SPEC_WARRIOR_ARMS:
        case SPEC_WARRIOR_FURY:
        case SPEC_WARRIOR_PROTECTION:
        case SPEC_HUNTER_BEASTMASTER:
        case SPEC_HUNTER_MARKSMAN:
        case SPEC_HUNTER_SURVIVAL:
        case SPEC_ROGUE_ASSASSINATION:
        case SPEC_ROGUE_COMBAT:
        case SPEC_ROGUE_SUBTLETY:
        case SPEC_DK_BLOOD:
        case SPEC_DK_FROST:
        case SPEC_DK_UNHOLY:
        case SPEC_MAGE_ARCANE:
        case SPEC_MAGE_FIRE:
        case SPEC_MAGE_FROST:
        case SPEC_WARLOCK_AFFLICTION:
        case SPEC_WARLOCK_DEMONOLOGY:
        case SPEC_WARLOCK_DESTRUCTION:
            healMultiplier = 0.4f;
            break;
        // Healing specializations receive a 100% bonus to healing from PvP Power.
        case SPEC_PALADIN_HOLY:
        case SPEC_PRIEST_DISCIPLINE:
        case SPEC_PRIEST_HOLY:
        case SPEC_SHAMAN_RESTORATION:
        case SPEC_DRUID_RESTORATION:
        case SPEC_MONK_MISTWEAVER:
            damage_value = 0.0f;
            break;
        // Damage specializations for Druids, Monks, Paladins, Priests, and Shaman receive a 70% bonus to healing from PvP Power.
        default:
            healMultiplier = 0.7f;
            break;
    }

    healMultiplier += float(GetTotalAuraModifier(SPELL_AURA_MOD_PVP_POWER_HEALING)) / 100.0f;

    heal_value *= healMultiplier;

    SetFloatValue(PLAYER_FIELD_PVP_POWER_DAMAGE, damage_value);
    SetFloatValue(PLAYER_FIELD_PVP_POWER_HEALING, heal_value);
}

void Player::UpdateMeleeHitChances()
{
    m_modMeleeHitChance = 0;
    AuraEffectList const & alist = GetAuraEffectsByType(SPELL_AURA_MOD_HIT_CHANCE);
    Item *weapon = this->GetWeaponForAttack(BASE_ATTACK);
    for (AuraEffectList::const_iterator itr = alist.begin(); itr != alist.end(); ++itr)
    {
        if ((*itr)->GetSpellInfo()->EquippedItemSubClassMask && !weapon)
            continue;
        if (weapon && !weapon->IsFitToSpellRequirements((*itr)->GetSpellInfo()))
            continue;
        m_modMeleeHitChance += (*itr)->GetAmount();
    }
    SetFloatValue(PLAYER_FIELD_UI_HIT_MODIFIER, m_modMeleeHitChance); 
    m_modMeleeHitChance += GetRatingBonusValue(CR_HIT_MELEE);

    if (Pet* pet = GetPet())
        pet->m_modMeleeHitChance = m_modMeleeHitChance;

    std::list<Creature*> l_Gargoyles;
    GetAllMinionsByEntry(l_Gargoyles, 27829);
    for (Creature* l_Gargoyle : l_Gargoyles)
        l_Gargoyle->m_modMeleeHitChance = m_modMeleeHitChance;
}

void Player::UpdateRangedHitChances()
{
    m_modRangedHitChance = (float)GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
    m_modRangedHitChance += GetRatingBonusValue(CR_HIT_RANGED);

    if (Pet* pet = GetPet())
        pet->m_modRangedHitChance = m_modRangedHitChance;

    std::list<Creature*> l_Gargoyles;
    GetAllMinionsByEntry(l_Gargoyles, 27829);
    for (Creature* l_Gargoyle : l_Gargoyles)
         l_Gargoyle->m_modRangedHitChance = m_modRangedHitChance;
}

void Player::UpdateSpellHitChances()
{
    /* client calculate its like: melee expertise + ranged expertise + hit rating
       but it should calculate it without ranged expertise. https://www.youtube.com/watch?t=11&v=C7202boxwME 
       
       so apply negative modifier to fix it clientside*/
    SetFloatValue(PLAYER_FIELD_UI_SPELL_HIT_MODIFIER, -GetFloatValue(PLAYER_RANGED_EXPERTISE));

    m_modSpellHitChance = (float)GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
    m_modSpellHitChance += GetRatingBonusValue(CR_HIT_SPELL) + GetFloatValue(PLAYER_EXPERTISE);

    if (Pet* pet = GetPet())
        pet->m_modSpellHitChance = m_modSpellHitChance;

    std::list<Creature*> l_Gargoyles;
    GetAllMinionsByEntry(l_Gargoyles, 27829);
    for (Creature* l_Gargoyle : l_Gargoyles)
        l_Gargoyle->m_modSpellHitChance = m_modSpellHitChance;
}

void Player::UpdateAllSpellCritChances()
{
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateSpellCritChance(i);
}

void Player::UpdateExpertise(WeaponAttackType attack)
{
    float expertise = GetRatingBonusValue(CR_EXPERTISE);

    Item* weapon = GetWeaponForAttack(attack, true);

    AuraEffectList const& expAuras = GetAuraEffectsByType(SPELL_AURA_MOD_EXPERTISE);
    for (AuraEffectList::const_iterator itr = expAuras.begin(); itr != expAuras.end(); ++itr)
    {
        // item neutral spell
        if ((*itr)->GetSpellInfo()->EquippedItemClass == -1)
            expertise += (*itr)->GetAmount();
        // item dependent spell
        else if (weapon && weapon->IsFitToSpellRequirements((*itr)->GetSpellInfo()))
            expertise += (*itr)->GetAmount();
    }

    if (expertise < 0)
        expertise = 0.0f;

    switch (attack)
    {
        case BASE_ATTACK: SetFloatValue(PLAYER_EXPERTISE, expertise);          break;
        case OFF_ATTACK:  SetFloatValue(PLAYER_OFFHAND_EXPERTISE, expertise);  break;
        case RANGED_ATTACK: SetFloatValue(PLAYER_RANGED_EXPERTISE, expertise); break;
        default: break;
    }

    UpdateSpellHitChances();
}

void Player::ApplyManaRegenBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseManaRegen, amount);
    UpdateManaRegen();
}

void Player::ApplyHealthRegenBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseHealthRegen, amount);
}

void Player::UpdateManaRegen()
{
    if (getPowerType() != POWER_MANA && !IsInFeralForm())
        return;

    // See http://us.battle.net/wow/en/forum/topic/6794873160 .

    uint8 percentOfMana = 2;
    float SpiritRegenIncrease = OCTRegenMPPerSpirit();   // Mana regen increase from spirit - per-point calculation * player stat.

    float CombatRegenFromSpirit = 0;
    float CombatRegenFromAurPct = 0;
    float BaseRegenFromAurPct = 0;
    float RegenFromModPowerRegen = /*GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_MANA) / 5.0f*/ 0.0f;
    {
        std::map<SpellGroup, int32> SameEffectSpellGroup;

        AuraEffectList const& list = GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN);
        for (auto const& eff : list)
        {
            if (eff->GetMiscValue() == POWER_MANA)
            {
                if (!sSpellMgr->AddSameEffectStackRuleSpellGroups(eff->GetSpellInfo(), eff->GetAmount(), SameEffectSpellGroup))
                {
                    auto _base = eff->GetBase();
                    if (_base && _base->GetMaxDuration() == 3600000) // is one-hour aura
                        RegenFromModPowerRegen += eff->GetAmount();
                    else
                        RegenFromModPowerRegen += eff->GetAmount() / 5.0f;
                }
            }
        }

        for (auto const& kvp : SameEffectSpellGroup)
        {
            RegenFromModPowerRegen += kvp.second / 5.0f;
        }
    }

    // Warlocks and Mages have 5% of maximum mana in base mana regen - blizzlike
    if (getClass() == CLASS_WARLOCK || getClass() == CLASS_MAGE)
        percentOfMana = 5;

    // 2% of base mana each 5 seconds.
    float combat_regen = float(CalculatePct(GetMaxPower(POWER_MANA), percentOfMana));
    float base_regen   = float(CalculatePct(GetMaxPower(POWER_MANA), percentOfMana));

    if (HasAuraType(SPELL_AURA_MOD_MANA_REGEN_BY_HASTE))
    {
        float HastePct = 1.0f + GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_SPELL) * GetRatingMultiplier(CR_HASTE_SPELL) / 100.0f;

        combat_regen *= HastePct;
        base_regen *= HastePct;
    }

    // Try to get aura with spirit addition to combat mana regen.
    // Meditation: Allows 50% of your mana regeneration from Spirit to continue while in combat.
    int32 PercentAllowCombatRegenBySpirit = 0;
    Unit::AuraEffectList const& ModPowerRegenPCTAuras = GetAuraEffectsByType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    for (AuraEffectList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
        PercentAllowCombatRegenBySpirit += (*i)->GetAmount();

    if (HasAuraType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT) && PercentAllowCombatRegenBySpirit != 0)
        CombatRegenFromSpirit += (float(PercentAllowCombatRegenBySpirit) / 100) * SpiritRegenIncrease; // Allows you mana regeneration from Spirit to continue while in combat.

    // Increase mana regen.
    int32 PercentIncreaseManaRegen = 0;
    int32 IncreaseManaRegen = combat_regen;

    // Increase mana from SPELL_AURA_MOD_POWER_REGEN_PERCENT
    Unit::AuraEffectList const& ModRegenPct = GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
    for (AuraEffectList::const_iterator i = ModRegenPct.begin(); i != ModRegenPct.end(); ++i)
        if (Powers((*i)->GetMiscValue()) == POWER_MANA)
            IncreaseManaRegen += IncreaseManaRegen * ((*i)->GetAmount() / 100.0f);

    // Increase mana from SPELL_AURA_INCREASE_MANA_REGEN , for example: Ring of Power
    Unit::AuraEffectList const& ModRegenPctUnk = GetAuraEffectsByType(SPELL_AURA_INCREASE_MANA_REGEN);
    for (AuraEffectList::const_iterator i = ModRegenPctUnk.begin(); i != ModRegenPctUnk.end(); ++i)
        IncreaseManaRegen += IncreaseManaRegen * ((*i)->GetAmount() / 100.0f);

    // If IncreaseManaRegen is bigger then combat_regen we have increased mana regen by auras, so we should add it
    if (HasAuraType(SPELL_AURA_MOD_POWER_REGEN_PERCENT) || HasAuraType(SPELL_AURA_MOD_MANA_REGEN_FROM_STAT) || HasAuraType(SPELL_AURA_INCREASE_MANA_REGEN) && IncreaseManaRegen > combat_regen)
    {
        IncreaseManaRegen -= combat_regen;
        BaseRegenFromAurPct   = IncreaseManaRegen;
        CombatRegenFromAurPct = IncreaseManaRegen;
    }

    // Calculate for 1 second, the client multiplies the field values by 5.
    base_regen   = ((base_regen   + BaseRegenFromAurPct   + RegenFromModPowerRegen) / 5.0f) + SpiritRegenIncrease;
    combat_regen = ((combat_regen + CombatRegenFromAurPct + RegenFromModPowerRegen) / 5.0f) + CombatRegenFromSpirit;

    // Not In Combat
    SetStatFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER, base_regen);
    // In Combat
    SetStatFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, combat_regen);
}

void Player::UpdateEnergyRegen()
{
    if (getPowerType() != POWER_ENERGY)
        return;

    float pct = GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_ENERGY);

    float haste = GetFloatValue(UNIT_MOD_HASTE);
    ApplyPercentModFloatVar(haste, pct, false);

    SetFloatValue(UNIT_MOD_HASTE_REGEN, haste);
}

void Player::UpdateRuneRegen(RuneType rune)
{
    if (rune > NUM_RUNE_TYPES)
        return;

    uint32 cooldown = 0;

    for (uint32 i = 0; i < MAX_RUNES; ++i)
        if (GetBaseRune(i) == rune)
        {
            cooldown = GetRuneBaseCooldown(i);
            break;
        }

    if (cooldown <= 0)
        return;

    float regen = float(1 * IN_MILLISECONDS) / float(cooldown);
    SetFloatValue(PLAYER_RUNE_REGEN_1 + uint8(rune), regen);
}

void Player::UpdateAllRunesRegen()
{
    if (getClass() != CLASS_DEATH_KNIGHT)
        return;

    for (uint8 i = 0; i < NUM_RUNE_TYPES; ++i)
    {
        if (uint32 cooldown = GetRuneTypeBaseCooldown(RuneType(i)))
        {
            float regen = float(1 * IN_MILLISECONDS) / float(cooldown);

            if (regen < 0.0099999998f)
                regen = 0.01f;

            SetFloatValue(PLAYER_RUNE_REGEN_1 + i, regen);
        }
    }

    float pct = GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_RUNES);

    float haste = GetFloatValue(UNIT_MOD_HASTE);
    ApplyPercentModFloatVar(haste, pct, false);

    SetFloatValue(UNIT_MOD_HASTE_REGEN, haste);
}

void Player::_ApplyAllStatBonuses()
{
    SetCanModifyStats(false);

    _ApplyAllAuraStatMods();
    _ApplyAllItemMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

void Player::_RemoveAllStatBonuses()
{
    SetCanModifyStats(false);

    _RemoveAllItemMods();
    _RemoveAllAuraStatMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

/*#######################################
########                         ########
########    MOBS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Creature::UpdateStats(Stats /*stat*/)
{
    return true;
}

bool Creature::UpdateAllStats()
{
    UpdateMaxHealth();
    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);

    for (uint8 i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    for (int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Creature::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Creature::UpdateArmor()
{
    float value = GetTotalAuraModValue(UNIT_MOD_ARMOR);
    SetArmor(int32(value));
}

void Creature::UpdateMaxHealth()
{
    float value = GetTotalAuraModValue(UNIT_MOD_HEALTH);
    SetMaxHealth((uint32)value);
}

void Creature::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float value  = GetTotalAuraModValue(unitMod);
    SetMaxPower(power, uint32(value));
}

void Creature::UpdateAttackPowerAndDamage(bool ranged)
{
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
    }

    float base_attPower  = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetInt32Value(index, (uint32)base_attPower);            //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetFloatValue(index_mult, attPowerMultiplier);          //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    //automatically update weapon damage after attack power modification
    if (ranged)
        UpdateDamagePhysical(RANGED_ATTACK);
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);
    }
}

void Creature::UpdateDamagePhysical(WeaponAttackType attType)
{
    UnitMods unitMod;
    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    //float att_speed = float(GetAttackTime(attType))/1000.0f;

    float weapon_mindamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(attType, MAXDAMAGE);

    /* difference in AP between current attack power and base value from DB */
    // creature's damage in battleground is calculated in Creature::SelectLevel
    // so don't change it with ap and dmg multipliers
    float att_pwr_change = GetTotalAttackPowerValue(attType) - GetCreatureTemplate()->attackpower;
    float base_value  = ((GetMap()->IsBattleground()) ? GetModifierValue(unitMod, BASE_VALUE) : (GetModifierValue(unitMod, BASE_VALUE) + (att_pwr_change * GetAPMultiplier(attType, false) / 14.0f)));
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);
    float dmg_multiplier = ((GetMap()->IsBattleground()) ? 1.0f : GetCreatureTemplate()->dmg_multiplier);

    if (!CanUseAttackType(attType))
    {
        weapon_mindamage = 0;
        weapon_maxdamage = 0;
    }

    float mindamage = ((base_value + weapon_mindamage) * dmg_multiplier * base_pct + total_value) * total_pct;
    float maxdamage = ((base_value + weapon_maxdamage) * dmg_multiplier * base_pct + total_value) * total_pct;

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
            break;
        case OFF_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, maxdamage);
            break;
        case RANGED_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, maxdamage);
            break;
    }
}

/*#######################################
########                         ########
########    PETS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Guardian::UpdateStats(Stats stat)
{
    if (stat >= MAX_STATS)
        return false;

    float value  = GetTotalStatValue(stat);
    ApplyStatBuffMod(stat, m_statFromOwner[stat], false);
    float ownersBonus = 0.0f;

    Unit* owner = GetOwner();
    // Handle Death Knight Glyphs and Talents
    float mod = 0.75f;

    switch (stat)
    {
        case STAT_STAMINA:
        {
            mod = 0.3f;

            if (IsPetGhoul() || IsPetGargoyle())
                mod = 0.45f;
            else if (owner->getClass() == CLASS_WARLOCK && isPet())
                mod = 0.75f;
            else if (owner->getClass() == CLASS_MAGE && isPet())
                mod = 0.75f;
            else
            {
                mod = 0.45f;

                if (isPet())
                {
                    switch (ToPet()->GetSpecializationId())
                    {
                        case SPEC_PET_FEROCITY: mod = 0.67f; break;
                        case SPEC_PET_TENACITY: mod = 0.78f; break;
                        case SPEC_PET_CUNNING: mod = 0.725f; break;
                    }
                }
            }

            ownersBonus = float(owner->GetStat(stat)) * mod;
            ownersBonus *= GetModifierValue(UNIT_MOD_STAT_STAMINA, TOTAL_PCT);
            value += ownersBonus;
            break;
        }
        case STAT_STRENGTH:
        {
            mod = 0.7f;

            ownersBonus = owner->GetStat(stat) * mod;
            value += ownersBonus;
            break;
        }
        case STAT_INTELLECT:
        {
            if (owner->getClass() == CLASS_WARLOCK || owner->getClass() == CLASS_MAGE)
            {
                mod = 0.3f;
                ownersBonus = owner->GetStat(stat) * mod;
            }
            else if (owner->getClass() == CLASS_DEATH_KNIGHT && GetEntry() == 31216)
            {
                mod = 0.3f;
                if (owner->GetSimulacrumTarget())
                    ownersBonus = owner->GetSimulacrumTarget()->GetStat(stat) * mod;
                else
                    ownersBonus = owner->GetStat(stat) * mod;
            }

            value += ownersBonus;
            break;
        }
        default:
            break;
    }

    SetStat(stat, int32(value));
    m_statFromOwner[stat] = ownersBonus;
    ApplyStatBuffMod(stat, m_statFromOwner[stat], true);

    switch (stat)
    {
        case STAT_STRENGTH:
            UpdateAttackPowerAndDamage();
            break;
        case STAT_AGILITY:
            UpdateArmor();
            break;
        case STAT_STAMINA:
            UpdateMaxHealth();
            break;
        case STAT_INTELLECT:
            UpdateMaxPower(POWER_MANA);
            if (owner->getClass() == CLASS_MAGE)
                UpdateAttackPowerAndDamage();
            break;
        case STAT_SPIRIT:
        default:
            break;
    }

    return true;
}

bool Guardian::UpdateAllStats()
{
    for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        UpdateStats(Stats(i));

    for (uint8 i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Guardian::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));

        // hunter and warlock pets gain 40% of owner's resistance
        if (isPet())
            value += float(CalculatePct(m_owner->GetResistance(SpellSchools(school)), 40));

        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Guardian::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    // All pets gain 100% of owner's armor value
    value = m_owner->GetArmor();
    switch (GetEntry())
    {
    case ENTRY_IMP:
    case ENTRY_FEL_IMP:
    case ENTRY_FELHUNTER:
    case ENTRY_OBSERVER:
    case ENTRY_SUCCUBUS:
    case ENTRY_SHIVARRA:
        value *= 3;
        break;
    case ENTRY_VOIDWALKER:
    case ENTRY_VOIDLORD:
    case ENTRY_FELGUARD:
    case ENTRY_WRATHGUARD:
        value *= 4;
        break;
    case ENTRY_GHOUL:
        value *= 1.25f;
        break;
    default:
        if (isHunterPet())
            value *= 1.7f;
        break;
    }
    value *= GetModifierValue(unitMod, BASE_PCT);
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    AddPct(value, GetOwner()->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_PET_STAT_PCT_FROM_MAX, 13, GetEntry()));

    SetArmor(int32(value));
}

void Guardian::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);

    float multiplicator;
    switch (GetEntry())
    {
        case ENTRY_IMP:
            multiplicator = 8.4f;
            break;
        case ENTRY_VOIDWALKER:
        case ENTRY_FELGUARD:
            multiplicator = 11.0f;
            break;
        case ENTRY_SUCCUBUS:
            multiplicator = 9.1f;
            break;
        case ENTRY_FELHUNTER:
            multiplicator = 14.46f;
            break;
        case ENTRY_WATER_ELEMENTAL:
            multiplicator = 1.0f;
            stamina = 0.0f;
            break;
        case ENTRY_BLOODWORM:
            SetMaxHealth(GetCreateHealth());
            return;
        default:
            multiplicator = 10.0f;
            break;
    }

    float value = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) + stamina * multiplicator;

    Unit* owner = GetOwner();
    switch (GetEntry())
    {
        case ENTRY_SUCCUBUS:
        case ENTRY_FELHUNTER:
        case ENTRY_FEL_IMP:
            value = owner->CountPctFromMaxHealth(40);
            break;
        case ENTRY_VOIDWALKER:
        case ENTRY_FELGUARD:
        case ENTRY_OBSERVER:
        case ENTRY_SHIVARRA:
        case ENTRY_GHOUL:
        case ENTRY_GARGOYLE:
            value = owner->CountPctFromMaxHealth(50);
            break;
        case ENTRY_IMP:
            value = owner->CountPctFromMaxHealth(30);
            break;
        case ENTRY_VOIDLORD:
        case ENTRY_WRATHGUARD:
            value = owner->CountPctFromMaxHealth(60);
            break;
        case ENTRY_XUEN:
            value = owner->GetMaxHealth();
            break;
        // Psyfiend
        case 59190:
            value = CalculatePct(owner->GetMaxHealth(), 2.5f);
            break;
        default:
            if (isHunterPet())
                value = owner->CountPctFromMaxHealth(70);
            break;
    }

    value *= GetModifierValue(unitMod, TOTAL_PCT);
    AddPct(value, owner->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_PET_STAT_PCT_FROM_MAX, 1, GetEntry()));

    SetMaxHealth((uint32)value);
}

void Guardian::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float addValue = (power == POWER_MANA) ? GetStat(STAT_INTELLECT) - GetCreateStat(STAT_INTELLECT) : 0.0f;
    float multiplicator = 15.0f;

    switch (GetEntry())
    {
        case ENTRY_IMP:
        case ENTRY_FEL_IMP:
        case ENTRY_WILD_IMP:
            multiplicator = 4.95f;
            break;
        case ENTRY_VOIDWALKER:
        case ENTRY_SUCCUBUS:
        case ENTRY_FELHUNTER:
        case ENTRY_FELGUARD:
        case ENTRY_VOIDLORD:
        case ENTRY_SHIVARRA:
        case ENTRY_WRATHGUARD:
            multiplicator = 11.5f;
            break;
        case ENTRY_WATER_ELEMENTAL:
            multiplicator = 1.0f;
            addValue = 0.0f;
            break;
        default:
            multiplicator = 15.0f;
            break;
    }

    float value  = GetModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) + addValue * multiplicator;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Guardian::UpdateAttackPowerAndDamage(bool ranged)
{
    if (ranged)
        return;

    float val = 0.0f;
    float bonusAP = 0.0f;
    UnitMods unitMod = UNIT_MOD_ATTACK_POWER;

    // imp's attack power
    if (GetEntry() == ENTRY_IMP)
        val = GetStat(STAT_STRENGTH) - 10.0f;
    else if (!isHunterPet())
        val = 2 * GetStat(STAT_STRENGTH) - 20.0f;

    Unit* owner = GetOwner();
    if (owner && owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (isHunterPet())
        {
            bonusAP = owner->GetTotalAttackPowerValue(RANGED_ATTACK); // Hunter pets gain 100% of owner's AP
            SetBonusDamage(int32(owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.5f)); // Bonus damage is equal to 50% of owner's AP
        }
        else if (IsPetGhoul()) // ghouls benefit from deathknight's attack power (may be summon pet or not)
        {
            bonusAP = owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.22f;
            SetBonusDamage(int32(owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.1287f));
        }
        else if (isPet() && GetEntry() != ENTRY_WATER_ELEMENTAL) // demons benefit from warlocks shadow or fire damage
        {
            int32 spd = owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL);
            SetBonusDamage(spd);
            bonusAP = spd;
        }
        else if (GetEntry() == ENTRY_WATER_ELEMENTAL) // water elementals benefit from mage's frost damage
        {
            SetBonusDamage(int32(m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST)));
        }
        // Summon Gargoyle AP
        else if (GetEntry() == 27829)
        {
            bonusAP = owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.4f;
            SetBonusDamage(int32(m_owner->GetTotalAttackPowerValue(BASE_ATTACK)));
        }
        else if (GetEntry() == ENTRY_XUEN)
        {
            val = 0;
            bonusAP = 0;
        }
    }

    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, val + bonusAP);

    //in BASE_VALUE of UNIT_MOD_ATTACK_POWER for creatures we store data of meleeattackpower field in DB
    float base_attPower = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    // base_attPower for Warlock pets
    if (owner && owner->getClass() == CLASS_WARLOCK && owner->GetTypeId() == TYPEID_PLAYER)
    {
        switch (GetEntry())
        {
            case ENTRY_IMP:
            case ENTRY_VOIDWALKER:
            case ENTRY_FELHUNTER:
            case ENTRY_FELGUARD:
            case ENTRY_FEL_IMP:
            case ENTRY_VOIDLORD:
            case ENTRY_OBSERVER:
            case ENTRY_WILD_IMP:
                base_attPower = owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 3.5f;
                break;
            case ENTRY_SUCCUBUS:
                base_attPower = owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 1.67f;
                break;
            case ENTRY_SHIVARRA:
                base_attPower = owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 1.11f;
                break;
            case ENTRY_WRATHGUARD:
                base_attPower = owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 2.33f;
                break;
        }
    }
    else if (GetEntry() == ENTRY_XUEN)
    {
        base_attPower = owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.60f;
    }
    else if (GetEntry() == ENTRY_GHOUL)
    {
        base_attPower = owner->GetTotalAttackPowerValue(BASE_ATTACK);
    }

    //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetInt32Value(UNIT_FIELD_ATTACK_POWER, (int32)base_attPower);
    //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, attPowerMultiplier);

    //automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK);

    // update off hand weapon damage for Shivarra and Wrathguard
    if (GetEntry() == ENTRY_SHIVARRA || GetEntry() == ENTRY_WRATHGUARD)
        UpdateDamagePhysical(OFF_ATTACK);
}

void Guardian::UpdateDamagePhysical(WeaponAttackType attType)
{
    if (attType > BASE_ATTACK)
    {
        if (GetEntry() == ENTRY_SHIVARRA || GetEntry() == ENTRY_WRATHGUARD)
        {
            // Shivarra and Wrathguard have offhand weapon
            if (attType != OFF_ATTACK)
                return;
        }
        else
            return;
    }

    float bonusDamage = 0.0f;
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetEntry() == ENTRY_FIRE_ELEMENTAL) // greater fire elemental
        {
            int32 spellDmg = int32(m_owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE)) - m_owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
            if (spellDmg > 0)
                bonusDamage = spellDmg * 0.4f;
        }
    }

    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;

    float att_speed = float(GetAttackTime(BASE_ATTACK))/1000.0f;

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType)/ 14.0f * att_speed  + bonusDamage;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);

    float weapon_mindamage = GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE);

    // mindamage and maxdamage for Warlock pets
    if (m_owner && m_owner->getClass() == CLASS_WARLOCK && m_owner->GetTypeId() == TYPEID_PLAYER && m_owner->getLevel() >= 70)
    {
        switch (GetEntry())
        {
            case ENTRY_IMP:
            case ENTRY_VOIDWALKER:
            case ENTRY_FELHUNTER:
            case ENTRY_FELGUARD:
            case ENTRY_WILD_IMP:
            case ENTRY_FEL_IMP:
            case ENTRY_VOIDLORD:
            case ENTRY_OBSERVER:
                base_value = 1068 + (m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 3.5f)/ 14.0f * 2 + bonusDamage;
                break;
            case ENTRY_SUCCUBUS:
                base_value = 1068 + (m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 1.67f)/ 14.0f * 3  + bonusDamage;
                break;
            case ENTRY_SHIVARRA:
                base_value = 713 + (m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 3.5f) / 14.0f * 3 + bonusDamage;
                break;
            case ENTRY_WRATHGUARD:
                base_value = 713 + (m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * 2.33f) / 14.0f * 2 + bonusDamage;
                break;
        }
        if (m_owner->HasAura(77219))
        {
            float Mastery = 1 + m_owner->GetFloatValue(PLAYER_MASTERY) / 100.0f;
            base_value *= Mastery;
        }
    }

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct;

    // Just Shivarra and Wrathguard have offhand weapon, deal 50% of mainhand damage
    if (attType == OFF_ATTACK)
    {
        mindamage *= 0.5f;
        maxdamage *= 0.5f;
        SetStatFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, mindamage);
        SetStatFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, maxdamage);
    }
    // all other pets need just mainhaind damage
    else
    {
        SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
        SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage); 
    }
}

void Guardian::SetBonusDamage(int32 damage)
{
    m_bonusSpellDamage = damage;
    if (GetOwner()->GetTypeId() == TYPEID_PLAYER)
        GetOwner()->SetUInt32Value(PLAYER_PET_SPELL_POWER, damage);
}
