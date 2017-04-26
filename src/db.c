#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "music.h"
#include "lookup.h"
#include "olc.h"

extern	int	_filbuf		( FILE* );

#if !defined(OLD_RAND)
void srandom(unsigned int);
time_t time(time_t *tloc);
#endif

/* externals for counting purposes */
extern	OBJ_DATA	*obj_free;
extern	CHAR_DATA	*char_free;
extern  DESCRIPTOR_DATA *descriptor_free;
extern	PC_DATA		*pcdata_free;
extern  AFFECT_DATA	*affect_free;

/* Globals.*/
HELP_DATA *	help_first;
HELP_DATA *	help_last;
HELP_AREA *	had_list;
SHOP_DATA *	shop_first;
SHOP_DATA *	shop_last;
NOTE_DATA *	note_free;
PROG_CODE *	mprog_list;
PROG_CODE *	oprog_list;
PROG_CODE *	rprog_list;

char			bug_buf[2*MIL];
CHAR_DATA *		char_list;
QUEST_DATA *	quest_list;
char *			help_greeting;
char			log_buf[2*MIL];
KILL_DATA		kill_table[MAX_LEVEL];
int				current_weather[MAX_AREATYPES];

OBJ_DATA *		object_list;
TIME_INFO_DATA	time_info;
MUD_TYPE		mud;
WEATHER_DATA	weather_info;

sh_int gsn_dispersion_field;
sh_int gsn_shunt;
sh_int gsn_eimbue;
sh_int gsn_detect_invis;
sh_int gsn_energy_crash;
sh_int gsn_reject_magic;
sh_int gsn_occult_wisdom;
sh_int gsn_will_force;
sh_int gsn_mortal_coil;
sh_int gsn_abjuring_intellect;
sh_int gsn_assassinate;
sh_int gsn_paranoia;
sh_int gsn_ambush;
sh_int gsn_backfist;
sh_int gsn_backstab;
sh_int gsn_dodge;
sh_int gsn_caltrops;
sh_int gsn_mounteddodge;
sh_int gsn_feint;
sh_int gsn_feralslash;
sh_int	gsn_sidestep;
sh_int	gsn_rebound;
sh_int	gsn_ricochet;
sh_int	gsn_evade;
sh_int	gsn_duck;
sh_int	gsn_mark_of_the_beast;
sh_int	gsn_primal_roar;
sh_int	gsn_rabid;
sh_int	gsn_envenom;
sh_int	gsn_lore;
sh_int	gsn_falconry;
sh_int	gsn_graverob;
sh_int	gsn_preparationattack;
sh_int	gsn_ready;
sh_int	gsn_hide;
sh_int	gsn_penumbralveil;
sh_int	gsn_peek;
sh_int	gsn_pick_lock;
sh_int	gsn_sneak;
sh_int	gsn_sonic_impact;
sh_int	gsn_fuse;
sh_int	gsn_alertness;
sh_int	gsn_awareness;
sh_int	gsn_survival;
sh_int	gsn_firstaid;
sh_int	gsn_blend;
sh_int	gsn_drunkfighting;
sh_int	gsn_leadership;
sh_int	gsn_overdrive;
sh_int	gsn_outmaneuver;
sh_int	gsn_desertcover;
sh_int	gsn_steal;
sh_int	gsn_swap;
sh_int	gsn_vanish;
sh_int	gsn_weapons_knowledge;
sh_int	gsn_slip;
sh_int	gsn_mithril_bash;
sh_int	gsn_palm;
sh_int	gsn_cutpurse;
sh_int	gsn_counter;
sh_int	gsn_counter_spin;
sh_int	gsn_riposte;
sh_int	gsn_coule;
sh_int	gsn_critical;
sh_int	gsn_cast_efficiency;
sh_int	gsn_mental_focus;
sh_int	gsn_magical_mastery;
sh_int	gsn_elemental_mastery;
sh_int	gsn_spell_proficiency;
sh_int	gsn_spellcraft;
sh_int	gsn_arcane_knowledge;
sh_int	gsn_disarm;
sh_int	gsn_swipe;
sh_int	gsn_enhanced_damage;
sh_int	gsn_embalm;
sh_int	gsn_poison_ivy;
sh_int	gsn_defender;
sh_int	gsn_parting_shot;
sh_int	gsn_improved_accuracy;
sh_int	gsn_heavy_hand;
sh_int	gsn_martial_arts;
sh_int	gsn_rage;
sh_int	gsn_swordefficiency;
sh_int	gsn_swordsmanship;
sh_int	gsn_swordmastery;
sh_int	gsn_deathblow;
sh_int	gsn_lancemastery;
sh_int	gsn_kick;
sh_int	gsn_kindred_spirits;
sh_int	gsn_kickaxe;
sh_int	gsn_kickroundhouse;
sh_int	gsn_kickhigh;
sh_int	gsn_kicklow;
sh_int	gsn_cleave;
sh_int	gsn_punch;
sh_int	gsn_punchuppercut;
sh_int	gsn_punchcross;
sh_int	gsn_punchjab;
sh_int	gsn_punchtigerfang;
sh_int	gsn_unending_strikes;
sh_int	gsn_fist_of_fury;
sh_int	gsn_dragon_fist;
sh_int	gsn_lion_combo;
sh_int	gsn_fierce_palm;
sh_int	gsn_twin_tiger_fang;
sh_int	gsn_flurry;
sh_int	gsn_strike;
sh_int	gsn_crush;
sh_int	gsn_mighty_swing;
sh_int	gsn_smash;
sh_int	gsn_heavy_smash;
sh_int	gsn_pommel_strike;
sh_int	gsn_shieldstrike;
sh_int	gsn_shadowstrike;
sh_int	gsn_sharpen;
sh_int	gsn_paralyzingfear;
sh_int	gsn_conceal;
sh_int	gsn_culmination;
sh_int	gsn_traps;
sh_int	gsn_downstrike;
sh_int	gsn_trample;
sh_int	gsn_final_strike;
sh_int	gsn_facekick;
sh_int	gsn_sap;
sh_int	gsn_strangle;
sh_int	gsn_bandage;
sh_int	gsn_parry;
sh_int	gsn_trompement;
sh_int	gsn_absorb;
sh_int	gsn_damage_reduction;
sh_int	gsn_lead_stance;
sh_int	gsn_spell_resistance;
sh_int	gsn_elude;
sh_int	gsn_heavy_armor;
sh_int	gsn_medium_armor;
sh_int	gsn_light_armor;
sh_int	gsn_rescue;
sh_int	gsn_guard;
sh_int	gsn_fortify;
sh_int	gsn_grasp;
sh_int	gsn_barb;
sh_int	gsn_grip;
sh_int	gsn_second_attack;
sh_int	gsn_third_attack;
sh_int	gsn_dual_wield;
sh_int	gsn_marksmanship;
sh_int	gsn_blindness;
sh_int	gsn_charm_person;
sh_int	gsn_curse;
sh_int	gsn_invis;
sh_int	gsn_mass_invis;
sh_int	gsn_poison;
sh_int	gsn_venom;
sh_int	gsn_morph_serpent;
sh_int	gsn_morph_canine;
sh_int	gsn_morph_feline;
sh_int	gsn_morph_ursa;
sh_int	gsn_morph_avian;
sh_int	gsn_morph_aquatic;
sh_int	gsn_phase;
sh_int	gsn_tranquility;
sh_int	gsn_animal_magnetism;
sh_int	gsn_earth_wisdom;
sh_int	gsn_resist_blights;
sh_int	gsn_salve;
sh_int	gsn_lurk;
sh_int	gsn_slither;
sh_int	gsn_maul;
sh_int	gsn_trail;
sh_int	gsn_pounce;
sh_int	gsn_scratch;
sh_int	gsn_clench;
sh_int	gsn_craze;
sh_int	gsn_aerial_strike;
sh_int	gsn_ascend;
sh_int	gsn_ravage;
sh_int	gsn_catalepsy;
sh_int	gsn_nightmare;
sh_int	gsn_nausea;
sh_int	gsn_plague;
sh_int	gsn_malady;
sh_int	gsn_rupture;
sh_int	gsn_narcolepsy;
sh_int	gsn_sleep;
sh_int	gsn_sanctuary;
sh_int	gsn_prismshotarray;
sh_int	gsn_spellshield;
sh_int	gsn_fly;
sh_int	gsn_trueshot;
sh_int	gsn_strafe;
sh_int	gsn_sharp_shooting;
sh_int	gsn_headshot;
sh_int	gsn_stalk;
sh_int	gsn_doubleshot;
sh_int	gsn_arrow_shower;
sh_int	gsn_raptor;
sh_int	gsn_stamina;
sh_int	gsn_arrow_infusion;
sh_int	gsn_instinct;
sh_int	gsn_exotic;
sh_int	gsn_clubs;
sh_int	gsn_swords;
sh_int	gsn_daggers;
sh_int	gsn_polearms;
sh_int	gsn_bludgeons;
sh_int	gsn_cleavers;
sh_int	gsn_lashes;
sh_int	gsn_bows;
sh_int	gsn_martial_arms;
sh_int	gsn_staffs;
sh_int	gsn_sabers;
sh_int	gsn_shortswords;
sh_int	gsn_lances;

