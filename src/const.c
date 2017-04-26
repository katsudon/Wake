#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "db.h"

const struct item_type item_table[]={
    {	ITEM_LIGHT,			"light"			},
    {	ITEM_SCROLL,		"scroll"		},
    {	ITEM_WAND,			"wand"			},
    {   ITEM_STAFF,			"staff"			},
    {   ITEM_WEAPON,		"weapon"		},
    {   ITEM_COUPON,		"coupon"		},
    {   ITEM_ARMOR,			"armor"			},
    {	ITEM_POTION,		"potion"		},
    {   ITEM_FURNITURE,		"furniture"		},
    {	ITEM_TRASH,			"trash"			},
    {	ITEM_LUMBER,		"lumber"		},
    {	ITEM_BONES,			"bones"			},
    {	ITEM_RIBS,			"ribs"			},
    {	ITEM_CONTAINER,		"container"		},
    {	ITEM_DRINK_CON,		"drink"			},
    {	ITEM_KEY,			"key"			},
    {	ITEM_FOOD,			"food"			},
    {	ITEM_MONEY,			"money"			},
    {	ITEM_BOAT,			"boat"			},
    {	ITEM_CORPSE_NPC,	"npc_corpse"	},
    {	ITEM_CORPSE_PC,		"pc_corpse"		},
    {   ITEM_FOUNTAIN,		"fountain"		},
    {   ITEM_INGOT,			"ingot"			},
    {   ITEM_FORGEFUEL,		"fuelforge"		},
    {   ITEM_FORGE,			"forge"			},
    {   ITEM_LOCKPICK,		"lockpick"		},
    {   ITEM_THROWINGKNIFE,	"throwing-knife"},
    {   ITEM_THROWINGAXE,	"throwing-axe"	},
    {   ITEM_THROWINGDART,	"throwing-dart"	},
    {   ITEM_FIRE,			"fire"			},
    {   ITEM_SCARS,			"scars"			},
    {   ITEM_PARCHMENT,		"parchment"		},
    {   ITEM_TRAPPARTS,		"trapparts"		},
    {   ITEM_TRAP,			"trap"			},
    {	ITEM_PILL,			"pill"			},
    {	ITEM_CALTROPS,		"caltrops"		},
    {	ITEM_PORTAL,		"portal"		},
    {	ITEM_WARP_STONE,	"warp_stone"	},
    {	ITEM_ROOM_KEY,		"room_key"		},
    {	ITEM_GEM,			"gem"			},
    {	ITEM_JEWELRY,		"jewelry"		},
	{	ITEM_QUIVER,		"quiver"		},
	{	ITEM_ARROW,			"arrow",		},
    {   ITEM_SHIELD,		"shield"		},
    {   ITEM_HONINGSTONE,	"honingstone"	},
    {   ITEM_NEEDLE,		"needle"		},
    {   ITEM_DARK,			"dark"			},
    {   ITEM_HERB,			"herb"			},
    {   ITEM_LOCKER,		"locker"		},
    {   0,					NULL			}
};

const struct spirit_type spirit_table[MAX_SPIRITS]={
    {	"Name",		"Verbset"								},
    {	"wildcat",	"stalks its next victim."				},
    {	"bear",		"lumbers through."						},
    {   "raven",	"perches on its pupil's shoulder."		},
    {   "crow",		"stares about with crusty, dead eyes."	},
    {   "vulture",	"circles the skies."					},
    {   "lion",		"prowls in search of prey."				},
    {	"wolf",		"scouts the area."	},
    {	"rat",		"scampers across the ground."			},
    {   "bobcat",	"NULLs about"	},
    {	"bull",		"NULLs about."	},
    {	"buffalo",	"roams lazily by."						},
    {	"coyote",	"NULLs about."	},
    {	"deer",		"NULLs about."	},
    {	"stag",		"NULLs about."	},
    {	"eagle",	"soars through the sky."				},
    {	"hawk",		"NULLs about"	},
    {	"owl",		"sits motionless on a perch."			},
    {	"panther",	"prowls close to its master."			},
    {   "puma",		"NULLs about."	},
    {	"lynx",		"lurks in the shadows."					},
    {	"warthog",	"sniffs about for the trail."			},
    {	NULL,	NULL			},
};

