#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "db.h"
const struct part_type part_table[] =
{
	{ 0, "NONE", "NONE", "NONE", "$n hits the ground ... DEAD." },
	{ 0, "NONE", "NONE", "NONE", "$n splatters {rblood {xon your armor." },
	{
		PRT_HEAD,
		"severed head",
		"the severed head of %s",
		"The severed head of %s lies in a pool of blood.",
		"$n's severed head plops on the ground."
	},
	{
		PRT_ARMS,
		"sliced-off arm",
		"the sliced-off arm of %s",
		"The sliced-off arm of %s is lying here.",
		"$n's arm is sliced from $s dead body."
	},
	{
		PRT_LEGS,
		"severed leg",
		"the severed leg of %s",
		"The severed leg of %s lies in a pool of blood.",
		"$n's leg is sliced from $s dead body."
	},
	{
		PRT_HEART,
		"torn heart",
		"the heart of %s",
		"The torn-out heart of %s is lying here.",
		"$n's heart is torn from $s chest."
	},
	{
		PRT_BRAINS,
		"splattered brains",
		"the brains of %s",
		"The splattered brains of %s are lying here.",
		"$n's head is shattered, and $s brains splash all over you."
	},
	{
		PRT_GUTS,
		"pile entrails guts",
		"the guts of %s",
		"A steaming pile of %s's guts are lying here.",
		"$n spills $s guts all over the floor."
	},
	{
		PRT_HANDS,
		"severed hand",
		"the severed hand of %s",
		"A bloody severed hand of %s is here.",
		"$n's hand is severed and flops to the ground."
	},
	{
		PRT_FEET,
		"severed foot",
		"the foot of %s",
		"A bloody severed foot of %s is here.",
		"$n's foot is rended from $s leg."
	},
	{
		PRT_FINGERS,
		"pile fingers",
		"a pile of %s's fingers",
		"A steaming pile of %s's bloody fingers are lying here.",
		"$n's fingers are ripped from $s hand and fall to the ground."
	},
	{
		PRT_EAR,
		"ripped ear",
		"the ripped out ear of %s",
		"A bloody ear of %s is here.",
		"$n's ear catches and rips from $s head."
	},
	{
		PRT_EYE,
		"torn-out eye",
		"the torn out eye of %s",
		"A bloody eye has been torn from %s's head.",
		"$n's eye is torn from $s head and flops across the ground."
	},
	{
		PRT_LONG_TONGUE,
		"long bloody tongue",
		"the long tongue of %s",
		"A long, bloody tongue belonging to %s is here.",
		"$n's tongue is torn out of $s mouth."
	},
	{
		PRT_EYESTALKS,
		"gory eyestalk",
		"the gory eyestalk of %s",
		"A gory eyestalk of %s is here.",
		"$n's eyestalk is lobbed off."
	},
	{
		PRT_TENTACLES,
		"severed tentacle",
		"the severed tentacle of %s",
		"A severed tentacle of %s is here.",
		"$n's tentacle is severed from $s body and flops to the floor."
	},
	{
		PRT_FINS,
		"ripped fin",
		"the fin of %s",
		"A ripped and bloody fin of %s's lies here.",
		"$n's fin is ripped from $s body."
	},
	{
		PRT_WINGS,
		"ripped wing",
		"the ripped wing of %s",
		"%s's bloody wing lies here.",
		"$n's wing is ripped from $s body and falls to the ground."
	},
	{
		PRT_TAIL,
		"lobbed tail",
		"the tail of %s",
		"A tail, possibly belonging to %s is in a pool of blood here.",
		"$n's tail is lobbed off and flops to the ground."
	},
	{
		PRT_NULLA,
		"pile entrails guts",
		"the guts of %s",
		"A steaming pile of %s's guts are lying here.",
		"$n spills $s guts all over the floor."
	},
	{
		PRT_NULLB,
		"pile entrails guts",
		"the guts of %s",
		"A steaming pile of %s's guts are lying here.",
		"$n spills $s guts all over the floor."
	},
	{
		PRT_NULLC,
		"pile entrails guts",
		"the guts of %s",
		"A steaming pile of %s's guts are lying here.",
		"$n spills $s guts all over the floor."
	},
	{
		PRT_CLAWS,
		"ripped claw",
		"the bloody claw of %s",
		"A bloody claw of %s's lies here.",
		"$n's claw catches and is ripped out."
	},
	{
		PRT_FANGS,
		"bloody knocked fang",
		"%s's fang",
		"A bloody fang has been left here.",
		"A fang is knocked out of $n's mouth and goes flying."
	},
	{
		PRT_HORNS,
		"pile entrails guts",
		"the guts of %s",
		"A steaming pile of %s's guts are lying here.",
		"$n spills $s guts all over the floor."
	},
	{
		PRT_SCALES,
		"few scales",
		"a few of %s's scales",
		"A few of %s's scales lie here.",
		"Several of $n's scales are torn from $m and scatter across the ground."
	},
	{
		PRT_TUSKS,
		"ripped tusk",
		"the bloody tusk of %s",
		"%s's tusk lies in a pool of blood.",
		"$n's tusk is ripped from $s mouth."
	}
};

const struct excl_type excl_table[] =
{
	{	EXL_FIGHTER,			CLASS_IMMORTAL,			"fighter",		1,	true	},
	{	EXL_HUMAN,				0,						"human",		0,	false	},
	{	EXL_ALGID,				0,						"algid",		0,	false	},
	{	EXL_DAURIN,				0,						"daurin",		0,	false	},
	{	EXL_QUOLIN,				0,						"quolin",		0,	false	},
	{	EXL_VITHE,				0,						"vithe",		0,	false	},
	{	EXL_GURHA,				0,						"gurha",		0,	false	},
	{	EXL_DEWDOP,				0,						"dewdop",		0,	false	},
	{	EXL_DWARF,				0,						"dwarf",		0,	false	},
	{	0,						0,						NULL,			true	}
};

const struct sect_type sect_table[] =
{//     name				color	bit				dirt	mvredct	 manaheal  hpheal  mvheal   indoors settable     nashneedstofixtheheals
    {	"inside",			"{W",	SECT_INSIDE,		-20,	1,		100,	100,	100,	true,	true	},
    {	"city",				"{W",	SECT_CITY,			-10,	0,		100,	100,	100,	false,	true	},
    {	"field",			"{G",	SECT_FIELD,			3,		2,		100,	100,	100,	false,	true	},
    {	"forest",			"{g",	SECT_FOREST,		2,		2,		100,	100,	100,	false,	true	},
    {	"hills",			"{y",	SECT_HILLS,			3,		3,		100,	100,	100,	false,	true	},
    {	"mountain",			"{y",	SECT_MOUNTAIN,		6,		4,		100,	100,	100,	false,	true	},
    {	"swim",				"{B",	SECT_WATER_SWIM,	0,		4,		100,	100,	100,	false,	true	},
    {	"noswim",			"{B",	SECT_WATER_NOSWIM,	0,		1,		100,	100,	100,	false,	true	},
    {   "underwater",		"{b",	SECT_UNDERWATER,	0,		6,		100,	100,	100,	true,	true	},
    {	"air",				"{C",	SECT_AIR,			0,		1,		100,	100,	100,	false,	true	},
    {	"desert",			"{Y",	SECT_DESERT,		10,		10,		10,		10,		10,		false,	true	},
    {	"volcano",			"{r",	SECT_VOLCANO,		5,		4,		100,	100,	100,	false,	true	},
    {	"rainforest",		"{g",	SECT_RAINFOREST,	0,		5,		100,	100,	100,	false,	true	},
    {	"swamp",			"{c",	SECT_SWAMP,			0,		5,		10,		10,		10,		false,	true	},
    {	"river",			"{B",	SECT_RIVER,			0,		4,		100,	100,	100,	false,	true	},
    {	"tunnel",			"{d",	SECT_TUNNEL,		7,		4,		100,	100,	100,	true,	true	},
    {	"forestcity",		"{g",	SECT_FORESTCITY,	-3,		2,		100,	100,	100,	false,	true	},
    {	"mountaincity",		"{y",	SECT_MOUNTAINCITY,	6,		3,		100,	100,	100,	false,	true	},
    {	"desertcity",		"{y",	SECT_DESERTCITY,	10,		3,		100,	100,	100,	false,	true	},
    {	"graveyard",		"{d",	SECT_GRAVEYARD,		10,		6,		10,		10,		10,		false,	true	},
    {	"road",				"{d",	SECT_ROAD,			6,		6,		100,	100,	100,	false,	true	},
    {	"dirtroad",			"{y",	SECT_DIRTROAD,		10,		2,		100,	100,	100,	false,	true	},
    {	"cityroad",			"{w",	SECT_CITYROAD,		-10,	0,		100,	100,	100,	false,	true	},
    {	"dirtpath",			"{y",	SECT_DIRTPATH,		10,		3,		100,	100,	100,	false,	true	},
    {	"goodtemple",		"{W",	SECT_GOODTEMPLE,	-20,	1,		100,	100,	100,	true,	true	},
    {	"neutraltemple",	"{w",	SECT_NEUTRALTEMPLE,	-20,	1,		100,	100,	100,	true,	true	},
    {	"eviltemple",		"{d",	SECT_EVILTEMPLE,	1,		1,		100,	100,	100,	true,	true	},
    {	"cliffside",		"{y",	SECT_CLIFFSIDE,		0,		3,		100,	100,	100,	false,	true	},
    {	"icetunnel",		"{C",	SECT_ICETUNNEL,		1,		5,		10,		10,		10,		true,	true	},
    {	"boardwalk",		"{Y",	SECT_BOARDWALK,		-10,	1,		100,	100,	100,	false,	true	},
    {	"beach",			"{Y",	SECT_BEACH,			5,		3,		100,	100,	100,	false,	true	},
    {	"SEWER",			"{d",	SECT_SEWER,			1,		1,		100,	100,	100,	true,	true	},
    {	NULL,				"{x",	0,					0,		0,		0,		0,		0,		true,	false	}
};

const struct position_type position_table[] =
{
    {	"dead",			"dead"	},
    {	"mortally wounded",	"mort"	},
    {	"incapacitated",	"incap"	},
    {	"stunned",		"stun"	},
    {	"sleeping",		"sleep"	},
    {	"resting",		"rest"	},
    {   "sitting",		"sit"   },
    {	"fighting",		"fight"	},
    {	"standing",		"stand"	},
    {	NULL,			NULL	}
};

const struct sex_type sex_table[]={
   {	"none",		"its",	"it",	"it"		},
   {	"male",		"his",	"him",	"he"		},
   {	"female",	"her",	"her",	"she"		},
   {	"either",	"plop",	"plop",	"plop"		},
   {	NULL,		NULL,	NULL,	NULL		}
};

const struct size_type size_table[] =
{
    {	"zero"		},
    {	"tiny"		},
    {	"small" 	},
    {	"medium"	},
    {	"large"		},
    {	"huge", 	},
    {	"giant" 	},
    {	NULL		}
};

