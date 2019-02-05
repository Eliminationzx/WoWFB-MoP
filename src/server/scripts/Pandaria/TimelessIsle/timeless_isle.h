/*
	* Copyright © 2016 Emiliyan Kurtseliyanski a.k.a JunkyBulgaria
	* ------------------------------------------------------------
	* Do whatever you want with this code but DO NOT remove the
	* copyright and respect it's authors.
*/

#define CELESTIAL_COURT_BOSS_INTRO_TIMER_1 1000
#define CELESTIAL_COURT_BOSS_INTRO_TIMER_2 15000

enum Datas
{
	DATA_CHI_JI							= 0,
	DATA_NIUZAO							= 1,
	DATA_YU_LON							= 2,
	DATA_XUEN							= 3,
	DATA_ORDOS							= 4,
};

enum Bosses
{
	BOSS_CHI_JI							= 71952,
	BOSS_NIUZAO							= 71954,
	BOSS_YU_LON							= 71955,
	BOSS_XUEN							= 71953,
	BOSS_ORDOS							= 72057
};

enum Rares
{
	RARE_ARCHIEREUS_OF_FLAME			= 73174,
	RARE_ARCHIEREUS_OF_FLAME_CLOAK		= 73666, // the above and this line are the SAME npcs, but with different ids. One is spawned at the very ground, close to the center of the Timeless Isle, the other one is spawned at the top place of the Timeless Isle, that you need the cloak to get there.
	RARE_BUFO							= 72775,
	RARE_CHAMPION_OF_THE_BLACK_FLAME	= 73171,
	RARE_CHELON							= 72045,
	RARE_CINDERFALL						= 73175,
	RARE_CRANEGNASHER					= 72049,
	RARE_DREAD_SHIP_VAZUVIUS			= 73281,
	RARE_EMERALD_GANDER					= 73158,
	RARE_EVERMAW						= 73279,
	RARE_FLINTLORD_GAIRAN				= 73172,
	RARE_GOLGANARR						= 72970,
	RARE_GREAT_TURTLE_FURYSHELL			= 73161,
	RARE_GUCHI_THE_SWARMBRINGER			= 72909,
	RARE_HUOLON							= 73167,
	RARE_IRONFUR_STEELHORN				= 73160,
	RARE_IMPERIAL_PYTHON				= 73163,
	RARE_JAKUR_OF_ORDON					= 73169,
	RARE_KARKANOS						= 72193,
	RARE_LEAFMENDER						= 73277,
	RARE_MONSTROUS_SPINECLAW			= 73166,
	RARE_RATTLESKEW						= 72048,
	RARE_ROCK_MOSS						= 73157,
	RARE_SPELURK						= 71864,
	RARE_SPIRIT_OF_JADEFIRE				= 72769,
	RARE_TSAVOKA						= 72808,
	RARE_URDUR_THE_CAUTERIZER			= 73173,
	RARE_WATCHER_OSU					= 73170,
	RARE_ZESQUA							= 72245,
	RARE_ZHU_GON_THE_SOUR				= 71919,
	RARE_STINKBRAID						= 73704,
};

enum Creatures
{
    NPC_SKUNKY_ALEMENTAL                = 71908,
	NPC_EMPEROR_SHAOHAO_TI				= 73303,
	NPC_ETERNAL_KILNMASTER				= 72896,
	NPC_HIGH_PRIEST_OF_ORDOS			= 72898,
	NPC_BLAZEBOUND_CHANTER				= 72897,

	// Generic (Invisible)
	NPC_TIME_LOST_SHRINE_TRIGGER		= 73461 // I think this is the correct ID :P
};

enum Factions
{
    FACTION_FRIENDLY = 35,
    FACTION_HOSTILE_NEUTRAL = 31
};

// Scripts information

enum npcSpells
{
    // Turtles
    SPELL_GEYSER_1 = 147572,
    SPELL_GEYSER_2 = 147573,
    SPELL_SHELL_SPINE_1 = 147571,
    SPELL_SHELL_SPINE_2 = 147590,
    SPELL_SNAPPING_BITE_1 = 147569,
    SPELL_SNAPPING_BITE_2 = 147589,
    SPELL_SNAPPING_BITE_3 = 147568,

    // Faction Sentries
    SPELL_INCAPACITATING_SHOUT = 18328,
    SPELL_MORTAL_STRIKE = 19643,
    SPELL_NET = 81210,
    SPELL_SNAP_KICK = 15618,

    // Adders
    SPELL_HUGE_FANG_1 = 147290,
    SPELL_HUGE_FANG_2 = 147289,