sh_int	gsn_bash;
sh_int	gsn_shieldbash;
sh_int	gsn_shieldtoss;
sh_int	gsn_shieldmastery;
sh_int	gsn_shieldfighting;
sh_int	gsn_shift;
sh_int	gsn_divert;
sh_int	gsn_eavesdrop;
sh_int	gsn_devour;
sh_int	gsn_devotion;
sh_int	gsn_untangle;
sh_int	gsn_throwingknife;
sh_int	gsn_throwingdart;
sh_int	gsn_throwingaxe;
sh_int	gsn_piety;
sh_int	gsn_dogma;
sh_int	gsn_sacred_guardian;
sh_int	gsn_faith;
sh_int	gsn_reverence;
sh_int	gsn_true_belief;
sh_int	gsn_poverty;
sh_int	gsn_florentine;
sh_int	gsn_poisoncloud;
sh_int	gsn_blindingdust;
sh_int	gsn_endure;
sh_int	gsn_durability;
sh_int	gsn_bravery;
sh_int	gsn_icebound;
sh_int	gsn_disappear;
sh_int	gsn_fervor;
sh_int	gsn_resilience;
sh_int	gsn_hydrobiology;
sh_int	gsn_thalassic_aura;
sh_int	gsn_frozen_blood;
sh_int	gsn_amiability;
sh_int	gsn_return;
sh_int	gsn_regeneration;
sh_int	gsn_introversion;
sh_int	gsn_innerpeace;
sh_int	gsn_fluidmotion;
sh_int	gsn_berserk;
sh_int	gsn_reckless_abandon;
sh_int	gsn_heightened_senses;
sh_int	gsn_catnap;
sh_int	gsn_counterfeit;
sh_int	gsn_distract;
sh_int	gsn_stealth;
sh_int	gsn_mark;
sh_int	gsn_gouge;
sh_int	gsn_needle;
sh_int	gsn_nerve;
sh_int	gsn_neural_agent;
sh_int	gsn_dual_backstab;
sh_int	gsn_engarde;
sh_int	gsn_feedback;
sh_int	gsn_dirt;
sh_int	gsn_mounteddirt;
sh_int	gsn_disabletraps;
sh_int	gsn_detecttraps;
sh_int	gsn_combatives;
sh_int	gsn_trip;
sh_int	gsn_holytouch;
sh_int	gsn_zeal;
sh_int	gsn_divinetouch;
sh_int	gsn_divinity;
sh_int	gsn_purity;
sh_int	gsn_mustang_heart;
sh_int	gsn_lastrites;
sh_int	gsn_kneeshot;
sh_int	gsn_snare;
sh_int	gsn_fast_healing;
sh_int	gsn_lance_fighting;
sh_int	gsn_haggle;
sh_int	gsn_meditation;
sh_int	gsn_mirage;
sh_int	gsn_pierce;
sh_int	gsn_scrolls;
sh_int	gsn_wands;
sh_int	gsn_recall;
sh_int	gsn_riding;
sh_int	gsn_horsemanship;
sh_int	gsn_charge;
sh_int	gsn_hunt;
sh_int	gsn_quickescape;
sh_int	gsn_smelt;
sh_int	gsn_whirlwind;
sh_int	gsn_dualwind;
sh_int	gsn_dual_shield;
sh_int	gsn_shield_block;
sh_int	gsn_improvise;
sh_int	gsn_brawler;
sh_int	gsn_blind_fighting;
sh_int	gsn_combat_proficiency;
sh_int	gsn_furor;
sh_int	gsn_paired_attack;
sh_int	gsn_double_blitz;
sh_int	gsn_batter;
sh_int	gsn_bulwark;
sh_int	gsn_defensive_stance;
sh_int	gsn_warrior_heart;
sh_int	gsn_thrust;
sh_int	gsn_cavalry;
sh_int	gsn_leverage;
sh_int	gsn_numbing_force;
sh_int	gsn_withdraw;
sh_int	gsn_concussive_blow;
sh_int	gsn_sword_blitz;
sh_int	gsn_cranial_strike;
sh_int	gsn_slashing_volley;
sh_int	gsn_rising_cleave;
sh_int	gsn_divider;
sh_int	gsn_lop;
sh_int	gsn_fissure;
sh_int	gsn_rift;
sh_int	gsn_hack;
sh_int	gsn_breach;
sh_int	gsn_destrier;
sh_int	gsn_spiraling_spear;
sh_int	gsn_spur;
sh_int	gsn_equestrian;
sh_int	gsn_courser;
sh_int	gsn_charger;
sh_int	gsn_barding;
sh_int	gsn_hobby;
sh_int	gsn_errant;
sh_int	gsn_hotblooded_horse;
sh_int	gsn_coldblooded_horse;
sh_int	gsn_daurin;
sh_int	gsn_warcry;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];
char *				string_hash			[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;
AREA_DATA *		current_area;

char *			string_space;
char *			top_string;
char			str_empty	[1];

int top_web_desc;
int	top_affect;
int	top_area;
int	top_ed;
int	top_exit;
int	top_help;
int	top_mob_index;
int	top_obj_index;
int	top_reset;
int	top_room;
int	top_shop;
int	top_vnum_room;
int	top_vnum_mob;
int	top_vnum_obj;
int	top_mprog_index;
int	mobile_count = 0;
int	newmobs = 0;
int	newobjs = 0;
int	top_oprog_index;
int	top_rprog_index;

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define			MAX_STRING	1413120
#define			MAX_PERM_BLOCK	131072

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};

int			nAllocString;
int			sAllocString;
int			nAllocPerm;
int			sAllocPerm;


/* Semi-locals.*/
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];

/* Local booting procedures.*/
void cql_init			( );
void fread_lockers		( );
void room_update		( AREA_DATA* );
void init_mm			( void );
void load_area			( FILE* );
void new_load_area		( FILE* );
void load_helps			( FILE*, char* );
void load_mobiles		( FILE* );
void load_old_obj		( FILE* );
void load_objects		( FILE* );
void load_resets		( FILE* );
void load_rooms			( FILE* );
void load_shops			( FILE* );
void load_specials		( FILE* );
void load_bans			( void );
void load_mobprogs		( FILE* );
void fix_exits			( void );
void fix_mobprogs		( void );
void reset_area			( AREA_DATA* );
void load_objprogs		( FILE* );
void load_roomprogs		( FILE* );
void fix_objprogs		( void );
void fix_roomprogs		( void );
void load_max_con		( void );
void write_max_con		( );
void init_aspell		(  );

void boot_db(){
	int sn,i;

	if ((string_space = (char *) calloc(1,MAX_STRING)) == NULL){
		bug("Boot_db: can't alloc %d string space.",MAX_STRING);
		exit(1);
	}
	top_string	= string_space;
	fBootDb		= true;

	init_mm();

    /* Set time and weather.*/
	long lhour, lday, lmonth;

	lhour			= (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	time_info.ticks = 0;
	lday			= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth			= lday   / 35;
	time_info.month	= lmonth % 12;
	time_info.year	= lmonth / 12;
	mud.t_con		= 0;
	mud.d_con		= 0;
	mud.max_con		= 0;

	log_string("Initializing database.");
	cql_init();

		 if (time_info.hour <  5)	weather_info.sunlight = SUN_DARK;
	else if (time_info.hour <  6)	weather_info.sunlight = SUN_RISE;
	else if (time_info.hour < 19)	weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hour < 20)	weather_info.sunlight = SUN_SET;
	else							weather_info.sunlight = SUN_DARK;

	for (sn = 0; sn < MAX_AREATYPES; sn++)
		current_weather[sn] = SKY_CLOUDLESS;

	weather_info.change	= 0;
	weather_info.sky = SKY_CLOUDLESS;

    /*
     * Assign gsn's and levels for skills which have them.
     */

	for ( sn = 0; sn < MAX_SKILL; sn++ ){
		if (skill_table[sn].pgsn != NULL)
			*skill_table[sn].pgsn = sn;
		for (i = 0; i < MAX_CLASS; i++)
			skill_table[sn].skill_level[i] = LEVEL_IMMORTAL;
	}

	log_string("Loading db");
	cql_load_db();
	log_string("Loading areas");
	FILE *fpList;

	if(!(fpList = fopen(AREA_LIST,"r"))){
		perror(AREA_LIST);
		exit(1);
	}

	for(;;){
		strcpy(strArea,fread_word(fpList));
		if (strArea[0] == '$')
			break;
		if (strArea[0] == '-')
			fpArea = stdin;
		else if (!(fpArea = fopen(strArea,"r"))){
			perror(strArea);
			exit(1);
		}

		current_area = NULL;

		for(;;){
			char *word;

			if (fread_letter(fpArea) != '#'){
				bug("Boot_db: # not found.",0);
				exit(1);
			}

			word = fread_word(fpArea);

				 if (word[0] == '$')				break;
			else if (!str_cmp(word,"AREA"))			load_area    (fpArea);
			else if (!str_cmp(word,"AREADATA"))		new_load_area(fpArea);
			else if (!str_cmp(word,"HELPS"))		load_helps   (fpArea,strArea);
			else if (!str_cmp(word,"MOBILES"))		load_mobiles (fpArea);
			else if (!str_cmp(word,"MOBPROGS"))		load_mobprogs(fpArea);
			else if (!str_cmp(word,"OBJPROGS"))		load_objprogs(fpArea);
			else if (!str_cmp(word,"ROOMPROGS"))	load_roomprogs(fpArea);
			else if (!str_cmp(word,"OBJOLD"))		load_old_obj (fpArea);
	  		else if (!str_cmp(word,"OBJECTS"))		load_objects (fpArea);
			else if (!str_cmp(word,"RESETS"))		load_resets  (fpArea);
			else if (!str_cmp(word,"ROOMS"))		load_rooms   (fpArea);
			else if (!str_cmp(word,"SHOPS"))		load_shops   (fpArea);
			else if (!str_cmp(word,"SPECIALS"))		load_specials(fpArea);
			else{
				bug("Boot_db: bad section name.",0);
				exit(1);
			}
		}

		if(fpArea != stdin)
			fclose(fpArea);
		fpArea = NULL;
	}
	fclose(fpList);

	log_string("Fix shite");
    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the songs, notes and ban files.
     */
	init_aspell();
	fix_exits();
	fix_mobprogs();
	fix_objprogs();
	fix_roomprogs();
	fBootDb	= false;
    convert_objects();

	log_string("Loading lockers");
	fread_lockers();

	area_update();
	load_boards();
	save_notes();
	load_bans();
	load_social_table();

    load_max_con();
    return;
}