const struct flag_type act_flags[] =
{
    {	"(bug)",			0,					false	},
	{	"npc",				AT_IS_NPC,			false	},
	{	"sentinel",			AT_SENTINEL,		true	},
	{	"scavenger",		AT_SCAVENGER,		true	},
	{	"nokill",			AT_NOKILL,			true	},
	{	"mount",			AT_MOUNT,			true	},
	{	"aggressive",		AT_AGGRESSIVE,		true	},
	{	"stay_area",		AT_STAY_AREA,		true	},
	{	"wimpy",			AT_WIMPY,			true	},
	{	"pet",				AT_PET,				true	},
	{	"remembers",		AT_REMEMBERS,		true	},
	{	"passive",			AT_PASSIVE,			true	},
	{	"nopeek",			AT_NOPEEK,			true	},
	{	"guard",			AT_GUARD,			true	},
	{	"undead",			AT_UNDEAD,			true	},
	{	"true_sight",		AT_TRUE_SIGHT,		true	},
	{	"cleric",			AT_CLERIC,			false	},
	{	"mage",				AT_MAGE,			false	},
	{	"thief",			AT_THIEF,			true	},
	{	"fighter",			AT_FIGHTER,			false	},
	{	"noalign",			AT_NOALIGN,			true	},
	{	"nopurge",			AT_NOPURGE,			true	},
	{	"outdoors",			AT_OUTDOORS,		true	},
	{	"aggressive_mob",	AT_AGGRESSIVE_MOB,	true	},
	{	"indoors",			AT_INDOORS,			true	},
	{	"warhorse",			AT_WARHORSE,		true	},
	{	"healer",			AT_IS_HEALER,		true	},
	{	"gain",				AT_GAIN,			true	},
	{	"update_always",	AT_UPDATE_ALWAYS,	true	},
	{	"changer",			AT_IS_CHANGER,		true	},
	{	"bank",				AT_BANKER,			true	},
	{	"NULL",				AT_NULL,			false	},
	{	NULL,				0,					false	}
};

const struct flag_type climate_flags[] =
{
    {	"arid",				CLIMATE_ARID,			true	},
    {	"temperate",		CLIMATE_TEMPERATE,		true	},
    {	"tropical",			CLIMATE_TROPICAL,		true	},
    {	"frigid",			CLIMATE_FRIGID,			true	},
    {	"desolate",			CLIMATE_DESOLATE,		true	},
    {	"ocean",			CLIMATE_OCEAN,			true	},
	{	NULL,				0,						false	}
};

const struct flag_type plr_flags[] =
{
    {	"(bug)",			0,					false	},
	{	"npc",				PL_IS_NPC,			false	},
	{	"bashed",			PL_BASHED,			false	},
	{	"autoexit",			PL_AUTOEXIT,		false	},
	{	"autoloot",			PL_AUTOLOOT,		false	},
	{	"autosac",			PL_AUTOSAC,			false	},
	{	"autogold",			PL_AUTOGOLD,		false	},
	{	"autosplit",		PL_AUTOSPLIT,		false	},
	{	"arena",			PL_ARENA,			false	},
	{	"autoweather",		PL_AUTOWEATHER,		false	},
	{	"forgin",			PL_FORGING,			false	},
	{	"automap",			PL_AUTOMAP,			false	},
	{	"holylight",		PL_HOLYLIGHT,		false	},
	{	"autoassist",		PL_AUTOASSIST,		false	},
	{	"can_loot",			PL_CANLOOT,			false	},
	{	"nosummon",			PL_NOSUMMON,		false	},
	{	"nofollow",			PL_NOFOLLOW,		false	},
	{	"mortal_leader",	PL_MORTAL_LEADER,	false	},
	{	"color",			PL_COLOR,			false	},
	{	"permit",			PL_PERMIT,			true	},
	{	"nullg",			PL_NULLG,			false	},
	{	"log",				PL_LOG,				false	},
	{	"deny",				PL_DENY,			false	},
	{	"freeze",			PL_FREEZE,			false	},
	{	"thief",			PL_THIEF,			false	},
	{	"killer",			PL_KILLER,			false	},
	{	"pk",				PL_PK,				false	},
	{	"pks",				PL_SCRAMBLE,		false	},
	{	NULL,				0,					false	}
};

const struct flag_type affect_flags[] ={
    {	"(none)",			AF_BLANK,			false	},
    {	"blind",			AF_BLIND,			true	},
    {	"invisible",		AF_INVISIBLE,		true	},
    {	"spellshield",		AF_SPELLSHIELD,		false	},
    {	"detect_invis",		AF_DETECT_INVIS,	true	},
    {	"detect_magic",		AF_DETECT_MAGIC,	true	},
    {	"detect_hidden",	AF_DETECT_HIDDEN,	true	},
    {	"phase",			AF_PHASE,			false	},
    {	"sanctuary",		AF_SANCTUARY,		true	},
    {	"faerie_fire",		AF_FAERIE_FIRE,		true	},
    {	"infrared",			AF_INFRARED,		true	},
    {	"curse",			AF_CURSE,			true	},
    {	"sapped",			AF_SAP,				false	},
    {	"poison",			AF_POISON,			true	},
    {	"deaf",				AF_DEAF,			true	},
    {	"overdrive",		AF_OVERDRIVE,		true	},
    {	"sneak",			AF_SNEAK,			true	},
    {	"hide",				AF_HIDE,			true	},
    {	"sleep",			AF_SLEEP,			true	},
    {	"charm",			AF_CHARM,			true	},
    {	"flying",			AF_FLYING,			true	},
    {	"pass_door",		AF_PASS_DOOR,		true	},
    {	"haste",			AF_HASTE,			true	},
    {	"calm",				AF_CALM,			true	},
    {	"plague",			AF_PLAGUE,			true	},
    {	"weaken",			AF_WEAKEN,			true	},
    {	"dark_vision",		AF_DARK_VISION,		true	},
    {	"berserk",			AF_BERSERK,			true	},
    {	"swim",				AF_SWIM,			true	},
    {	"regeneration",		AF_REGENERATION,	true	},
    {	"slow",				AF_SLOW,			true	},
    {	"mirage",			AF_MIRAGE,			false	},
    {	"rupture",			AF_RUPTURE,			false	},
    {	"phantomsword",		AF_PSWORD,			false	},
    {	"doublestrike",		AF_DSTRIKE,			false	},
    {	"entangled",		AF_ENTANGLED,		false	},
    {	"snared",			AF_SNARED,			false	},
    {	"alertness",		AF_ALERTNESS,		false	},
    {	"camoflage",		AF_CAMOFLAGE,		true	},
    {	"leadership",		AF_LEADERSHIP,		false	},
    {	"ague's echo",		AF_AGUE,			false	},
    {	"preventheal",		AF_PREVENTHEAL,		true	},
    {	"perfectsneak",		AF_PERFECTSNEAK,	false	},
    {	"degeneration",		AF_DEGENERATION,	true	},
    {	"guardian",			AF_GUARDIAN,		true	},
    {	"confusion",		AF_CONFUSION,		false	},
    {	"weary",			AF_WEARY,			false	},
    {	"slowcast",			AF_SLOWCAST,		true	},
    {	"manaregen",		AF_MANAREGEN,		true	},
    {	"fortify",			AF_FORTIFY,			false	},
    {	"forcevoid",		AF_FORCEVOID,		false	},
    {	"insomnia",			AF_INSOMNIA,		false	},
    {	"narcolepsy",		AF_NARCOLEPSY,		false	},
    {	"catalepsy",		AF_CATALEPSY,		false	},
    {	"silence",			AF_SILENCE,			false	},
    {	"doublecast",		AF_DOUBLECAST,		false	},
    {	"lethargy",			AF_LETHARGY,		false	},
    {	"buffdam_take",		AF_BDAMTAKE,		false	},
    {	"buffdam_give",		AF_BDAMGIVE,		false	},
    {	"defender",			AF_DEFENDER,		false	},
    {	"bulwark",			AF_BULWARK,			false	},
    {	"amiable",			AF_AMIABLE,			false	},
    {	"senses",			AF_SENSES,			false	},
    {	"slowwalk",			AF_SLOWWALK,		false	},
    {	"lightsleep",		AF_LIGHTSLEEP,		false	},
    {	"marking",			AF_MARK,			false	},
    {	"nightmare",		AF_NIGHTMARE,		false	},
    {	"ravage",			AF_RAVAGE,			false	},
    {	"strafe",			AF_STRAFE,			false	},
    {	"stalk",			AF_STALK,			false	},
    {	"sharpshoot",		AF_SHARPSHOOT,		false	},
    {	"immolation",		AF_IMMOLATION,		false	},
    {	"drowning",			AF_DROWNING,		false	},
    {	"energy shield",	AF_ESHIELD,			false	},
    {	"tranquility",		AF_TRANQUILITY,		false	},
    {	"envenomed",		AF_VENOM,			false	},
    {	"windwalk",			AF_WINDWALK,		false	},
    {	"dullspell",		AF_DULLSPELL,		false	},
    {	NULL,				0,					false	}
};

const struct flag_type off_flags[] =
{
    {	"(bug)",			0,					false	},
    {	"area_attack",		OF_AREA_ATTACK,		true	},
    {	"backstab",			OF_BACKSTAB,		true	},
    {	"bash",				OF_BASH,			true	},
    {	"berserk",			OF_BERSERK,			true	},
    {	"disarm",			OF_DISARM,			true	},
    {	"sonic_impact",		OF_SONIC_IMPACT,	true	},
    {	"fade",				OF_FADE,			true	},
    {	"fast",				OF_FAST,			true	},
    {	"kick",				OF_KICK,			true	},
    {	"dirt_kick",		OF_KICK_DIRT,		true	},
    {	"critical",			OF_CRITICAL,		true	},
    {	"rescue",			OF_RESCUE,			true	},
    {	"tail",				OF_TAIL,			true	},
    {	"trip",				OF_TRIP,			true	},
    {	"crush",			OF_CRUSH,			true	},
    {	"second_attack",	OF_SECONDATTACK,	true	},
    {	"third_attack",		OF_THIRDATTACK,		true	},
    {	"nullD",			OF_NULLD,			false	},
    {	"downstrike",		OF_DOWNSTRIKE,		true	},
    {	"falcon",			OF_FALCON,			false	},
    {	"wait",				OF_WAIT,			false	},
    {	"brawl",			OF_BRAWL,			false	},
    {	"nullG",			OF_NULLG,			false	},
    {	"assist_all",		AST_ALL,			true	},
    {	"assist_align",		AST_ALIGN,			true	},
    {	"assist_race",		AST_RACE,			true	},
    {	"assist_players",	AST_PLAYERS,		true	},
    {	"assist_guard",		AST_GUARD,			true	},
    {	"assist_vnum",		AST_VNUM,			true	},
    {	NULL,				0,					false	}
};

const struct flag_type def_flags[] =
{
    {	"(bug)",			0,					false	},
    {	"dodge",			DF_DODGE,			true	},
    {	"parry",			DF_PARRY,			true	},
    {	"sidestep",			DF_SIDESTEP,		true	},
    {	"counter",			DF_COUNTER,			true	},
    {	"riposte",			DF_RIPOSTE,			true	},
    {	"trompement",		DF_TROMPEMENT,		true	},
    {	"dualshield",		DF_DUALSHIELD,		true	},
    {	"shieldblock",		DF_SHIELDBLOCK,		true	},
    {	"feint",			DF_FEINT,			true	},
    {	"evade",			DF_EVADE,			true	},
    {	"duck",				DF_DUCK,			true	},
    {	"counterspin",		DF_COUNTERSPIN,		true	},
    {	"absorb",			DF_ABSORB,			true	},
    {	"coule",			DF_COULE,			true	},
    {	"divert",			DF_DIVERT,			true	},
    {	"rebound",			DF_REBOUND,			true	},
    {	"ricochet",			DF_RICOCHET,		true	},
    {	"prepattack",		DF_PREPATTACK,		true	},
    {	"shunt",			DF_SHUNT,			true	},
    {	"truebelief",		DF_TRUEBELIEF,		true	},
    {	NULL,				0,					false	}
};

