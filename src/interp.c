#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "olc.h"
//#include "imc.h"


char last_command[MSL];
bool check_social	( CHAR_DATA*,char*,char* );

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * Log-all switch.
 */
bool				fLogAll		= false;

/*
 * Command table.
 */
const struct cmd_type cmd_table []={
    /*
     * Common movement commands.
     */
    { "north",			do_north,			POS_STANDING,	0,  LOG_NEVER, 0, false },
    { "east",			do_east,			POS_STANDING,	0,  LOG_NEVER, 0, false },
    { "south",			do_south,			POS_STANDING,	0,  LOG_NEVER, 0, false },
    { "west",			do_west,			POS_STANDING,	0,  LOG_NEVER, 0, false },
    { "up",				do_up,				POS_STANDING,	0,  LOG_NEVER, 0, false },
    { "down",			do_down,			POS_STANDING,	0,  LOG_NEVER, 0, false },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "affects",		do_affects,			POS_DEAD,		0,  LOG_NORMAL, 1, true		},
    { "assist",			do_assist,			POS_STANDING,	0,  LOG_NORMAL, 1, false	},
    { "cast",			do_cast,			POS_FIGHTING,	0,  LOG_NORMAL, 1, false	},
    { "auction",		do_auction,			POS_SLEEPING,	0,  LOG_NORMAL, 1, false	},
    { "buy",			do_buy,				POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "channels",		do_channels,		POS_DEAD,		0,  LOG_NORMAL, 1, true		},
    { "exits",			do_exits,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "get",			do_get,				POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "group",			do_group,			POS_SLEEPING,	0,  LOG_NORMAL, 1, true		},
    { "hit",			do_kill,			POS_FIGHTING,	0,  LOG_NORMAL, 0, false	},//
    { "inventory",		do_inventory,		POS_DEAD,		0,  LOG_NORMAL, 1, false	},
    { "kill",			do_kill,			POS_FIGHTING,	0,  LOG_NORMAL, 1, false	},
    { "knock",			do_knock,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},/*Written by John Patrick (j.s.patrick@ieee.org)*/
    { "look",			do_look,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "chat",			do_chat,   			POS_SLEEPING,	0,  LOG_NORMAL, 1, true		}, 
    { "order",			do_order,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "rest",			do_rest,			POS_SLEEPING,	0,  LOG_NORMAL, 1, false	},
    { "retell",			do_rtell,			POS_RESTING,	0,  LOG_NORMAL, 1, true		},
    { "rtell",			do_rtell,			POS_RESTING,	0,  LOG_NORMAL, 1, true		},
    { "sit",			do_sit,				POS_SLEEPING,	0,  LOG_NORMAL, 1, false	},
    { "stand",			do_stand,			POS_SLEEPING,	0,  LOG_NORMAL, 1, false	},
    { "tell",			do_tell,			POS_RESTING,	0,  LOG_NORMAL, 1, true		},
    { "train",			do_train,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "change",			do_change,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "unlock",			do_unlock,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "wield",			do_wear,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},
    { "catnap",			do_catnap,			POS_RESTING,	0,  LOG_NORMAL, 1, false	},

    /*
     * Informational commands.
     */
    { "areas",			do_areas,			POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "bank",			do_bank,			POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "bug",			do_bug,				POS_DEAD,		0,  LOG_NORMAL, 1, true },//
    { "board",			do_board,			POS_SLEEPING,	0,  LOG_NORMAL, 1, true },
    { "commands",		do_commands,		POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "compare",		do_compare,			POS_RESTING,	0,  LOG_NORMAL, 1, false },//
    { "consider",		do_consider,		POS_RESTING,	0,  LOG_NORMAL, 1, false },//
    { "count",			do_count,			POS_SLEEPING,	0,  LOG_NORMAL, ML, true },//
    { "credits",		do_credits,			POS_DEAD,		0,  LOG_NORMAL, 1, true },//
    { "equipment",		do_equipment,		POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "examine",		do_examine,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "help",			do_help,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "info",			do_info,			POS_SLEEPING,		0,  LOG_NORMAL, 1, true },
    { "motd",			do_motd,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
	{ "plan",			do_skillstat,		POS_DEAD,		0,	LOG_NORMAL, 1, true },
    { "practice",		do_practice,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
	{ "race",			do_race,			POS_SLEEPING,	0,	LOG_NORMAL,	1, true },
    { "read",			do_read,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "redeem",			do_redeem,			POS_SLEEPING,	0,  LOG_NORMAL, 1, true },
    { "report",			do_report,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "rules",			do_rules,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "score",			do_score,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "skills",			do_skills,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "skygaze",		do_skygaze,			POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "scan",			do_scan,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "socials",		do_socials,			POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "show",			do_show,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "spells",			do_spells,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "story",			do_story,			POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "time",			do_time,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "typo",			do_typo,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "weather",		do_weather,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "who",			do_who,				POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "whois",			do_whois,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "wizlist",		do_wizlist,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "worth",			do_worth,			POS_SLEEPING,	0,  LOG_NORMAL, 1, true },

    /*
     * Configuration commands.
     */
    { "alia",			do_alia,			POS_DEAD,		0,  LOG_NORMAL, 0, true },
    { "alias",			do_alias,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autolist",		do_autolist,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autoassist",		do_autoassist,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autoexit",		do_autoexit,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autogold",		do_autogold,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autolook",		do_autolook,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autoloot",		do_autoloot,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autosac",		do_autosac,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autosplit",		do_autosplit,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "automap",		do_automap,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "autoweather",	do_autoweather,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "brief",			do_brief,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
	{ "color",			do_color,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "combine",		do_combine,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "compact",		do_compact,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "description",	do_newdesc,			POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "delet",			do_delet,			POS_DEAD,		0,  LOG_ALWAYS, 0, false },
    { "delete",			do_delete,			POS_STANDING,	0,  LOG_ALWAYS, 1, false },
    { "nofollow",		do_nofollow,		POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "noloot",			do_noloot,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "nonotes",		do_nonotes,			POS_DEAD,		0,	LOG_ALWAYS, 1, true },
    { "nosummon",		do_nosummon,		POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "olddesc",		do_description,		POS_DEAD,		0,  LOG_NORMAL, 1, false },
    { "outfit",			do_outfit,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "password",		do_password,		POS_DEAD,		0,  LOG_NEVER,  1, true },
    { "prompt",			do_prompt,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "scroll",			do_scroll,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "title",			do_title,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "unalias",		do_unalias,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "convert",		do_convert,			POS_DEAD,		0,  LOG_NORMAL, 1, true },
    { "coolskills",		do_rape_skill,			POS_DEAD,		0,  LOG_NORMAL, 1, true },

    /*
     * Communication commands.
     */
    { "note",			do_note,			POS_DEAD,		0,	LOG_NORMAL, 1, false },
    { "falconry",		do_falconry,		POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "afk",			do_afk,				POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "answer",			do_answer,			POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "deaf",			do_deaf,			POS_DEAD,		0,	LOG_NORMAL, 1, true },
    { "emote",			do_emote,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "pmote",			do_pmote,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { ".",				do_gossip,			POS_SLEEPING,	0,	LOG_NORMAL, 0, false },
    { "gossip",			do_gossip,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { ",",				do_emote,			POS_RESTING,	0,	LOG_NORMAL, 0, false },
    { "history",		do_history,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "newbie",			do_newbie,			POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "gtell",			do_gtell,			POS_DEAD,		0,	LOG_NORMAL, 1, true },
    { "housetalk",		do_housetalk,		POS_SLEEPING,	0,	LOG_NORMAL, 0, false },
    { "ht",				do_housetalk,		POS_SLEEPING,	0,	LOG_NORMAL, 0, false },
    { ";",				do_gtell,			POS_DEAD,		0,	LOG_NORMAL, 0, true },
    { "question",		do_question,		POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "quote",			do_quote,			POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "quiet",			do_quiet,			POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "reply",			do_reply,			POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "replay",			do_replay,			POS_SLEEPING,	0,	LOG_NORMAL, 1, true },
    { "say",			do_say,				POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "sayto",			do_sayto,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "smote",			do_smote,			POS_DEAD,		0,	LOG_NORMAL, 1, false },
	{ "osay",			do_osay,			POS_RESTING,	0,	LOG_NORMAL, 1, true },
	{ "osayto",			do_osayto,			POS_RESTING,	0,	LOG_NORMAL, 1, true },
	{ "\"",				do_osay,			POS_RESTING,	0,	LOG_NORMAL, 1, true },
	{ "oocsay",			do_osay,			POS_RESTING,	0,	LOG_NORMAL, 1, true },
    { "yell",			do_yell,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "'",	    		do_say,				POS_RESTING,	0,	LOG_NORMAL, 0, false },

    /*
     * Object manipulation commands.
     */
    { "barb",			do_barb,			POS_RESTING,	0,	LOG_NORMAL, 0, false },
    { "brandish",		do_brandish,		POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "close",			do_close,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "conceal",		do_conceal,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "cutpurse",		do_cutpurse,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "devour",			do_devour,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "disable",		do_disable,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "dislodge",		do_dislodge,		POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "donate",			do_donate,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "drink",			do_drink,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "drop",			do_drop,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
	{ "dual wield",		do_dual_wield,		POS_RESTING,    0,  LOG_NORMAL, 1, false },
    { "eat",			do_eat,				POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "embalm",			do_embalm,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "envenom",		do_envenom,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "fill",			do_fill,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "gather",			do_gather,			POS_STANDING,	0,  LOG_NORMAL, 1, false },
    { "give",			do_give,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "grasp",			do_grasp,			POS_RESTING,	0,	LOG_NORMAL, 0, false },
    { "graverob",		do_graverob,		POS_STANDING,	0,  LOG_NORMAL, 1, false },
    { "grip",			do_grip,			POS_RESTING,	0,  LOG_NORMAL, 0, false },
    { "heal",			do_heal,			POS_RESTING,	0,  LOG_NORMAL, 1, false }, 
    { "hold",			do_wear,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "improvise",		do_improvise,		POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "infuse",			do_arrow_infusion,	POS_STANDING,	0,  LOG_NORMAL, 1, false },
    { "junk",			do_sacrifice,		POS_RESTING,    0,  LOG_NORMAL, 0, false },
    { "list",			do_list,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "pick",			do_pick,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "lore",			do_lore,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "lock",			do_lock,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "open",			do_open,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "pour",			do_pour,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "put",			do_put,				POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "quaff",			do_quaff,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "prepare",		do_prepare,			POS_STANDING,	0,  LOG_NORMAL, 1, false },
    { "ready",			do_ready,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "recite",			do_recite,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "reload",			do_reload,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "remove",			do_remove,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "sell",			do_sell,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "sacrifice",		do_sacrifice,		POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "sharpen",		do_sharpen,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "exchange",		do_exchange,		POS_RESTING,	0,  LOG_NORMAL, 1, false },
//    { "smelt",		do_smelt,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "take",			do_get,				POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "value",			do_value,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "wear",			do_wear,			POS_RESTING,	0,  LOG_NORMAL, 1, false },
    { "zap",			do_zap,				POS_RESTING,	0,  LOG_NORMAL, 1, false },

    /*
     * Combat commands.
     */
    { "assassinate",	do_assassinate,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "ambush",			do_ambush,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "backfist",		do_backfist,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "backstab",		do_backstab,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "blindingdust",	do_blindingdust,	POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "charge",			do_charge,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "cleave",			do_cleave,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "culminate",		do_culminate,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "draw",			do_draw,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "fire",			do_fire,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "bash",			do_bash,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "batter",			do_batter,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "bs",				do_backstab,		POS_FIGHTING,	0,	LOG_NORMAL, 0, false },
    { "berserk",		do_berserk,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "bulwark",		do_bulwark,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "caltrops",		do_caltrops,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "defensive",		do_defensive_stance,POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "dirt",			do_dirt,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "disappear",		do_disappear,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "disarm",			do_disarm,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "blitz",			do_double_blitz,	POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "drag",			do_drag,			POS_FIGHTING,	0,	LOG_NORMAL,	1, false },
    { "die",			do_die,				POS_MORTAL,		0,	LOG_NORMAL,	1, false },
    { "escape",			do_quickescape,		POS_FIGHTING,	0,	LOG_NEVER,	0, false },
    { "feralslash",		do_feral_slash,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "flee",			do_flee,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "fortify",		do_fortify,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "gouge",			do_gouge,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "guard",			do_guard,			POS_RESTING,	0,	LOG_NORMAL, 0, false },
    { "hunt",			do_track,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "heavysmash",		do_heavy_smash,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "kick",			do_kick,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "kneeshot",		do_kneeshot,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "mark",			do_mark,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "mirage",			do_mirage,			POS_FIGHTING,	0,	LOG_NORMAL, 0, false },
    { "mighty swing",	do_mighty_swing,	POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "murde",			do_murde,			POS_FIGHTING,	0,	LOG_NORMAL, 0, false },
    { "needle",			do_needle,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "nerve",			do_nerve,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "pierce",			do_pierce,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "poisoncloud",	do_poisoncloud,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "pommel strike",	do_pommel_strike,	POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "punch",			do_punch,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "push",			do_push,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "rescue",			do_rescue,			POS_FIGHTING,	0,	LOG_NORMAL, 0, false },
    { "reckless",		do_reckless_abandon,POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "rupture",		do_rupture,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "sap",			do_sap,				POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "shadowstrike",	do_shadowstrike,	POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "shieldbash",		do_shieldbash,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "shieldstrike",	do_shieldstrike,	POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "shieldtoss",		do_shieldtoss,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "shift",			do_shift,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "smash",			do_smash,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "snare",			do_snare,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "flurry",			do_flurry,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "stalk",			do_stalk,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "stance",			do_engarde,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "strafe",			do_strafe,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "strangle",		do_strangle,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "strike",			do_strike,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "swap",			do_swap,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "lift",			do_lift,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "surrender",		do_surrender,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "terrorize",		do_terrorize,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "throw",			do_throw,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "trap",			do_traps,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "trip",			do_trip,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "warriorheart",	do_warrior_heart,	POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "whirlwind",		do_whirlwind,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "vanish",			do_vanish,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "zeal",			do_zeal,			POS_RESTING,	0,	LOG_NORMAL, 1, false },

    /*
     * Miscellaneous commands.
     */
    { "apply",			do_apply,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "alertness",		do_alertness,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "awareness",		do_awareness,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "bandage",		do_bandage,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "blend",			do_blend,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "duel", 			do_duel,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "demote",			do_demote,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "desert cover",	do_desertcover,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "divinetouch",	do_divinetouch,		POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "eavesdrop",		do_eavesdrop,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "enter", 			do_enter,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "exit",			do_quit,			POS_DEAD,		0,	LOG_NORMAL, 1, false },//nash needs to figure out how to make this work
    { "expel",			do_expel,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "firstaid",		do_firstaid,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "follow",			do_follow,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "gain",			do_gain,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "go",				do_enter,			POS_STANDING,	0,	LOG_NORMAL, 0, false },
    { "hawkeye",		do_hawkeye,			POS_STANDING,	0,  LOG_NORMAL, 1, false },
    { "hide",			do_hide,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "holytouch",		do_holytouch,		POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "hurl",			do_hurl,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "morph",			do_morph,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
//    { "play",			do_play,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "qui",			do_qui,				POS_DEAD,		0,	LOG_NORMAL, 0, false },
    { "quit",			do_quit,			POS_DEAD,		0,	LOG_NORMAL, 1, false },
    { "recall",			do_recall,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "/",				do_recall,			POS_FIGHTING,	0,	LOG_NORMAL, 0, false },
    { "rent",			do_rent,			POS_DEAD,		0,	LOG_NORMAL, 0, false },
    { "save",			do_save,			POS_DEAD,		0,	LOG_NORMAL, 1, false },
    { "sleep",			do_sleep,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "sneak",			do_sneak,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "makefire",		do_firemaking,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "leadership",		do_leadership,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "overdrive",		do_overdrive,		POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "promote",		do_promote,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "redeem",			do_redeem,			POS_RESTING,	0,	LOG_ALWAYS, 1, true  },
    { "split",			do_split,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "steal",			do_steal,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "study",			do_gain,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "tame",			do_tame,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "unmorph",		do_unmorph,			POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "untangle",		do_untangle,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "visible",		do_visible,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "wake",			do_wake,			POS_SLEEPING,	0,	LOG_NORMAL, 1, false },
    { "where",			do_where,			POS_RESTING,	0,	LOG_NORMAL, 1, false },
    { "mount",			do_mount,			POS_FIGHTING,	0,  LOG_NORMAL, 1, false },
    { "dismount",		do_dismount,		POS_FIGHTING,	0,	LOG_NORMAL, 1, false },
    { "slip",			do_slip,			POS_STANDING,	0,	LOG_NORMAL, 1, false },
    { "palm",			do_palm,			POS_STANDING,	0,	LOG_NORMAL, 1, false },

    /*
     * Nash commands.
     */
	{ "daurin",			do_daurin,			POS_RESTING,	0,	LOG_NORMAL, 1, false },

    // Mob command interpreter (placed here for faster scan...)
    { "mob",		do_mob,		POS_DEAD,	 0,  LOG_NEVER,  0, false },

    // Immortal commands.
	//Admin
	{ "cql",			do_cql,				POS_DEAD,		ML,	LOG_NORMAL, 1, true },
	{ "mgc",			do_mobgroupcheck,	POS_DEAD,		ML,	LOG_NORMAL, 1, true },
    { "dump",			do_dump,			POS_DEAD,		ML,	LOG_ALWAYS, 1, true },
    { "permban",		do_permban,			POS_DEAD,		ML,	LOG_ALWAYS, 1, true },
    { "shutdow",		do_shutdow,			POS_DEAD,		ML, LOG_NORMAL, 0, false },
    { "shutdown",		do_shutdown,		POS_DEAD,		ML, LOG_ALWAYS, 1, false },
    { "illegalname",	do_illegalname,		POS_DEAD,		ML, LOG_NORMAL, 1, true },
    { "at",				do_at,				POS_DEAD,		ML,	LOG_NORMAL, 1, true },
    { "multitell",		do_multitell,		POS_DEAD,		ML,	LOG_NORMAL, 1, true },

	//DemiAdmin
	{ "changelog",		do_changelog,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
	{ "classedit",		do_classedit,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
	{ "homeedit",		do_hometownedit,	POS_DEAD,		DA,	LOG_NORMAL, 1, true },
	{ "classstat",		do_classstat,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
    { "scatter",		do_scatter,			POS_DEAD,		DA, LOG_NORMAL, 1, true },
    { "unlink",			do_unlink,			POS_DEAD,		DA, LOG_NORMAL, 1, true },
    { "numberrand",		do_randomnumber,	POS_DEAD,		DA, LOG_NORMAL, 1, true },
    { "specialt",		do_specialthing,	POS_DEAD,		DA, LOG_NORMAL, 1, true },
	{ "nobility",		do_noble,			POS_DEAD,		DA,	LOG_NORMAL, 1, true },
	{ "guildset",		do_guildset,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
	{ "guildedit",		do_guildedit,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
	{ "guildstat",		do_guildstat,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
    { "allow",			do_allow,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
    { "pban",			do_ban,				POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
	{ "copyover",		do_copyover,		POS_DEAD,		DA, LOG_ALWAYS, 1, false },
    { "deny",			do_deny,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
    { "freeze",			do_freeze,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
    { "newlock",		do_newlock,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
	{ "pretitle",		do_pretitle,		POS_DEAD,		DA,	LOG_NORMAL, 1, true },
    { "protect",		do_protect,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
    { "reboo",			do_reboo,			POS_DEAD,		DA, LOG_NORMAL, 0, false },
    { "reboot",			do_reboot,			POS_DEAD,		DA, LOG_ALWAYS, 1, false },
    { "rename",			do_rename,			POS_DEAD,		DA,	LOG_NORMAL, 1, false },
    { "trust",			do_trust,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
    { "flag",			do_flag,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },
    { "wizlock",		do_wizlock,			POS_DEAD,		DA,	LOG_ALWAYS, 1, true },

    { "newmap",			do_newmap,			POS_DEAD,		L3, LOG_NORMAL, 1, true },
    { "prefi",			do_prefi,			POS_DEAD,		L4,	LOG_NORMAL, 0, true },
    { "prefix",			do_prefix,			POS_DEAD,		L4,	LOG_NORMAL, 1, true },
	//Senior Imm
    { "log",			do_log,				POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "noemote",		do_noemote,			POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "noshout",		do_noshout,			POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "notell",			do_notell,			POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "pkset",			do_setpk,			POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "pload",			do_pload,			POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "punload",		do_punload,			POS_DEAD,		L1,	LOG_ALWAYS, 1, false },
	{ "pretitle",		do_pretitle,		POS_DEAD,		L1,	LOG_NORMAL, 1, true },
    { "protect",		do_protect,			POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "rename",			do_rename,			POS_DEAD,		L1,	LOG_NORMAL, 1, false },
    { "return",			do_return,			POS_DEAD,		L1,	LOG_NORMAL, 1, false },
	{ "sedit",			do_sedit,			POS_DEAD,		L1,	LOG_NORMAL, 1, true },
    { "set",			do_set,				POS_DEAD,		L1,	LOG_ALWAYS, 1, true },
    { "switch",			do_switch,			POS_DEAD,		L1,	LOG_ALWAYS, 1, false },
    { "aedit",			do_aedit,			POS_DEAD,		L1,	LOG_NORMAL, 1, false },

	//Headimm
    { "debugger",		do_debugset,		POS_DEAD,		L2,	LOG_ALWAYS, 1, true },
    { "disconnect",		do_disconnect,		POS_DEAD,		L2,	LOG_ALWAYS, 1, false },
    { "gecho",			do_echo,			POS_DEAD,		L2,	LOG_NORMAL, 1, true },
    { "hedit",			do_hedit,			POS_DEAD,		L2,	LOG_NORMAL, 1, true },
    { "mpdump",			do_mpdump,			POS_DEAD,		L2,	LOG_NEVER,  1, false },
    { "mwhere",			do_mwhere,			POS_DEAD,		L2,	LOG_NORMAL, 1, true },
    { "opdump",			do_opdump,			POS_DEAD,		L2,	LOG_NEVER,  1, false },
    { "owhere",			do_owhere,			POS_DEAD,		L2,	LOG_NORMAL, 1, true },
    { "rpdump",			do_rpdump,			POS_DEAD,		L2,	LOG_NEVER,  1, false },
    { "string",			do_string,  		POS_DEAD,		L2,	LOG_ALWAYS, 1, false },
    { "stupidflag",		do_stupidflag,		POS_DEAD,		L2,	LOG_ALWAYS, 1, true },
    { "violate",		do_violate,			POS_DEAD,		L2,	LOG_ALWAYS, 1, true },

	//Immortal 110
    { "spellall",		do_spellall,		POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "forall",			do_forall,			POS_DEAD,		L3,	LOG_ALWAYS, 1, false },
    { "smite",			do_smite,			POS_DEAD,		L3,	LOG_ALWAYS, 1, false },
    { "doubles",		do_doublexp,  		POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "reward",			do_gift,			POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "snoop",			do_snoop,			POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "vnum",			do_vnum,			POS_DEAD,		L3,	LOG_NORMAL, 1, true },
    { "wizinvis",		do_invis,			POS_DEAD,		L3,	LOG_NORMAL, 1, false },
    { "nochannels",		do_nochannels,		POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "memory",			do_memory,			POS_DEAD,		L3,	LOG_NORMAL, 1, true },
    { "pardon",			do_pardon,			POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "pecho",			do_pecho,			POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "zecho",			do_zecho,			POS_DEAD,		L3,	LOG_ALWAYS, 1, true },
    { "clone",			do_clone,			POS_DEAD,		L3,	LOG_ALWAYS, 1, true },

	//builder 108
    { "advance",		do_advance,			POS_DEAD,		L4,	LOG_ALWAYS, 1, true },
    { "force",			do_force,			POS_DEAD,		L4,	LOG_ALWAYS, 1, false },
    { "mpedit",			do_mpedit,			POS_DEAD,		L4,	LOG_NORMAL, 1, false },
    { "mpstat",			do_mpstat,			POS_DEAD,		L4,	LOG_NEVER,  1, false },
    { "opedit",			do_opedit,			POS_DEAD,		L4,	LOG_ALWAYS, 1, false },
    { "opstat",			do_opstat,			POS_DEAD,		L4,	LOG_NEVER,  1, false },
    { "peace",			do_peace,			POS_DEAD,		L4,	LOG_NORMAL, 1, false },
    { "restore",		do_restore,			POS_DEAD,		L4,	LOG_ALWAYS, 1, true },
    { "rpedit",			do_rpedit,			POS_DEAD,		L4,	LOG_ALWAYS, 1, false },
    { "rpstat",			do_rpstat,			POS_DEAD,		L4,	LOG_NEVER,  1, false },
    { "sla",			do_sla,				POS_DEAD,		L4,	LOG_NORMAL, 0, false },
    { "slay",			do_slay,			POS_DEAD,		L4,	LOG_ALWAYS, 1, false },

	//JBuilder 106 
	{ "map",			do_map,				POS_DEAD,		L5,	LOG_NORMAL, 1, true },
    { "alist",			do_alist,			POS_DEAD,		L5,	LOG_NORMAL, 1, true },
	{ "areareset",		do_areset,			POS_DEAD,		L5,	LOG_NORMAL,	1, false },
    { "asave",			do_asave,			POS_DEAD,		L5,	LOG_NORMAL, 1, true },
    { "edit",			do_olc,				POS_DEAD,		L5,	LOG_NORMAL, 1, false },
    { "incognito",		do_incognito,		POS_DEAD,		L5,	LOG_NORMAL, 1, false },
    { "load",			do_load,			POS_DEAD,		L5,	LOG_ALWAYS, 1, true },
    { "medit",			do_medit,			POS_DEAD,		L5,	LOG_NORMAL, 1, false },
    { "mload",			do_mload,			POS_DEAD,		L5,	LOG_ALWAYS, 1, true },
    { "mstat",			do_mstat,			POS_DEAD,		L5,	LOG_NORMAL, 1, true },
    { "oedit",			do_oedit,			POS_DEAD,		L5,	LOG_NORMAL, 1, false },
    { "oload",			do_oload,			POS_DEAD,		L5,	LOG_ALWAYS, 1, false },
    { "ostat",			do_ostat,			POS_DEAD,		L5,	LOG_NORMAL, 1, true },
    { "purge",			do_purge,			POS_DEAD,		L5,	LOG_ALWAYS, 1, false },
    { "redit",			do_redit,			POS_DEAD,		L5,	LOG_NORMAL, 1, false },
    { "resets",			do_resets,			POS_DEAD,		L5,	LOG_NORMAL, 1, true },
	{ "roomreset",		do_rreset,			POS_DEAD,		L5,	LOG_NORMAL,	1, false },
    { "slay",			do_slay,			POS_DEAD,		L5,	LOG_ALWAYS, 1, false },
    { "stat",			do_stat,			POS_DEAD,		L5,	LOG_NORMAL, 1, true },
    { "transfer",		do_transfer,		POS_DEAD,		L5,	LOG_ALWAYS, 1, false },

	//Demi
    { "spellcheck",		do_spellcheck,		POS_DEAD,		L6,	LOG_NORMAL, 1, true },
    { "echo",			do_recho,			POS_DEAD,		L6,	LOG_ALWAYS, 1, true },
	{ "goto",			do_goto,			POS_DEAD,		L6,	LOG_NORMAL, 1, false },
    { "holylight",		do_holylight,		POS_DEAD,		L6,	LOG_NORMAL, 1, true },
    { "imotd",			do_imotd,			POS_DEAD,		L6,	LOG_NORMAL, 1, true },
    { "poofin",			do_bamfin,			POS_DEAD,		L6,	LOG_NORMAL, 1, true },
    { "poofout",		do_bamfout,			POS_DEAD,		L6,	LOG_NORMAL, 1, true },
    { "sockets",		do_sockets,			POS_DEAD,		L6,	LOG_NORMAL, 1, true },

	//King
	{ "busy",			do_busy,			POS_DEAD,		L7,	LOG_NORMAL, 1, true },

	//HouseLeader
    { "scribe",			do_scribe,			POS_DEAD,		L8,	LOG_ALWAYS, 1, false },
    { ":",				do_immtalk,			POS_DEAD,		L8,	LOG_NORMAL, 0, true },
    { "immtalk",		do_immtalk,			POS_DEAD,		L8,	LOG_NORMAL, 1, true },
    { "restring",		do_lstring,  		POS_RESTING,	L8,	LOG_ALWAYS, 1, true },
    { "wizhelp",		do_wizhelp,			POS_DEAD,		L8,	LOG_NORMAL, 1, true },
    { "wiznet",			do_wiznet,			POS_DEAD,		L8,	LOG_NORMAL, 1, true },


    { "murder",			do_murder,			POS_FIGHTING,	5,	LOG_ALWAYS, 1, false },
    { "shout",			do_shout,			POS_RESTING,	NW,	LOG_NORMAL, 1, false },
    { "lifeline",		do_lifeline,		POS_DEAD,		0,	LOG_ALWAYS, 1, true },
	{ "wiznetall",		do_wiznetall,		POS_DEAD,		L5,	LOG_NORMAL, 1, true },//down here for alphabetizing reasons
    /* End of list. */
    { "",		0,		POS_DEAD,	 0,  LOG_NORMAL, 0 }
};

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MIL],logline[MIL],buf[MIL];
    int cmd,trust;
    bool found;

    // Strip leading spaces.
    while ( isspace(*argument) )
		argument++;
    if ( argument[0] == '\0' )
		return;

	//log_f("INTERP %s : %s",ch->name,argument);
    // Implement freeze command.
    if ( !IS_NPC(ch) && ch->isplr(PL_FREEZE) )
    {
		send_to_char( "You're totally frozen!\n\r", ch );
		return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    
	/*Lets see who is doing what? -Ferric*/
    strcpy( buf, argument);
    //sprintf(last_command,"%s in room[%d]: %s.",ch->name,ch->in_room->vnum, buf);
	//log_string(last_command);

	OBJ_DATA *obj,*obj_next;
	ROOM_INDEX_DATA *room;
	CHAR_DATA *tch,*mob,*mob_next;

	for(mob = ch->in_room->people;mob;mob = mob_next){
		mob_next = mob->next_in_room;
		if(IS_NPC(mob) && HAS_TRIGGER_MOB(mob,TRIG_INTERP) && mob->position == mob->pIndexData->default_pos)
			p_act_trigger(argument,mob,NULL,NULL,ch,NULL,NULL,TRIG_INTERP);
		for(obj = mob->carrying;obj;obj = obj_next){
			obj_next = obj->next_content;
			if(HAS_TRIGGER_OBJ(obj,TRIG_INTERP))
				p_act_trigger(argument,NULL,obj,NULL,ch,NULL,NULL,TRIG_INTERP);
		}
	}
	for(obj = ch->in_room->contents;obj;obj = obj_next){
		obj_next = obj->next_content;
		if(HAS_TRIGGER_OBJ(obj,TRIG_INTERP))
			p_act_trigger(buf,NULL,obj,NULL,ch,NULL,NULL,TRIG_INTERP);
	}

	for(mob = ch->in_room->people;mob;mob = mob_next){
		mob_next = mob->next_in_room;

		for(obj = mob->carrying;obj;obj = obj_next){
			obj_next = obj->next_content;
			if(HAS_TRIGGER_OBJ( obj, TRIG_INTERP))
				p_act_trigger(buf,NULL,obj,NULL,ch,NULL,NULL,TRIG_INTERP);
		}
	}

	if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_INTERP))
		p_act_trigger(buf,NULL,NULL,ch->in_room,ch,NULL,NULL,TRIG_INTERP);

    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while ( isspace(*argument) )
			argument++;
    }
    else
		argument = one_argument( argument, command );

    /* Look for command in command table. */
    found = false;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
		if ( command[0] == cmd_table[cmd].name[0] && !str_prefix(command,cmd_table[cmd].name) && cmd_table[cmd].level <= trust )
		{
			found = true;
			break;
		}
    }

    /* Log and snoop.*/
    if ( cmd_table[cmd].log == LOG_NEVER )
		strcpy( logline, "" );

    if ( ( !IS_NPC(ch) && ch->isplr(PL_LOG) ) || fLogAll || cmd_table[cmd].log == LOG_ALWAYS )
    {
		sprintf( log_buf, "Log %s: %s", ch->name, logline );
		wiznet(log_buf,ch,NULL,WZ_SECURE,0,get_trust(ch));
		log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
		write_to_buffer( ch->desc->snoop_by, "% ",    2 );
		write_to_buffer( ch->desc->snoop_by, logline, 0 );
		write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
		/* Look for command in socials table. */
		if (!check_social(ch,command,argument))// &&   !imc_command_hook( ch,command, argument ))
			send_to_char("Huh?\n\r",ch);
		return;

    }

    /*
     * Character not in position for command?
     
	if (ch->bashwait > 0)
	{
		send_to_char("Better stand up first.\n\r",ch);
		return;
	}*/

	if(!cmd_table[cmd].is_system && ch->spelltimer > 0){
		if(ch->isaff(AF_RAVAGE)){
			if(number_percent() < 50){
				ch->send("Negative energy surges around you, and your body screams in pain!\n\r");
				damage(ch,ch,ch->hit / 10,skill_lookup("ravage"),DAM_NEGATIVE,true);
				WAIT_STATE(ch,10);
			}
		}
		ch->send("You interrupt your pending spell..\n\r");
		ch->spelltimer = 0;
		ch->spellfailed = 0;
		ch->spellsn = 0;
		ch->spellvo = NULL;
		ch->spellvictim = NULL;
		ch->spelltarget = 0;
	}

    if ( ch->position < cmd_table[cmd].position )
    {
		switch( ch->position )
		{
			case POS_DEAD:
				send_to_char( "Lie still; you are DEAD.\n\r", ch );
				break;
			case POS_MORTAL:
			case POS_INCAP:
				send_to_char( "You are hurt far too bad for that.\n\r", ch );
				break;
			case POS_STUNNED:
				send_to_char( "You are too stunned to do that.\n\r", ch );
				break;
			case POS_SLEEPING:
				send_to_char( "In your dreams, or what?\n\r", ch );
				break;
			case POS_RESTING:
				send_to_char( "Nah... You feel too relaxed...\n\r", ch);
				break;
			case POS_SITTING:
				send_to_char( "Better stand up first.\n\r",ch);
				break;
			case POS_FIGHTING:
				send_to_char( "No way!  You are still fighting!\n\r", ch);
				break;
		}
		return;
    }

    /*
     * Dispatch the command.
     */
	(*cmd_table[cmd].do_fun) (ch, argument);

	tail_chain ();
	return;
}

/* function to keep argument safe in all commands -- no static strings */
void do_function(CHAR_DATA *ch, DO_FUN *do_fun,char *argument){
    char *command_string;
    
    /* copy the string */
    command_string = str_dup(argument);
    
    /* dispatch the command */
    (*do_fun) (ch, command_string);
    
    /* free the string */
    free_string(command_string);
}
    
bool check_social(CHAR_DATA *ch,char *command,char *argument){
    char arg[MIL];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = false;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
		if ( command[0] == social_table[cmd].name[0] && !str_prefix(command,social_table[cmd].name) )
		{
			found = true;
			break;
		}
    }

    if ( !found )
		return false;

    if ( !IS_NPC(ch) && ch->iscomm(CM_NOEMOTE) )
    {
		send_to_char( "You are anti-social!\n\r", ch );
		return true;
    }

    switch ( ch->position )
    {
		case POS_DEAD:
			send_to_char( "Lie still; you are DEAD.\n\r", ch );
			return true;
		case POS_INCAP:
		case POS_MORTAL:
			send_to_char( "You are hurt far too bad for that.\n\r", ch );
			return true;
		case POS_STUNNED:
			send_to_char( "You are too stunned to do that.\n\r", ch );
			return true;
		case POS_SLEEPING:
			if ( !str_cmp( social_table[cmd].name, "snore" ) )
				break;
			send_to_char( "In your dreams, or what?\n\r", ch );
			return true;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
		act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
		act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
		send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
		act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
		act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
		if(!str_cmp(victim->name,"Nash") && !str_prefix(social_table[cmd].name,"laces")){
			ch->send("No such luck.\n\r");
			return true;
		}
		act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
		act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
		act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );
/*
		if ( !IS_NPC(ch) && IS_NPC(victim)
		&&   !victim->isaff(AF_CHARM)
		&&   IS_AWAKE(victim) 
		&&   victim->desc == NULL)
		{
			switch ( number_bits( 4 ) )
			{
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act( social_table[cmd].others_found, victim, NULL, ch, TO_NOTVICT );
				act( social_table[cmd].char_found, victim, NULL, ch, TO_CHAR    );
				act( social_table[cmd].vict_found, victim, NULL, ch, TO_VICT    );
				break;

			case 9: case 10: case 11: case 12:
				act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
				act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
				act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
				break;
			}
		}*/
    }

    return true;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number(char *arg){
    if(!(*arg))
        return false;

    if(*arg == '+' || *arg == '-')
        arg++;

    for (;*arg != '\0';arg++)
        if (!isdigit(*arg))
            return false;
    return true;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument(char *argument,char *arg){
	char *pdot;
	int number;

	for (pdot = argument;*pdot;pdot++){
		if (*pdot == '.'){
			*pdot = '\0';
			number = atoi(argument);
			*pdot = '.';
			strcpy(arg,pdot + 1);
			return number;
		}
	}

	strcpy(arg,argument);
	return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument,char *arg){
	char *pdot;
	int number;

	for(pdot = argument;*pdot != '\0';pdot++){
		if(*pdot == '*'){
			*pdot = '\0';
			number = atoi(argument);
			*pdot = '*';
			strcpy(arg,pdot+1);
			return number;
		}
	}

	strcpy(arg,argument);
	return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
		argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

    while ( *argument != '\0' )
    {
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*arg_first = LOWER(*argument);
		arg_first++;
		argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
		argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    int cmd, col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO && cmd_table[cmd].level <= get_trust(ch) && cmd_table[cmd].show)
		{
			printf_to_char(ch,"%-15s",cmd_table[cmd].name);
			if ( ++col % 5 == 0 )
				send_to_char("\n\r",ch);
		}
    }
 
    if ( col % 5 != 0 )
		send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    int cmd, col, cLevel;
	bool found;
 
	for (cLevel = 101; cLevel <= ML;++cLevel)
	{
		found=false;
		col = 0;
		for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
		{
			if ( cmd_table[cmd].level == cLevel && (cmd_table[cmd].level <= get_trust(ch) || cmd_table[cmd].level <= ch->level) && cmd_table[cmd].show)
			{
				if(!found)
				{
					found=true;
					printf_to_char(ch,"{dLevel: %d{x\n\r",cLevel);
				}
				printf_to_char(ch,"%-12s", cmd_table[cmd].name);
					if ( ++col % 6 == 0 )
				send_to_char( "\n\r", ch );
			}
		}
		if(found)
			send_to_char("\n\r",ch);
	}
 
    if ( col % 6 != 0 )
		send_to_char( "\n\r", ch );
    return;
}