/* Snarf an 'area' header line.*/
void load_area(FILE *fp)
{
    AREA_DATA *pArea;
	char buf[MSL];

    pArea				= (AREA_DATA *)alloc_perm(sizeof(*pArea));
    pArea->file_name	= fread_string(fp);
	
	sprintf(buf,"%s",pArea->file_name);
	log_string(buf);

	pArea->climate		= CLIMATE_ARID;
    pArea->area_flags   = AREA_LOADING;
    pArea->security     = 9;
    pArea->builders     = str_dup("None");
    pArea->vnum         = top_area;

    pArea->name			= fread_string(fp);
    pArea->credits		= fread_string(fp);
    pArea->min_vnum		= fread_number(fp);
    pArea->max_vnum		= fread_number(fp);
    pArea->age			= 15;
    pArea->nplayer		= 0;
	pArea->arenaviewervnum=0;
    pArea->empty		= false;

    if (!area_first)
		area_first = pArea;
    if (area_last)
    {
		area_last->next = pArea;
        REMOVE_BIT(area_last->area_flags,AREA_LOADING);
    }
    area_last		= pArea;
    pArea->next		= NULL;
    current_area	= pArea;

    top_area++;
    return;
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY(literal,field,value) if (!str_cmp(word,literal)) { field  = value; fMatch = true; break; }

#define SKEY(string,field) if (!str_cmp(word,string)) { free_string(field); field = fread_string(fp); fMatch = true; break; }

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void new_load_area(FILE *fp){
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    pArea					= (AREA_DATA *)alloc_perm(sizeof(*pArea));
    pArea->age				= 15;
    pArea->nplayer			= 0;
    pArea->arenaviewervnum	= 0;
    pArea->file_name		= str_dup(strArea);
    pArea->vnum				= top_area;
    pArea->name				= str_dup("New Area");
    pArea->builders			= str_dup("");
    pArea->security			= 9;
    pArea->min_vnum			= 0;
    pArea->max_vnum			= 0;
    pArea->low_range		= 1;
    pArea->high_range		= 100;
    pArea->area_flags		= 0;
    pArea->finished			= 0;
    pArea->area_group		= 0;
    pArea->locked			= false;
	pArea->climate			= CLIMATE_TEMPERATE;

	for ( ; ; ){
		word   = feof(fp) ? (char*)"End" : fread_word(fp);
		fMatch = false;

		switch (UPPER(word[0])){
			case 'E':
				if (!str_cmp(word,"End")){
					fMatch = true;
					if (area_first == NULL)
						area_first		= pArea;
					if (area_last  != NULL)
						area_last->next = pArea;
					area_last			= pArea;
					pArea->next			= NULL;
					current_area		= pArea;
					top_area++;

					return;
				}
				break;
			case 'B':
				SKEY("Builders",pArea->builders);
				break;
			case 'C':
				KEY("Climate",pArea->climate,fread_number(fp));
				SKEY("Credits",pArea->credits);
				break;
			case 'F':
				KEY("Finished",pArea->finished,fread_number(fp));
				break;
			case 'G':
				KEY("GROUP",pArea->area_group,fread_number(fp));
				break;
			case 'L':
				KEY("Locked",pArea->locked,fread_number(fp));
				if (!str_cmp(word,"LRNGE")){
					pArea->low_range = fread_number(fp);
					pArea->high_range = fread_number(fp);
				}
				break;
			case 'N':
				SKEY("Name",pArea->name);
				break;
			case 'S':
				KEY("Security",pArea->security,fread_number(fp));
				break;
			case 'V':
				if (!str_cmp(word,"VNUMs"))
				{
					pArea->min_vnum = fread_number(fp);
					pArea->max_vnum = fread_number(fp);
				}
				break;
		}
	}
}

/* Sets vnum range for area using OLC protection features.*/
void assign_area_vnum(int vnum)
{
    if (area_last->min_vnum == 0 || area_last->max_vnum == 0)
        area_last->min_vnum = area_last->max_vnum = vnum;
    if (vnum != URANGE(area_last->min_vnum, vnum,area_last->max_vnum))
	{
        if (vnum < area_last->min_vnum)
            area_last->min_vnum = vnum;
        else
            area_last->max_vnum = vnum;
	}
    return;
}

/* Snarf a help section.*/
void load_helps(FILE *fp,char *fname)
{
    HELP_DATA *pHelp;
    int level;
    char *keyword;

    for ( ; ; )
    {
		HELP_AREA * had;

		level		= fread_number(fp);
		keyword		= fread_string(fp);

		if (keyword[0] == '$')
			break;

		if (!had_list)
		{
			had			= new_had ();
			had->filename		= str_dup(fname);
			had->area		= current_area;
			if (current_area)
				current_area->helps	= had;
			had_list		= had;
		}
		else if (str_cmp(fname,had_list->filename))
		{
			had			= new_had ();
			had->filename		= str_dup(fname);
			had->area		= current_area;
			if (current_area)
				current_area->helps	= had;
			had->next		= had_list;
			had_list		= had;
		}
		else
			had			= had_list;

		pHelp		= new_help();
		pHelp->level	= level;
		pHelp->keyword	= keyword;
		pHelp->text	= fread_string(fp);

		if (!str_cmp( pHelp->keyword,"greeting"))
			help_greeting = pHelp->text;

		if (help_first == NULL)
			help_first = pHelp;
		if (help_last  != NULL)
			help_last->next = pHelp;

		help_last		= pHelp;
		pHelp->next		= NULL;

		if (!had->first)
			had->first	= pHelp;
		if (!had->last)
			had->last	= pHelp;

		had->last->next_area	= pHelp;
		had->last		= pHelp;
		pHelp->next_area	= NULL;

		top_help++;
    }

    return;
}

/* Snarf an obj section.  old style     //Removed and replaced with an error function because who ports in merc areas*/
void load_old_obj(FILE *fp)
{
    log_string("Bad area file loaded");
    return;
}

void new_reset(ROOM_INDEX_DATA *pR,RESET_DATA *pReset)
{
    RESET_DATA *pr;

    if (!pR)
       return;

    pr = pR->reset_last;

    if (!pr)
    {
        pR->reset_first = pReset;
        pR->reset_last  = pReset;
    }
    else
    {
        pR->reset_last->next = pReset;
        pR->reset_last       = pReset;
        pR->reset_last->next = NULL;
    }

/*    top_reset++; no estamos asignando memoria!!!! */

    return;
}

void load_resets(FILE *fp)
{
    RESET_DATA *pReset;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *pRoomIndex;
    int rVnum = -1;

    if (!area_last)
    {
		bug("Load_resets: no #AREA seen yet.",0);
		exit(1);
    }

    for ( ; ; )
    {
		char letter;

		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*')
		{
			fread_to_eol(fp);
			continue;
		}

		pReset		= new_reset_data();
		pReset->command	= letter;
		/* if_flag */	  fread_number(fp);
		pReset->arg1	= fread_number(fp);
		pReset->arg2	= fread_number(fp);
		pReset->arg3	= (letter == 'G' || letter == 'R') ? 0 : fread_number(fp);
		pReset->arg4	= (letter == 'P' || letter == 'M') ? fread_number(fp) : 0;
				  fread_to_eol(fp);

		switch( pReset->command )
		{
			case 'M':
			case 'O':
				rVnum = pReset->arg3;
				break;
			case 'P':
			case 'G':
			case 'E':
				break;
			case 'D':
				pRoomIndex = get_room_index( (rVnum = pReset->arg1) );
				if ( pReset->arg2 < 0 ||   pReset->arg2 >= MAX_DIR || !pRoomIndex || !( pexit = pRoomIndex->exit[pReset->arg2] ) || !IS_SET( pexit->rs_flags, EX_ISDOOR ) )
				{
					bugf("Load_resets: 'D': exit %d, room %d not door.",pReset->arg2,pReset->arg1);
					exit(1);
				}

				switch (pReset->arg3)
				{
					default:
						bug("Load_resets: 'D': bad 'locks': %d.",pReset->arg3);
						break;
					case 0:
						break;
					case 1:
						SET_BIT(pexit->rs_flags,EX_CLOSED);
						SET_BIT(pexit->exit_info,EX_CLOSED);
						break;
					case 2:
						SET_BIT(pexit->rs_flags,EX_CLOSED|EX_LOCKED);
						SET_BIT(pexit->exit_info,EX_CLOSED|EX_LOCKED);
						break;
				}
				break;

			case 'R':
				rVnum = pReset->arg1;
				break;
		}

		if (rVnum == -1)
		{
 			bugf("load_resets : rVnum == -1");
 			exit(1);
 		}

		if (pReset->command != 'D')
			new_reset(get_room_index(rVnum),pReset);
		else
			free_reset_data(pReset);
    }

    return;
}

void load_rooms(FILE *fp){
	int i;
	ROOM_INDEX_DATA *pRoomIndex;

	if (area_last == NULL){
		bug("Load_resets: no #AREA seen yet.",0);
		exit(1);
	}

	for ( ; ; ){
		sh_int vnum;
		char letter;
		int door, iHash;

		letter				= fread_letter(fp);
		if (letter != '#'){
			bug("Load_rooms: # not found.",0);
			exit(1);
		}

		vnum				= fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = false;
		if (get_room_index(vnum) != NULL){
			bug("Load_rooms: vnum %d duplicated.",vnum);
			exit(1);
		}
		fBootDb = true;

		pRoomIndex					= (ROOM_INDEX_DATA *) alloc_perm(sizeof(*pRoomIndex));
		pRoomIndex->owner			= str_dup("");
		pRoomIndex->people			= NULL;
		pRoomIndex->contents		= NULL;
		pRoomIndex->extra_descr		= NULL;
		pRoomIndex->arenavn			= 0;
		pRoomIndex->area			= area_last;
		pRoomIndex->affected		= NULL;
		pRoomIndex->vnum			= vnum;
		pRoomIndex->name			= fread_string(fp);
		pRoomIndex->description		= fread_string(fp);
		/* Area number */			  fread_number(fp);
		pRoomIndex->room_flags		= fread_flag(fp);
		/* horrible hack */
  		if ( 3000 <= vnum && vnum < 3400)
		   SET_BIT(pRoomIndex->room_flags,ROOM_LAW);
		pRoomIndex->sector_type		= fread_number(fp);

		pRoomIndex->light		= 0;
		for ( door = 0; door <= 5; door++ )
			pRoomIndex->exit[door]	= NULL;

		/* defaults */
		pRoomIndex->heal_rate		= 100;
		pRoomIndex->mana_rate		= 100;

		for ( ; ; ){
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			if (letter == 'A')
				pRoomIndex->arenavn = fread_number(fp);
			else if (letter == 'V')
				pRoomIndex->arenaviewvn = fread_number(fp);
			else if (letter == 'H')
				pRoomIndex->heal_rate = fread_number(fp);
			else if (letter == 'M')
				pRoomIndex->mana_rate = fread_number(fp);

			else if (letter == 'C'){
			}
			else if (letter == 'D'){
				EXIT_DATA *pexit;
				int locks;

				door = fread_number(fp);
				if (door < 0 || door > 5){
					bug("Fread_rooms: vnum %d has bad door number.",vnum);
					exit(1);
				}

				pexit					=(EXIT_DATA *) alloc_perm(sizeof(*pexit));
				pexit->description		= fread_string(fp);
				pexit->keyword			= fread_string(fp);
				pexit->exit_info		= 0;
				pexit->rs_flags         = 0;
				locks					= fread_number(fp);
				pexit->key				= fread_number(fp);
				pexit->u1.vnum			= fread_number(fp);
				pexit->orig_door		= door;
				pexit->rs_flags			= fread_flag(fp);
				pexit->exit_info		= pexit->rs_flags;
				pRoomIndex->exit[door]	= pexit;
				top_exit++;
			}
			else if (letter == 'E'){
				EXTRA_DESCR_DATA *ed;

				ed						= (EXTRA_DESCR_DATA *) alloc_perm(sizeof(*ed));
				ed->keyword				= fread_string(fp);
				ed->description			= fread_string(fp);
				ed->next				= pRoomIndex->extra_descr;
				pRoomIndex->extra_descr	= ed;
				top_ed++;
			}
			else if (letter == 'O'){
				if (pRoomIndex->owner[0] != '\0'){
					bug("Load_rooms: duplicate owner.",0);
					exit(1);
				}

				pRoomIndex->owner = fread_string(fp);
			}
			else if (letter == 'R'){
				PROG_LIST *pRprog;
				char *word;
				int trigger = 0;

				pRprog					= (PROG_LIST *)alloc_perm(sizeof(*pRprog));
				word					= fread_word(fp);
				if (!(trigger = flag_lookup(word,rprog_flags))){
					bug("ROOMprogs: invalid trigger.",0);
					exit(1);
				}
				SET_BIT(pRoomIndex->rprog_flags,trigger);
				pRprog->trig_type		= trigger;
				pRprog->vnum			= fread_number(fp);
				pRprog->trig_phrase		= fread_string(fp);
				pRprog->next			= pRoomIndex->rprogs;
				pRoomIndex->rprogs		= pRprog;
			}

			else{
				bug("Load_rooms: vnum %d has flag not 'DES'.",vnum);
				exit(1);
			}
		}

		iHash							= vnum % MAX_KEY_HASH;
		pRoomIndex->next				= room_index_hash[iHash];
		room_index_hash[iHash]			= pRoomIndex;
		top_room++;
		top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;
		assign_area_vnum(vnum);
	}

	return;
}

void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		pShop			= (SHOP_DATA *)alloc_perm( sizeof(*pShop) );
		pShop->keeper		= fread_number( fp );
		if ( pShop->keeper == 0 )
			break;
		for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			pShop->buy_type[iTrade]	= fread_number( fp );
		pShop->profit_buy	= fread_number( fp );
		pShop->profit_sell	= fread_number( fp );
		pShop->open_hour	= fread_number( fp );
		pShop->close_hour	= fread_number( fp );
					  fread_to_eol( fp );
		pMobIndex		= get_mob_index( pShop->keeper );
		pMobIndex->pShop	= pShop;

		if ( shop_first == NULL )
			shop_first = pShop;
		if ( shop_last  != NULL )
			shop_last->next = pShop;

		shop_last	= pShop;
		pShop->next	= NULL;
		top_shop++;
    }

    return;
}