const struct flag_type form_flags[] =
{
    {	"(bug)",			0,					false	},
    {	"edible",			FRM_EDIBLE,			true	},
    {	"poison",			FRM_POISON,			true	},
    {	"magical",			FRM_MAGICAL,		true	},
    {	"instant_decay",	FRM_INSTANT_DECAY,	true	},
    {	"other",			FRM_OTHER,			true	},
    {	"silent",			FRM_SILENT,			true	},
    {	"nullb",			FRM_NULLB,			false	},
    {	"animal",			FRM_ANIMAL,			true	},
    {	"sentient",			FRM_SENTIENT,		true	},
    {	"undead",			FRM_UNDEAD,			true	},
    {	"construct",		FRM_CONSTRUCT,		true	},
    {	"mist",				FRM_MIST,			true	},
    {	"intangible",		FRM_INTANGIBLE,		true	},
    {	"biped",			FRM_BIPED,			true	},
    {	"centaur",			FRM_CENTAUR,		true	},
    {	"insect",			FRM_INSECT,			true	},
    {	"spider",			FRM_SPIDER,			true	},
    {	"crustacean",		FRM_CRUSTACEAN,		true	},
    {	"worm",				FRM_WORM,			true	},
    {	"blob",				FRM_BLOB,			true	},
    {	"nullc",			FRM_NULLC,			false	},
    {	"nulld",			FRM_NULLD,			false	},
    {	"mammal",			FRM_MAMMAL,			true	},
    {	"bird",				FRM_BIRD,			true	},
    {	"reptile",			FRM_REPTILE,		true	},
    {	"snake",			FRM_SNAKE,			true	},
    {	"dragon",			FRM_DRAGON,			true	},
    {	"amphibian",		FRM_AMPHIBIAN,		true	},
    {	"fish",				FRM_FISH,			true	},
    {	"cold_blood",		FRM_COLD_BLOOD,		true	},
    {	NULL,				0,					false	}
};

const struct flag_type part_flags[] =
{
    {	"(bug)",			0,					false	},
    {	"head",				PRT_HEAD,			true	},
    {	"arms",				PRT_ARMS,			true	},
    {	"legs",				PRT_LEGS,			true	},
    {	"heart",			PRT_HEART,			true	},
    {	"brains",			PRT_BRAINS,			true	},
    {	"guts",				PRT_GUTS,			true	},
    {	"hands",			PRT_HANDS,			true	},
    {	"feet",				PRT_FEET,			true	},
    {	"fingers",			PRT_FINGERS,		true	},
    {	"ear",				PRT_EAR,			true	},
    {	"eye",				PRT_EYE,			true	},
    {	"long_tongue",		PRT_LONG_TONGUE,	true	},
    {	"eyestalks",		PRT_EYESTALKS,		true	},
    {	"tentacles",		PRT_TENTACLES,		true	},
    {	"fins",				PRT_FINS,			true	},
    {	"wings",			PRT_WINGS,			true	},
    {	"tail",				PRT_TAIL,			true	},
    {	"nulla",			PRT_NULLA,			false	},
    {	"nullb",			PRT_NULLB,			false	},
    {	"nullc",			PRT_NULLC,			false	},
    {	"claws",			PRT_CLAWS,			true	},
    {	"fangs",			PRT_FANGS,			true	},
    {	"horns",			PRT_HORNS,			true	},
    {	"scales",			PRT_SCALES,			true	},
    {	"tusks",			PRT_TUSKS,			true	},
    {	NULL,				0,					false	}
};

const struct flag_type comm_flags[] =
{
    {	"(bug)",			0,					false	},
    {	"quiet",			CM_QUIET,			true	},
    {   "deaf",				CM_DEAF,			true	},
    {   "nowiz",			CM_NOWIZ,			true	},
    {   "noauction",		CM_NOAUCTION,		true	},
    {   "nogossip",			CM_NOGOSSIP,		true	},
    {   "noquestion",		CM_NOQUESTION,		true	},
    {   "nochat",			CM_NOCHAT,			true	},
    {   "nohouse",			CM_NOHOUSE,			true	},
    {   "noquote",			CM_NOQUOTE,			true	},
    {   "shoutsoff",		CM_SHOUTSOFF,		true	},
	{	"busy",				CM_BUSY,			true	},
    {   "compact",			CM_COMPACT,			true	},
    {   "brief",			CM_BRIEF,			true	},
    {   "prompt",			CM_PROMPT,			true	},
    {   "combine",			CM_COMBINE,			true	},
    {   "telnet_ga",		CM_TELNET_GA,		true	},
    {   "show_affects",		CM_SHOW_AFFECTS,	true	},
    {   "nonewbhelp",			CM_NONEWBIE,		true	},
    {   "stupid",			CM_STUPID,			true	},
    {   "noemote",			CM_NOEMOTE,			false	},
    {   "noshout",			CM_NOSHOUT,			false	},
    {   "notell",			CM_NOTELL,			false	},
    {   "nochannels",		CM_NOCHANNELS,		false	},
    {   "snoop_proof",		CM_SNOOP_PROOF,		false	},
    {   "afk",				CM_AFK,				true	},
    {   "debugset",			CM_DEBUG,			false	},
    {   "no-ooc",			CM_NOOOC,			false	},
    {   "rom_exits",		CM_ROMEXITS,		false	},
    {   "rom_look",			CM_ROMLOOK,			false	},
    {   "morphed",			CM_MORPH,			false	},
    {   "nonotes",			CM_NONOTES,			false	},
    {   "lifeline",			CM_LIFELINE,		false	},
    {	NULL,				0,					false	}
};

const struct flag_type mprog_flags[] =
{
    {	"act",				TRIG_ACT,			true	},
    {	"bribe",			TRIG_BRIBE,			true 	},
    {	"death",			TRIG_DEATH,			true    },
    {	"entry",			TRIG_ENTRY,			true	},
    {	"fight",			TRIG_FIGHT,			true	},
    {	"give",				TRIG_GIVE,			true	},
    {	"greet",			TRIG_GREET,			true    },
    {	"grall",			TRIG_GRALL,			true	},
    {	"kill",				TRIG_KILL,			true	},
    {	"hpcnt",			TRIG_HPCNT,			true    },
    {	"random",			TRIG_RANDOM,		true	},
    {	"speech",			TRIG_SPEECH,		true	},
    {	"exit",				TRIG_EXIT,			true    },
    {	"exall",			TRIG_EXALL,			true    },
    {	"delay",			TRIG_DELAY,			true    },
    {	"surr",				TRIG_SURR,			true    },
    {	"interp",			TRIG_INTERP,		true    },
    {	NULL,				0,					false	}
};

const struct material_type material_flags[] ={
	//   name				ismetal		isforge		ismill
    {	"unknown",			false,		false,		false	},
    {	"paper",			false,		false,		false	},
    {	"birch",			false,		false,		true	},
    {	"bone",				false,		false,		true	},
    {	"cotton",			false,		false,		false	},
    {	"food",				false,		false,		false	},
    {	"flesh",			false,		false,		false	},
    {	"granite",			false,		false,		true	},
    {	"leather",			false,		false,		false	},
    {	"rock",				false,		false,		true	},
    {	"scales",			false,		true,		true	},
    {	"shell",			false,		true,		true	},
    {	"skin",				false,		false,		true	},
    {	"slate",			false,		false,		true	},
    {	"stone",			false,		false,		true	},
    {	"wood",				false,		false,		true	},
    {	"wool",				false,		false,		false	},
    {	"velvet",			false,		false,		false	},
    {	"silk",				false,		false,		false	},
    {	"plush",			false,		false,		false	},
    {	"plaid",			false,		false,		false	},
    {	"flannel",			false,		false,		false	},
    {	"oilcloth",			false,		false,		false	},
    {	"hemp",				false,		false,		false	},
    {	"jute",				false,		false,		false	},
    {	"fur",				false,		false,		false	},
    {	"felt",				false,		false,		false	},
    {	"fleece",			false,		false,		false	},
    {	"cloth",			false,		false,		false	},
    {	"linen",			false,		false,		false	},
    {	"horn",				false,		true,		true	},
    {	"cord",				false,		false,		false	},
    {	"mahogany",			false,		true,		true	},
    {	"walnut",			false,		true,		true	},
    {	"oak",				false,		true,		true	},
    {	"maple",			false,		true,		true	},
    {	"cherrywood",		false,		true,		true	},
    {	"rosewood",			false,		true,		true	},
    {	"alabaster",		false,		true,		true	},
    {	"amethyst",			false,		true,		true	},
    {	"balas",			false,		true,		true	},
    {	"basalt",			false,		true,		true	},
    {	"boxwood",			false,		true,		true	},
    {	"canvas",			false,		false,		false	},
    {	"cedar",			false,		true,		true	},
    {	"coal",				false,		true,		true	},
    {	"emerald",			false,		true,		true	},
    {	"feather",			false,		false,		false	},
    {	"fir",				false,		false,		false	},
    {	"glass",			false,		true,		true	},
    {	"granite",			false,		true,		true	},
    {	"hair",				false,		false,		false	},
    {	"hemlock",			false,		true,		true	},
    {	"ivory",			false,		true,		true	},
    {	"jade",				false,		true,		true	},
    {	"jasper",			false,		true,		true	},
    {	"limestone",		false,		true,		true	},
    {	"marble",			false,		true,		true	},
    {	"ooka",				false,		false,		false	},
    {	"parchment",		false,		false,		false},
    {	"pine",				false,		true,		true},
    {	"redwood",			false,		true,		true},
    {	"sandstone",		false,		true,		true},
    {	"slate",			false,		true,		true},
    {	"soapstone",		false,		true,		true},
    {	"spruce",			false,		true,		true},
    {	"straw",			false,		false,		false},
    {	"suede",			false,		false,		false},
    {	"sulfur",			false,		true,		true},
    {	"teak",				false,		true,		true},
    {	"turquiose",		false,		true,		true},
    {	"porcelin",			false,		true,		true},
    {	"skin",				false,		false,		false},
    {	"crystal",			false,		true,		true},
    {	"agate",			false,		true,		true},
    {	"lapis",			false,		true,		true},
    {	"driftwood",		false,		true,		true},
    {	"amber",			false,		true,		true},
    {	"bloodstone",		false,		true,		true},
    {	"garnet",			false,		true,		true},
    {	"moonstone",		false,		true,		true},
    {	"ruby",				false,		true,		true},
    {	"sapphire",			false,		true,		true},
    {	"topaz",			false,		true,		true},
    {	"opal",				false,		true,		true},
    {	"pearl",			false,		true,		true},
    {	"wax",				false,		true,		false},
    {	"diamond",			false,		true,		true},
    {	"flax",				false,		false,		false},
    {	"ironwood",			false,		true,		true},
    {	"sailcloth",		false,		false,		false},
    {	"wicker",			false,		false,		false},
    {	"twead",			false,		false,		false},
    {	"tile",				false,		false,		true},
    {	"brick",			false,		false,		true},
    {	"ricepaper",		false,		false,		false},
    {	"vellum",			false,		false,		false},
    {	"string",			false,		false,		false},
    {	"plant",			false,		false,		true},
    {	"liquid",			false,		false,		false},
    {	"lacquer",			false,		false,		false},
    {	"ceramic",			false,		false,		true},
    {	"reed",				false,		false,		false},
    {	"ink",				false,		false,		false},
    {	"paint",			false,		false,		false},
    {	"coral",			false,		true,		true},
    {	"gauze",			false,		false,		false},
    {	"gossamer",			false,		false,		false},
    {	"flint",			false,		true,		true},
    {	"quartz",			false,		true,		true},
    {	"onyx",				false,		true,		true},
    {	"fabric",			false,		false,		false},
    {	"willow",			false,		false,		false},
    {	"adamantine",		true,		true,		false},
    {	"brass",			true,		true,		false},
    {	"bronze",			true,		true,		false},
    {	"chain",			true,		true,		false},
    {	"copper",			true,		true,		true},
    {	"gold",				true,		true,		true},
    {	"iron",				true,		true,		true},
    {	"mercury",			true,		true,		false},
    {	"mithril",			true,		true,		false},
    {	"orichalcum",		true,		true,		false},
    {	"platinum",			true,		true,		true},
    {	"silver",			true,		true,		true},
    {	"stainless steel",	true,		true,		false},
    {	"steel",			true,		true,		false},
    {	"tin",				true,		true,		true},
    {	"wire",				true,		true,		false},
    {	"pewter",			true,		true,		false},
    {	"zinc",				true,		true,		true},
    {	"nickel",			true,		true,		true},
    {	"lead",				true,		true,		true},
    {	NULL,				false,		false,		false}
};