const struct charmy_type ndemon_table[MAX_NDEMONS]={
	//  name,			short,			long					hpmult	class			race		mkelvl		desc
	{"imp",			"an imp",			"An imp.\n\r",				10,	CLASS_WIZARD,		"demon",	50,		""	},
	{"defiler",		"a defiler",		"A defiler.\n\r",			12,	CLASS_NECROMANCER,	"demon",	60,		""	},
	{"soul crusher","a soul crusher",	"A soul crusher.\n\r",		15,	CLASS_ANTIMAGE,		"demon",	70,		""	},
	{"bane",		"a bane",			"A bane.\n\r",				17,	CLASS_ASSASSIN,		"demon",	80,		""	},
	{"hellhound",	"a hellhound",		"A hellhound.\n\r",			20,	CLASS_HUNTER,		"demon",	90,		""	},
	{"demon",		"a demon",			"A demon.\n\r",				22,	CLASS_SORCERER,		"demon",	100,	""	},
	{"balor",		"a balor",			"A balor.\n\r",				50,	CLASS_NIGHTSTALKER,	"demon",	101,	""	},

	{	NULL,				NULL,		NULL,						1,	CLASS_TENDERFOOT,	"human",	121,	""	}
};
const struct charmy_type nundead_table[MAX_NUNDEADS]={
	//  name,			short,				long
	{	"skeleton",		"a skeleton",		"A skeleton.\n\r",		40,	CLASS_MERCENARY,	"skeleton",	50,		""	},
	{	"zombie",		"a zombie",			"A zombie.\n\r",		50,	CLASS_MERCENARY,	"undead",	60,		""	},
	{	"ghoul",		"a ghoul",			"A ghoul.\n\r",			60,	CLASS_ROGUE,		"undead",	70,		""	},
	{	"mummy",		"a mummy",			"A mummy.\n\r",			70,	CLASS_MERCENARY,	"undead",	80,		""	},
	{	"lich",			"a lich",			"A lich.\n\r",			80,	CLASS_WIZARD,		"skeleton",	90,		""	},
	{	"revenant",		"a revenant",		"A revenant.\n\r",		90,	CLASS_SORCERER,		"skeleton",	100,	""	},
	{	"putrid horror","a putrid horror",	"A putrid horror.\n\r",	100,CLASS_NIGHTSTALKER,	"human",	101,	""	},

	{	NULL,				NULL,		NULL,						1,	CLASS_TENDERFOOT,	"human",	121,	""	}
};
const struct charmy_type nspirit_table[MAX_NSPIRITS]={
	//  name,			short,				long
	{	"ghost",		"a ghost",			"A ghost.\n\r",			20,	CLASS_ANTIMAGE,		"spirit",	50,		""	},
	{	"shadow",		"a shadow",			"A shadow.\n\r",		25,	CLASS_SORCERER,		"shadow",	60,		""	},
	{	"nightshade",	"a nightshade",		"A nightshade.\n\r",	30,	CLASS_ASSASSIN,		"shadow",	70,		""	},
	{	"spectre",		"a spectre",		"A spectre.\n\r",		35,	CLASS_NECROMANCER,	"spirit",	80,		""	},
	{	"banshee",		"a banshee",		"A banshee.\n\r",		40,	CLASS_MINSTREL,		"spirit",	90,		""	},
	{	"phantom",		"a phantom",		"A phantom.\n\r",		45,	CLASS_WIZARD,		"spirit",	100,	""	},
	{	"wraith",		"a wraith",			"A wraith.\n\r",		75,	CLASS_HUNTER,		"shadow",	101,	""	},

	{	NULL,				NULL,		NULL,						1,	CLASS_TENDERFOOT,	"human",	121,	""	}
};