    // Yaks
    SPELL_HEADBUTT_1 = 147384,
    SPELL_HEADBUTT_2 = 147354,
    SPELL_HEADBUTT_3 = 147382,
    SPELL_IRON_FUR = 147368,
    SPELL_OX_CHARGE = 147385,
    SPELL_OX_CHARGE_TRIGGER = 147386,

    // Cranes
    SPELL_GUST_OF_WIND = 147310,
    SPELL_HEALING_SONG = 147309,
    SPELL_WINDFEATHER = 147306,

    // Silkworms
    SPELL_VENOM_SPIT = 147535,
    SPELL_POISON_SPIT = 147496,

    // Crabs
    SPELL_CLAW_FLURRY_1 = 147557,
    SPELL_CLAW_FLURRY_2 = 147560,

    // Spectrals
    SPELL_GHOST_VISUAL = 145945,
    SPELL_DRUNKEN_STUPOR = 146948,
    SPELL_CHI_BURST = 148729,
    SPELL_HEALING_MISTS = 148728,
    SPELL_SPINNING_CRANE_KICK = 148730,

    // Cats
    SPELL_DEFENSIVE_LEAP = 147650,
    SPELL_POUNCE_CHARGE = 147649,
    SPELL_POUNCE_STUN = 147651,
    SPELL_RENDING_SWIPE = 147646,
    SPELL_GREATER_SWIPE = 147652,

    // Alementals
    SPELL_BREW_BOLT = 147345,
    SPELL_RAIN_OF_SKUNKY_ALE = 147347,
    SPELL_SKUNKY_ALE_CRASH = 144882,

    // Spirits
    SPELL_SPIRITFIRE_BOLT = 147446,
    SPELL_SPIRITFLAME_STRIKE = 147448,
    SPELL_JADEFIRE_BOLT = 147454,
    SPELL_JADEFLAME_STRIKE = 147456,

    // Giants
    SPELL_BOULDER = 147546,
    SPELL_FRENZY = 147510,
    SPELL_STOMP = 147500,
    SPELL_FRENZY_STOMP = 147512,
    SPELL_EXHAUSTION = 147513,

    // Frogs
    SPELL_GULP_FROG_TOXIN = 147655,
    SPELL_TOXIC_SKIN = 147654,
    SPELL_INSTAKILL_TOXIN = 147656,

    // Dragons
    SPELL_FIRE_BLOSSOM = 147818,
    SPELL_FLAME_BREATH = 147817,
    SPELL_LIGHTNING_BREATH = 147826,
    SPELL_STORM_BLOSSOM = 147828,

    // Ordos minions
    SPELL_BLAZING_BLOW = 148003,
    SPELL_CONJURE_ETERNAL_KILN = 148004,
    SPELL_FIERY_CHARGE = 147704,
    SPELL_CAUTERIZE = 147997,
    SPELL_CONJURE_FLARECORE_GOLEM = 148001,
    SPELL_FIRE_STORM = 147998,
    SPELL_GREATER_PYROBLAST = 148002,

    // Eternal Kiln
    SPELL_KILNFIRE = 148005,

    // Damp shamblers
    SPELL_MURKY_CLOUD = 147805,
    SPELL_RENEWING_MISTS = 147769,
    SPELL_POISON_BREATH = 144892,

    // Ordon guards
    SPELL_CRACKING_BLOW = 147674,
    SPELL_DEFENSIVE_SHIELD = 147689,
    SPELL_OATH_OF_GUARDIANSHIP = 147682,

    // Ordos warriors
    SPELL_BLAZING_CLEAVE = 147702,
    SPELL_BURNING_FURY = 147703,
    //SPELL_FIERY_CHARGE = 147704, Already in enum
    SPELL_FORAGE = 149463,

    // Ordon watchers
    SPELL_FALLING_FLAMES = 147723,
    SPELL_PYROBLAST = 147720,

    // Karkanos
    SPELL_GUSHING_MAW = 147843,

    // Zesqua
    SPELL_FURIOUS_SPLASH = 147335,
    SPELL_WATER_BLOSSOM = 147439,
    SPELL_WATER_BOLT_VOLLEY = 147344,

    // Jademist Dancer
    SPELL_STEAM_BLAST = 147863,

    // Molten Guardian
    SPELL_MOLTEN_INFERNO_1 = 147879,
    SPELL_MOLTEN_INFERNO_2 = 147880,
    SPELL_MOLTEN_INFERNO_3 = 147881,

    // Cranegnasher
    SPELL_BITE_MORSEL = 147400,
    SPELL_CLAW = 147395,
    SPELL_POUNCE = 147394,
    SPELL_RAKE = 147396,
    SPELL_STEALTH = 128933,

    // Nice Sprite
    SPELL_NATURE_S_WRATH = 114997,
    SPELL_STICKY_SAP = 144424,