void load_specials(FILE *fp)
{
    for ( ; ; )
    {
		MOB_INDEX_DATA *pMobIndex;
		char letter;

		switch ( letter = fread_letter( fp ) )
		{
		default:
			bug( "Load_specials: letter '%c' not *MS.", letter );
			exit( 1 );

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex		= get_mob_index	( fread_number ( fp ) );
			pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
			if ( pMobIndex->spec_fun == 0 ){
				bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
				//exit( 1 );
			}
			break;
		}

		fread_to_eol( fp );
    }
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
    extern const sh_int rev_dir [];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    RESET_DATA *pReset;
    ROOM_INDEX_DATA *iLastRoom, *iLastObj;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		for ( pRoomIndex  = room_index_hash[iHash]; pRoomIndex != NULL; pRoomIndex  = pRoomIndex->next )
		{
			bool fexit;

			iLastRoom = iLastObj = NULL;

			/* OLC : nuevo chequeo de resets */
			for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
			{
	    		switch( pReset->command )
	    		{
	    			default:
	    				bugf( "fix_exits : room %d with reset cmd %c", pRoomIndex->vnum, pReset->command );
	    				exit(1);
	    				break;
					case 'M':
						get_mob_index( pReset->arg1 );
						iLastRoom = get_room_index( pReset->arg3 );
						break;
					case 'O':
						get_obj_index( pReset->arg1 );
						iLastObj = get_room_index( pReset->arg3 );
						break;
					case 'P':
						get_obj_index( pReset->arg1 );
						if (iLastObj == NULL)
						{
							bugf( "fix_exits : reset in room %d with iLastObj NULL", pRoomIndex->vnum );
							exit(1);
						}
						break;
					case 'G':
					case 'E':
						get_obj_index( pReset->arg1 );
						if (iLastRoom == NULL)
						{
							bugf( "fix_exits : reset in room %d with iLastRoom NULL", pRoomIndex->vnum );
							exit(1);
						}
						iLastObj = iLastRoom;
						break;
					case 'D':
						bugf( "???" );
						break;
					case 'R':
						get_room_index( pReset->arg1 );
						if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR )
						{
							bugf( "fix_exits : reset in room %d with arg2 %d >= MAX_DIR",
								pRoomIndex->vnum, pReset->arg2 );
							exit(1);
						}
						break;
				}
			}

			fexit = false;
			for ( door = 0; door <= 5; door++ )
			{
				if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
				{
					if ( pexit->u1.vnum <= 0 || get_room_index(pexit->u1.vnum) == NULL)
						pexit->u1.to_room = NULL;
					else
					{
			   			fexit = true;
						pexit->u1.to_room = get_room_index( pexit->u1.vnum );
					}
				}
			}
			//if (!fexit)      Why the hell is this even here?
			//	SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
		}
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		for ( pRoomIndex  = room_index_hash[iHash];
			  pRoomIndex != NULL;
			  pRoomIndex  = pRoomIndex->next )
		{
			for ( door = 0; door <= 5; door++ )
			{
				if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
				&&   ( to_room   = pexit->u1.to_room            ) != NULL
				&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
				&&   pexit_rev->u1.to_room != pRoomIndex 
				&&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
				{
					/*sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
					pRoomIndex->vnum, door,
					to_room->vnum,    rev_dir[door],
					(pexit_rev->u1.to_room == NULL)
						? 0 : pexit_rev->u1.to_room->vnum );
					bug( buf, 0 );*/
				}
			}
		}
    }

    return;
}

void load_mobprogs(FILE *fp){
	PROG_CODE *pMprog;

	if (area_last == NULL){
		bug("Load_mobprogs: no #AREA seen yet.",0);
		exit(1);
	}

	for (;;){
		sh_int vnum;
		char letter;

		letter = fread_letter(fp);
		if (letter != '#'){
			bug("Load_mobprogs: # not found.",0);
			exit(1);
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = false;
		if (get_prog_index(vnum,PRG_MPROG) != NULL){
			bug("Load_mobprogs: vnum %d duplicated.",vnum);
			exit(1);
		}
		fBootDb = true;

		pMprog = (PROG_CODE *)alloc_perm(sizeof(*pMprog));
		pMprog->vnum = vnum;
		pMprog->code = fread_string(fp);
		if (mprog_list == NULL)
			mprog_list = pMprog;
		else{
			pMprog->next = mprog_list;
			mprog_list = pMprog;
		}
		top_mprog_index++;
	}
}

void fix_mobprogs( void )
{
    MOB_INDEX_DATA *pMobIndex;
    PROG_LIST        *list;
    PROG_CODE        *prog;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
		for ( pMobIndex   = mob_index_hash[iHash]; pMobIndex   != NULL; pMobIndex   = pMobIndex->next )
		{
			for( list = pMobIndex->mprogs; list != NULL; list = list->next )
			{
				if ( ( prog = get_prog_index( list->vnum, PRG_MPROG ) ) != NULL )
					list->code = prog->code;
				else
				{
					bug( "Fix_mobprogs: code vnum %d not found.", list->vnum );
					exit( 1 );
				}
			}
		}
    }
}

// Repopulate areas periodically.
void area_update( void )
{
    AREA_DATA *pArea;
    char buf[MSL];
    int hash;
    ROOM_INDEX_DATA *room;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
		room_update ( pArea );
		if ( ++pArea->age < 3 )
			continue;

		/*
		 * Check age and reset.
		 * Note: Mud School resets every 3 minutes (not 15).
		 */
		if ( /*(!pArea->empty && wuzzitdo????*/ (pArea->nplayer == 0 && pArea->age >= 10) || pArea->age >= 20)//nash make this work for levelrange instead
		{
			ROOM_INDEX_DATA *pRoomIndex;

			reset_area( pArea );
			sprintf(buf,"%s has just been reset.",pArea->name);
			wiznet(buf,NULL,NULL,WZ_RESETS,0,0);
		
			pArea->age = number_range( 0, 3 );
			pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
			if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
				pArea->age = 15 - 2;
			else if (pArea->nplayer == 0) 
				pArea->empty = true;
		}
    }
    /*
     * ROOMprog Triggers!
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for ( room = room_index_hash[hash]; room; room = room->next )
		{
			if ( room->area->empty )
	    		continue;
	    		
			if ( HAS_TRIGGER_ROOM( room, TRIG_DELAY ) && room->rprog_delay > 0 )
			{
				if ( --room->rprog_delay <= 0 )
					p_percent_trigger( NULL, NULL, room, NULL, NULL, NULL, TRIG_DELAY );
			}
			else if ( HAS_TRIGGER_ROOM( room, TRIG_RANDOM ) )
				p_percent_trigger( NULL, NULL, room, NULL, NULL, NULL, TRIG_RANDOM );
		}

    return;
}

/* Reset one room.  Called by reset_area and olc.*/
void reset_room( ROOM_INDEX_DATA *pRoom )
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob, *mob, *LastMob = NULL;
    OBJ_DATA    *pObj, *LastObj = NULL;
    int iExit, level = 0;
    bool last;

    if ( !pRoom )
        return;

    pMob        = NULL;
    last        = false;
    
    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] ))  
        {
            pExit->exit_info = pExit->rs_flags;
            if ( ( pExit->u1.to_room != NULL )
              && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex,  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
		int count,limit=0;

        switch ( pReset->command )
        {
        default:
            bug( "Reset_room: bad command %c.", pReset->command );
            break;
        case 'M':
            if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }

			if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
			{
				bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
				continue;
			}
            if ( pMobIndex->count >= pReset->arg2 )
            {
                last = false;
                break;
            }
			count = 0;
			for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
				if (mob->pIndexData == pMobIndex)
				{
					count++;
					if (count >= pReset->arg4)
					{
		    			last = false;
		    			break;
					}
				}

			if (count >= pReset->arg4)
				break;
            pMob = create_mobile(pMobIndex);

            /*if (room_is_dark(pRoom))
                pMob->setaff(AF_INFRARED);*/

            ROOM_INDEX_DATA *pRoomIndexPrev;

            pRoomIndexPrev = get_room_index(pRoom->vnum - 1);
			if ( pRoomIndexPrev && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
				pMob->setact(AT_PET);

            char_to_room( pMob, pRoom );

            LastMob = pMob;
            level  = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 );
            last = true;
            break;
        case 'O':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'O' 1 : bad vnum %d", pReset->arg1 );
                bugf("%d %d %d %d",pReset->arg1,pReset->arg2,pReset->arg3,pReset->arg4 );
                continue;
            }
            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'O' 2 : bad vnum %d.", pReset->arg3 );
                bugf("%d %d %d %d",pReset->arg1,pReset->arg2,pReset->arg3,pReset->arg4 );
                continue;
            }
            if ( /*pRoom->area->nplayer > 0 ||*/ count_obj_list( pObjIndex, pRoom->contents ) > 0 )
			{
				last = false;
				break;
			}

            pObj = create_object( pObjIndex, UMIN(number_fuzzy( level ), LEVEL_HERO -1) );
            pObj->cost = 0;
            pObj->droprate = 1000;//NASHNASHNASH
           // pObj->timer = 0;//NASHNASHNASH
            obj_to_room( pObj, pRoom );
		    last = true;
            break;
        case 'P':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if (pReset->arg2 > 50) /* old format */
                limit = 6;
            else if (pReset->arg2 == -1) /* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

            if ( /*pRoom->area->nplayer > 0*/( LastObj = get_obj_type( pObjToIndex ) ) == NULL
			|| ( LastObj->in_room == NULL && !last)
			|| ( pObjIndex->count >= limit /* && number_range(0,4) != 0 */ )
			|| ( count = count_obj_list( pObjIndex, LastObj->contains ) ) > pReset->arg4  )
			{
				last = false;
				break;
			}
				                /* lastObj->level  -  ROM */
	    while (count < pReset->arg4)
	    {
            pObj = create_object( pObjIndex, number_fuzzy( LastObj->level ) );
            obj_to_obj( pObj, LastObj );
			count++;
			if (pObjIndex->count >= limit)
				break;
	    }

	    /* fix object lock state! */
	    LastObj->value[1] = LastObj->pIndexData->value[1];
	    last = true;
            break;
        case 'G':
        case 'E':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }
            if ( !last )
                break;
            if ( !LastMob )
            {
                bug( "Reset_room: 'E' or 'G': null mob for vnum %d.", pReset->arg1 );
                last = false;
                break;
            }
            if ( LastMob->pIndexData->pShop )   /* Shop-keeper? */
            {
                int olevel=0,i,j;
				pObj = create_object( pObjIndex, olevel );
				SET_BIT(pObj->extra_flags,ITM_INVENTORY);
            }
			else
			{
				int limit;
				if (pReset->arg2 > 50 )  /* old format */
					limit = 6;
				else if ( pReset->arg2 == -1 || pReset->arg2 == 0 )  /* no limit */
					limit = 999;
				else
					limit = pReset->arg2;
				if ( pObjIndex->count < limit || number_range(0,4) == 0 )
				{
					pObj = create_object( pObjIndex,UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
					/* error message if it is too high */
					if (pObj->level > LastMob->level + 10)
						fprintf(stderr,"Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
							pObj->short_descr,pObj->pIndexData->vnum,pObj->level,LastMob->short_descr,LastMob->pIndexData->vnum,LastMob->level);
				}
				else
					break;
			}
            obj_to_char( pObj, LastMob );
            if ( pReset->command == 'E' )
                equip_char( LastMob, pObj, pReset->arg3 );
            last = true;
            break;
        case 'D':
            break;
        case 'R':
            if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
                continue;
            }
            {
                EXIT_DATA *pExit;
                int d0,d1;

                for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                {
                    d1                   = number_range( d0, pReset->arg2-1 );
                    pExit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;
        }
    }

    return;
}