const struct god_type god_table[MAX_GOD]={
	{ NULL,				0,	0 },
	{ "Nesolis",		0,	0 },
	{ NULL,				0,	0 }
};

const struct nobility_type nobility_flags[MAX_NOBILITY]={
    { NOBLE_SERF,		{ "Thing",	"Village Idiot","Town Moron"	}},
    { NOBLE_PEASANT,	{ "Thing",		"Peasant",	"Peasant"		}},
    { NOBLE_FREEMAN,	{ "Thing",		"Freeman",	"Freewoman"		}},
    { NOBLE_PAGE,		{ "Thing",		"Page",		"Handmaid"		}},
    { NOBLE_SQUIRE,		{ "Thing",		"Squire",	"Squiress"		}},
    { NOBLE_KNIGHT,		{ "Thing",		"Sir",		"Dame"			}},
    { NOBLE_LORD,		{ "Thing",		"Lord",		"Lady"			}},
    { NOBLE_BARON,		{ "Thing",		"Baron",	"Baroness"		}},
    { NOBLE_VISCOUNT,	{ "Thing",		"Viscount",	"Viscountess"	}},
    { NOBLE_EARL,		{ "Thing",		"Earl",		"Countess"		}},
    { NOBLE_COUNT,		{ "Thing",		"Marquis",	"Marchioness"	}},
    { NOBLE_DUKE,		{ "Thing",		"Duke",		"Duchess"		}},
    { NOBLE_PRINCE,		{ "Thing",		"Prince",	"Princess"		}}, 
    { NOBLE_CHIEFTAN,	{ "Thing",		"Chieftain","Chieftain"		}},
    { NOBLE_KING,		{ "Thing",		"King",		"Queen"			}},
    { NOBLE_EMPEROR,	{ "Thing",		"Emperor",	"Empress"		}},
    { NOBLE_SITH,		{ "Thing",		"Darth",	"Darth"			}},
    { NOBLE_JEDI,		{ "Thing",		"Jedi",		"Jedi"			}},
    { NOBLE_IMMORTAL,	{ "Thing",		"God",		"Goddess"		}}
};