const struct flag_type area_flags[] =
{
    {	"none",					AREA_NONE,			false	},
    {	"changed",				AREA_CHANGED,		true	},
    {	"added",				AREA_ADDED,			true	},
    {	"loading",				AREA_LOADING,		false	},
    {	NULL,					0,					false	}
};

const struct flag_type sex_flags[] =
{
    {	"male",					SEX_MALE,			true	},
    {	"female",				SEX_FEMALE,			true	},
    {	"neutral",				SEX_NEUTRAL,		true	},
    {   "random",				3,			        true    },
    {	"none",					SEX_NEUTRAL,		true	},
    {	NULL,					0,					false	}
};

const struct flag_type exit_flags[] =
{
    {   "door",					EX_ISDOOR,			true    },
    {   "hidden",				EX_HIDDEN,			true    },
    {   "secretdoor",			EX_SDOOR,			true    },
    {   "superinvis",			EX_SUPERINVIS,		true    },
    {	"closed",				EX_CLOSED,			true	},
    {	"locked",				EX_LOCKED,			true	},
    {	"pickproof",			EX_PICKPROOF,		true	},
    {   "nopass",				EX_NOPASS,			true	},
    {   "easy",					EX_EASY,			true	},
    {   "hard",					EX_HARD,			true	},
    {	"infuriating",			EX_INFURIATING,		true	},
    {	"clifftop",				EX_CLIFFTOP,		true	},
    {	"noclose",				EX_NOCLOSE,			true	},
    {	"nolock",				EX_NOLOCK,			true	},
    {	"oneway",				EX_ONEWAY,			false	},
    {	"quiet",				EX_QUIET,			true	},
    {	"random",				EX_RANDOM,			true	},
    {	NULL,					0,					false	}
};

const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,					true	},
    {	"closed and unlocked",	1,					true	},
    {	"closed and locked",	2,					true	},
    {	NULL,					0,					false	}
};

const struct flag_type room_flags[] =
{
    {	"dark",					ROOM_DARK,			true	},
    {	"no_mob",				ROOM_NO_MOB,		true	},
    {	"indoors",				ROOM_INDOORS,		true	},
    {	"bank",					ROOM_BANK,			true	},
    {	"private",				ROOM_PRIVATE,		true    },
    {	"safe",					ROOM_SAFE,			true	},
    {	"solitary",				ROOM_SOLITARY,		true	},
    {	"pet_shop",				ROOM_PET_SHOP,		true	},
    {	"arena-foyer",			ROOM_ARENA_FOYER,	true	},
    {	"arena",				ROOM_ARENA,			true	},
    {	"nomap",				ROOM_NOMAP,			true	},
    {	"mount_shop",			ROOM_MOUNT_SHOP,	true	},
    {	"no_teleport",			ROOM_NO_RECALL,		true	},
    {	"imp_only",				ROOM_IMP_ONLY,		true    },
    {	"gods_only",			ROOM_GODS_ONLY,		true    },
    {	"heroes_only",			ROOM_HEROES_ONLY,	true	},
    {	"newbies_only",			ROOM_NEWBIES_ONLY,	true	},
    {	"law",					ROOM_LAW,			true	},
    {   "nowhere",				ROOM_NOWHERE,		true	},
    {	"light",				ROOM_LIGHT,			true	},
    {	NULL,					0,					false	}
};

const struct flag_type sector_flags[] =
{
    {	"inside",				SECT_INSIDE,		true	},
    {	"city",					SECT_CITY,			true	},
    {	"field",				SECT_FIELD,			true	},
    {	"forest",				SECT_FOREST,		true	},
    {	"hills",				SECT_HILLS,			true	},
    {	"mountain",				SECT_MOUNTAIN,		true	},
    {	"road",					SECT_ROAD,			true	},
    {	"dirtroad",				SECT_DIRTROAD,		true	},
    {	"dirtpath",				SECT_DIRTPATH,		true	},
    {	"swim",					SECT_WATER_SWIM,	true	},
    {	"noswim",				SECT_WATER_NOSWIM,	true	},
    {   "underwater",			SECT_UNDERWATER,	true	},
    {	"air",					SECT_AIR,			true	},
    {	"desert",				SECT_DESERT,		true	},
    {	"volcano",				SECT_VOLCANO,		true	},
    {	"swamp",				SECT_SWAMP,			true	},
    {	"river",				SECT_RIVER,			true	},
    {	"tunnel",				SECT_TUNNEL,		true	},
    {	"rainforest",			SECT_RAINFOREST,	true	},
    {	"forestcity",			SECT_FORESTCITY,	true	},
    {	"desertcity",			SECT_DESERTCITY,	true	},
    {	"mountaincity",			SECT_MOUNTAINCITY,	true	},
    {	"cityroad",				SECT_CITYROAD,		true	},
    {	"graveyard",			SECT_GRAVEYARD,		true	},
    {	"goodtemple",			SECT_GOODTEMPLE,	true	},
    {	"neutraltemple",		SECT_NEUTRALTEMPLE,	true	},
    {	"eviltemple",			SECT_EVILTEMPLE,	true	},
    {	"cliffside",			SECT_CLIFFSIDE,		true	},
    {	"icetunnel",			SECT_ICETUNNEL,		true	},
    {	"boardwalk",			SECT_BOARDWALK,		true	},
    {	"beach",				SECT_BEACH,			true	},
    {	"sewer",				SECT_SEWER,			true	},
    {	NULL,					0,					false	}
};

const struct flag_type type_flags[] =
{
    {	"light",				ITEM_LIGHT,			true	},
    {	"scroll",				ITEM_SCROLL,		true	},
    {	"wand",					ITEM_WAND,			true	},
    {	"staff",				ITEM_STAFF,			true	},
    {	"weapon",				ITEM_WEAPON,		true	},
    {	"coupon",				ITEM_COUPON,		false	},
    {	"armor",				ITEM_ARMOR,			true	},
    {	"potion",				ITEM_POTION,		true	},
    {	"furniture",			ITEM_FURNITURE,		true	},
    {	"trapparts",			ITEM_TRAPPARTS,		true	},
    {	"caltrops",				ITEM_CALTROPS,		true	},
    {	"trash",				ITEM_TRASH,			true	},
    {	"ribs",					ITEM_RIBS,			true	},
    {	"container",			ITEM_CONTAINER,		true	},
    {	"drinkcontainer",		ITEM_DRINK_CON,		true	},
    {	"key",					ITEM_KEY,			true	},
    {	"food",					ITEM_FOOD,			true	},
    {	"money",				ITEM_MONEY,			true	},
    {	"boat",					ITEM_BOAT,			true	},
    {	"fountain",				ITEM_FOUNTAIN,		true	},
    {	"lockpick",				ITEM_LOCKPICK,		true	},
    {	"throwing-knife",		ITEM_THROWINGKNIFE,	true	},
    {	"throwing-axe",			ITEM_THROWINGAXE,	true	},
    {	"throwing-dart",		ITEM_THROWINGDART,	true	},
    {	"forge",				ITEM_FORGE,			true	},
    {	"fuelforge",			ITEM_FORGEFUEL,		true	},
    {	"parchment",			ITEM_PARCHMENT,		true	},
    {	"pill",					ITEM_PILL,			true	},
    {   "portal",				ITEM_PORTAL,		true	},
    {   "warpstone",			ITEM_WARP_STONE,	true	},
    {	"roomkey",				ITEM_ROOM_KEY,		true	},
    { 	"gem",					ITEM_GEM,			true	},
    {	"jewelry",				ITEM_JEWELRY,		true	},
	{	"quiver",				ITEM_QUIVER,		true	},
	{	"arrow",				ITEM_ARROW,			true	},
    {	"shield",				ITEM_SHIELD,		true	},
    {	"honingstone",			ITEM_HONINGSTONE,	true	},
    {	"needle",				ITEM_NEEDLE,		true	},
    {	"bones",				ITEM_BONES,			false	},
    {	"fire",					ITEM_FIRE,			false	},
    {	"ingot",				ITEM_INGOT,			false	},
    {	"lumber",				ITEM_LUMBER,		false	},
    {	"npccorpse",			ITEM_CORPSE_NPC,	false	},
    {	"pc corpse",			ITEM_CORPSE_PC,		false	},
    {	"scars",				ITEM_SCARS,			false	},
    {	"trap",					ITEM_TRAP,			false	},
    {	"dark",					ITEM_DARK,			false	},
    {	"herb",					ITEM_HERB,			false	},
    {	"locker",				ITEM_LOCKER,		true	},
    {	NULL,					0,					false	}
};

const struct flag_type pick_table[] =
{
    {	NULL,					0,					true	},
    {	"easy",					1,					true	},
    {	"normal",				2,					true	},
    {	"hard",					3,					true	},
    {	"infuriating",			4,					true	},
    {	NULL,					0,					false	}
};