/*  Reset one area.*/
void reset_area(AREA_DATA *pArea)
{
    ROOM_INDEX_DATA *pRoom;
    int  vnum;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
        if ( ( pRoom = get_room_index(vnum) ) )
            reset_room(pRoom);
    }

    return;
}

/* Create an instance of a mobile.*/
CHAR_DATA *create_mobile(MOB_INDEX_DATA *pMobIndex){
	CHAR_DATA *mob;
	int i;
	long wealth;
	AFFECT_DATA af;

	mobile_count++;

	if (pMobIndex == NULL){
		bug("Create_mobile: NULL pMobIndex.",0);
		exit(1);
	}

	mob = new_char();

	mob->pIndexData	= pMobIndex;
	mob->pclass = 3;

	mob->name			= str_dup(pMobIndex->player_name);
	mob->short_descr	= str_dup(pMobIndex->short_descr);
	mob->long_descr		= str_dup(pMobIndex->long_descr);
	mob->description	= str_dup(pMobIndex->description);
	mob->mid			= get_mob_id();
	mob->spec_fun		= pMobIndex->spec_fun;
	mob->prompt			= NULL;
	mob->mprog_target   = NULL;
	mob->trainer		= pMobIndex->trainer;
	mob->icpmsg			= str_dup(pMobIndex->icpmsg);
	mob->ocpmsg			= str_dup(pMobIndex->ocpmsg);
	mob->cmsg			= str_dup(pMobIndex->cmsg);
	mob->guild			= pMobIndex->guild;

	if (pMobIndex->wealth == 0){
		mob->silver = 0;
		mob->gold   = 0;
	}
	else{
		wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	}

	/* read from prototype */
 	mob->group			= pMobIndex->group;
	mob->pclass			= pMobIndex->pclass;
	clone_bits(pMobIndex->act,mob->act,MAX_ACT);
	clone_bits(pMobIndex->affected_by,mob->affected_by,MAX_AFF);
	mob->setcomm(CM_NOCHANNELS);
	mob->setcomm(CM_NOSHOUT);
	mob->setcomm(CM_NOTELL);
	mob->alignment		= pMobIndex->alignment;
	mob->level			= pMobIndex->level;
	mob->hitroll		= pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	mob->max_hit		= dice(pMobIndex->hit[DICE_NUMBER], pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
	mob->hit			= mob->max_hit;
	mob->setmaxmana(dice(pMobIndex->mana[DICE_NUMBER], pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS]);
	mob->setmana(mob->getmaxmana());
	mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
	mob->damage[DICE_TYPE]	= pMobIndex->damage[DICE_TYPE];
	mob->dam_type		= pMobIndex->dam_type;
	if (mob->dam_type == 0)
    	switch(number_range(1,3)){
			case (1): mob->dam_type = 3;        break;  /* slash */
			case (2): mob->dam_type = 7;        break;  /* pound */
			case (3): mob->dam_type = 11;       break;  /* pierce */
		}
	for (i = 0; i < 4; i++)
		mob->armor[i]	= pMobIndex->ac[i];
	clone_bits(pMobIndex->off_bits,mob->off_bits,MAX_OFF);
	clone_bits(pMobIndex->def_bits,mob->def_bits,MAX_DEF);
	clone_ints(pMobIndex->res,mob->res,MAX_RES);
	clone_bits(pMobIndex->form,mob->form,MAX_FRM);
	clone_bits(pMobIndex->parts,mob->parts,MAX_PRT);
	mob->start_pos		= pMobIndex->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->sex			= pMobIndex->sex;
	if (mob->sex == 3) /* random sex */	mob->sex = number_range(1,2);
	mob->race			= pMobIndex->race;
	mob->size			= pMobIndex->size;
	mob->material		= str_dup(pMobIndex->material);
	mob->lefty			= pMobIndex->lefty;

	//Mob stats computed here including modifiers like size and fast off flag
	for (i = 0;i<MAX_STATS;i++)
		mob->perm_stat[i] = pMobIndex->perm_stat[i];
	for (i=0;i<MAX_STATS;i++)
		if (!stat_flags[i].settable)
			mob->perm_stat[i] = number_range(mob->level/2,mob->level*1.5);
	mob->perm_stat[STAT_STR] += mob->size/2;
	mob->perm_stat[STAT_END] += mob->size;

	/* let's get some spell action */
	if (mob->isaff(AF_SANCTUARY))
		affect_set(mob,TO_AFFECTS,skill_lookup("sanctuary"),mob->level,1,-1,APPLY_NONE,0,AF_SANCTUARY);

	if (mob->isaff(AF_HASTE))
		affect_set(mob,TO_AFFECTS,skill_lookup("haste"),mob->level,1,-1,APPLY_AGI,1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 60),AF_HASTE);

	mob->max_move = number_range(mob->level * 50,mob->level * 100);
	mob->move = mob->max_move;
	mob->position = mob->start_pos;
	mob->setact(AT_IS_NPC);

	/* link the mob to the world list */
	mob->next		= char_list;
	char_list		= mob;
	pMobIndex->count++;
	return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
		return;
    
    /* start fixing values */ 
    clone->name 				= str_dup(parent->name);
    clone->version				= parent->version;
    clone->short_descr			= str_dup(parent->short_descr);
    clone->long_descr			= str_dup(parent->long_descr);
    clone->description			= str_dup(parent->description);
    clone->group				= parent->group;
    clone->sex					= parent->sex;
    clone->pclass				= parent->pclass;
    clone->race					= parent->race;
    clone->level				= parent->level;
    clone->trust				= 0;
    clone->timer				= parent->timer;
    clone->wait					= parent->wait;
    clone->hit					= parent->hit;
    clone->max_hit				= parent->max_hit;
    clone->setmana(parent->getmana());
    clone->setmaxmana(parent->getmaxmana());
    clone->move					= parent->move;
    clone->max_move				= parent->max_move;
    clone->gold					= parent->gold;
    clone->silver				= parent->silver;
    clone->exp					= parent->exp;
	clone_bits(parent->act,clone->act,MAX_ACT);
    clone_bits(parent->affected_by,clone->affected_by,MAX_AFF);
    clone_bits(parent->form,clone->form,MAX_FRM);
    clone_bits(parent->parts,clone->parts,MAX_PRT);
    clone_bits(parent->off_bits,clone->off_bits,MAX_OFF);
    clone_bits(parent->def_bits,clone->def_bits,MAX_DEF);
	clone_bits(parent->comm,clone->comm,MAX_CMM);
	clone_ints(parent->res,clone->res,MAX_RES);
	clone->invis_level			= parent->invis_level;
	clone->position				= parent->pIndexData->default_pos;
	if(parent->position != POS_FIGHTING)
	    clone->position				= parent->position;
    clone->saving_throw			= parent->saving_throw;
    clone->saving_spell_throw	= parent->saving_spell_throw;
    clone->alignment			= parent->alignment;
    clone->hitroll				= parent->hitroll;
    clone->damroll				= parent->damroll;
    clone->wimpy				= parent->wimpy;
    clone->size					= parent->size;
    clone->material				= str_dup(parent->material);
    clone->dam_type				= parent->dam_type;
    clone->start_pos			= parent->start_pos;
    clone->default_pos			= parent->default_pos;
    clone->spec_fun				= parent->spec_fun;

    for (i = 0; i < 4; i++)
    	clone->armor[i]	= parent->armor[i];

    for (i = 0; i < MAX_STATS; i++){
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
		clone->damage[i]	= parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);
}

/* Create an instance of an object.*/
OBJ_DATA *create_object(OBJ_INDEX_DATA *pObjIndex,int level){
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    int i;

    if (pObjIndex == NULL){
		bug("Create_object: NULL pObjIndex.",0);
		exit(1);
    }

    obj = new_obj();

    obj->pIndexData		= pObjIndex;
    obj->in_room		= NULL;
    obj->enchanted		= false;

    if (pObjIndex->new_format)
		obj->level		= pObjIndex->level;
    else
		obj->level		= UMAX(0,level);
    obj->wear_loc		= -1;

    obj->name			= str_dup(pObjIndex->name);
    obj->short_descr	= str_dup(pObjIndex->short_descr);
    obj->description	= str_dup(pObjIndex->description);
    obj->material		= str_dup(pObjIndex->material);
    obj->lore			= str_dup(pObjIndex->lore);
    obj->item_type		= pObjIndex->item_type;
    obj->wear_flags		= pObjIndex->wear_flags;
    obj->value[0]		= pObjIndex->value[0];
    obj->value[1]		= pObjIndex->value[1];
    obj->value[2]		= pObjIndex->value[2];
    obj->value[3]		= pObjIndex->value[3];
    obj->value[4]		= pObjIndex->value[4];
    obj->weight			= pObjIndex->weight;
	obj->exclude_flags	= pObjIndex->exclude_flags;
	obj->extra_flags	= pObjIndex->extra_flags;
	obj->armortype_flags= pObjIndex->armortype_flags;
	obj->id				= 0;
	clone_bits(pObjIndex->wflags,obj->wflags,MAX_WPN);

	obj->cost	= pObjIndex->cost;
	obj->droprate	= pObjIndex->droprate;
	obj->timer	= pObjIndex->timer;

    /* Mess with object properties.*/
    switch (obj->item_type)
    {
		default:
			bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
			break;
		case ITEM_LIGHT:
		case ITEM_DARK:
			if (obj->value[2] == 999)
				obj->value[2] = -1;
			break;
		case ITEM_FURNITURE:
		case ITEM_TRASH:
		case ITEM_LUMBER:
		case ITEM_NEEDLE:
		case ITEM_BONES:
		case ITEM_RIBS:
		case ITEM_CONTAINER:
		case ITEM_CALTROPS:
		case ITEM_DRINK_CON:
		case ITEM_KEY:
		case ITEM_COUPON:
		case ITEM_FOOD:
		case ITEM_BOAT:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_FOUNTAIN:
		case ITEM_LOCKPICK:
		case ITEM_THROWINGDART:
		case ITEM_THROWINGAXE:
		case ITEM_THROWINGKNIFE:
		case ITEM_INGOT:
		case ITEM_FORGEFUEL:
		case ITEM_FORGE:
		case ITEM_FIRE:
		case ITEM_SCARS:
		case ITEM_PARCHMENT:
		case ITEM_TRAPPARTS:
		case ITEM_TRAP:
		case ITEM_PORTAL:
		case ITEM_ROOM_KEY:
		case ITEM_GEM:
		case ITEM_JEWELRY:
		case ITEM_SCROLL:
		case ITEM_WAND:
		case ITEM_STAFF:
		case ITEM_QUIVER:
		case ITEM_ARROW:
		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_SHIELD:
		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_MONEY:
		case ITEM_LOCKER:
		case ITEM_HERB:
			break;
		case ITEM_WARP_STONE:
			obj->value[0] = 0;
			break;
		case ITEM_HONINGSTONE:
			obj->value[0] = obj->level;
			break;
    }

    for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) 
		if (paf->location == APPLY_SPELL_AFFECT)
			affect_to_obj(obj,paf);

    obj->next		= object_list;
    object_list		= obj;
    pObjIndex->count++;

    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent,OBJ_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed,*ed_new;

    if (parent == NULL || clone == NULL)
		return;

    /* start fixing the object */
    clone->name 			= str_dup(parent->name);
    clone->short_descr 		= str_dup(parent->short_descr);
    clone->description		= str_dup(parent->description);
    clone->lore				= str_dup(parent->lore);
    clone->item_type		= parent->item_type;
	clone->extra_flags		= parent->extra_flags;
	clone->exclude_flags	= parent->exclude_flags;
    clone->armortype_flags	= parent->armortype_flags;
    clone->wear_flags		= parent->wear_flags;
    clone->weight			= parent->weight;
    clone->cost				= parent->cost;
    clone->level			= parent->level;
    clone->condition		= parent->condition;
    clone->droprate			= parent->droprate;
    clone->timer			= parent->timer;
    clone->material			= str_dup(parent->material);
    clone->timer			= parent->timer;
	clone_bits(parent->wflags,clone->wflags,MAX_WPN);

    for (i = 0;  i < 5; i ++)
		clone->value[i]	= parent->value[i];

    /* affects */
    clone->enchanted	= parent->enchanted;
  
    for (paf = parent->affected; paf != NULL; paf = paf->next) 
		affect_to_obj(clone,paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next)
    {
        ed_new					= new_extra_descr();
        ed_new->keyword    		= str_dup( ed->keyword);
        ed_new->description		= str_dup( ed->description );
        ed_new->next			= clone->extra_descr;
        clone->extra_descr		= ed_new;
    }
}