/* weapon selection table */
const struct weapon_type weapon_table [] ={
//   name               n00b vnum                       weaponINT
   { "exotic",			OBJ_VNUM_SCHOOL_LONGSWORD,		WEAPON_EXOTIC,			5,	&gsn_exotic,		gsn_exotic,			false,	1,	1	},//00

   { "club",			OBJ_VNUM_SCHOOL_CLUB,			WEAPON_CLUB,			2,	&gsn_clubs,			gsn_clubs,			true,	0,	0	},//01

   { "longsword",		OBJ_VNUM_SCHOOL_LONGSWORD,		WEAPON_LONGSWORD,		5,	&gsn_swords,		gsn_swords,			true,	3,	1	},//02
   { "bastardsword",	OBJ_VNUM_SCHOOL_BASTARD,		WEAPON_BASTARDSWORD,	8,	&gsn_swords,		gsn_swords,			true,	3,	2	},//03

   { "dagger",			OBJ_VNUM_SCHOOL_DAGGER,			WEAPON_DAGGER,			2,	&gsn_daggers,		gsn_daggers,		true,	1,	1	},//04
   { "dirk",			OBJ_VNUM_SCHOOL_DIRK,			WEAPON_DIRK,			1,	&gsn_daggers,		gsn_daggers,		true,	0,	1	},//05

   { "spear",			OBJ_VNUM_SCHOOL_SPEAR,			WEAPON_SPEAR,			8,	&gsn_polearms,		gsn_polearms,		false,	2,	2	},//06
   { "polearm",			OBJ_VNUM_SCHOOL_POLEARM,		WEAPON_POLEARM,			9,	&gsn_polearms,		gsn_polearms,		false,	3,	3	},//07

   { "mace",			OBJ_VNUM_SCHOOL_MACE,			WEAPON_MACE,			7,	&gsn_bludgeons, 	gsn_bludgeons,		false,	2,	2	},//08
   { "hammer",			OBJ_VNUM_SCHOOL_HAMMER,			WEAPON_HAMMER,			6,	&gsn_bludgeons,		gsn_bludgeons,		true,	3,	1	},//09
   { "warhammer",		OBJ_VNUM_SCHOOL_WARHAMMER,		WEAPON_WARHAMMER,		9,	&gsn_bludgeons,		gsn_bludgeons,		false,	5,	5	},//10

   { "axe",				OBJ_VNUM_SCHOOL_AXE,			WEAPON_AXE,				6,	&gsn_cleavers,		gsn_cleavers,		false,	1,	2	},//11
   { "battleaxe",		OBJ_VNUM_SCHOOL_BATTLEAXE,		WEAPON_BATTLEAXE,		9,	&gsn_cleavers,		gsn_cleavers,		false,	5,	4	},//12
   { "machete",			OBJ_VNUM_SCHOOL_MACHETE,		WEAPON_MACHETE,			5,	&gsn_swords,		gsn_swords,			true,	3,	1	},//13
   { "scythe",			OBJ_VNUM_SCHOOL_SCYTHE,			WEAPON_SCYTHE,			8,	&gsn_cleavers,		gsn_cleavers,		false,	3,	1	},//14

   { "flail",			OBJ_VNUM_SCHOOL_FLAIL,			WEAPON_FLAIL,			4,	&gsn_lashes,		gsn_lashes,			false,	1,	1	},//15
   { "whip",			OBJ_VNUM_SCHOOL_WHIP,			WEAPON_WHIP,			3,	&gsn_lashes,		gsn_lashes,			false,	0,	1	},//16

   { "rapier",			OBJ_VNUM_SCHOOL_RAPIER,			WEAPON_RAPIER,			4,	&gsn_swords,		gsn_swords,			true,	3,	-1	},//17
   { "katana",			OBJ_VNUM_SCHOOL_KATANA,			WEAPON_KATANA,			5,	&gsn_swords,		gsn_swords,			true,	3,	1	},//18
   { "scimitar",		OBJ_VNUM_SCHOOL_SCIMITAR,		WEAPON_SCIMITAR,		6,	&gsn_swords,		gsn_swords,			true,	3,	2	},//19

   { "sai",				OBJ_VNUM_SCHOOL_SAI,			WEAPON_SAI,				2,	&gsn_martial_arms,	gsn_martial_arms,	true,	0,	4	},//20
   { "tonfa",			OBJ_VNUM_SCHOOL_TONFA,			WEAPON_TONFA,			2,	&gsn_martial_arms,	gsn_martial_arms,	true,	3,	0	},//21
   { "gauntlet",		OBJ_VNUM_SCHOOL_GAUNTLET,		WEAPON_GAUNTLET,		1,	&gsn_martial_arms,	gsn_martial_arms,	true,	0,	0	},//22
   { "nunchaku",		OBJ_VNUM_SCHOOL_NUNCHAKU,		WEAPON_NUNCHAKU,		3,	&gsn_martial_arms,	gsn_martial_arms,	true,	0,	1	},//23

   { "maingauche",		OBJ_VNUM_SCHOOL_MAINGAUCHE,		WEAPON_MAINGAUCHE,		3,	&gsn_shortswords,	gsn_shortswords,	true,	0,	2	},//24
   { "gladius",			OBJ_VNUM_SCHOOL_GLADIUS,		WEAPON_GLADIUS,			7,	&gsn_shortswords,	gsn_shortswords,	true,	3,	2	},//25

   { "lance",			OBJ_VNUM_SCHOOL_LANCE,			WEAPON_LANCE,			9,	&gsn_lances,		gsn_lances,			false,	6,	6	},//26

   { "staff",			OBJ_VNUM_SCHOOL_STAFF,			WEAPON_STAFF,			8,	&gsn_staffs,		gsn_staffs,			false,	1,	2	},//27
   { "quarterstaff",	OBJ_VNUM_SCHOOL_QUARTERSTAFF,	WEAPON_QUARTERSTAFF,	6,	&gsn_staffs,		gsn_staffs,			false,	2,	3	},//28

   { "longbow",			OBJ_VNUM_SCHOOL_LONGBOW,		WEAPON_LONGBOW,			7,	&gsn_bows,			gsn_bows,			false,	0,	0	},//29
   { "shortbow",		OBJ_VNUM_SCHOOL_SHORTBOW,		WEAPON_SHORTBOW,		7,	&gsn_bows,			gsn_bows,			false,	0,	0	},//30

   { NULL,				0,								0,						0,	NULL,				NULL,				false,	0,	0	}
};