const struct flag_type extra_flags[] =
{
    {	"nothing",				0,					false	},
    {	"glow",					ITM_GLOW,			true	},
    {	"hum",					ITM_HUM,			true	},
    {	"dark",					ITM_DARK,			true	},
    {	"lock",					ITM_LOCK,			true	},
    {	"evil",					ITM_EVIL,			true	},
    {	"invis",				ITM_INVIS,			true	},
    {	"magic",				ITM_MAGIC,			true	},
    {	"nodrop",				ITM_NODROP,			true	},
    {	"gripped",				ITM_GRIPPED,		false	},
    {	"barbed",				ITM_BARBED,			false	},
    {	"forging",				ITM_FORGING,		false	},
    {	"antineutral",			ITM_ANTI_NEUTRAL,	true	},
    {	"noremove",				ITM_NOREMOVE,		true	},
    {	"inventory",			ITM_INVENTORY,		true	},
    {	"nopurge",				ITM_NOPURGE,		true	},
    {	"rotdeath",				ITM_ROT_DEATH,		true	},
    {	"visdeath",				ITM_VIS_DEATH,		true	},
	{	"lodged",				ITM_LODGED,			true	},
	{	"embalmed",				ITM_EMBALM,			false	},
	{	"nolocate",				ITM_NOLOCATE,		true	},
    {	"meltdrop",				ITM_MELT_DROP,		true	},
    {	"hadtimer",				ITM_HAD_TIMER,		true	},
    {	"sellextract",			ITM_SELL_EXTRACT,	true	},
    {	"grasped",				ITM_GRASPED,		false	},
    {	"burnproof",			ITM_BURN_PROOF,		true	},
    {	"nouncurse",			ITM_NOUNCURSE,		true	},
    {	"hidden",				ITM_HIDDEN,			true	},
    {	"flaming",				ITM_FLAMING,		true	},
    {	"frosted",				ITM_FROSTED,		true	},
    {	"shocking",				ITM_SHOCKED,		true	},
    {	"shockproof",			ITM_SHOCK_PROOF,	true	},
    {	"inlayed",				ITM_INLAY,			true	},
    {	NULL,					0,					false	}
};


const struct flag_type exclude_flags[] =
{
	{"fighter",EXL_FIGHTER,true },
	{"mystic",EXL_MYSTIC,true },
	{"sorcerer",EXL_SORCERER,true },
	{"archer",EXL_ARCHER,true },
	{"assassin",EXL_ASSASSIN,true },
	{"bandit",EXL_BANDIT,true },
	{"berserker",EXL_BERSERKER,true },
	{"brawler",EXL_BRAWLER,true },
	{"cavalier",EXL_CAVALIER,true },
	{"defender",EXL_DEFENDER,true },
	{"knight",EXL_KNIGHT,true },
	{"martialartist",EXL_MARTIALARTIST,true },
	{"nomad",EXL_NOMAD,true },
	{"ranger",EXL_RANGER,true },
	{"rogue",EXL_ROGUE,true },
	{"swashbuckler",EXL_SWASHBUCKLER,true },
	{"swordfighter",EXL_SWORDFIGHTER,true },
	{"thief",EXL_THIEF,true },
	{"warrior",EXL_WARRIOR,true },
	{"alchemist",EXL_ALCHEMIST,true },
	{"bard",EXL_BARD,true },
	{"cleric",EXL_CLERIC,true },
	{"druid",EXL_DRUID,true },
	{"necromancer",EXL_NECROMANCER,true },
	{"priest",EXL_PRIEST,true },
	{"shaman",EXL_SHAMAN,true },
	{"summoner",EXL_SUMMONER,true },
	{"illusionist",EXL_ILLUSIONIST,true },
	{"wizard",EXL_WIZARD,true },
	{"pyromancer",EXL_PYROMANCER,true },
	{"cryomancer",EXL_CRYOMANCER,true },
	{"geomancer",EXL_GEOMANCER,true },
	{"mage",EXL_MAGE,true },
	{"antimage",EXL_ANTIMAGE,true },
	{"conjurer",EXL_CONJURER,true },
	{"enchanter",EXL_ENCHANTER,true },
	{"aeromancer",EXL_AEROMANCER,true },
	{"hydromancer",EXL_HYDROMANCER,true },
	{"elementalist",EXL_ELEMENTALIST,true },
	{"human",EXL_HUMAN,true },
	{"algid",EXL_ALGID,true },
	{"daurin",EXL_DAURIN,true },
	{"quolin",EXL_QUOLIN,true },
	{"vithe",EXL_VITHE,true },
	{"gurha",EXL_GURHA,true },
	{"dewdop",EXL_DEWDOP,true },
	{"dwarf",EXL_DWARF,true },
	{	NULL,					0,						false	}
};

const struct flag_type armortype_flags[] =
{
    {	"Not Armor",		0,						false	},
    {	"clothing",			1,						true	},
    {	"light_armor",		2,						true	},
    {	"medium_armor",		3,						true	},
    {	"heavy_armor",		4,						true	},
    {	NULL,				0,						false	}
};

const struct flag_type wear_flags[] =
{
    {	"take",				ITEM_TAKE,				true	},
    {	"finger",			ITEM_WEAR_FINGER,		true	},
    {	"ear",				ITEM_WEAR_EAR,			true	},
    {	"neck",				ITEM_WEAR_NECK,			true	},
    {	"elbow",			ITEM_WEAR_ELBOW,		true	},
    {	"torso",			ITEM_WEAR_TORSO,			true	},
    {	"shin",				ITEM_WEAR_SHIN,			true	},
    {	"knee",				ITEM_WEAR_KNEE,			true	},
    {	"head",				ITEM_WEAR_HEAD,			true	},
    {	"legs",				ITEM_WEAR_LEGS,			true	},
    {	"shoulder",			ITEM_WEAR_SHOULDER,		true	},
    {	"quiver",			ITEM_WEAR_QUIVER,		true	},
    {	"feet",				ITEM_WEAR_FEET,			true	},
    {	"hands",			ITEM_WEAR_HANDS,		true	},
    {	"face",				ITEM_WEAR_FACE,			true	},
    {	"pendant",			ITEM_WEAR_PENDANT,		true	},
    {	"tattoo",			ITEM_WEAR_TATTOO,		true	},
    {	"arms",				ITEM_WEAR_ARMS,			true	},
    {	"shield",			ITEM_WEAR_SHIELD,		true	},
    {	"about",			ITEM_WEAR_ABOUT,		true	},
    {	"waist",			ITEM_WEAR_WAIST,		true	},
    {	"wrist",			ITEM_WEAR_WRIST,		true	},
    {	"ankle",			ITEM_WEAR_ANKLE,		true	},
    {	"wield",			ITEM_WIELD,				true	},
    {	"hold",				ITEM_HOLD,				true	},
    {   "nosac",			ITEM_NO_SAC,			true	},
    {	"wearfloat",		ITEM_WEAR_FLOAT,		true	},
    {	"floatlight",		ITEM_WEAR_FLOAT_LIGHT,	false	},
//    {	"twohands",			ITEM_TWO_HANDS,			true    },
    {	NULL,				0,						false	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",				APPLY_NONE,				true	},
    {	"sex",				APPLY_SEX,				true	},
    {	"class",			APPLY_CLASS,			false	},
    {	"level",			APPLY_LEVEL,			false	},
    {	"age",				APPLY_AGE,				false	},
    {	"height",			APPLY_HEIGHT,			false	},
    {	"weight",			APPLY_WEIGHT,			false	},
    {	"mana",				APPLY_MANA,				true	},
    {	"hp",				APPLY_HIT,				true	},
    {	"move",				APPLY_MOVE,				true	},
    {	"gold",				APPLY_GOLD,				false	},
    {	"experience",		APPLY_EXP,				false	},
    {	"armor",			APPLY_AC,				true	},
    {	"hitroll",			APPLY_HITROLL,			true	},
    {	"damroll",			APPLY_DAMROLL,			true	},
    {	"saves",			APPLY_SAVES,			true	},
    {	"savingspell",		APPLY_SAVING_SPELL,		true	},
    {	"spellaffect",		APPLY_SPELL_AFFECT,		false	},
    {	"strength",			APPLY_STR,				true	},
    {	"endurance",		APPLY_END,				true	},
    {	"agility",			APPLY_AGI,				true	},
    {	"intelligence",		APPLY_INT,				true	},
    {	"resistance",		APPLY_RES,				true	},
    {	"faith",			APPLY_FTH,				true	},
    {	"wisdom",			APPLY_WIS,				true	},
    {	"charisma",			APPLY_CHA,				false	},
    {	"luck",				APPLY_LCK,				true	},
    {	NULL,				0,						false	}
};

const struct flag_type stat_flags[]={
    {	"strength",			STAT_STR,				true	},
    {	"endurance",		STAT_END,				true	},
    {	"agility",			STAT_AGI,				true	},
    {	"intelligence",		STAT_INT,				true	},
    {	"resistance",		STAT_RES,				true	},
    {	"faith",			STAT_FTH,				false	},
    {	"intelligence",		STAT_WIS,				true	},
    {	"charisma",			STAT_CHA,				true	},
    {	"luck",				STAT_LCK,				false	},
    {	NULL,				0,						false	}
};

const struct flag_type abbrev_stat_flags[]={//Nashwhytwo
    {	"str",		STAT_STR,				true	},
    {	"end",		STAT_END,				true	},
    {	"agi",		STAT_AGI,				true	},
    {	"int",		STAT_INT,				true	},
    {	"res",		STAT_RES,				true	},
    {	"fth",		STAT_FTH,				false	},
    {	"wis",		STAT_WIS,				true	},
    {	"cha",		STAT_CHA,				true	},
    {	"lck",		STAT_LCK,				false	},
    {	NULL,		0,						false	}
};

const struct flag_type stat_mini[] =
{
    {	"str",	STAT_STR,				true	},
    {	"end",	STAT_END,				true	},
    {	"agi",	STAT_AGI,				true	},
    {	"int",	STAT_INT,				true	},
    {	"res",	STAT_RES,				true	},
    {	"fth",	STAT_FTH,				true	},
    {	"wis",	STAT_WIS,				true	},
    {	"cha",	STAT_CHA,				true	},
    {	"lck",	STAT_LCK,				true	},
    {	NULL,				0,			false	}
};


/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",			WEAR_NONE,			true	},
    {	"as a light",				WEAR_LIGHT,			true	},
    {	"on the left finger",		WEAR_FINGER_L,		true	},
    {	"on the right finger",		WEAR_FINGER_R,		true	},
    {	"on the left ear",			WEAR_EAR_L,			true	},
    {	"on the right ear",			WEAR_EAR_R,			true	},
    {	"on the left elbow",		WEAR_ELBOW_L,		true	},
    {	"on the right elbow",		WEAR_ELBOW_R,		true	},
    {	"on the left shin",			WEAR_SHIN_L,		true	},
    {	"on the right shin",		WEAR_SHIN_R,		true	},
    {	"on the left knee",			WEAR_KNEE_L,		true	},
    {	"on the right knee",		WEAR_KNEE_R,		true	},
    {	"around the neck (1)",		WEAR_NECK_1,		true	},
    {	"around the neck (2)",		WEAR_NECK_2,		true	},
    {	"on the body",				WEAR_TORSO,			true	},
    {	"over the head",			WEAR_HEAD,			true	},
    {	"on the legs",				WEAR_LEGS,			true	},
    {	"over the shoulder",		WEAR_SHOULDER,		true	},
    {	"worn as a quiver",			WEAR_QUIVER,		true	},
    {	"on the feet",				WEAR_FEET,			true	},
    {	"on the hands",				WEAR_HANDS,			true	},
    {	"on the face",				WEAR_FACE,			true	},
    {	"as a pendant",				WEAR_PENDANT,		true	},
    {	"as a tattoo",				WEAR_TATTOO,		true	},
    {	"on the arms",				WEAR_ARMS,			true	},
    {	"as a shield",				WEAR_SHIELD,		true	},
    {	"about the shoulders",		WEAR_ABOUT,			true	},
    {	"around the waist",			WEAR_WAIST,			true	},
    {	"on the left wrist",		WEAR_WRIST_L,		true	},
    {	"on the right wrist",		WEAR_WRIST_R,		true	},
    {	"on the left ankle",		WEAR_ANKLE_L,		true	},
    {	"on the right ankle",		WEAR_ANKLE_R,		true	},
    {	"wielded",					WEAR_WIELD,			true	},
    {	"dualwield",				WEAR_SECONDARY,		true	},
    {	"held in the left hand",	WEAR_HOLD_L,		true	},
    {	"held in the right hand",	WEAR_HOLD_R,		true	},
    {	"floating nearby",			WEAR_FLOAT,			true	},
    {	"floating nearby",			WEAR_FLOAT_LIGHT,	true	},
    {	NULL,						0,					0		}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",				WEAR_NONE,				true	},
    {	"light",			WEAR_LIGHT,				true	},
    {	"lfinger",			WEAR_FINGER_L,			true	},
    {	"rfinger",			WEAR_FINGER_R,			true	},
    {	"lear",				WEAR_EAR_L,				true	},
    {	"rear",				WEAR_EAR_R,				true	},
    {	"lelbow",			WEAR_ELBOW_L,			true	},
    {	"relbow",			WEAR_ELBOW_R,			true	},
    {	"lshin",			WEAR_SHIN_L,			true	},
    {	"rshin",			WEAR_SHIN_R,			true	},
    {	"lknee",			WEAR_KNEE_L,			true	},
    {	"rknee",			WEAR_KNEE_R,			true	},
    {	"neck1",			WEAR_NECK_1,			true	},
    {	"neck2",			WEAR_NECK_2,			true	},
    {	"torso",			WEAR_TORSO,				true	},
    {	"head",				WEAR_HEAD,				true	},
    {	"legs",				WEAR_LEGS,				true	},
    {	"shoulder",			WEAR_SHOULDER,			true	},
    {	"quiver",			WEAR_QUIVER,			true	},
    {	"feet",				WEAR_FEET,				true	},
    {	"hands",			WEAR_HANDS,				true	},
    {	"face",				WEAR_FACE,				true	},
    {	"pendant",			WEAR_PENDANT,			true	},
    {	"tattoo",			WEAR_TATTOO,			true	},
    {	"arms",				WEAR_ARMS,				true	},
    {	"shield",			WEAR_SHIELD,			true	},
    {	"about",			WEAR_ABOUT,				true	},
    {	"waist",			WEAR_WAIST,				true	},
    {	"lwrist",			WEAR_WRIST_L,			true	},
    {	"rwrist",			WEAR_WRIST_R,			true	},
    {	"lankle",			WEAR_ANKLE_L,			true	},
    {	"rankle",			WEAR_ANKLE_R,			true	},
    {	"wielded",			WEAR_WIELD,				true	},
    {	"dualwield",		WEAR_SECONDARY,			true	},
    {	"rhold",			WEAR_HOLD_R,			true	},
    {	"lhold",			WEAR_HOLD_L,			true	},
    {	"floating",			WEAR_FLOAT,				true	},
	{	"lodge_leg",		WEAR_LODGE_LEG,			true	},
	{	"lodge_arm",		WEAR_LODGE_ARM,			true	},
	{	"lodge_rib",		WEAR_LODGE_RIB,			true	},
	{	"floatlight",		WEAR_FLOAT_LIGHT,		true	},
    {	NULL,				0,						0		}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,						true	},
    {	"pickproof",		2,						true	},
    {	"closed",			4,						true	},
    {	"locked",			8,						true	},
    {	"puton",			16,						true	},
    {	NULL,				0,						0		}
};