    // Ordon Candlekeeper
    SPELL_BREATH_OF_FIRE = 147416,
    SPELL_BURNING_SACRIFICE = 147422,

    // Garnia
    SPELL_RUBY_BOLT = 147896,

    // Rattleskew
    SPELL_CURSE_OF_PAIN = 38048,
    SPELL_DROWNING_DEATH = 16555,
    SPELL_SHADOW_BOLT = 43330,
    SPELL_SPIKED_BONE = 128511,

    // Southsea Lookout
    SPELL_SPY_GLASS = 141528,

    // Leafmender
    SPELL_GREAT_WRATH = 147562,
    SPELL_HEALING_TOUCH = 147563,
    SPELL_SUN_WORSHIP = 147565,

    // Playful Water Spirit
    SPELL_BUBBLE_POP = 147444,
    SPELL_WATER_BOLT = 32011,
    SPELL_WATER_SPOUT = 39207,

    // Cove Shark
    SPELL_SAVAGE_BITE = 126062,
    SPELL_SCENT_OF_BLOOD = 126063,

    // Stinkbraid
    SPELL_LEAPING_CLEAVE = 83622,
    SPELL_ROTTEN_STENCH = 85253,

    // Spectral Pirate
    SPELL_BLACK_FOG = 128049,
    SPELL_SINISTER_STRIKE = 129040
};

enum EmperorActions
{
    ACTION_XUEN = 1,
    ACTION_CHIJI = 2,
    ACTION_NIUZAO = 3,
    ACTION_YULON = 4,
    ACTION_MOVE_TO_MIDDLE = 100
};

enum EmprerorTalk
{
    EMPEROR_TALK_INTRO_YULON,
    EMPEROR_TALK_INTRO_XUEN,
    EMPEROR_TALK_INTRO_CHIJI,
    EMPEROR_TALK_INTRO_NIUZAO,
    EMPEROR_TALK_OUTRO_YULON,
    EMPEROR_TALK_OUTRO_XUEN,
    EMPEROR_TALK_OUTRO_CHIJI,
    EMPEROR_TALK_OUTRO_NIUZAO,
    EMPEROR_TALK_INTRO
};

// August Celestials Only
const Position summonPos[4] = 
{
    {-553.17f, -4949.02f, -6.27f, -2.30f},
    {-740.80f, -4955.08f, -6.27f, 5.63f},
    {-746.32f, -5078.48f, -6.27f, 0.69f},
    {-551.16f, -5080.01f, -6.27f, 2.28f},
};

const Position CentrPos[1] =
{
    {-650.14f, -5024.67f, -6.27f},
};

const Position WallPos[7] =
{
    {-680.63f, -5091.60f, -6.19f, 1.58f},
    {-713.89f, -5091.50f, -6.19f, 1.57f},
    {-580.55f, -5091.66f, -6.19f, 1.58f},
    {-613.91f, -5091.65f, -6.19f, 1.54f},
    {-547.26f, -5091.75f, -6.19f, 1.56f},
    {-647.20f, -5091.63f, -6.19f, 1.57f},
    {-747.19f, -5091.65f, -6.19f, 1.59f},
};

enum Says
{
    //Annoncer
    SAY_CHIJI_START         = 2,
    SAY_CHIJI_FINISH        = 6,
    SAY_XUEN_START          = 1,
    SAY_XUEN_FINISH         = 5,
    SAY_YULON_START         = 0,
    SAY_YULON_FINISH        = 4,
    SAY_NIUZAO_START        = 3,
    SAY_NIUZAO_FINISH       = 7,

    //Misc
    SAY_ENTER_POS           = 1,
    SAY_AGGRO               = 0,

    //ChiJi
    SAY_CHIJI_BEACON        = 4,
    SAY_CHIJI_RUSH_1        = 6,
    SAY_CHIJI_BLAZ_SONG     = 4,
    SAY_CHIJI_RUSH_2        = 5,
    SAY_CHIJI_END           = 2,

    //Xuen
    SAY_XUEN_BARRAGE        = 5,
    SAY_XUEN_LIGHTNING      = 6,
    SAY_XUEN_PLAYER_DEATH   = 3,
    SAY_XUEN_AGGILITY       = 4,
    SAY_XUEN_END            = 2,

    //Yulon
    SAY_YULON_PLAYER_DEATH   = 3,
    SAY_YULON_WALL_1         = 4,
    SAY_YULON_WALL_2         = 6,
    SAY_YULON_END            = 2,

    //Niuzao
    SAY_NIUZAO_CHARGE_1      = 4,
    SAY_NIUZAO_PLAYER_DEATH  = 3,
    SAY_NIUZAO_QUAKE         = 6,
    SAY_NIUZAO_CHARGE_2      = 5,
    SAY_NIUZAO_END           = 2,
};

