//////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef DEF_STORMSTOUT_BREWERY_HPP
# define DEF_STORMSTOUT_BREWERY_HPP

# include "SpellScript.h"
# include "Map.h"
# include "Creature.h"
# include "CreatureAIImpl.h"
# include "ObjectMgr.h"
# include "ScriptMgr.h"
# include "ScriptedCreature.h"
# include "SpellScript.h"
# include "SpellAuras.h"
# include "SpellAuraEffects.h"
# include "Player.h"
# include "Vehicle.h"
# include "Group.h"
# include "InstanceScript.h"
# include "Unit.h"
# include "ObjectAccessor.h"
# include "CreatureTextMgr.h"
# include "CreatureAI.h"
# include "PassiveAI.h"

# define MAX_ENCOUNTERS 3

# define HOZEN_KILLS_NEEDED 40 ///< Hozen kills needed to spawn Ook-Ook.

enum DataTypes ///< Events / Encounters.
{
    DATA_OOKOOK_EVENT               = 0,
    DATA_HOPTALLUS_EVENT            = 1,
    DATA_YANZHU_THE_UNCASKED_EVENT  = 2,

    /// For Ook-Ook Hozen.
    DATA_HOZEN_KILLED               = 3,

    /// For Yan Zhu Event
    DATA_YANZHU_EVENT_SPURT         = 4
};

enum Data     ///< GUID handling.
{
    DATA_OOKOOK                     = 0,
    DATA_HOPTALLUS                  = 1,
    DATA_YANZHU_THE_UNCASKED        = 2
};

enum InstanceSpells
{
    SPELL_BANANA_BAR        = 107297, ///< Alt Power bar for Hozen disrupted (summon Ook-Ook bar, 40).
    SPELL_ANCESTRAL_BREWM_V = 113124, ///< Ancestral Bewmaster cosmetic.
    SPELL_DUSTY_EXPLOSION   = 115996
};

enum CreaturesIds
{
    // Bosses
    BOSS_OOKOOK                     = 56637,
    BOSS_HOPTALLUS                  = 56717,
    BOSS_YANZHU_THE_UNCASKED        = 59479,

    // NPCs
    NPC_AUNTIE_STORMSTOUT           = 59822,
    NPC_CHEN_STORMSTOUT_ENTRANCE    = 59704,
    NPC_CHEN_STORMSTOUT_YANZHU      = 64361,
    NPC_ANCESTRAL_BREWMASTER        = 59075, ///< Friendly.

    // - Ook-Ook.
    NPC_OOK_BARREL                  = 56682,
    NPC_HOZEN_HOLLERER              = 56783,
    NPC_BARREL_TOSS_TARGET          = 55091,

    // - Hoptallus.
    NPC_HOPPER                      = 59464,
    NPC_EXPLOSIVE_BREW_BARREL       = 59429, ///< Hold by Hopper
    NPC_HOPPLING                    = 60208,
    NPC_BOPPER                      = 59551,
    NPC_BOPPER_HAMMER               = 59539, ///< Hammer dropped by Boppers.
    NPC_NIBBLER                     = 65392, ///< Virmen with wheelbarrow

    // Yan-zhu the Uncasked
    NPC_YEASTY_BREW_ALEMENTAL_Y     = 66413,
    NPC_FIZZY_BUBBLE                = 59799,
    NPC_WALL_OF_SUDS                = 59512,
    NPC_BUBBLE_SHIELD               = 65522,

    // - Instance.
    NPC_BEER_BARREL_BUNNY           = 66215, ///< Npc on the ground (Beam cast on).
    NPC_LEAKING_BEER_BARREL         = 73186, ///< Npc on beer barrel.

    NPC_AQUA_DANCER                 = 56865,
    NPC_FIERY_TRICKSTER             = 56867,
    NPC_SODDEN_HOZEN_BRAWLER        = 59605,
    NPC_INFLAMED_HOZEN_BRAWLER      = 56924,
    NPC_SLEEPY_HOZEN_BRAWLER        = 56863,
    NPC_DRUNKEN_HOZEN_BRAWLER       = 56862,
    NPC_HOZEN_BOUNCER               = 56849,
    NPC_HOZEN_PARTY_ANIMAL          = 59684,
    NPC_STOUT_BREW_ALEMENTAL        = 59519,
    NPC_SUDSY_BREW_ALEMENTAL        = 59522,
    NPC_POOL_OF_SUDS                = 56748,
    NPC_UNRULY_ALEMENTAL            = 56684,
    NPC_BUBBLING_BREW_ALEMENTAL     = 59521,
    NPC_BUBBLE_SHIELD_TRASH         = 59487,
    NPC_BLOATED_BREW_ALEMENTAL      = 59518,
    NPC_FIZZY_BREW_ALEMENTAL        = 59520,
    NPC_CARBONATION_POOL            = 56746,
    NPC_YEASTY_BREW_ALEMENTAL       = 59494,
    NPC_HABANERO_BREW               = 56731,
    NPC_GENERAL_PURPOSE_BUNNY_JMF   = 55091,
    NPC_GENERAL_PURPOSE_BUNNY_B     = 45979
};

Position const HoptallusSummonPosition  = { -713.955f, 1254.574f, 164.790f }; ///< Hoptallus summon position.

enum GameObjects
{
    GAMEOBJECT_BREWERY_DOOR         = 209938, ///< Instance doors.
    GAMEOBJECT_PART_CHEWED_CARROT   = 210938, ///< Hoptallus door.
    GAMEOBJECT_OOK_OOK_DOOR         = 211127, ///< Great Barrel
    GAMEOBJECT_GIANT_CARROT         = 211126, ///< Big Carrot blocking stairs after Hoptallus
    GAMEOBJECT_GIANT_BARREL         = 211138, ///< Giant Barrel in which Hoptallus hides

    ChallengeModeDoor               = 211989
};

enum eSharedActions
{
    ACTION_OOK_OOK_DEAD = 1,
    ACTION_OOK_OOK_FIGHT,
    ACTION_OOK_OOK_TALK,
    ACTION_HOPTALLUS_APPEARS,
    ACTION_SUMMON_YANZHU_WAVES
};

enum eMisc
{
    DISPLAYID_INVISIBLE = 11686,
    SCENE_HOPTALLUS_KEG = 59
};

enum eScenarioDatas
{
    ScenarioID  = 45,

    /// Bosses datas
    OokOok      = 19236,
    Hoptallus   = 19237,
    YanZhu      = 19108,
    Ennemies    = 19615,
    KillCount   = 25
};

#endif ///< DEF_STORMSTOUT_BREWERY_HPP