const struct flag_type ac_type[] =
{
    {   "pierce",			AC_PIERCE,				true    },
    {   "bash",				AC_BASH,				true    },
    {   "slash",			AC_SLASH,				true    },
    {   "exotic",			AC_EXOTIC,				true    },
    {   NULL,				0,						0       }
};

const struct flag_type size_flags[] =
{
    {   "tiny",				SIZE_TINY,				true    },
    {   "small",			SIZE_SMALL,				true    },
    {   "medium",			SIZE_MEDIUM,			true    },
    {   "large",			SIZE_LARGE,				true    },
    {   "huge",				SIZE_HUGE,				true    },
    {   "giant",			SIZE_GIANT,				true    },
    {   NULL,				0,						0       },
};

const struct flag_type weapon_class[]={
    {   "exotic",			WEAPON_EXOTIC,		true    },
    {   "club",				WEAPON_CLUB,		true    },
    {   "longsword",		WEAPON_LONGSWORD,	true    },
    {   "bastardsword",		WEAPON_BASTARDSWORD,true    },
    {   "dagger",			WEAPON_DAGGER,		true    },
    {   "dirk",				WEAPON_DIRK,		true    },
    {   "spear",			WEAPON_SPEAR,		true    },
    {   "polearm",			WEAPON_POLEARM,		true    },
    {   "mace",				WEAPON_MACE,		true    },
    {   "hammer",			WEAPON_HAMMER,		true    },
    {   "warhammer",		WEAPON_WARHAMMER,	true    },
    {   "axe",				WEAPON_AXE,			true    },
    {   "battleaxe",		WEAPON_BATTLEAXE,	true    },
    {   "machete",			WEAPON_MACHETE,		true    },
    {   "scythe",			WEAPON_SCYTHE,		true    },
    {   "flail",			WEAPON_FLAIL,		true    },
    {   "whip",				WEAPON_WHIP,		true    },
    {   "rapier",			WEAPON_RAPIER,		true    },
    {   "katana",			WEAPON_KATANA,		true    },
    {   "scimitar",			WEAPON_SCIMITAR,	true    },
    {   "sai",				WEAPON_SAI,			true    },
    {   "tonfa",			WEAPON_TONFA,		true    },
    {   "gauntlet",			WEAPON_GAUNTLET,	true    },
    {   "nunchaku",			WEAPON_NUNCHAKU,	true    },
    {   "maingauche",		WEAPON_MAINGAUCHE,	true    },
    {   "gladius",			WEAPON_GLADIUS,		true    },
    {   "lance",			WEAPON_LANCE,		true    },
    {   "staff",			WEAPON_STAFF,		true    },
    {   "quarterstaff",		WEAPON_QUARTERSTAFF,true    },
    {   "longbow",			WEAPON_LONGBOW,		true    },
    {   "shortbow",			WEAPON_SHORTBOW,	true    },
    {   NULL,				0,						0       }
};

const struct flag_type trap_class[] =
{
    {   "target",			TRAP_TARGET,			true    },
    {   "exploding",		TRAP_EXPLODING,			true    },
    {   "snare",			TRAP_SNARE,				true    },
    {   "web",				TRAP_WEB,				true    },
    {   NULL,				0,						0       }
};

const struct flag_type arrow_flags[] =
{
    {   "normal",			ARR_NULL,				true    },
    {   "fire",				ARR_FIRE,				false    },
    {   "cold",				ARR_COLD,				false    },
    {   "shock",			ARR_SHOCK,				false    },
    {   "sharp",			ARR_SHARP,				true    },
    {   "poison",			ARR_POISON,				false    },
    {   "barb",				ARR_BARB,				false    },
    {   NULL,				0,						0       }
};

const struct flag_type weapon_type2[] =
{
    {   "vampiric",			WPN_VAMPIRIC,			true    },
    {   "sharp",			WPN_SHARP,				true    },
    {   "vorpal",			WPN_VORPAL,				true    },
    {   "twohands",			WPN_TWO_HANDS,			true    },
    {	"poison",			WPN_POISON,				true	},
    {   "serrated",			WPN_SERRATED,			true    },
    {   "powerdrain",		WPN_POWERDRAIN,			true    },
    {   "pestilence",		WPN_PESTILENCE,			true	},
    {   "readied",			WPN_READIED,			false   },
    {   "flaming",			WPN_FLAMING,			true    },
    {   "frost",			WPN_FROST,				true    },
    {	"shocking",			WPN_SHOCKING,			true    },
    {	"water",			WPN_WATER,				true    },
    {	"earth",			WPN_EARTH,				true    },
    {	"light",			WPN_LIGHT,				true    },
    {	"sound",			WPN_SOUND,				true    },
    {	"wind",				WPN_WIND,				true    },
    {	"magic",			WPN_MAGIC,				true    },
    {	"holy",				WPN_HOLY,				true    },
    {	"demonic",			WPN_DEMONIC,			true    },
    {	"neural",			WPN_NEURAL,				false    },
    {	"thorned",			WPN_THORNED,			false    },
    {   NULL,				0,						0       }
};

const struct flag_type trap_type[] =
{
    {   "flaming",			TRAPT_FLAMING,			true    },
    {   "frost",			TRAPT_FROST,			true    },
    {	"shocking",			TRAPT_SHOCKING,			true    },
    {	"poison",			TRAPT_POISON,			true	},
    {   "powerdrain",		TRAPT_POWERDRAIN,		true    },
    {   NULL,				0,						0       }
};

const struct flag_type res_flags[] =
{
    {	"(buggy)",			0,						false	},
    {	"summon",			RS_SUMMON,				true	},
    {   "charm",			RS_CHARM,				true    },
    {   "magic",			RS_MAGIC,				true    },
    {   "weapon",			RS_WEAPON,				true    },
    {   "bash",				RS_BASH,				true    },
    {   "pierce",			RS_PIERCE,				true    },
    {   "slash",			RS_SLASH,				true    },
    {   "fire",				RS_FIRE,				true    },
    {   "cold",				RS_COLD,				true    },
    {   "electricity",		RS_LIGHTNING,			true    },
    {   "acid",				RS_ACID,				true    },
    {   "poison",			RS_POISON,				true    },
    {   "negative",			RS_NEGATIVE,			true    },
    {   "holy",				RS_HOLY,				true    },
    {   "energy",			RS_ENERGY,				true    },
    {   "mental",			RS_MENTAL,				true    },
    {   "disease",			RS_DISEASE,				true    },
    {   "water",			RS_WATER,				true    },
    {   "light",			RS_LIGHT,				true    },
    {	"sound",			RS_SOUND,				true	},
    {	"traps",			RS_TRAPS,				true	},
    {	"earth",			RS_EARTH,				true	},
    {	"(blank2)",			RS_NULLC,				false	},
    {	"wood",				RS_WOOD,				true	},
    {	"plant",			RS_PLANT,				true	},
    {	"iron",				RS_IRON,				true	},
    {	"wind",				RS_WIND,				true	},
    {	"crystal",			RS_CRYSTAL,				true	},
    {	"harm",				RS_HARM,				true	},
    {	NULL,				0,						false	}
};