enum Npc
{
    NPC_SHAOHAO         = 73303,
    NPC_CHIJI           = 71952,
    NPC_XUEN            = 71953,
    NPC_YULON           = 71955,
    NPC_NIUZAO          = 71954,
    //Summons
    NPC_FIRESTORM       = 71971,
    NPC_BEACON_OF_HOPE  = 71978,
    NPC_CHILD_OF_CHIJI  = 71990,
    NPC_JADEFIRE_BLAZE  = 72016,
    NPC_JADEFIRE_WALL   = 72020,
};

enum Spells
{
    SPELL_CELESTIAL_SPAWN       = 149307,
    //ChiJi
    SPELL_FIRESTORM             = 144461,
    SPELL_FIRESTORM_AURA        = 144463,
    SPELL_INSPIRING_SONG        = 144468,
    SPELL_BEACON_OF_HOPE        = 144473,
    SPELL_BEACON_OF_HOPE_AURA   = 144474,
    SPELL_BLAZING_SONG          = 144471,
    SPELL_BLAZING_NOVA          = 144493,
    SPELL_CRANE_RUSH            = 144470,
    //Xuen
    SPELL_CHI_BARRAGE           = 144642,
    SPELL_CRACKLING_LIGHTNING   = 144635,
    SPELL_LEAP                  = 144640,
    SPELL_SPECTRAL_SWIPE        = 144638,
    SPELL_AGILITY               = 144631,
    //Yulon
    SPELL_JADEFLAME_BUFFET      = 144630,
    SPELL_JADEFIRE_BREATH       = 144530,
    SPELL_JADEFIRE_BLAZE        = 144537,
    SPELL_JADEFIRE_WALL         = 144533,
    //Niuzao
    SPELL_MASSIVE_QUAKE         = 144611,
    SPELL_OXEN_FORTITUDE        = 144606,
    SPELL_HEADBUTT              = 144610,
    SPELL_CHARGE                = 144608,

    //Ordos
    SPELL_BANISHMENT            = 148705,
    SPELL_CELESTIAL_WINDS       = 149322,
    SPELL_ANCIENT_FLAME         = 144695,
    SPELL_MAGMA_CRUSH           = 144688,
    SPELL_BURNING_SOUL          = 144689,
    SPELL_ETERNAL_AGONY         = 144696,

    //Other
    SPELL_GHOSTLY_VOID          = 147495,
    SPELL_DESATURATE            = 129290,
    SPELL_SPIRIT_STRANGLE       = 144059,
};

enum Events
{
    //Annoncer
    EVENT_CHIJI_START       = 1,
    EVENT_XUEN_START        = 2,
    EVENT_YULON_START       = 3,
    EVENT_NIUZAO_START      = 4,
    EVENT_CHIJI_END         = 5,
    EVENT_XUEN_END          = 6,
    EVENT_YULON_END         = 7,
    EVENT_NIUZAO_END        = 8,
    //ChiJi
    EVENT_FIRESTORM         = 3,
    EVENT_INSPIRING_SONG    = 4,
    EVENT_BEACON            = 5,
    EVENT_BLAZING_SONG      = 6,
    //Xuen
    EVENT_CHI_BARRAGE       = 3,
    EVENT_LIGHTNING         = 4,
    EVENT_LEAP              = 5,
    EVENT_SPECTRAL_SWIPE    = 6,
    EVENT_AGILITY           = 7,
    //Yulon
    EVENT_JADEFLAME_BUFFET  = 3,
    EVENT_JADEFIRE_BREATH   = 4,
    EVENT_JADEFIRE_BOLT     = 5,
    EVENT_FIRE_WALL         = 6,
    //Niuzao
    EVENT_MASSIVE_QUAKE     = 3,
    EVENT_OXEN_FORTITUDE    = 4,
    EVENT_HEADBUTT          = 5,
    EVENT_MOVE_PATH_1       = 6,
    EVENT_MOVE_PATH_2       = 7,
};

enum Actions
{
    ACTION_MOVE_CENTR_POSS  = 1,
    ACTION_CHIJI_END        = 2,
    ACTION_CHIJI_FAIL       = 3,
    ACTION_XUEN_END         = 4,
    ACTION_XUEN_FAIL        = 5,
    ACTION_YULON_END        = 6,
    ACTION_YULON_FAIL       = 7,
    ACTION_NIUZAO_END       = 8,
    ACTION_NIUZAO_FAIL      = 9,
};

static Position _timelessIsleMiddle = { -650.04f, -5016.84f, -6.27f, 1.573f };

#define MIDDLE_FACING_ANGLE 1.573f