/* Clear a new character.*/
void clear_char(CHAR_DATA *ch)
{
    static CHAR_DATA ch_zero;
    int i;

    *ch					= ch_zero;
    ch->name			= &str_empty[0];
    ch->short_descr		= &str_empty[0];
    ch->long_descr		= &str_empty[0];
    ch->description		= &str_empty[0];
    ch->prompt			= &str_empty[0];
    ch->logon			= current_time;
    ch->lines			= PAGELEN;
    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;
    ch->position		= POS_STANDING;
    ch->hit				= 20;
    ch->max_hit			= 20;
    ch->setmana(100);
    ch->setmaxmana(100);
    ch->move			= 100;
    ch->max_move		= 100;
    ch->on				= NULL;
    for (i = 0; i < MAX_STATS; i ++){
		ch->perm_stat[i]	= 13;
		ch->mod_stat[i]		= 0;
    }
    return;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
		if (is_name((char *) name,ed->keyword))
			return ed->description;
    return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum)
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex != NULL; pMobIndex  = pMobIndex->next )
		if (pMobIndex->vnum == vnum)
			return pMobIndex;

    if (fBootDb)
    {
		bug("Get_mob_index: bad vnum %d.",vnum);
		exit(1);
    }

    return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex != NULL; pObjIndex  = pObjIndex->next )
		if (pObjIndex->vnum == vnum)
			return pObjIndex;

    if (fBootDb)
    {
		bug("Get_obj_index: bad vnum %d.",vnum);
		exit(1);
    }

    return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
    ROOM_INDEX_DATA *pRoomIndex;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex != NULL; pRoomIndex  = pRoomIndex->next )
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;

    if (fBootDb)
    {
		bug("Get_room_index: bad vnum %d.",vnum);
		exit(1);
    }

    return NULL;
}

PROG_CODE *get_prog_index(int vnum,int type)
{
    PROG_CODE *prg;

    switch (type)
    {
		case PRG_MPROG:
			prg = mprog_list;
			break;
		case PRG_OPROG:
			prg = oprog_list;
			break;
		case PRG_RPROG:
			prg = rprog_list;
			break;
		default:
			return NULL;
    }

    for( ; prg; prg = prg->next )
		if (prg->vnum == vnum)
            return(prg);
    return NULL;
}

/* Read a letter from a file.*/
char fread_letter(FILE *fp){
	char c;

	do{
		c = getc(fp);
	}while (isspace(c));

	return c;
}

/*
 * Read a number from a file.
 */
int fread_number_line(FILE *fp, int line, const char *file){
	int number;
	bool sign;
	char c;
	char err[MSL];
	do{
		c = getc(fp);
	}while (isspace(c));

	number = 0;

	sign   = false;
	if (c == '+')
		c = getc(fp);
	else if (c == '-'){
		sign = true;
		c = getc(fp);
	}

	if (!isdigit(c)){
		sprintf(err,"Fread_number:bad format (%c) from (%d)(%s)",c, line, file);
		bug(err,0);
		//exit(1);
	}

	while (isdigit(c)){
		number = number * 10 + c - '0';
		c      = getc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else if (c != ' ')
		ungetc(c,fp);

	return number;
}

int fread_flag(FILE *fp){
	int number;
	char c;
	bool negative = false;

	do{
		c = getc(fp);
	}while ( isspace(c));

	if (c == '-'){
		negative = true;
		c = getc(fp);
	}

	number = 0;

	if (!isdigit(c)){
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')){
			number += flag_convert(c);
			c = getc(fp);
		}
	}

	while (isdigit(c)){
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (c == '|')
		number += fread_flag(fp);

	else if  (c != ' ')
		ungetc(c,fp);

	if (negative)
		return -1 * number;

	return number;
}

l_int flag_convert(char letter){
	long bitsum = 0;
	char i;

	if ('A' <= letter && letter <= 'Z'){
		bitsum = 1;
		for (i = letter;i > 'A';i--)
			bitsum *= 2;
	}
	else if ('a' <= letter && letter <= 'z'){
		bitsum = 67108864;/* 2^26 */
		for (i = letter;i > 'a';i--)
			bitsum *= 2;
	}

	return bitsum;
}

/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string(FILE *fp){
	char *plast;
	char c;

	plast = top_string + sizeof(char *);
	if (plast > &string_space[MAX_STRING - MSL])
	{
		bug("Fread_string: MAX_STRING %d exceeded.",MAX_STRING);
		exit(1);
	}

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do{
		c = getc(fp);
	}while (isspace(c));

	if ((*plast++ = c) == '~')
		return &str_empty[0];

	for (;;){
		/*
		 * Back off the char type lookup,
		 *   it was too dirty for portability.
		 *   -- Furey
		 */

		switch (*plast = getc(fp)){
			default:
				plast++;
				break;
			case EOF:/* temp fix */
				bug("Fread_string: EOF",0);
				return NULL;/* exit( 1 ); */
				break;
			case '\n':
				plast++;
				*plast++ = '\r';
				break;
			case '\r':
				break;
			case '~':
				plast++;
				{
					union{
						char *	pc;
						char	rgc[sizeof(char *)];
					}u1;
					int ic, iHash;
					char *pHash, *pHashPrev, *pString;

					plast[-1] = '\0';
					iHash     = UMIN(MAX_KEY_HASH - 1,plast - 1 - top_string);
					for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev ){
						for ( ic = 0; ic < sizeof(char *); ic++ )
							u1.rgc[ic] = pHash[ic];
						pHashPrev = u1.pc;
						pHash    += sizeof(char *);
						if (top_string[sizeof(char *)] == pHash[0] && !strcmp(top_string+sizeof(char *)+1,pHash+1))
							return pHash;
					}
					if (fBootDb){
						pString		= top_string;
						top_string		= plast;
						u1.pc		= string_hash[iHash];
						for ( ic = 0; ic < sizeof(char *); ic++ )
							pString[ic] = u1.rgc[ic];
						string_hash[iHash]	= pString;
						nAllocString += 1;
						sAllocString += top_string - pString;
						return pString + sizeof(char *);
					}
					else
						return str_dup(top_string + sizeof(char *));
				}
		}
	}
}

char *fread_string_eol(FILE *fp){
	static bool char_special[256-EOF];
	char *plast;
	char c;

	if (char_special[EOF-EOF] != true){
		char_special[EOF -  EOF] = true;
		char_special['\n' - EOF] = true;
		char_special['\r' - EOF] = true;
	}

	plast = top_string + sizeof(char*);
	if (plast > &string_space[MAX_STRING - MSL]){
		bug("Fread_string: MAX_STRING %d exceeded.",MAX_STRING);
		exit(1);
	}

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do{
		c = getc(fp);
	}while (isspace(c));

	if ((*plast++ = c) == '\n')
		return &str_empty[0];

	for (;;){
		if (!char_special[(*plast++ = getc(fp)) - EOF])
			continue;

		switch (plast[-1]){
			default:
				break;
			case EOF:
				bug("Fread_string_eol  EOF",0);
				exit(1);
				break;
			case '\n':
			case '\r':
				{
					union{
						char *      pc;
						char        rgc[sizeof(char *)];
					}u1;
					int ic, iHash;
					char *pHash, *pHashPrev, *pString;

					plast[-1] = '\0';
					iHash     = UMIN(MAX_KEY_HASH - 1,plast - 1 - top_string);
					for (pHash = string_hash[iHash];pHash;pHash = pHashPrev){
						for (ic = 0;ic < sizeof(char*);ic++)
							u1.rgc[ic] = pHash[ic];
						pHashPrev = u1.pc;
						pHash    += sizeof(char*);

						if (top_string[sizeof(char*)] == pHash[0] && !strcmp(top_string+sizeof(char*)+1,pHash+1))
							return pHash;
					}

					if (fBootDb){
						pString             = top_string;
						top_string          = plast;
						u1.pc               = string_hash[iHash];
						for (ic = 0;ic < sizeof(char*);ic++)
							pString[ic] = u1.rgc[ic];
						string_hash[iHash]  = pString;

						nAllocString += 1;
						sAllocString += top_string - pString;
						return pString + sizeof(char *);
					}
					else
						return str_dup(top_string + sizeof(char*));
				}
		}
	}
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp){
	char c;

	do{
		c = getc(fp);
	}while (c != '\n' && c != '\r');

	do{
		c = getc(fp);
	}while (c == '\n' || c == '\r');

	ungetc(c,fp);
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp){
	static char word[MIL];
	char *pword,cEnd;

	do{
		cEnd = getc(fp);
	}while(isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"')
		pword   = word;
	else{
		word[0] = cEnd;
		pword   = word+1;
		cEnd    = ' ';
	}

	for (;pword < word + MAX_INPUT_LENGTH;pword++){
		*pword = getc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd){
			if (cEnd == ' ')
				ungetc(*pword,fp);
			*pword = '\0';
			return word;
		}
	}

	log_string(word);
	bug("Fread_word: word too long.",0);
	exit(1);
	return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem(int sMem){
	char *pMem;
	int *magic, iList;

	sMem += sizeof(*magic);

	for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
		if (sMem <= rgSizeList[iList])
			break;

	if (iList == MAX_MEM_LIST){
		bug("Alloc_mem: size %d too large.",sMem);
		exit(1);
	}

	if (rgFreeList[iList] == NULL)
		pMem              = (char *)alloc_perm(rgSizeList[iList]);
	else{
		pMem              = (char *)rgFreeList[iList];
		rgFreeList[iList] = * ((void **) rgFreeList[iList]);
	}

	magic = (int *) pMem;
	*magic = MAGIC_NUM;
	pMem += sizeof(*magic);

	return  (void *)pMem;
}

/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem(void *pMem2,int sMem){
	int iList,*magic;
	char *pMem = (char*)pMem2;
	pMem -= sizeof(*magic);
	magic = (int*)pMem;

	if (*magic != MAGIC_NUM){
		bug("Attempt to recyle invalid memory of size %d.",sMem);
		bug((char*)pMem + sizeof(*magic),0);
		abort();
		return;
	}

	*magic = 0;
	sMem += sizeof(*magic);

	for (iList = 0;iList < MAX_MEM_LIST;iList++)
		if (sMem <= rgSizeList[iList])
			break;

	if (iList == MAX_MEM_LIST){
		bug("Free_mem: size %d too large.",sMem);
		exit(1);
	}

	* ((void **) pMem) = rgFreeList[iList];
	rgFreeList[iList]  = pMem;
}

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm(int sMem){
	static char *pMemPerm;
	static int iMemPerm;
	void *pMem;

	while (sMem % sizeof(long) != 0)
		sMem++;
	if (sMem > MAX_PERM_BLOCK){
		bug("Alloc_perm: %d too large.",sMem);
		exit(1);
	}

	if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK){
		iMemPerm = 0;
		if (!(pMemPerm =(char *) calloc(1,MAX_PERM_BLOCK))){
			perror("Alloc_perm");
			exit(1);
		}
	}

	pMem        = pMemPerm + iMemPerm;
	iMemPerm   += sMem;
	nAllocPerm += 1;
	sAllocPerm += sMem;
	return pMem;
}