const struct flag_type dam_flags[] =
{
    {	"(buggy)",			0,						0	},
    {	"summon",			DAM_OTHER,				RS_SUMMON	},
    {   "charm",			DAM_CHARM,				RS_CHARM	},
    {   "magic",			DAM_ENERGY,				RS_ENERGY	},
    {   "weapon",			DAM_OTHER,				RS_WEAPON	},
    {   "bash",				DAM_BASH,				RS_BASH		},
    {   "pierce",			DAM_PIERCE,				RS_PIERCE	},
    {   "slash",			DAM_SLASH,				RS_SLASH	},
    {   "fire",				DAM_FIRE,				RS_FIRE		},
    {   "cold",				DAM_COLD,				RS_COLD		},
    {   "electricity",		DAM_LIGHTNING,			RS_LIGHTNING},
    {   "acid",				DAM_ACID,				RS_ACID		},
    {   "poison",			DAM_POISON,				RS_POISON	},
    {   "negative",			DAM_NEGATIVE,			RS_NEGATIVE	},
    {   "holy",				DAM_HOLY,				RS_HOLY		},
    {   "energy",			DAM_ENERGY,				RS_ENERGY	},
    {   "mental",			DAM_MENTAL,				RS_MENTAL	},
    {   "disease",			DAM_DISEASE,			RS_DISEASE	},
    {   "water",			DAM_WATER,				RS_WATER	},
    {   "light",			DAM_LIGHT,				RS_LIGHT	},
    {	"sound",			DAM_SOUND,				RS_SOUND	},
    {	"traps",			DAM_TRAPS,				RS_TRAPS	},
    {	"earth",			DAM_EARTH,				RS_EARTH	},
    {	"(blank2)",			DAM_OTHER,				RS_NULLC	},
    {	"wood",				DAM_WOOD,				RS_WOOD		},
    {	"plant",			DAM_PLANT,				RS_PLANT	},
    {	"iron",				DAM_IRON,				RS_IRON		},
    {	"wind",				DAM_WIND,				RS_WIND		},
    {	"crystal",			DAM_CRYSTAL,			RS_CRYSTAL	},
    {	"harm",				DAM_HARM,				RS_HARM	},
    {	NULL,				0,						false		}
};