const struct wiznet_type wiznet_table[]={
   {    "on",           WZ_ON,			L5 },
   {    "prefix",		WZ_PREFIX,		L5 },
   {    "ticks",        WZ_TICKS,		L5 },
   {    "logins",       WZ_LOGINS,		L5 },
   {    "sites",        WZ_SITES,		L4 },
   {    "links",        WZ_LINKS,		L7 },
   {	"newbies",		WZ_NEWBIE,		L5 },
   {	"spam",			WZ_SPAM,		L5 },
   {    "deaths",       WZ_DEATHS,		L5 },
   {    "playerkills",	WZ_PKS,			L5 },
   {    "resets",       WZ_RESETS,		L4 },
   {    "mobdeaths",    WZ_MOBDEATHS,	L4 },
   {    "flags",		WZ_FLAGS,		L5 },
   {	"penalties",	WZ_PENALTIES,	L5 },
   {	"saccing",		WZ_SACCING,		L5 },
   {	"levels",		WZ_LEVELS,		L5 },
   {	"load",			WZ_LOAD,		L2 },
   {	"restore",		WZ_RESTORE,		L2 },
   {	"snoops",		WZ_SNOOPS,		L2 },
   {	"switches",		WZ_SWITCHES,	L2 },
   {	"secure",		WZ_SECURE,		L1 },
   {	NULL,			0,				0  }
};

/* attack table  -- not very organized :( */
const struct attack_type attack_table [MAX_DAMAGE_MESSAGE]={
    { 	"none",			"hit",			-1				}, /*  0 */
    {	"slice",		"slice", 		DAM_SLASH		},	
    {   "stab",			"stab",			DAM_PIERCE		},
    {	"slash",		"slash",		DAM_SLASH		},
    {	"whip",			"whip",			DAM_SLASH		},
    {   "claw",			"claw",			DAM_SLASH		},  /*  5 */
    {	"blast",		"blast",		DAM_BASH		},
    {   "pound",		"pound",		DAM_BASH		},
    {	"crush",		"crush",		DAM_BASH		},
    {   "grep",			"grep",			DAM_SLASH		},
    {	"bite",			"bite",			DAM_PIERCE		},  /* 10 */
    {   "pierce",		"pierce",		DAM_PIERCE		},
    {   "suction",		"suction",		DAM_BASH		},
    {	"beating",		"beating",		DAM_BASH		},
    {   "digestion",	"digestion",	DAM_ACID		},
    {	"charge",		"charge",		DAM_BASH		},  /* 15 */
    { 	"slap",			"slap",			DAM_BASH		},
    {	"punch",		"punch",		DAM_BASH		},
    {	"wrath",		"wrath",		DAM_ENERGY		},
    {	"magic",		"magic",		DAM_ENERGY		},
    {   "divine",		"divine power",	DAM_HOLY		},  /* 20 */
    {	"cleave",		"cleave",		DAM_SLASH		},
    {	"scratch",		"scratch",		DAM_PIERCE		},
    {   "peck",			"peck",			DAM_PIERCE		},
    {   "peckb",		"peck",			DAM_BASH		},
    {   "chop",			"chop",			DAM_SLASH		},  /* 25 */
    {   "sting",		"sting",		DAM_PIERCE		},
    {   "smash",		"smash",		DAM_BASH		},
    {   "shbite",		"shocking bite",DAM_LIGHTNING	},
    {	"flbite",		"flaming bite", DAM_FIRE		},
    {	"frbite",		"freezing bite",DAM_COLD		},  /* 30 */
    {	"acbite",		"acidic bite", 	DAM_ACID		},
    {	"chomp",		"chomp",		DAM_PIERCE		},
    {  	"drain",		"life drain",	DAM_NEGATIVE	},
    {   "thrust",		"thrust",		DAM_PIERCE		},
    {   "slime",		"slime",		DAM_ACID		},
    {	"shock",		"shock",		DAM_LIGHTNING	},
    {   "thwack",		"thwack",		DAM_BASH		},
    {   "flame",		"flame",		DAM_FIRE		},
    {   "chill",		"chill",		DAM_COLD		},
    {   NULL,			NULL,			0				}
};