/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup(const char *str){
	char *str_new;

	if (!str[0])
		return &str_empty[0];

	if (str >= string_space && str < top_string)
		return (char*) str;

	str_new = (char*) alloc_mem(strlen(str) + 1);
	strcpy(str_new,str);
	return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string(char *pstr){
	if (!pstr || pstr == &str_empty[0]  || (pstr >= string_space && pstr < top_string))
		return;

	free_mem(pstr, strlen_color(pstr) + 1);
}

/*void do_areas(CHAR_DATA *ch,char *argument){
    AREA_DATA *pArea1, *pArea2;
    int iArea, iAreaHalf;

    if (argument[0] != '\0')
    {
		send_to_char_bw("No argument is used with this command.\n\r",ch);
		return;
    }

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = area_first;
    pArea2    = area_first;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
		pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
		printf_to_char(ch,"%-39s%-39s\n\r",pArea1->credits,(pArea2 != NULL) ? pArea2->credits : "" );
		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
    }

    return;
}*/

const struct flag_type area_groups[]={
	{ "Blank",														AREA_GROUP_BLANK,false },
	{ "{C-{w= {CI{cm{wm{xortal {GA{gr{we{xas {w={C-",				AREA_GROUP_IMMORTAL,true },
	{ "{R-={r* {RT{rh{de {RE{rs{do{wn {RK{ri{dn{wg{xdom {r*{R=-",	AREA_GROUP_ESON,true },
	{ NULL,															0,false }
};

void do_areas(CHAR_DATA *ch,char *argument){
	AREA_DATA *pArea;
	BUFFER *output;
	char buf[MSL],cmd[MIL];
	bool found;
	int iArea,value,value2,count = 0,snum = 0;

	output = new_buf();
	argument = one_argument(argument,cmd);

	if(!cmd[0]){
		iArea = top_area;

		for(int i = 0;area_groups[i].name;i++){
			pArea = area_first;
			if(snum % 2 != 0) add_buf(output,"\n\r");
			snum = 0;
			found = false;
			for(;pArea;){
				if(pArea->finished == 1 && pArea->area_group == i && (pArea->low_range < LEVEL_IMMORTAL || get_trust(ch) >= LEVEL_IMMORTAL)){
					if(!found){
						sprintf(buf,"\n\r%s{x\n\r",area_groups[i].name);
						add_buf(output,buf);
						found = true;
					}
					sprintf(buf,"{x ({d%3d{x-{d%3d{x) %-26s",pArea->low_range,pArea->high_range,pArea->credits);
					if(++snum % 2 == 0)
						strcat(buf,"\n\r");
					else
						strcat(buf," ");
					add_buf(output,buf);
				}
				pArea = pArea->next;
			}
		}
		if(snum % 2 != 0)
			add_buf(output,"\n\r");
	}
/*	else if (!str_prefix (cmd, "range")){
		char max[MIL];
		argument = one_argument(argument,max);
		if (argument[0] =='\0' || max[0] =='\0'){
			send_to_char ("Syntax: areas range <level1> <level2>\n\r", ch);
			return;
		}

		if (!is_number (argument) || !is_number (max)){
			send_to_char ("That's not a number!\n\r", ch);
			return;
		}

		value  = atoi (argument);
		value2 = atoi (max);

		iArea = top_area;
		pArea    = area_first;

		for(iArea = 0;pArea;iArea++){
			if(pArea == NULL || pArea->credits == '\0' || !(strcmp(pArea->credits,"(null)"))){
			;
			}
			else{
				if(value >= pArea->low_range && value2 <= pArea->high_range){
					if(pArea->low_range == 52 && pArea->high_range == 57){
						sprintf(buf,"{d[{wPROMOTE{d]  {W%-33s {R%s{x\n\r",
							pArea->name,
							(!str_cmp (pArea->credits, "FINISHED!!")) ? "FINISHED!!" : "");
						count++;
						add_buf( output, buf );
					}
					else{
						sprintf( buf, "{d[{w%s%d-%s%d{d]  {W%-33s {R%s{x\n\r",
							(pArea->low_range < 10) ? "00" : (pArea->low_range > 9 && pArea->low_range < 100) ? "0" : "",
							pArea->low_range,
							(pArea->high_range < 10) ? "00" : (pArea->high_range > 9 && pArea->high_range < 100) ? "0" : "",
							pArea->high_range, pArea->name,
							(!str_cmp (pArea->credits, "FINISHED!!")) ? "FINISHED!!" : "");
						count++;
						add_buf( output, buf );
					}
				}
			}
			pArea = pArea->next;
		}
	}
	else if (!str_prefix (cmd, "finished")){
		if (argument[0] =='\0'){
			send_to_char ("Syntax: areas finished <yes|no>\n\rie: 'areas finished yes'\n\r", ch);
			return;
		}

		iArea = top_area;
		pArea    = area_first;

		for ( iArea = 0; pArea != NULL ; iArea++ ){
			if(pArea == NULL || pArea->credits == '\0' || !(strcmp(pArea->credits,"(null)"))){
			;
			}
			else{
				if(!str_cmp(argument, "yes") && !str_cmp (pArea->credits, "FINISHED!!")){
					if(pArea->low_range == 52 && pArea->high_range == 57){
						sprintf( buf, "{d[{wPROMOTE{D]  {W%-33s {R%s{x\n\r",
							pArea->name,
							(!str_cmp (pArea->credits, "FINISHED!!")) ? "FINISHED!!" : "");
						count++;
						add_buf( output, buf );
					}
					else{
						sprintf( buf, "{d[{w%s%d-%s%d{D]  {W%-33s {R%s{x\n\r",
							(pArea->low_range < 10) ? "00" : (pArea->low_range > 9 && pArea->low_range < 100) ? "0" : "",
							pArea->low_range,
							(pArea->high_range < 10) ? "00" : (pArea->high_range > 9 && pArea->high_range < 100) ? "0" : "",
							pArea->high_range, pArea->name,
							(!str_cmp (pArea->credits, "FINISHED!!")) ? "FINISHED!!" : "");
						count++;
						add_buf( output, buf );
					}
				}
				else if(!str_cmp(argument, "no") && str_cmp (pArea->credits, "FINISHED!!") ){
					if(pArea->low_range == 52 && pArea->high_range == 57){
						sprintf( buf, "{d[{wPROMOTE{D]  {W%-33s {R%s{x\n\r",
							pArea->name,
							(!str_cmp (pArea->credits, "FINISHED!!")) ? "FINISHED!!" : "");
						count++;
						add_buf( output, buf );
					}
					else{
						sprintf( buf, "{d[{w%s%d-%s%d{D]  {W%-33s {R%s{x\n\r",
							(pArea->low_range < 10) ? "00" : (pArea->low_range > 9 && pArea->low_range < 100) ? "0" : "",
							pArea->low_range,
							(pArea->high_range < 10) ? "00" : (pArea->high_range > 9 && pArea->high_range < 100) ? "0" : "",
							pArea->high_range, pArea->name,
							(!str_cmp (pArea->credits, "FINISHED!!")) ? "FINISHED!!" : "");
						count++;
						add_buf( output, buf );
					}
				}
			}
			pArea = pArea->next;
		}
	}
	else{
		send_to_char("Syntax: areas [game|range|author|finished] <game|range|author|(yes/no)>\n\r",ch);
		return;
	}*/


	send_to_char("{d[ {wLEVEL {d]  {WAREA NAME                         {rBUILDER(S){x\n\r"
	"{D--------------------------------------------------------------------------------{w\n\r",ch);
	if(count > 0){
		sprintf(buf,"Areas found: %d\n\r",count);
		add_buf( output, buf);
	}
	page_to_char(buf_string(output),ch);
	free_buf(output);
}



void do_memory(CHAR_DATA *ch,char *argument){
    printf_to_char(ch,"Affects %5d\n\r",top_affect);
    printf_to_char(ch,"Areas   %5d\n\r",top_area);
    printf_to_char(ch,"ExDes   %5d\n\r",top_ed);
    printf_to_char(ch,"Exits   %5d\n\r",top_exit);
    printf_to_char(ch,"Helps   %5d\n\r",top_help);
    printf_to_char(ch,"Mobs    %5d(%d new format)\n\r",top_mob_index,newmobs);
    printf_to_char(ch,"(in use)%5d\n\r",mobile_count);
    printf_to_char(ch,"Objs    %5d(%d new format)\n\r",top_obj_index,newobjs);
    printf_to_char(ch,"Resets  %5d\n\r",top_reset);
    printf_to_char(ch,"Rooms   %5d\n\r",top_room);
    printf_to_char(ch,"Shops   %5d\n\r",top_shop);

    printf_to_char(ch,"Strings %5d strings of %7d bytes (max %d).\n\r",nAllocString,sAllocString,MAX_STRING);

    printf_to_char(ch,"Perms   %5d blocks  of %7d bytes.\n\r",nAllocPerm,sAllocPerm);
}

void do_dump(CHAR_DATA *ch,char *argument){
	int count,count2,num_pcs,aff_count;
	CHAR_DATA *fch;
	MOB_INDEX_DATA *pMobIndex;
	PC_DATA *pc;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	DESCRIPTOR_DATA *d;
	AFFECT_DATA *af;
	FILE *fp;
	int vnum,nMatch = 0;

	/* open file */
	fclose(fpReserve);
	fp = fopen("mem.dmp","w");

	/* report use of data structures */

	num_pcs = 0;
	aff_count = 0;

	/* mobile prototypes */
	fprintf(fp,"MobProt	%4d (%8d bytes)\n",
	top_mob_index, top_mob_index * (sizeof(*pMobIndex)));

	/* mobs */
	count = 0;
	count2 = 0;
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		count++;
		if (fch->pcdata != NULL)
			num_pcs++;
		for (af = fch->affected; af != NULL; af = af->next)
			aff_count++;
	}
	for (fch = char_free; fch != NULL; fch = fch->next)
		count2++;

	fprintf(fp,"Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

	/* pcdata */
	count = 0;
	for (pc = pcdata_free; pc != NULL; pc = pc->next)
		count++;

	fprintf(fp,"Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
	num_pcs, num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));

	/* descriptors */
	count = 0; count2 = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		count++;
	for (d= descriptor_free; d != NULL; d = d->next)
		count2++;

	fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

	/* object prototypes */
	for ( vnum = 0; nMatch < top_obj_index; vnum++ )
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
			for (af = pObjIndex->affected; af != NULL; af = af->next)
				aff_count++;
			nMatch++;
		}

	fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
	top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


	/* objects */
	count = 0;
	count2 = 0;
	for (obj = object_list; obj != NULL; obj = obj->next)
	{
		count++;
		for (af = obj->affected; af != NULL; af = af->next)
			aff_count++;
	}
	for (obj = obj_free; obj != NULL; obj = obj->next)
		count2++;

	fprintf(fp,"Objs	%4d (%8d bytes), %2d free (%d bytes)\n",count,count * (sizeof(*obj)),count2,count2 * (sizeof(*obj)));

	/* affects */
	count = 0;
	for (af = affect_free; af != NULL; af = af->next)
		count++;

	fprintf(fp,"Affects	%4d (%8d bytes), %2d free (%d bytes)\n",aff_count,aff_count * (sizeof(*af)),count,count * (sizeof(*af)));

	/* rooms */
	fprintf(fp,"Rooms	%4d (%8d bytes)\n",top_room,top_room * (sizeof(*room)));

	 /* exits */
	fprintf(fp,"Exits	%4d (%8d bytes)\n",top_exit,top_exit * (sizeof(*exit)));

	fclose(fp);

	/* start printing out mobile data */
	fp = fopen("mob.dmp","w");

	fprintf(fp,"\nMobile Analysis\n");
	fprintf(fp,"---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
			nMatch++;
			fprintf(fp,"#%-4d %3d active %3d killed     %s\n",pMobIndex->vnum,pMobIndex->count,pMobIndex->killed,pMobIndex->short_descr);
		}
	fclose(fp);

	/* start printing out object data */
	fp = fopen("obj.dmp","w");

	fprintf(fp,"\nObject Analysis\n");
	fprintf(fp,"---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
			nMatch++;
			fprintf(fp,"#%-4d %3d active %3d reset      %s\n",pObjIndex->vnum,pObjIndex->count,pObjIndex->reset_num,pObjIndex->short_descr);
		}

	/* close file */
	fclose(fp);
	fpReserve = fopen(NULL_FILE,"r");
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number)
{
    switch (number_bits(2))
    {
		case 0:  number -= 1; break;
		case 3:  number += 1; break;
    }

    return UMAX(1,number);
}