const struct flag_type position_flags[] =
{
    {   "dead",				POS_DEAD,				false   },
    {   "mortal",			POS_MORTAL,				false   },
    {   "incap",			POS_INCAP,				false   },
    {   "stunned",			POS_STUNNED,			false   },
    {   "sleeping",			POS_SLEEPING,			true    },
    {   "resting",			POS_RESTING,			true    },
    {   "sitting",			POS_SITTING,			true    },
    {   "fighting",			POS_FIGHTING,			false   },
    {   "standing",			POS_STANDING,			true    },
    {   NULL,				0,						0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",		GATE_NORMAL_EXIT,		true	},
    {	"no_curse",			GATE_NOCURSE,			true	},
    {   "go_with",			GATE_GOWITH,			true	},
    {   "buggy",			GATE_BUGGY,				true	},
    {	"random",			GATE_RANDOM,			true	},
    {	"noenter",			GATE_NOENTER,			true	},
    {   NULL,				0,						0		}
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",			STAND_AT,				true	},
    {	"stand_on",			STAND_ON,				true	},
    {	"stand_in",			STAND_IN,				true	},
    {	"sit_at",			SIT_AT,					true	},
    {	"sit_on",			SIT_ON,					true	},
    {	"sit_in",			SIT_IN,					true	},
    {	"rest_at",			REST_AT,				true	},
    {	"rest_on",			REST_ON,				true	},
    {	"rest_in",			REST_IN,				true	},
    {	"sleep_at",			SLEEP_AT,				true	},
    {	"sleep_on",			SLEEP_ON,				true	},
    {	"sleep_in",			SLEEP_IN,				true	},
    {	"put_at",			PUT_AT,					true	},
    {	"put_on",			PUT_ON,					true	},
    {	"put_in",			PUT_IN,					true	},
    {	"put_inside",		PUT_INSIDE,				true	},
    {	"sit_under",		SIT_UNDER,				true	},
    {	"rest_under",		REST_UNDER,				true	},
    {	"sleep_under",		SLEEP_UNDER,			true	},
    {	NULL,				0,						false	}
};

const	struct	flag_type	apply_types	[]	=
{
	{	"affects",			TO_AFFECTS,				true	},
	{	"object",			TO_OBJECT,				true	},
	{	"immune",			TO_IMMUNE,				true	},
	{	"resist",			TO_RESIST,				true	},
	{	"vuln",				TO_VULN,				true	},
	{	"weapon",			TO_WEAPON,				true	},
	{	NULL,				0,						false	}
};

const struct flag_type oprog_flags[] =
{
    {	"act",				TRIG_ACT,				true	},
    {	"fight",			TRIG_FIGHT,				true	},
    {	"give",				TRIG_GIVE,				true	},
    {   "greet",			TRIG_GRALL,				true	},
    {	"random",			TRIG_RANDOM,			true	},
    {   "speech",			TRIG_SPEECH,			true	},
    {	"exall",			TRIG_EXALL,				true	},
    {	"delay",			TRIG_DELAY,				true	},
    {	"drop",				TRIG_DROP,				true	},
    {	"get",				TRIG_GET,				true	},
    {	"sit",				TRIG_SIT,				true	},
    {	"interp",			TRIG_INTERP,			true    },
    {	"put",				TRIG_PUT,				true    },
    {	NULL,				0,						false	},
};

const struct flag_type rprog_flags[] =
{
    {	"act",				TRIG_ACT,				true	},
    {	"fight",			TRIG_FIGHT,				true	},
    {	"drop",				TRIG_DROP,				true	},
    {	"greet",			TRIG_GRALL,				true	},
    {	"random",			TRIG_RANDOM,			true	},
    {	"speech",			TRIG_SPEECH,			true	},
    {	"exall",			TRIG_EXALL,				true	},
    {	"delay",			TRIG_DELAY,				true	},
    {	"interp",			TRIG_INTERP,			true    },
    {	NULL,				0,						false	},
};

/*const struct flag_type qprize_flags[]={
    {	NULL,				0,						false	},
};

const struct flag_type qinput_flags[]={
	{	"item_give"			QIN_ITEMGIVE,			true	),
	{	"money_give"		QIN_MONEYGIVE,			true	),
	{	"kill"				QIN_KILL,				true	),
	{	"speech"			QIN_ITEMGIVE,			true	),
	{	"item_get"			QIN_ITEMGIVE,			true	),
	{	"item_quaff"		QIN_ITEMGIVE,			true	),
	{	"item_brandish"		QIN_ITEMGIVE,			true	),
	{	"spell_cast"			QIN_ITEMGIVE,			true	),
	{	"item_eat"			QIN_ITEMGIVE,			true	),
	{	"item_drink"			QIN_ITEMGIVE,			true	),
	{	"item_open"			QIN_ITEMGIVE,			true	),
	{	"item_sit"			QIN_ITEMGIVE,			true	),
	{	"arrive"			QIN_ITEMGIVE,			true	),
	{	"item_give"			QIN_ITEMGIVE,			true	),
	{	"item_give"			QIN_ITEMGIVE,			true	),
    {	NULL,				0,						false	},
};*/

const	struct	bit_type	bitvector_type	[]	=
{
	{	affect_flags,	"affect"	},
	{	apply_flags,	"apply"		},
	{	res_flags,		"imm"		},
	{	res_flags,		"res"		},
	{	res_flags,		"vuln"		},
	{	weapon_type2,	"weapon"	},
	{	trap_type,		"trap"	}
};
const struct weight_type weight_table [] =
{
{"{Rb{Yu{Bg{Gg{Ce{Md{x",		0,						{0,0,0,0,0,0,0,0} },
{ "emaciated",					WEIGHT_EMACIATED,		{0,1,1,1,1,1,1,1} },
{ "skinny",						WEIGHT_SKINNY,			{0,1,1,1,1,0,0,0} },
{ "slim",						WEIGHT_SLIM,			{0,1,1,1,1,0,0,0} },
{ "petite",						WEIGHT_PETITE,			{0,1,0,1,1,1,0,0} },
{ "slender",					WEIGHT_SLENDER,			{0,1,1,1,1,0,0,0} },
{ "athletic",					WEIGHT_ATHLETIC,		{0,1,1,1,1,0,0,0} },
{ "average",					WEIGHT_AVERAGE,			{0,1,1,1,1,1,1,1} },
{ "muscular",					WEIGHT_MUSCULAR,		{0,1,0,0,1,1,1,1} },
{ "voluptuous",					WEIGHT_VOLUPTUOUS,		{0,1,1,1,1,1,0,0} },
{ "plump",						WEIGHT_PLUMP,			{0,1,0,0,0,1,1,1} },
{ "husky",						WEIGHT_HUSKY,			{0,1,0,0,0,1,1,1} },
{ "obese",						WEIGHT_OBESE,			{0,1,0,0,0,1,0,1} },
{ "flabby",						WEIGHT_FLABBY,			{0,1,0,0,0,1,0,1} },
{ "busty",						WEIGHT_BUSTY,			{0,0,0,0,0,0,0,0} },
{ NULL,							0,						{0,0,0,0,0,0,0,0} }
};

const struct weight_type height_table [] =
{
{"{Rb{Yu{Bg{Gg{Ce{Md{x",		0,						{0,0,0,0,0,0,0,0} },
{ "tiny",						HEIGHT_TINY,			{0,1,1,1,1,1,0,0} },
{ "short",						HEIGHT_SHORT,			{0,1,1,1,1,1,0,1} },
{ "average",					HEIGHT_AVERAGE,			{0,1,1,1,1,1,1,1} },
{ "tall",						HEIGHT_TALL,			{0,1,1,1,1,0,1,1} },
{ "giant",						HEIGHT_GIANT,			{0,1,1,1,1,0,1,1} },
{ NULL,							0,						{0,0,0,0,0,0,0,0} }
};

const struct weight_type eye_table [] =
{
/*00*/{"{Rb{Yu{Bg{Gg{Ce{Md{x",	0,						{0,0,0,0,0,0,0,0} },
/*01*/{ "blue",					EYE_BLUE,				{0,1,1,1,1,0,0,1} },
/*02*/{ "green",				EYE_GREEN,				{0,1,1,1,1,1,0,1} },
/*03*/{ "hazel",				EYE_HAZEL,				{0,1,0,0,0,0,0,1} },
/*04*/{ "yellow",				EYE_YELLOW,				{0,0,0,0,1,0,0,1} },
/*05*/{ "lavender",				EYE_LAVENDER,			{0,1,1,1,1,0,0,1} },
/*06*/{ "red",					EYE_RED,				{0,0,0,0,1,0,0,1} },
/*07*/{ "black",				EYE_BLACK,				{0,0,1,1,0,1,1,1} },
/*08*/{ "white",				EYE_WHITE,				{0,1,1,1,0,0,0,1} },
/*09*/{ "violet",				EYE_VIOLET,				{0,1,1,1,1,0,0,1} },
/*10*/{ "azure",				EYE_AZURE,				{0,1,1,1,1,0,0,0} },
/*11*/{ "emerald",				EYE_EMERALD,			{0,1,0,1,1,0,0,0} },
/*12*/{ "grey",					EYE_GREY,				{0,1,0,1,0,0,0,1} },
/*13*/{ "brown",				EYE_BROWN,				{0,1,0,1,0,1,0,1} },
/*14*/{ "cyan",					EYE_CYAN,				{0,0,0,1,1,0,0,0} },
/*15*/{ "amber",				EYE_AMBER,				{0,1,0,1,1,0,0,1} },
/*16*/{ "olive",				EYE_OLIVE,				{0,1,0,0,0,0,0,1} },
/*17*/{ "silver",				EYE_SILVER,				{0,0,1,1,0,0,0,1} },
/*18*/{ "gold",					EYE_GOLD,				{0,1,1,1,1,0,1,1} },
/*19*/{ "sapphire",				EYE_SAPPHIRE,			{0,0,0,1,1,0,0,0} },
/*20*/{ "amethyst",				EYE_AMETHYST,			{0,0,0,1,1,0,0,0} },
/*21*/{ "ruby",					EYE_RUBY,				{0,0,0,0,1,0,0,0} },
/*22*/{ "onyx",					EYE_ONYX,				{0,0,0,0,0,0,1,0} },
/*23*/{ "topaz",				EYE_TOPAZ,				{0,0,0,0,1,0,0,0} },
/*24*/{ "aqua",					EYE_AQUA,				{0,0,0,1,1,0,0,0} },
/*25*/{ "copper",				EYE_COPPER,				{0,0,0,0,0,0,0,0} },
/*26*/{ "turqoise",				EYE_TURQOISE,			{0,0,0,1,1,0,0,0} },
/*27*/{ "purple",				EYE_PURPLE,				{0,0,0,1,1,0,0,0} },

/*28*/{ NULL,					0,						{0,0,0,0,0,0,0,0} }
};

const struct weight_type hair_table [] ={
/*00*/{"{Rb{Yu{Bg{Gg{Ce{Md{x",	0,						{0,0,0,0,0,0,0,0} },
/*01*/{ "blonde",				HAIR_BLONDE,			{0,1,1,1,1,1,1,1} },
/*02*/{ "blue",					HAIR_BLUE,				{0,0,0,1,0,0,0,0} },
/*04*/{ "red",					HAIR_RED,				{0,1,0,0,1,1,0,0} },
/*05*/{ "gold",					HAIR_GOLD,				{0,0,0,0,1,0,0,0} },
/*06*/{ "brown",				HAIR_BROWN,				{0,1,0,0,1,0,1,0} },
/*07*/{ "white",				HAIR_WHITE,				{0,1,1,1,1,1,1,1} },
/*08*/{ "black",				HAIR_BLACK,				{0,1,1,0,1,0,1,1} },
/*09*/{ "auburn",				HAIR_AUBURN,			{0,1,0,0,1,1,1,0} },
/*10*/{ "green",				HAIR_GREEN,				{0,0,0,0,0,0,0,1} },
/*11*/{ "silver",				HAIR_SILVER,			{0,1,1,1,1,0,0,0} },
/*12*/{ "grey",					HAIR_GREY,				{0,1,1,1,1,1,1,1} },
/*13*/{ "salt and pepper",		HAIR_SALTPEPPER,		{0,1,0,0,1,1,0,0} },
/*14*/{ "strawberry blonde",	HAIR_STRAWBERRYBLONDE,	{0,1,0,0,1,1,0,0} },
	/*16*/{ "violet",				HAIR_VIOLET,			{0,0,0,0,0,0,0,0} },
/*17*/{ "speckled",				HAIR_SPECKLED,			{0,1,0,0,0,0,0,0} },
	/*18*/{ "emerald",				HAIR_EMERALD,			{0,0,0,0,0,0,0,0} },
	/*19*/{ "sapphire",				HAIR_SAPPHIRE,			{0,0,0,0,0,0,0,0} },
/*22*/{ "onyx",					HAIR_ONYX,				{0,0,0,1,0,0,0,0} },
/*24*/{ "aqua",					HAIR_AQUA,				{0,0,0,1,0,0,0,0} },
/*25*/{ "copper",				HAIR_COPPER,			{0,0,0,0,1,0,0,0} },
/*26*/{ "ruby",					HAIR_RUBY,				{0,0,0,0,0,0,0,0} },
/*27*/{ "turqoise",				HAIR_TURQOISE,			{0,0,0,1,0,0,0,0} },
	/*28*/{ "amber",				HAIR_AMBER,				{0,0,0,0,0,0,0,0} },
/*12*/{ NULL,					0,						{0,0,0,0,0,0,0,0} }
};

const struct weight_type sub_hair_table [] =
{
{"{Rb{Yu{Bg{Gg{Ce{Md{x",		0,						{0,0,0,0,0,0,0,0} },
{ "short",						S_HAIR_SHORT,			{0,1,1,1,1,1,1,1} },
{ "long",						S_HAIR_LONG,			{0,1,1,1,1,1,1,1} },
{ "spiky",						S_HAIR_SPIKY,			{0,1,1,1,1,1,1,1} },
{ "curly",						S_HAIR_CURLY,			{0,1,1,1,1,1,1,1} },
{ "frazzled",					S_HAIR_FRAZZLED,		{0,1,1,1,1,1,1,1} },
{ "greying",					S_HAIR_GREYING,			{0,1,1,1,1,1,1,1} },
{ "dyed",						S_HAIR_DYED,			{0,1,1,1,1,1,1,1} },
{ "braided",					S_HAIR_BRAIDED,			{0,1,1,1,1,1,1,1} },
{ "shaved",						S_HAIR_SHAVED,			{0,1,1,1,1,1,1,1} },
{ "thinning",					S_HAIR_THINNING,		{0,1,1,1,1,1,1,1} },
{ "luscious",					S_HAIR_LUSCIOUS,		{0,1,1,1,1,1,1,1} },
{ "wavy",						S_HAIR_WAVY,			{0,1,1,1,1,1,1,1} },
{ "shiny",						S_HAIR_SHINY,			{0,1,1,1,1,1,1,1} },
{ "bald",						S_HAIR_BALD,			{0,1,1,1,1,1,1,1} },
{ "straight",					S_HAIR_STRAIGHT,		{0,1,1,1,1,1,1,1} },
{ "unkempt",					S_HAIR_UNKEMPT,			{0,1,1,1,1,1,1,1} },
{ "light",						S_HAIR_LIGHT,			{0,1,1,1,1,1,1,1} },
{ NULL,							0,						{0,0,0,0,0,0,0,0} }
};

/*const	struct	hometown_type	hometown_table [MAX_HOMES] =
{//	{	"name",						guild							recall,	death, map,		donor	canuse	},	
	{	"Immortal",					"",								1,		1,		38,		1279,	0 },
	{	"The Eson Kingdom",			"The Eson Kingdom",				2,		2,		38,		1279,	0 },
	{	"The Kingdom of Skoth",		"The Kingdom of Skothgard",		2,		2,		38,		1279,	0 },
	{	"The Halnsbrock States",	"The Halnsbrock States",		2,		2,		38,		1279,	0 },
	{	"The Guild",				"The Guild",					2,		2,		38,		1279,	0 },
	{	"The Kingdom of Qual'Ama",	"The Kingdom of QualAma",		2,		2,		38,		1279,	0 },
	{	"The Daurin Federation",	"The Daurin Federation",		2,		2,		38,		1279,	0 },
	{	"Mirage",					"Mirage",						2,		2,		38,		1279,	0 },
	{	"The Dwarven Republic",		"The Dwarven Republic",			2,		2,		38,		1279,	0 },
	{	"The Verthogt Clan",		"The Verthogt Clan",			2,		2,		38,		1279,	0 },
	{	"The Mallok Tribe",			"The Mallok Tribe",				2,		2,		38,		1279,	0 },
	{	"The Algid Empire",			"The Algid Empire",				2,		2,		38,		1279,	0 },
	{	"The Church of Ignus",		"The Church of Ignus",			2,		2,		38,		1279,	0 },
	{	"Havengard",				"populace",						1102,	1300,	1100,	1279,	1 },
	{	"Skothgard",				"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Halnsbrock",				"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Qual'Ama",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Anelle",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Delfarro",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Elhedra",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Meridia",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Shessac",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Nazaya",					"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Tollensburg",				"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Mallok's Plateau",			"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Verthogt Down",			"populace",						1107,	1269,	1100,	1279,	1 },
	{	"Xou",						"populace",						1107,	1269,	1100,	1279,	1 },
	{	NULL,						NULL,							0,		0,		0,		0,		0 }
};*/

const struct full_class_type class_files[MAX_CLASS+1] ={
	{"immortal",	"immortal.don"},
	{"tenderfoot",	"0tenderfoot.don"},

	{"fighter",		"1fighter.don"},
	{"mystic",		"1mystic.don"},
	{"devout",		"1devout.don"},

	{"duelist",		"2duelist.don"},
	{"warrior",		"2warrior.don"},
	{"rogue",		"2rogue.don"},
	{"enchanter",	"2enchanter.don"},
	{"mage",		"2mage.don"},
	{"abjurer",		"2abjurer.don"},
	{"templar",		"2templar.don"},
	{"acolyte",		"2acolyte.don"},
	{"ranger",		"2ranger.don"},

	{"myrmidon",	"3myrmidon.don"},
	{"monk",		"3monk.don"},
	{"cavalier",	"3cavalier.don"},
	{"mercenary",	"3mercenary.don"},
	{"hunter",		"3hunter.don"},
	{"assassin",	"3assassin.don"},
	{"chemist",		"3chemist.don"},
	{"antimage",	"3antimage.don"},
	{"wizard",		"3wizard.don"},
	{"sorcerer",	"3sorcerer.don"},
	{"sage",		"3sage.don"},
	{"minstrel",	"3minstrel.don"},
	{"paladin",		"3paladin.don"},
	{"nightstalker","3nightstalker.don"},
	{"priest",		"3priest.don"},
	{"necromancer",	"3necromancer.don"},
	{"druid",		"3druid.don"},
	{"shaman",		"3shaman.don"},
	{NULL,			NULL},
};

/*const struct full_class_type guild_files[MAX_GUILD] ={
	{"immortal",		"immortal.don"},
	{"eson",			"eson.don"},
	{"skothgard",		"skothgard.don"},
	{"halnsbrock",		"halnsbrock.don"},
	{"guild",			"guild.don"},
	{"qualama",			"qualama.don"},
	{"federation",		"federation.don"},
	{"mirage",			"mirage.don"},
	{"republic",		"republic.don"},
	{"mallok",			"mallok.don"},
	{"verthogt",		"verthogt.don"},
	{"algid",			"algid.don"},
	{"church",			"church.don"},
	{"populace",		"populace.don"}
};
*/
const struct chan_type chan_table[]={
	{	"BLANK",								"BLANK",							"BLANK",				"BLANK","w",CM_NOQUOTE,		false},
	{	"%s housetalks, '{g%s{x'\n\r",			"%s housetalk, '{g%s{x'\n\r",		"{y[{YHouse{y] {g%s{x",	"HouseTalk","g",CM_NOHOUSE,		true},
	{	"%s questions, '{m%s{x'\n\r",			"%s question, '{m%s{x'\n\r",		"meh",					"Question","m",CM_NOQUESTION,	false},
	{	"%s answers, '{m%s{x'\n\r",				"%s answer, '{m%s{x'\n\r",			"meh",					"Answer","m",CM_NOQUESTION,	false},
	{	"{d[{c%s{d]{x: '{r%s{x'\n\r",			"{d[{c%s{d]{x: '{r%s{x'\n\r",		"{d[{cImm{d] {r%s{x",	"ImmTalk","c",CM_NOWIZ,		true},
	{	"{Y[{MNewb{Y] {x%s: '{M%s{x'\n\r",		"{Y[{MNewb{Y] {x%s: '{M%s{x'\n\r",	"{y[{MNewb{y] {M%s{x",	"Newbie","M",CM_NONEWBIE,	true},
	{	"%s chats, '{d%s{x'\n\r",				"%s chat, '{d%s{x'\n\r",			"{w[{dChat{w] {d%s{x",	"Chat","d",CM_NOCHAT,		true},
	{	"%s gossips, '{c%s{x'\n\r",				"%s gossip, '{c%s{x'\n\r",			"meh",					"Gossip","c",CM_NOGOSSIP,	false},
	{	"%s auctions, '{y%s{x'\n\r",			"%s auction, '{y%s{x'\n\r",			"meh",					"Auction","y",CM_NOAUCTION,	false},
	{	NULL,									"meh",								"meh",					" "," ",0,				false}
};