struct	class_type	classes[MAX_CLASS];

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app [STAT_MAX+1]={
// tohit,todam,carry,wield
    { -5, -4,   0,  0 }, /* 0 */
    { -5, -4,   3,  1 },
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 }, /* 5 */
    { -1,  0,  70,  6 },
    { -1,  0,  80,  7 },
    {  0,  0,  90,  8 },
    {  0,  0, 100,  9 },
    {  1,  1, 115, 10 }, /* 10 */
    {  1,  2, 115, 11 },
    {  2,  3, 130, 12 },
    {  2,  4, 130, 13 },
    {  3,  5, 140, 14 },
    {  3,  6, 150, 15 }, /* 15 */
    {  4,  7, 165, 16 },
    {  4,  8, 180, 22 },
    {  5,  9, 200, 25 },
    {  5, 10, 225, 30 },
    {  6, 11, 250, 35 }, /* 20 */
    {  6, 12, 300, 40 },
    {  7, 13, 350, 45 },
    {  7, 14, 400, 50 },
    {  8, 15, 450, 55 },
    {  8, 16, 500, 60 }, /* 25 */
    {  9, 17, 550, 65 },
    {  9, 18, 600, 70 }
};

const struct int_app_type int_app [STAT_MAX+1]={
    {  3 },	/* 0 */
    {  5 },
    {  7 },
    {  8 },
    {  9 },
    { 10 },	/* 5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 },	/* 25 */
    { 90 },
    { 95 }
};

const struct wis_app_type wis_app [STAT_MAX+1]={
    { 0 },	/*  0 */
    { 0 },
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },	/* 25 */
    { 5 },
    { 5 }
};

const struct dex_app_type dex_app [STAT_MAX+1]={
    {  100 },   /* 0 */
    {   80 },   /* 1 */
    {   60 },
    {   40 },
    {   20 },
    {    0 },   /* 5 */
    {  -10 },
    {  -20 },
    {  -30 },
    {  -40 },
    {  -50 },   /* 10 */
    {  -60 },
    {  -70 },
    {  -80 },
    {  -90 },
    { -100 },   /* 15 */
    { -110 },
    { -120 },
    { -130 },
    { -140 },
    { -150 },   /* 20 */
    { -160 },
    { -170 },
    { -180 },
    { -190 },
    { -200 },    /* 25 */
    { -210 },
    { -220 }
};

const struct end_app_type end_app [STAT_MAX+1]={
    {  -4,  0 },  /* 0 */
    {  -3,  1 },
    {  -2,  2 },
    {  -2,  4 },
    {  -1,  6 },
    {  -1,  8 },  /* 5 */
    {  -1, 10 },
    {   0, 12 },
    {   0, 14 },
    {   0, 16 },
    {   0, 18 },  /* 10 */
    {   0, 20 },
    {   0, 22 },
    {   0, 24 },
    {   0, 26 },
    {   1, 28 },  /* 15 */
    {   2, 30 },
    {   2, 32 },
    {   3, 34 },
    {   3, 36 },
    {   4, 38 },  /* 20 */
    {   4, 40 },
    {   5, 42 },
    {   5, 44 },
    {   6, 46 },
    {   6, 48 },  /* 25 */
    {   7, 50 },
    {   7, 52 }
};