/*
 * Generate a random number.
 */
int number_range(int from,int to){
	int power, number;

	if (from == 0 && to == 0)
		return 0;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2;power < to;power <<= 1)
	;

	while ((number = number_mm() & (power -1)) >= to)
	;

	return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent(void){
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
	;

	return 1 + percent;
}

//cheap hack to turn above function into bool form
bool np_lessthan(int num){
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
	;

	if ((percent += 1) < num)
		return true;
	return false;
}
bool np_morethan(int num){
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
	;

	if ((percent += 1) > num)
		return true;
	return false;
}

/*
 * Generate a random door.
 */
int number_door(void){
	int door;

	while ((door = number_mm() & (8-1)) > 5)
	;

	return door;
}

int get_random_door(ROOM_INDEX_DATA *room){
	EXIT_DATA *pexit;
	int doors[6],tries,current=0,answer;
	for(tries=0;tries<6;tries++){
		if ((pexit = room->exit[tries]) && pexit->u1.to_room && !IS_SET(pexit->exit_info,EX_CLOSED)){
			doors[current] = tries;
			current++;
		}
	}
	if(current==0)
		return -1;
	return doors[number_range(0,current-1)];
}

int number_bits(int width){
    return number_mm() & ((1 << width) - 1);
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you, 
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif
 
void init_mm(){
	#if defined (OLD_RAND)
		int *piState;
		int iState;

		piState     = &rgiState[2];

		piState[-2] = 55 - 55;
		piState[-1] = 55 - 24;

		piState[0]  = ((int) current_time) & ((1 << 30) - 1);
		piState[1]  = 1;
		for ( iState = 2; iState < 55; iState++ )
			piState[iState] = (piState[iState-1] + piState[iState-2]) & ((1 << 30) - 1);
	#else
		srandom(time(NULL));
	#endif
}
 
long number_mm(void){
	#if defined (OLD_RAND)
		int *piState, iState1, iState2, iRand;

		piState             = &rgiState[2];
		iState1             = piState[-2];
		iState2             = piState[-1];
		iRand               = (piState[iState1] + piState[iState2])
							& ((1 << 30) - 1);
		piState[iState1]    = iRand;
		if ( ++iState1 == 55 )
			iState1 = 0;
		if ( ++iState2 == 55 )
			iState2 = 0;
		piState[-2]         = iState1;
		piState[-1]         = iState2;
		return iRand >> 6;
	#else
		return random() >> 6;
	#endif
}

/*
 * Roll some dice.
 */
int dice(int number,int size){
	int idice,sum,min;

	switch (size){
		case 0: return 0;
		case 1: return number;
	}

	min = size/2;

	for (idice = 0,sum = 0; idice < number; idice++ )
		sum += number_range(min,size);

	return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate(int level,int value_00,int value_32){
	return value_00 + level * ((value_32*2) - value_00) / 60;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde(char *str){
	for (;*str != '\0';str++)
		if (*str == '~')
			*str = '-';
}

/*
 * Compare strings, case insensitive.
 * Return true if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr,const char *bstr){
	if (!astr){
		bug("Str_cmp: null astr.",0);
		bug(bstr,0);
		return true;
	}

	if (!bstr){
		bug("Str_cmp: null bstr.",0);
		return true;
	}

	for (;*astr || *bstr;astr++,bstr++)
		if (LOWER(*astr) != LOWER(*bstr))
			return true;

	return false;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return true if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr,const char *bstr){
	if (!astr){
		bug("Strn_cmp: null astr.",0);
		return true;
	}

	if (!bstr){
		bugf("Strn_cmp: null bstr. astr = %s", astr);
		return true;
	}

	for (;*astr;astr++,bstr++)
		if (LOWER(*astr) != LOWER(*bstr))
			return true;

	return false;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns true is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr,const char *bstr){
	int sstr1, sstr2, ichar;
	char c0;

	if ((c0 = LOWER(astr[0])) == '\0')
		return false;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0;ichar <= sstr2 - sstr1;ichar++)
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr,bstr + ichar))
			return false;

	return true;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return true if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr,const char *bstr){
	int sstr1, sstr2;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	if (sstr1 <= sstr2 && !str_cmp(astr,bstr + sstr2 - sstr1))
		return false;
	else
		return true;
}

/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str){
	static char strcap[MSL];
	int i;

	for (i = 0;str[i];i++)
		strcap[i] = LOWER(str[i]);
	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}

/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA *ch,char *file,char *str){
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	fclose(fpReserve);
	if ((fp = fopen(file,"a")) == NULL)
	{
		perror(file);
		send_to_char("Could not open the file!\n\r",ch);
	}
	else
	{
		fprintf(fp,"[%5d] %s: %s\n",ch->in_room ? ch->in_room->vnum : 0,ch->name,str);
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE,"r");
}

/*
 * Reports a bug.
 */
void bug(const char *str,int param){
	char buf[MSL];

	if (fpArea){
		int iLine, iChar;

		if (fpArea == stdin)
			iLine = 0;
		else{
			iChar = ftell(fpArea);
			fseek(fpArea,0,0);
			for (iLine = 0;ftell(fpArea) < iChar;iLine++){
				while (getc(fpArea) != '\n')
					;
			}
			fseek(fpArea,iChar,0);
		}

		sprintf(buf,"[*****] FILE: %s LINE: %d",strArea,iLine);
		log_string(buf);
	}

	strcpy(buf,"[*****] BUG: ");
	sprintf(buf + strlen_color(buf),str,param);
	log_string(buf);
}

/*
 * Writes a string to the log.
 */
void log_string(const char *str){
    char *strtime;
    strtime = ctime(&current_time);
    strtime[strlen_color(strtime)-1] = '\0';
    fprintf(stderr,"%s :: %s\n",strtime,str);
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void){
    return;
}

void load_objprogs(FILE *fp){
	PROG_CODE *pOprog;

	if (!area_last){
		bug("Load_objprogs: no #AREA seen yet.",0);
		exit(1);
	}

	for (;;){
		sh_int vnum;
		char letter;

		letter = fread_letter(fp);
		if (letter != '#'){
			bug("Load_objprogs: # not found.",0);
			exit(1);
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = false;
		if (get_prog_index(vnum,PRG_OPROG)){
			bug("Load_objprogs: vnum %d duplicated.",vnum);
			exit(1);
		}
		fBootDb = true;

		pOprog		= (PROG_CODE *)alloc_perm(sizeof(*pOprog));
		pOprog->vnum  	= vnum;
		pOprog->code  	= fread_string(fp);
		if (oprog_list == NULL)
			oprog_list = pOprog;
		else{
			pOprog->next = oprog_list;
			oprog_list 	= pOprog;
		}
		top_oprog_index++;
	}
}

void load_roomprogs(FILE *fp){
	PROG_CODE *pRprog;

	if (!area_last){
		bug("Load_roomprogs: no #AREA seen yet.",0);
		exit(1);
	}

	for (;;){
		sh_int vnum;
		char letter;

		letter		  = fread_letter(fp);
		if (letter != '#'){
			bug("Load_roomprogs: # not found.",0);
			exit(1);
		}

		vnum		 = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = false;
		if (get_prog_index(vnum,PRG_RPROG)){
			bug("Load_roomprogs: vnum %d duplicated.",vnum);
			exit(1);
		}
		fBootDb = true;

		pRprog		= (PROG_CODE*)alloc_perm(sizeof(*pRprog));
		pRprog->vnum  	= vnum;
		pRprog->code  	= fread_string(fp);
		if (rprog_list == NULL)
			rprog_list = pRprog;
		else{
			pRprog->next = rprog_list;
			rprog_list 	= pRprog;
		}
		top_rprog_index++;
	}
}

void fix_objprogs(void){
	OBJ_INDEX_DATA *pObjIndex;
	PROG_LIST        *list;
	PROG_CODE        *prog;
	int iHash;

	for (iHash = 0;iHash < MAX_KEY_HASH;iHash++){
		for (pObjIndex = obj_index_hash[iHash];pObjIndex;pObjIndex = pObjIndex->next){
			for(list = pObjIndex->oprogs;list;list = list->next){
				if ((prog = get_prog_index(list->vnum,PRG_OPROG)))
					list->code = prog->code;
				else{
					bug("Fix_objprogs: code vnum %d not found.",list->vnum);
					exit(1);
				}
			}
		}
	}
}

void fix_roomprogs(void){
	ROOM_INDEX_DATA *pRoomIndex;
	PROG_LIST *list;
	PROG_CODE *prog;
	int iHash;

	for (iHash = 0;iHash < MAX_KEY_HASH;iHash++){
		for (pRoomIndex = room_index_hash[iHash];pRoomIndex;pRoomIndex = pRoomIndex->next){
			for(list = pRoomIndex->rprogs;list;list = list->next){
				if ((prog = get_prog_index(list->vnum,PRG_RPROG)))
					list->code = prog->code;
				else{
					bug("Fix_roomprogs: code vnum %d not found.",list->vnum);
					exit(1);
				}
			}
		}
	}
}