const struct liq_type liq_table[]={
/*    name					color	        proof,ful,trst,fud,sze */
    { "water",				"clear",		{   0,	5,	12,	0,	2	} },
    { "milk",				"white",		{   0,	10,	9,	3,	3	} },
    { "lemonade",			"pink",			{   0,	5,	9,	2,	2	} },
    { "coke",				"brown",		{   0,	5,	9,	2,	3	} }, 
    { "root beer",			"brown",		{   0,	5,	9,	2,	3	} },
    { "orange juice",		"orange",		{   0,	5,	9,	3,	4	} },
    { "cranberry juice",	"red",			{   0,	8,  9,	2,	4	} },
    { "tea",				"tan",			{   0,	5,  8,	0,	2	} },
	{ "apple juice",		"amber",		{   0,	5,  8,	1,	3	} },
    { "coffee",				"black",		{   0,	7,  8,	0,	5	} },
    { "blood",				"red",			{   0,	10,	-1, 2,	8	} },
    { "salt water",			"clear",		{   0,	6,	-2, 0,	5	} },
    { "slime mold juice",	"green",		{   0,	10,	-8, 1,	5	} },

    { "beer",				"amber",		{   6,	5,	8,	1,	2	} },
    { "ale",				"brown",		{   7,	5,	8,	1,	2	} },
    { "dark ale",			"dark",			{   8,	5,	8,	1,	2	} },
    { "rose wine",			"pink",			{  13,	5,	8,	1,	5	} },
    { "white wine",			"golden",		{  14,	5,	8,	1,	5	} },
    { "red wine",			"burgundy",		{  15,	5,	8,	1,	5	} },
    { "champagne",			"golden",		{  16,	5,	8,	1,	5	} },
    { "mead",				"honey-colored",{  17,	10,	8,	2,	3	} },
    { "elvish wine",		"green",		{  17,	10,	8,	1,	5	} },
    { "amontillado",		"burgundy",		{  17,	10,	8,	1,	5	} },
    { "sherry",				"red",			{  19,	10,	7,	1,	5	} },
    { "benedictine wine",	"burgundy",		{  20,	5,	8,	1,	5	} },
    { "icewine",			"purple",		{  25,	10,	6,	1,	5	} },	
    { "framboise",			"red",			{  25,	5,	7,	1,	5	} },
    { "vinewine",			"greenish",		{  35,	25,	4,	1,	6	} },
    { "brandy",				"golden",		{  40,	5,	5,	0,	4	} },
    { "schnapps",			"clear",		{  45,	5,	5,	0,	2	} },
    { "cordial",			"clear",		{  50,	5,	5,	0,	2	} },
    { "whisky",				"golden",		{  60,	5,	5,	0,	2	} },
    { "vodka",				"clear",		{  65,	5,	5,	0,	2	} },
    { "aquavit",			"clear",		{  70,	5,	5,	0,	2	} },
    { "local specialty",	"clear",		{  75,	5,	3,	0,	2	} },
    { "rum",				"amber",		{  75,	5,	4,	0,	2	} },
	{ "soju",				"clear",		{  85,	5,	2,	0,	2	} },
    { "firebreather",		"boiling",		{  95,	5,	4,	0,	2	} }, 
    { "absinthe",			"green",		{ 100,	5,	4,	0,	2	} },

    { NULL,					NULL,			{   0,	0,	0,	0,	0	} }
};

const struct int_app_type weapon_percent[]={
	{100},
	{100},//1
	{93},//2
	{87},//3
	{81},//4
	{75},//5
	{68},//6
	{62},//7
	{56},//8
	{50}//9
};
